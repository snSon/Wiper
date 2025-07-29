# remote/Wiper/processing/dehazing/dehazing_pipeline.py

import torch
import numpy as np
import torchvision.transforms as transforms
import cv2
import os

import torch.nn.functional as F # F.interpolate 사용
from dehazing.JetDehaze.JetDehaze import JetDehazeNet
from dehazing.AOD.AOD import AODNet

# 모델 및 설정 초기화 (1회만)
device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

#------ JetDehaze or AOD 선택하기 ---------#

## JetDehaze 가져오기
model = JetDehazeNet().to(device)
MODEL_PATH = os.path.join(os.path.dirname(__file__), "JetDehaze/JetDehaze.pth")

# AOD 가져오기
# model = AODNet().to(device)
# MODEL_PATH = os.path.join(os.path.dirname(__file__), "AOD/AOD_NET.pth")

#------ JetDehaze or AOD 선택하기 ---------#

model.load_state_dict(torch.load(MODEL_PATH, map_location=device))
model.eval()
# model.half()  # FP16로 변환 ""cpu 기반 API에서는 반드시 꺼야 함""

to_tensor = transforms.ToTensor() # cpu version api

# ToTensor 변환 함수
@torch.inference_mode()
def apply_dehazing_tensor(t_img): # t_img: (B, 3, H, W) fp16/32, [0, 1] RGB

     # 1) dtype 통일 (model dtype과 일치)
    target_dtype = next(model.parameters()).dtype
    if t_img.dtype != target_dtype:
        t_img = t_img.to(target_dtype)

    # 2) device 통일
    if t_img.device != device:
        t_img = t_img.to(device, non_blocking=True)

    # 3) 다운스케일 (640×360)  ※ 원본 해상도보다 작으면 생략
    if t_img.shape[2] > 360:
        t_small = F.interpolate(t_img, size=(360, 640), mode="bilinear", align_corners=False)
    else:
        t_small = t_img

    # 4) 모델 추론
    t_out = model(t_small)
    
    # 5) 원본 해상도로 복원
    t_out = F.interpolate(t_out, size=t_img.shape[2:], mode="bilinear", align_corners=False)

    # 6) 값 범위 보정
    return t_out.clamp_(0, 1)

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