# This file is test sources for yolov5. 
import cv2 as cv
import torch
import numpy as np
from PIL import Image

device = torch.device('cpu')
model = torch.hub.load('ultralytics/yolov5', 'custom', path='yolov5s.pt', force_reload=True)
model.to(device).float().eval()

# debug
# print(cv.getBuildInformation())

cap = cv.VideoCapture(0)
cap.set(cv.CAP_PROP_FOURCC, cv.VideoWriter_fourcc(*'MJPG'))  # format force
cap.set(cv.CAP_PROP_FRAME_WIDTH, 640)
cap.set(cv.CAP_PROP_FRAME_HEIGHT, 480)
cap.set(cv.CAP_PROP_FPS, 30) 

# debug 
# print(cv.getBuildInformation())

if not cap.isOpened():
    print("Failed Opened the camera.")
    exit()

while True:
    ret, frame = cap.read()
    print(ret, frame)
    if not ret or frame is None:
        print("Failed to read the frames.")
        break
    
    print("Original frame shape:", frame.shape if frame is not None else "No Frame")

    if len(frame.shape) == 2:
        frame = cv.cvtColor(frame, cv.COLOR_GRAY2BGR)

    elif len(frame.shape) == 1 and frame.shape[0] == 921600:
        frame = frame.reshape((480, 640, 3))

    print("Processed frame shape:", frame.shape)

    frame_rgb = cv.cvtColor(frame, cv.COLOR_BGR2RGB) 
    frame_pil = Image.fromarray(frame_rgb) 

    with torch.no_grad():
        outputs = model(frame_pil)

    for box in outputs.xyxy[0]:
        x1,y1,x2,y2,conf,cls=box.tolist()
        cv.rectangle(frame, (int(x1), int(y1)), (int(x2), int(y2)), (0, 255, 0), 2)
        cv.putText(frame, f'{conf:.2f}', (int(x1), int(y1) - 10), cv.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)

    cv.imshow('frame', frame)

    if cv.waitKey(10) & 0xFF == ord('q'):
        break

cap.release()
cv.destroyAllWindows()
