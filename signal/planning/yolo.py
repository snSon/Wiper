import cv2
#import torch
import numpy as np
import os
from yolov5 import YOLOv5

# 입력/출력 경로
input_path = "video/foggy_dehazed_2.mp4"
output_path = "video/foggy_dehazed_detect_2.mp4"
log_dir = "foggy_de_2"
os.makedirs(log_dir, exist_ok=True)

# YOLOv5 모델 로드
#model = torch.hub.load("ultralytics/yolov5", "yolov5s", pretrained=True)
model = YOLOv5("/home/hanul/Wiper_workspace/jiwan/Wiper/processing/object_detection/custom_yolov5/models/yolov5s.engine")

cap = cv2.VideoCapture(input_path)
fps = cap.get(cv2.CAP_PROP_FPS)
width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
fourcc: int = cv2.VideoWriter_fourcc(*"mp4v")
out = cv2.VideoWriter(output_path, fourcc, fps, (width, height))
   
frame_idx = 0 
while True:
    ret, frame = cap.read()
    if not ret:
        break
    resized = cv2.resize(frame, (640, 640))
    results = model.predict(resized)
    # 스케일 비율 계산
    scale_x = width / 640
    scale_y = height / 640
    
    boxes = results.xyxy[0].cpu().numpy()  # [x1, y1, x2, y2, conf, cls]
    log_path = os.path.join(log_dir, f"frame_{frame_idx:03d}.txt")
    with open(log_path, "w") as log_file:
        for obj in results.xyxy[0].cpu().numpy():  # [x1, y1, x2, y2, conf, cls]
            x1, y1, x2, y2, conf, cls = obj
            x1 = int(x1 * scale_x)
            y1 = int(y1 * scale_y)
            x2 = int(x2 * scale_x)
            y2 = int(y2 * scale_y)

            label = int(cls)
            cx = (x1 + x2) / 2
            cy = (y1 + y2) / 2
            w = x2 - x1
            h = y2 - y1

            cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 255, 0), 2)
            cv2.putText(frame, f"{label} {conf:.2f}", (x1, y1 - 5),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 1)
            log_file.write(f"{cls:.1f} {cx:.1f} {cy:.1f} {w:.1f} {h:.1f} {conf:.2f}\n")

    out.write(frame)
    frame_idx += 1

cap.release()
out.release()
print("[✓] 추론 완료 (640x640) → 결과는 원본 해상도에 시각화 후 저장")