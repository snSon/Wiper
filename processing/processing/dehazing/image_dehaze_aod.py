import cv2
import torch
import torchvision.transforms as transforms
import numpy as np
from net import dehaze_net

# 경로 설정
input_video_path = "depth100_foggy_video.mp4"
output_video_path = "depth100_aod_dehazed_video.mp4"
model_path = "dehazer.pth"

# 1. 모델 로딩
device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
model = dehaze_net().to(device)
model.load_state_dict(torch.load(model_path, map_location=device))
model.eval()

# 2. 영상 불러오기
cap = cv2.VideoCapture(input_video_path)
fps = cap.get(cv2.CAP_PROP_FPS)
w = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
h = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
fourcc = cv2.VideoWriter_fourcc(*'mp4v')
out = cv2.VideoWriter(output_video_path, fourcc, fps, (w, h))

# 3. transform 정의
transform = transforms.Compose([
    transforms.ToTensor(),  # [H, W, C] → [C, H, W], 정규화 [0,1]
])

# 4. 프레임 반복 처리
frame_count = 0
while cap.isOpened():
    ret, frame = cap.read()
    if not ret:
        break

    # 전처리: BGR → RGB → Tensor
    rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    input_tensor = transform(rgb_frame).unsqueeze(0).to(device)

    # 모델 추론
    with torch.no_grad():
        output_tensor = model(input_tensor)

    # 후처리: Tensor → NumPy
    output_image = output_tensor.squeeze(0).cpu().clamp(0, 1).numpy()
    output_image = np.transpose(output_image, (1, 2, 0)) * 255
    output_image = output_image.astype(np.uint8)
    output_image_bgr = cv2.cvtColor(output_image, cv2.COLOR_RGB2BGR)

    out.write(output_image_bgr)
    frame_count += 1
    print(frame_count)

cap.release()
out.release()

print(f"[✔] 디헤이징 완료: {output_video_path} ({frame_count} frames)")
