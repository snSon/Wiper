# -*- coding: utf-8 -*-

import cv2
import torch
import argparse
import numpy as np
import torchvision.transforms as transforms

from pathlib import Path
from dehazing.haze_filter import apply_fog_tensor
from dehazing.dehaze_utils import dehaze_image_np
from dehazing.roi_utils import create_alpha_mask, apply_roi_blend
from dehazing.net import dehaze_net

from object_detection.custom_yolov5.models.common import DetectMultiBackend
from object_detection.custom_yolov5.utils.general import non_max_suppression, scale_coords
from object_detection.custom_yolov5.utils.dataloaders import LoadStreams, LoadImages
from object_detection.custom_yolov5.utils.torch_utils import select_device

# -------------------------
# 1. 인자 파싱
# -------------------------
parser = argparse.ArgumentParser()
parser.add_argument('--weights', type=str, default='object_detection/custom_yolov5/models/yolov5s.pt', help='pt or engine file')
parser.add_argument('--enable_haze', action='store_true')
parser.add_argument('--enable_aod', action='store_true')
parser.add_argument('--enable_roi', action='store_true')
parser.add_argument('--enable_blend', action='store_true')
parser.add_argument('--output_suffix', type=str, default="")
args = parser.parse_args()

# -------------------------
# 2. 기본 설정
# -------------------------
input_video_path = "input/test_drive_30.mp4"
output_video_path = f"output/final_output_{args.output_suffix}.mp4"
model_path = "dehazing/dehazer.pth"
device = select_device('')  # auto

# -------------------------
# 3. Dehazing 모델 준비
# -------------------------
dehaze_model = dehaze_net().to(device)
dehaze_model.load_state_dict(torch.load(model_path, map_location=device))
dehaze_model.eval()

# -------------------------
# 4. YOLOv5 모델 준비
# -------------------------
yolo_model = DetectMultiBackend(args.weights, device=device)
yolo_model.warmup(imgsz=(1, 3, 640, 640))
names = yolo_model.names
stride = yolo_model.stride
imgsz = 640
to_tensor = transforms.ToTensor()

# -------------------------
# 5. 비디오 준비
# -------------------------
cap = cv2.VideoCapture(input_video_path)
fps = cap.get(cv2.CAP_PROP_FPS)
w, h = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH)), int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
fourcc = cv2.VideoWriter_fourcc(*'mp4v')
out = cv2.VideoWriter(output_video_path, fourcc, fps, (w, h))

# -------------------------
# 6. ROI 블렌딩용 마스크 준비
# -------------------------
roi_w, roi_h = w // 2, h // 2
alpha_mask = create_alpha_mask(roi_h, roi_w)

# -------------------------
# 7. 프레임 루프 시작
# -------------------------
frame_count = 0
while cap.isOpened():
    ret, frame = cap.read()
    if not ret:
        break

    rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

    # --- 1. 안개 추가 ---
    if args.enable_haze:
        rgb_tensor = to_tensor(rgb_frame).unsqueeze(0).to(device)
        rgb_tensor = apply_fog_tensor(rgb_tensor)
        rgb_frame = (rgb_tensor.squeeze(0).cpu().numpy().transpose(1, 2, 0) * 255).astype(np.uint8)

    # --- 2. 디헤이징 (전체) ---
    if args.enable_aod:
        output_image = dehaze_image_np(dehaze_model, rgb_frame, device)
    else:
        output_image = rgb_frame.astype(np.float32) / 255.0

    # --- 3. ROI 중심 재디헤이징 ---
    if args.enable_roi:
        x1, y1 = (w - roi_w) // 2, (h - roi_h) // 2
        roi = output_image[y1:y1+roi_h, x1:x1+roi_w, :]
        roi_output = dehaze_image_np(dehaze_model, (roi * 255).astype(np.uint8), device)

        if args.enable_blend:
            output_image = apply_roi_blend(output_image, roi_output, x1, y1, alpha_mask)
        else:
            output_image[y1:y1+roi_h, x1:x1+roi_w, :] = roi_output

    # --- 4. YOLO 추론 ---
    img_resized = cv2.resize((output_image * 255).astype(np.uint8), (imgsz, imgsz))
    img_tensor = to_tensor(img_resized).unsqueeze(0).to(device).float() / 255.0

    with torch.no_grad():
        pred = yolo_model(img_tensor)
        pred = non_max_suppression(pred, 0.25, 0.45)

    # --- 5. 결과 시각화 ---
    final_frame = (output_image * 255).astype(np.uint8)
    final_frame_bgr = cv2.cvtColor(final_frame, cv2.COLOR_RGB2BGR)

    for det in pred:
        if len(det):
            det[:, :4] = scale_coords(img_tensor.shape[2:], det[:, :4], final_frame_bgr.shape).round()
            for *xyxy, conf, cls in det:
                label = f'{names[int(cls)]} {conf:.2f}'
                cv2.rectangle(final_frame_bgr, (int(xyxy[0]), int(xyxy[1])), (int(xyxy[2]), int(xyxy[3])), (0, 255, 0), 2)
                cv2.putText(final_frame_bgr, label, (int(xyxy[0]), int(xyxy[1]) - 5),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1)

    out.write(final_frame_bgr)
    frame_count += 1
    print(f"[INFO] Frame {frame_count} processed.")

# -------------------------
# 8. 마무리
# -------------------------
cap.release()
out.release()
print(f"[✔] 처리 완료: {output_video_path} ({frame_count} frames)")
