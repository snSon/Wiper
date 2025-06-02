import cv2
import torch
import numpy as np
from torchvision import transforms

from haze_filter import apply_fog_tensor

# --- 설정 ---
video_path = "Wiper/input/test_drive_30.mp4" # 경로 설정
output_path = "Wiper/output/test_drive_30.mp4" # 경로 설정
device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

# --- 비디오 읽기 설정 ---
cap = cv2.VideoCapture(video_path)
if not cap.isOpened():
    raise FileNotFoundError(f"비디오 파일을 열 수 없습니다: {video_path}")
fps = cap.get(cv2.CAP_PROP_FPS)
width  = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))

# --- 비디오 저장기 설정 ---
fourcc = cv2.VideoWriter_fourcc(*'mp4v')
out = cv2.VideoWriter(output_path, fourcc, fps, (width, height))

# --- transform 준비 ---
to_tensor = transforms.ToTensor()
to_pil = transforms.ToPILImage()
frame_counter = 0
# --- 프레임별 처리 ---
while cap.isOpened():
    ret, frame = cap.read()
    if not ret:
        break

    # BGR -> RGB 및 정규화
    rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    img_tensor = to_tensor(rgb_frame).unsqueeze(0).to(device)  # [1, 3, H, W]
    # 안개 효과 적용
    foggy_tensor = apply_fog_tensor(img_tensor)  # [1, 3, H, W]
    foggy_np = foggy_tensor.squeeze(0).cpu().permute(1, 2, 0).numpy()  # [H, W, 3]
    # 정규화 해제 (0~255) 및 RGB -> BGR
    foggy_uint8 = (foggy_np * 255).astype(np.uint8)
    foggy_bgr = cv2.cvtColor(foggy_uint8, cv2.COLOR_RGB2BGR)
    frame_counter+=1
    print("frame num",frame_counter)
    # 비디오에 프레임 저장
    out.write(foggy_bgr)

# --- 자원 정리 ---
cap.release()
out.release()
cv2.destroyAllWindows()
print("done")

