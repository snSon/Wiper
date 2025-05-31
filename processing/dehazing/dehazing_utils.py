# remote/Wiper/processing/dehazing/dehazing_pipeline.py

import torch
import numpy as np
import torchvision.transforms as transforms
import cv2
import os

from dehazing.JetDehaze.JetDehaze import JetDehazeNet
from dehazing.AOD.AOD import AODNet

# 모델 및 설정 초기화 (1회만)
device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

# JetDehaze 가져오기
# model = JetDehazeNet().to(device)
# MODEL_PATH = os.path.join(os.path.dirname(__file__), "JetDehaze/JetDehaze.pth")

# AOD 가져오기
model = AODNet().to(device)
MODEL_PATH = os.path.join(os.path.dirname(__file__), "AOD/AOD_NET.pth")

model.load_state_dict(torch.load(MODEL_PATH, map_location=device))
model.eval()
to_tensor = transforms.ToTensor()

def apply_dehazing(frame_bgr):
    frame_bgr = cv2.resize(frame_bgr, (640, 360))
    h, w = frame_bgr.shape[:2]
    rgb_frame = cv2.cvtColor(frame_bgr, cv2.COLOR_BGR2RGB)
    
    try:
        with torch.no_grad():
            input_tensor = to_tensor(rgb_frame).unsqueeze(0).to(device)
            output_tensor = model(input_tensor)
    except Exception as e:
        print(f"Model inference error: {e}")
    output_image = output_tensor.squeeze(0).cpu().clamp(0, 1).numpy().transpose(1, 2, 0)

    # RGB → BGR 변환
    final_frame_bgr = (output_image * 255).astype(np.uint8)
    return cv2.cvtColor(final_frame_bgr, cv2.COLOR_RGB2BGR)