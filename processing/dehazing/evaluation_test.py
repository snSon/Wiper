"""
AOD, FFA, JetDehaze, MSBDN 등 폴더별 모델을 일괄 로드하여
RESIDE SOTS(outdoor) 테스트셋에서 PSNR/SSIM을 계산하는 스크립트.

폴더 구조:
models/
  AOD/
    aod_model.py        # 또는 AOD_model.py (대소문자 허용)
    aod.pth             # 또는 AOD.pth (대소문자 허용)
  FFA/
    ffa_model.py
    ffa.pth
  JetDehaze/
    jetdehaze_model.py
    jetdehaze.pth
  MSBDN/
    msbdn_model.py
    msbdn.pth

데이터 구조:
SOTS_root/
  outdoor/
    hazy/  *_XX_XX.png, *_XX_XX.jpg  (흐린 이미지)
    gt/    *.png, *.jpg  (정답 이미지; 파일명 일치 가정)

사용 예시:
python test.py \
  --models_root ./models \
  --data_root   ./SOTS \
  --split       outdoor \
  --models      AOD,FFA,JetDehaze,MSBDN \
  --out_csv     ./results_sots_outdoor.csv \
  --device      cuda \
  --save_dir    ./_preds  # (선택) 복원 결과 저장

"""

import argparse
import csv
import glob
import importlib.util
import math
import os
import sys
import time
from typing import List, Tuple

import numpy as np
from PIL import Image

import torch
import torch.nn as nn
import torch.nn.functional as F
import piq

# 유틸: 동적 임포트 & 모델 빌드

def _try_paths(base_dir: str, base_name: str, suffixes: List[str]) -> List[str]:
    """가능한 대소문자 조합 후보 경로 리스트를 반환."""
    cands = []
    bases = {base_name, base_name.lower(), base_name.upper(), base_name.capitalize()}
    for b in bases:
        for sfx in suffixes:
            cands.append(os.path.join(base_dir, b + sfx))
    # 중복 제거, 존재하는 것만
    out = []
    seen = set()
    for p in cands:
        if p in seen:
            continue
        seen.add(p)
        if os.path.exists(p):
            out.append(p)
    return out

def load_model_from_folder(folder: str, model_name: str, device: str = "cuda") -> nn.Module:
    """폴더에서 {name}_model.py 및 {name}.pth를 찾아 모델을 구성하고 가중치 로드."""
    # 1) python 파일 탐색
    py_candidates = _try_paths(folder, f"{model_name}_model", suffixes=[".py"]) + \
                    _try_paths(folder, f"{model_name}",        suffixes=["_model.py", ".py"])  # 약간 여유
    if not py_candidates:
        raise FileNotFoundError(f"[load_model_from_folder] 모델 파이썬 파일을 찾을 수 없습니다: {folder}")
    py_path = py_candidates[0]

    # 2) 가중치 파일 탐색
    pth_candidates = _try_paths(folder, model_name, suffixes=[".pth", ".pt"]) + \
                     _try_paths(folder, model_name.lower(), suffixes=[".pth", ".pt"]) + \
                     _try_paths(folder, model_name.upper(), suffixes=[".pth", ".pt"]) + \
                     _try_paths(folder, model_name.capitalize(), suffixes=[".pth", ".pt"])
    if not pth_candidates:
        raise FileNotFoundError(f"[load_model_from_folder] 가중치(.pth/.pt)를 찾을 수 없습니다: {folder}")
    pth_path = pth_candidates[0]

    # 3) 모듈 임포트
    spec = importlib.util.spec_from_file_location(f"{model_name}_module", py_path)
    if spec is None or spec.loader is None:
        raise ImportError(f"파이썬 모듈 로딩 실패: {py_path}")
    module = importlib.util.module_from_spec(spec)
    sys.modules[f"{model_name}_module"] = module
    spec.loader.exec_module(module)  # type: ignore

    # 4) 모델 생성 우선순위: build_model() -> 후보 nn.Module 자동 탐색
    net = None
    if hasattr(module, "build_model") and callable(module.build_model):
        net = module.build_model()
    else:
        # nn.Module 서브클래스 후보 수집
        candidates = []
        for attr_name in dir(module):
            obj = getattr(module, attr_name)
            if isinstance(obj, type) and issubclass(obj, nn.Module) and obj is not nn.Module:
                candidates.append(obj)
        if not candidates:
            raise RuntimeError("nn.Module 서브클래스를 찾지 못했습니다. build_model()을 구현하세요.")
        # 이름 휴리스틱 우선
        name_lc = model_name.lower()
        def _score(cls):
            n = cls.__name__.lower()
            score = 0
            if n == name_lc or n.startswith(name_lc):
                score += 5
            for k in ["dehaze", "haze", "net", "generator", "model"]:
                if k in n:
                    score += 1
            return score
        candidates.sort(key=_score, reverse=True)
        # 인자 없는 생성 시도 → 실패 시 다음 후보
        last_err = None
        for cls in candidates:
            try:
                net = cls()
                break
            except Exception as e:  # noqa
                last_err = e
                continue
        if net is None:
            raise RuntimeError(f"모델 인스턴스화 실패. build_model() 구현을 권장합니다. 마지막 오류: {last_err}")

    # 5) 가중치 로드
    device_map = torch.device(device if torch.cuda.is_available() and device.startswith("cuda") else "cpu")
    ckpt = torch.load(pth_path, map_location=device_map)
    state = ckpt.get("state_dict", ckpt)
    missing, unexpected = net.load_state_dict(state, strict=False)
    if missing or unexpected:
        print(f"[경고] state_dict strict=False 로 로드 (missing={len(missing)}, unexpected={len(unexpected)})")
    net.to(device_map).eval()
    return net

# SOTS 데이터 로더 (파일 매칭)

def list_pairs(hazy_dir: str, gt_dir: str) -> List[Tuple[str, str]]:
    exts = ["*.png", "*.jpg", "*.jpeg", "*.bmp", "*.tif"]
    hazy_files = []
    for e in exts:
        hazy_files.extend(glob.glob(os.path.join(hazy_dir, e)))
    pairs = []
    for h in sorted(hazy_files):
        base = os.path.splitext(os.path.basename(h))[0]
        base = base.split('_')[0]

        # 동일 파일명 매칭 우선
        for e in exts:
            g = os.path.join(gt_dir, base + e[1:])  # e includes '*'
            g = os.path.join(gt_dir, base + os.path.splitext(e)[1])
            if os.path.exists(g):
                pairs.append((h, g))
                break
        else:
            # 확장자 다르면 다른 확장자도 탐색
            for e in exts:
                g2 = os.path.join(gt_dir, base + os.path.splitext(e)[1])
                if os.path.exists(g2):
                    pairs.append((h, g2))
                    break
    return pairs


# PSNR / SSIM (piq, GPU 기반)


def psnr_torch(pred: torch.Tensor, tgt: torch.Tensor, max_val: float = 1.0) -> float:
    """piq.psnr 사용. pred/tgt: (1,C,H,W) [0,1]"""
    with torch.no_grad():
        # piq.psnr는 data_range 지정 필요
        val = piq.psnr(pred, tgt, data_range=max_val, reduction='mean')
        return float(val.item())


def ssim_torch(img1: torch.Tensor, img2: torch.Tensor,
               window_size: int = 11, sigma: float = 1.5, C1: float = 0.01 ** 2, C2: float = 0.03 ** 2) -> float:
    """piq.ssim 사용. img1/img2: (1,C,H,W) [0,1]
    window_size, sigma, C1, C2 인자는 호환 유지를 위한 더미이며 piq 내부 기본값 사용.
    """
    with torch.no_grad():
        val = piq.ssim(img1, img2, data_range=1.0, reduction='mean')
        return float(val.item())

# 전처리/후처리

def pil_to_tensor(img: Image.Image) -> torch.Tensor:
    arr = np.array(img).astype(np.float32) / 255.0  # HWC, [0,1]
    if arr.ndim == 2:  # gray → 3채널 복제
        arr = np.stack([arr, arr, arr], axis=-1)
    if arr.shape[2] == 4:  # RGBA → RGB
        arr = arr[:, :, :3]
    arr = np.transpose(arr, (2, 0, 1))  # CHW
    return torch.from_numpy(arr)


def tensor_to_pil(t: torch.Tensor) -> Image.Image:
    t = t.clamp(0, 1).detach().cpu().numpy()
    t = np.transpose(t, (1, 2, 0))  # HWC
    t = (t * 255.0 + 0.5).astype(np.uint8)
    return Image.fromarray(t)


def pad_to_multiple(x: torch.Tensor, multiple: int = 16) -> Tuple[torch.Tensor, Tuple[int, int, int, int]]:
    B, C, H, W = x.shape
    pad_h = (multiple - (H % multiple)) % multiple
    pad_w = (multiple - (W % multiple)) % multiple
    pad = (0, pad_w, 0, pad_h)  # left,right,top,bottom (we use right/bottom only)
    x_pad = F.pad(x, pad, mode='reflect')
    return x_pad, pad


def unpad(x: torch.Tensor, pad: Tuple[int, int, int, int]) -> torch.Tensor:
    _, _, H, W = x.shape
    l, r, t, b = pad
    return x[:, :, 0: H - b, 0: W - r]

# 메인 루프

def evaluate_model(model: nn.Module, pairs: List[Tuple[str, str]], device: torch.device,
                   save_dir: str = None, pad_mult: int = 16) -> Tuple[float, float, List[dict]]:
    os.makedirs(save_dir, exist_ok=True) if save_dir else None
    psnr_list, ssim_list = [], []
    rows = []
    for hazy_path, gt_path in pairs:
        # 로드
        hazy = Image.open(hazy_path).convert('RGB')
        gt   = Image.open(gt_path).convert('RGB')
        # 크기 다르면 GT를 hazy 크기에 맞춤
        if gt.size != hazy.size:
            gt = gt.resize(hazy.size, Image.BICUBIC)

        x = pil_to_tensor(hazy).unsqueeze(0).to(device)
        y_gt = pil_to_tensor(gt).unsqueeze(0).to(device)

        x_pad, pad = pad_to_multiple(x, pad_mult)

        with torch.no_grad():
            tic = time.time()
            y_pred = model(x_pad)
            if isinstance(y_pred, (list, tuple)):
                y_pred = y_pred[0]
            if y_pred.dim() == 3:  # (C,H,W)
                y_pred = y_pred.unsqueeze(0)
            y_pred = unpad(y_pred, pad)

            def _normalize_01(t: torch.Tensor) -> torch.Tensor:
                mn = float(t.min());
                mx = float(t.max())

                if mn >= -1.2 and mx <= 1.2 and mn < 0.0:
                    t = (t + 1.0) / 2.0

                elif mx > 1.2 and mx <= 260.0:
                    t = t / 255.0

                elif mx > 1.2 or mn < -0.2:
                    t = torch.sigmoid(t)
                return t.clamp(0.0, 1.0)

            y_pred = _normalize_01(y_pred)
            toc = time.time()

        psnr = psnr_torch(y_pred, y_gt)
        ssim = ssim_torch(y_pred, y_gt)
        psnr_list.append(psnr)
        ssim_list.append(ssim)

        row = {
            'image': os.path.basename(hazy_path),
            'psnr': f"{psnr:.4f}",
            'ssim': f"{ssim:.4f}",
            'time_ms': f"{(toc - tic)*1000:.2f}",
            'H': y_gt.shape[2],
            'W': y_gt.shape[3]
        }
        rows.append(row)

        if save_dir:
            out_img = tensor_to_pil(y_pred[0])
            out_path = os.path.join(save_dir, os.path.basename(hazy_path))
            out_img.save(out_path)

    avg_psnr = float(np.mean(psnr_list)) if psnr_list else 0.0
    avg_ssim = float(np.mean(ssim_list)) if ssim_list else 0.0
    return avg_psnr, avg_ssim, rows


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--models_root', type=str, required=True, help='모델 폴더 루트')
    parser.add_argument('--models', type=str, default='AOD,FFA,JetDehaze,MSBDN', help='평가할 모델 폴더명(쉼표구분)')
    parser.add_argument('--data_root', type=str, required=True, help='SOTS 루트 경로')
    parser.add_argument('--split', type=str, default='outdoor', help='outdoor / indoor')
    parser.add_argument('--hazy_dirname', type=str, default='hazy')
    parser.add_argument('--gt_dirname', type=str, default='gt')  # SOTS 기준
    parser.add_argument('--device', type=str, default='cuda')
    parser.add_argument('--pad_mult', type=int, default=16, help='모델 다운샘플 배수 맞춤용 패딩')
    parser.add_argument('--save_dir', type=str, default=None, help='(선택) 복원 결과 저장 디렉토리')
    parser.add_argument('--out_csv', type=str, default='./results.csv')
    args = parser.parse_args()

    device = torch.device(args.device if torch.cuda.is_available() and args.device.startswith('cuda') else 'cpu')

    # 데이터 페어 수집
    hazy_dir = os.path.join(args.data_root, args.split, args.hazy_dirname)
    gt_dir   = os.path.join(args.data_root, args.split, args.gt_dirname)
    pairs = list_pairs(hazy_dir, gt_dir)
    if not pairs:
        raise RuntimeError(f"테스트 페어를 찾지 못했습니다. hazy={hazy_dir}, gt={gt_dir}")
    print(f"[데이터] {len(pairs)}개 이미지 페어")

    model_names = [m.strip() for m in args.models.split(',') if m.strip()]

    # CSV 헤더 준비
    fieldnames = ['model', 'image', 'psnr', 'ssim', 'time_ms', 'H', 'W']
    all_rows = []

    for name in model_names:
        folder = os.path.join(args.models_root, name)
        if not os.path.isdir(folder):
            print(f"[경고] 모델 폴더가 없음: {folder}")
            continue

        print(f"\n===== [{name}] 로딩 =====")
        model = load_model_from_folder(folder, name, device=args.device)
        print(f"[{name}] 평가 시작...")

        save_dir_model = os.path.join(args.save_dir, name) if args.save_dir else None
        avg_psnr, avg_ssim, rows = evaluate_model(model, pairs, device=device,
                                                  save_dir=save_dir_model, pad_mult=args.pad_mult)
        for r in rows:
            r2 = {**r}
            r2['model'] = name
            all_rows.append(r2)

        print(f"[{name}] 평균 PSNR={avg_psnr:.4f}, SSIM={avg_ssim:.4f}")

    # CSV 저장
    os.makedirs(os.path.dirname(args.out_csv) or '.', exist_ok=True)
    with open(args.out_csv, 'w', newline='', encoding='utf-8') as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames)
        writer.writeheader()
        for r in all_rows:
            writer.writerow(r)

    # 모델별 요약 출력
    summary = {}
    for r in all_rows:
        key = r['model']
        summary.setdefault(key, {'psnr': [], 'ssim': []})
        summary[key]['psnr'].append(float(r['psnr']))
        summary[key]['ssim'].append(float(r['ssim']))

    print("\n===== 요약 =====")
    for k, v in summary.items():
        mpsnr = np.mean(v['psnr']) if v['psnr'] else 0.0
        mssim = np.mean(v['ssim']) if v['ssim'] else 0.0
        print(f"{k:12s} | PSNR: {mpsnr:.4f} | SSIM: {mssim:.4f}")
    print(f"\nCSV 저장: {args.out_csv}")


if __name__ == '__main__':
    main()
