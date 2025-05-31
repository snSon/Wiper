# haze_apply.py

import os
import cv2
import torch
import numpy as np
from pathlib import Path
from haze_filter import apply_fog

# 사용가능한 확장자
IMG_EXTS = {".png", ".jpg", ".jpeg", ".bmp", ".tiff"}
VID_EXTS = {".mp4", ".avi", ".mov", ".mkv"}

# 이미지 처리
def process_image(in_path: Path, out_path: Path, beta, A, layers, device):
    # 이미지 가져오기
    img_bgr = cv2.imread(str(in_path))
    
    # 이미지 가져오기 실패 시 예외 처리
    if img_bgr is None:
        print(f"[WARN] Unable to read image: {in_path}")
        return
    
    img_rgb = cv2.cvtColor(img_bgr, cv2.COLOR_BGR2RGB)
    
    # Image -> Tensor
    tensor = (
        torch.from_numpy(img_rgb)
             .float().div(255.0)
             .permute(2,0,1)
             .unsqueeze(0)
             .to(device)
    )
    
    # hazing 적용
    with torch.no_grad():
        foggy = apply_fog(tensor, beta=beta, A=A, layers=layers)
    
    # Tensor -> Image
    out_np = (
        foggy.squeeze(0)
             .permute(1,2,0)
             .cpu()
             .numpy()
             .clip(0,1) * 255.0
    ).round().astype(np.uint8)
    out_bgr = cv2.cvtColor(out_np, cv2.COLOR_RGB2BGR)
    
    # 이미지 저장
    out_path.parent.mkdir(parents=True, exist_ok=True)
    cv2.imwrite(str(out_path), out_bgr)
    print(f"[IMAGE] {in_path} → {out_path}")

# 비디오 처리
def process_video(in_path: Path, out_path: Path, beta, A, layers, device):
    # 비디오 가져오기
    cap = cv2.VideoCapture(str(in_path))
    if not cap.isOpened():
        print(f"[WARN] Cannot open video: {in_path}")
        return
    
    fps    = cap.get(cv2.CAP_PROP_FPS) or 30.0
    width  = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
    height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
    
    # 비디오 저장 방식 처리
    fourcc = cv2.VideoWriter_fourcc(*'mp4v')
    out_path.parent.mkdir(parents=True, exist_ok=True)
    writer = cv2.VideoWriter(str(out_path), fourcc, fps, (width, height))

    frame_idx = 0
    while True:
        ret, frame_bgr = cap.read()
        if not ret:
            break
        
        # Image -> Tensor
        frame_rgb = cv2.cvtColor(frame_bgr, cv2.COLOR_BGR2RGB)
        tensor = (
            torch.from_numpy(frame_rgb)
                 .float().div(255.0)
                 .permute(2,0,1)
                 .unsqueeze(0)
                 .to(device)
        )
        
        # hazing 적용
        with torch.no_grad():
            foggy = apply_fog(tensor, beta=beta, A=A, layers=layers)
        
        # Tensor -> Image
        out_np = (
            foggy.squeeze(0)
                 .permute(1,2,0)
                 .cpu()
                 .numpy()
                 .clip(0,1) * 255.0
        ).round().astype(np.uint8)
        out_bgr = cv2.cvtColor(out_np, cv2.COLOR_RGB2BGR)
        
        # 비디오 저장
        writer.write(out_bgr)
        frame_idx += 1

    cap.release()
    writer.release()
    print(f"[VIDEO] {in_path} → {out_path}")

def main():
    # 파라미터 파싱 (입력/출력 경로 고정)
    import argparse
    p = argparse.ArgumentParser(
        description="Apply fog effect recursively to hazing_input → hazing_output")
    p.add_argument("--beta",   type=float, default=1.0, help="안개 밀도 계수")
    p.add_argument("--A",      type=float, default=0.9, help="대기광 강도 (0~1)")
    p.add_argument("--layers", type=int,   default=100, help="안개 레이어 수")
    args = p.parse_args()

    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    print(f"Using device: {device}")

    # 입력/출력 폴더 고정
    input_dir  = Path("hazing_input").resolve()
    output_dir = Path("hazing_output").resolve()

    # hazing_input 폴더가 없으면 예외 발생
    if not input_dir.exists():
        raise FileNotFoundError(f"Input folder not found: {input_dir}")

    # 재귀 탐색하여 이미지/비디오 처리
    for root, _, files in os.walk(input_dir):
        root_path = Path(root)
        for fname in files:
            in_path = root_path / fname
            rel = in_path.relative_to(input_dir)
            out_path = output_dir / rel

            # 이미지 파일인 경우
            if in_path.suffix.lower() in IMG_EXTS:
                process_image(in_path, out_path, args.beta, args.A, args.layers, device)
            # 비디오 파일인 경우 (.mp4로 출력)
            elif in_path.suffix.lower() in VID_EXTS:
                process_video(in_path, out_path.with_suffix(".mp4"),
                              args.beta, args.A, args.layers, device)
            else:
                # 지원되지 않는 확장자는 스킵
                continue

if __name__ == "__main__":
    main()
