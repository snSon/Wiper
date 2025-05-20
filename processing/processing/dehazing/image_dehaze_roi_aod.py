import cv2
import torch
import torchvision.transforms as transforms
import numpy as np
from net import dehaze_net

# 경로 설정
input_video_path = "foggy_video.mp4"
output_video_path = "depth100_roi_aod_dehazed_video.mp4"
model_path = "dehazer.pth"

# 1. 모델 불러오기
device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
model = dehaze_net().to(device)
model.load_state_dict(torch.load(model_path, map_location=device))
model.eval()

# 2. 영상 준비
cap = cv2.VideoCapture(input_video_path)
fps = cap.get(cv2.CAP_PROP_FPS)
w = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
h = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
fourcc = cv2.VideoWriter_fourcc(*'mp4v')
out = cv2.VideoWriter(output_video_path, fourcc, fps, (w, h))

# 3. 전처리 transform
to_tensor = transforms.ToTensor()

# 4. 프레임 반복 처리
frame_count = 0
while cap.isOpened():
    ret, frame = cap.read()
    if not ret:
        break

    # BGR → RGB
    rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

    # 1차 디헤이징
    input_tensor = to_tensor(rgb_frame).unsqueeze(0).to(device)
    with torch.no_grad():
        output_tensor = model(input_tensor)

    output_image = output_tensor.squeeze(0).cpu().clamp(0, 1).numpy()
    output_image = np.transpose(output_image, (1, 2, 0))  # [H, W, C]

    # 중앙 ROI (가로/세로 1/2)
    roi_w, roi_h = w // 2, h // 2
    x1, y1 = (w - roi_w) // 2, (h - roi_h) // 2
    x2, y2 = x1 + roi_w, y1 + roi_h
    center_roi = output_image[y1:y2, x1:x2, :]

    # ROI 재디헤이징
    center_tensor = to_tensor(center_roi).unsqueeze(0).to(device)
    with torch.no_grad():
        center_output = model(center_tensor)

    center_output_image = center_output.squeeze(0).cpu().clamp(0, 1).numpy()
    center_output_image = np.transpose(center_output_image, (1, 2, 0))

    # 다시 합성
    output_image[y1:y2, x1:x2, :] = center_output_image

    # RGB → BGR + 저장
    final_frame = (output_image * 255).astype(np.uint8)
    final_frame_bgr = cv2.cvtColor(final_frame, cv2.COLOR_RGB2BGR)
    out.write(final_frame_bgr)

    frame_count += 1
    print(frame_count)

cap.release()
out.release()
print(f"[✔] ROI 재디헤이징 포함 영상 저장 완료: {output_video_path} ({frame_count} frames)")
