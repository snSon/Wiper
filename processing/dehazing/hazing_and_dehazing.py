import cv2
import torch
import argparse
import numpy as np
import torchvision.transforms as transforms
from pathlib import Path
from haze_filter import apply_fog
from net import dehaze_net

# --- 인자 처리 ---
parser = argparse.ArgumentParser()
parser.add_argument('--enable_haze', action='store_true')
parser.add_argument('--enable_aod', action='store_true')
parser.add_argument('--enable_roi', action='store_true')
parser.add_argument('--enable_blend', action='store_true')
parser.add_argument('--output_suffix', type=str, default="")
args = parser.parse_args()

# --- 설정 ---
input_video_path = "input/test_drive_30.mp4"
output_video_path = f"output/final_output_{args.output_suffix}.mp4"
model_path = "processing/dehazing/dehazer.pth"
enable_haze = args.enable_haze
enable_aod = args.enable_aod
enable_roi = args.enable_roi
enable_blend = args.enable_blend

# --- 준비 ---
device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
model = dehaze_net().to(device)
model.load_state_dict(torch.load(model_path, map_location=device))
model.eval()
to_tensor = transforms.ToTensor()

cap = cv2.VideoCapture(input_video_path)
fps = cap.get(cv2.CAP_PROP_FPS)
w, h = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH)), int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
fourcc = cv2.VideoWriter_fourcc(*'mp4v')
out = cv2.VideoWriter(output_video_path, fourcc, fps, (w, h))

# Alpha 마스크 사전 생성 (blend용)
roi_w, roi_h = w // 2, h // 2
alpha = np.zeros((roi_h, roi_w, 1), dtype=np.float32)
for i in range(roi_h):
    for j in range(roi_w):
        dy = abs(i - roi_h / 2) / (roi_h / 2)
        dx = abs(j - roi_w / 2) / (roi_w / 2)
        dist = np.sqrt(dx ** 2 + dy ** 2)
        alpha[i, j, 0] = np.clip(1.0 - dist, 0, 1)

# --- 처리 ---
frame_count = 0
while cap.isOpened():
    ret, frame = cap.read()
    if not ret:
        break

    rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

    # 1. HAZE 적용
    if enable_haze:
        rgb_tensor = to_tensor(rgb_frame).unsqueeze(0).to(device)
        with torch.no_grad():
            rgb_tensor = apply_fog(rgb_tensor, beta=2.3, A=0.8)
        rgb_frame = (rgb_tensor.squeeze(0).cpu().clamp(0, 1).numpy().transpose(1, 2, 0) * 255).astype(np.uint8)

    # 2. AODNet 기본 디헤이징
    input_tensor = to_tensor(rgb_frame).unsqueeze(0).to(device)
    with torch.no_grad():
        output_tensor = model(input_tensor)
    output_image = output_tensor.squeeze(0).cpu().clamp(0, 1).numpy().transpose(1, 2, 0)

    # 3. ROI 중심 재디헤이징
    if enable_roi:
        x1, y1 = (w - roi_w) // 2, (h - roi_h) // 2
        x2, y2 = x1 + roi_w, y1 + roi_h
        center_roi = output_image[y1:y2, x1:x2, :]

        center_tensor = to_tensor(center_roi).unsqueeze(0).to(device)
        with torch.no_grad():
            center_output = model(center_tensor)

        center_output_image = center_output.squeeze(0).cpu().clamp(0, 1).numpy().transpose(1, 2, 0)

        if enable_blend:
            # 블렌딩
            blended_roi = center_output_image * alpha + output_image[y1:y2, x1:x2, :] * (1 - alpha)
            output_image[y1:y2, x1:x2, :] = blended_roi
        else:
            # 단순 치환
            output_image[y1:y2, x1:x2, :] = center_output_image

    # 최종 저장
    final_frame = (output_image * 255).astype(np.uint8)
    final_frame_bgr = cv2.cvtColor(final_frame, cv2.COLOR_RGB2BGR)
    out.write(final_frame_bgr)

    frame_count += 1
    print(f"[INFO] Frame {frame_count} processed.")

cap.release()
out.release()
print(f"[✔] 처리 완료: {output_video_path} ({frame_count} frames)")