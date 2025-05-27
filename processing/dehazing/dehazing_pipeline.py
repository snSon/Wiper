# remote/Wiper/processing/dehazing/dehazing_pipeline.py

import torch
import numpy as np
import torchvision.transforms as transforms
import cv2
import os

from dehazing.model import JetDehazeNet
from dehazing.haze_filter import apply_fog


# 모델 및 설정 초기화 (1회만)
device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
model = JetDehazeNet().to(device)

MODEL_PATH = os.path.join(os.path.dirname(__file__), "JetDehaze.pth")
model.load_state_dict(torch.load(MODEL_PATH, map_location=device))
model.eval()
to_tensor = transforms.ToTensor()


# 블렌딩 마스크 생성 함수
def generate_alpha_mask(roi_w, roi_h):
    alpha = np.zeros((roi_h, roi_w, 1), dtype=np.float32)
    for i in range(roi_h):
        for j in range(roi_w):
            dy = abs(i - roi_h / 2) / (roi_h / 2)
            dx = abs(j - roi_w / 2) / (roi_w / 2)
            dist = np.sqrt(dx ** 2 + dy ** 2)
            alpha[i, j, 0] = np.clip(1.0 - dist, 0, 1)
    return alpha

alpha_mask_cache = {}

def dehaze_frame_bgr(frame_bgr, enable_haze=False, enable_aod=True, enable_roi=False, enable_blend=False):
    frame_bgr = cv2.resize(frame_bgr, (640, 360))
    h, w = frame_bgr.shape[:2]
    rgb_frame = cv2.cvtColor(frame_bgr, cv2.COLOR_BGR2RGB)

    # Haze 적용
    if enable_haze:
        rgb_tensor = to_tensor(rgb_frame).unsqueeze(0).to(device)
        with torch.no_grad():
            rgb_tensor = apply_fog(rgb_tensor, beta=2.3, A=0.8)
        rgb_frame = (rgb_tensor.squeeze(0).cpu().clamp(0, 1).numpy().transpose(1, 2, 0) * 255).astype(np.uint8)

    # 기본 AOD 디헤이징
    input_tensor = to_tensor(rgb_frame).unsqueeze(0).to(device)
    try:
        with torch.no_grad():
            output_tensor = model(input_tensor)
    except Exception as e:
        print(f"Model inference error: {e}")
    output_image = output_tensor.squeeze(0).cpu().clamp(0, 1).numpy().transpose(1, 2, 0)


    if enable_roi:
        roi_w, roi_h = w // 2, h // 2
        x1, y1 = (w - roi_w) // 2, (h - roi_h) // 2
        x2, y2 = x1 + roi_w, y1 + roi_h
        center_roi = output_image[y1:y2, x1:x2, :]

        center_tensor = to_tensor(center_roi).unsqueeze(0).to(device)
        with torch.no_grad():
            center_output = model(center_tensor)
        center_output_image = center_output.squeeze(0).cpu().clamp(0, 1).numpy().transpose(1, 2, 0)

        if enable_blend:
            key = (roi_w, roi_h)
            if key not in alpha_mask_cache:
                alpha_mask_cache[key] = generate_alpha_mask(roi_w, roi_h)
            alpha = alpha_mask_cache[key]

            blended_roi = center_output_image * alpha + output_image[y1:y2, x1:x2, :] * (1 - alpha)
            output_image[y1:y2, x1:x2, :] = blended_roi
        else:
            output_image[y1:y2, x1:x2, :] = center_output_image

    # RGB → BGR 변환
    final_frame_bgr = (output_image * 255).astype(np.uint8)
    return cv2.cvtColor(final_frame_bgr, cv2.COLOR_RGB2BGR)
