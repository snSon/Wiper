import os
import cv2
import numpy as np
from skimage.metrics import structural_similarity as compare_ssim
from skimage.metrics import peak_signal_noise_ratio as compare_psnr

# Ground Truth 디렉토리
gt_dir = "gt_images"

# 모델 결과 디렉토리 목록
result_dirs = [
    "results_JetDehazeNet",
    "results_MSBDN",
    "results_FFA",
    "results_DehazeNet"
]

# GT 이미지 파일 목록
gt_files = [
    fname for fname in os.listdir(gt_dir)
    if fname.lower().endswith((".jpg", ".jpeg", ".png", ".bmp"))
]

for result_dir in result_dirs:
    print(f"\n[INFO] Evaluating: {result_dir}")

    ssim_scores = []
    psnr_scores = []

    for fname in gt_files:
        gt_path = os.path.join(gt_dir, fname)
        pred_path = os.path.join(result_dir, fname)

        if not os.path.exists(pred_path):
            print(f"[WARNING] Missing prediction for: {fname}")
            continue

        gt = cv2.imread(gt_path)
        pred = cv2.imread(pred_path)

        if gt is None or pred is None:
            print(f"[WARNING] Failed to load image: {fname}")
            continue

        # BGR → RGB
        gt_rgb = gt[:, :, ::-1]
        pred_rgb = pred[:, :, ::-1]

        # 크기 일치시키기
        pred_rgb = cv2.resize(pred_rgb, (gt_rgb.shape[1], gt_rgb.shape[0]))

        # 너무 작은 이미지 건너뛰기
        h, w = gt_rgb.shape[:2]
        if min(h, w) < 7:
            print(f"[SKIP] {fname}: Too small for SSIM")
            continue

        # win_size 설정
        win_size = min(h, w)
        if win_size % 2 == 0:
            win_size -= 1
        win_size = max(3, win_size)  # 최소 3 이상

        # SSIM / PSNR 계산
        ssim = compare_ssim(pred_rgb, gt_rgb, data_range=255, channel_axis=2, win_size=win_size)
        psnr = compare_psnr(gt_rgb, pred_rgb, data_range=255)

        ssim_scores.append(ssim)
        psnr_scores.append(psnr)

        print(f"[INFO] {fname} | SSIM: {ssim:.4f}, PSNR: {psnr:.2f} dB")

    # 평균 출력
    if ssim_scores:
        print(f"[RESULT] {result_dir} | Avg SSIM: {np.mean(ssim_scores):.4f} | Avg PSNR: {np.mean(psnr_scores):.2f} dB")
    else:
        print(f"[RESULT] {result_dir} | No valid images.")