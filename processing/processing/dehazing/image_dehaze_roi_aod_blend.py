import cv2
import torch
import torchvision.transforms as transforms
import numpy as np
from net import dehaze_net

# 경로 설정
input_video_path = "foggy_video.mp4"
output_video_path = "depth100_dehazed_video_with_blended_center.mp4"
model_path = "dehazer.pth"

# 모델 로딩
device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
model = dehaze_net().to(device)
model.load_state_dict(torch.load(model_path, map_location=device))
model.eval()

# 영상 불러오기
cap = cv2.VideoCapture(input_video_path)
fps = cap.get(cv2.CAP_PROP_FPS)
w = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
h = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
fourcc = cv2.VideoWriter_fourcc(*'mp4v')
out = cv2.VideoWriter(output_video_path, fourcc, fps, (w, h))

# Transform 정의
to_tensor = transforms.ToTensor()

# Alpha 마스크 미리 생성 (고정 크기이므로 1회만 생성)
roi_w, roi_h = w // 2, h // 2
alpha = np.zeros((roi_h, roi_w, 1), dtype=np.float32)
for i in range(roi_h):
    for j in range(roi_w):
        dy = abs(i - roi_h / 2) / (roi_h / 2)
        dx = abs(j - roi_w / 2) / (roi_w / 2)
        dist = np.sqrt(dx**2 + dy**2)
        alpha[i, j, 0] = np.clip(1.0 - dist, 0, 1)

# 프레임 반복
frame_count = 0
while cap.isOpened():
    ret, frame = cap.read()
    if not ret:
        break

    rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    input_tensor = to_tensor(rgb_frame).unsqueeze(0).to(device)

    with torch.no_grad():
        output_tensor = model(input_tensor)

    output_image = output_tensor.squeeze(0).cpu().clamp(0, 1).numpy()
    output_image = np.transpose(output_image, (1, 2, 0))

    # ROI 설정 및 재디헤이징
    x1, y1 = (w - roi_w) // 2, (h - roi_h) // 2
    x2, y2 = x1 + roi_w, y1 + roi_h
    center_roi = output_image[y1:y2, x1:x2, :]

    center_tensor = to_tensor(center_roi).unsqueeze(0).to(device)
    with torch.no_grad():
        center_output = model(center_tensor)

    center_output_image = center_output.squeeze(0).cpu().clamp(0, 1).numpy()
    center_output_image = np.transpose(center_output_image, (1, 2, 0))

    # 부드러운 블렌딩
    blended_roi = center_output_image * alpha + output_image[y1:y2, x1:x2, :] * (1 - alpha)
    output_image[y1:y2, x1:x2, :] = blended_roi

    # 저장
    final_frame = (output_image * 255).astype(np.uint8)
    final_frame_bgr = cv2.cvtColor(final_frame, cv2.COLOR_RGB2BGR)
    out.write(final_frame_bgr)

    frame_count += 1
    print(f"[INFO] Processed frame {frame_count}")

cap.release()
out.release()
print(f"[✔] 영상 디헤이징 + ROI 블렌딩 완료: {output_video_path} ({frame_count} frames)")
