# aod_yolo_main.py
import cv2
import torch
import numpy as np
from aod_net import AODNet

# Load AOD-Net
aod_model = AODNet().cuda().eval()
aod_model.load_state_dict(torch.load("aod_net.pth"))

# Load YOLOv5
yolo_model = torch.hub.load('ultralytics/yolov5', 'yolov5s', pretrained=True)
yolo_model.cuda().eval()

# 디헤이징 함수
def dehaze_frame(frame):
    img = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    img = torch.from_numpy(img).float().permute(2, 0, 1).unsqueeze(0).cuda() / 255.0
    with torch.no_grad():
        output = aod_model(img)
    out_img = output.squeeze().permute(1, 2, 0).cpu().numpy()
    out_img = (np.clip(out_img, 0, 1) * 255).astype('uint8')
    return cv2.cvtColor(out_img, cv2.COLOR_RGB2BGR)

# 비디오 스트림 시작
#cap = cv2.VideoCapture(0)  #  CSI 카메라는 'nvarguscamerasrc' 사용 가능

while cap.isOpened():
    ret, frame = cap.read()
    if not ret:
        break

    # AOD-Net 디헤이징
    dehazed = dehaze_frame(frame)

    # YOLOv5 객체 감지
    results = yolo_model(dehazed)
    results.render()

    # 결과 시각화
    cv2.imshow("AOD + YOLOv5", results.ims[0])
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
