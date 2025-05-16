import cv2
import torch
import torchvision.transforms as transforms
import numpy as np
from net import dehaze_net

# 경로 설정
input_path = "input/foggy_image.png"
output_path = "output/roi_aod_dehazed_image.png"
model_path = "processing/dehazing/dehazer.pth"

# 1. 모델 불러오기
device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
model = dehaze_net().to(device)
model.load_state_dict(torch.load(model_path, map_location=device))
model.eval()

# 2. 이미지 불러오기 및 전처리
image = cv2.imread(input_path)
image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
h, w, _ = image.shape

to_tensor = transforms.ToTensor()
to_image = transforms.ToPILImage()

# 3. 전체 디헤이징 1차 적용
input_tensor = to_tensor(image).unsqueeze(0).to(device)
with torch.no_grad():
    output_tensor = model(input_tensor)

output_image = output_tensor.squeeze(0).cpu().clamp(0, 1).numpy()
output_image = np.transpose(output_image, (1, 2, 0))  # [H, W, C]

# 4. 중앙 영역 ROI 설정 (가로 1/2, 세로 1/2 크기)
roi_w, roi_h = w // 2, h // 2
x1, y1 = (w - roi_w) // 2, (h - roi_h) // 2
x2, y2 = x1 + roi_w, y1 + roi_h
center_roi = output_image[y1:y2, x1:x2, :]

# 5. 중앙 영역 재디헤이징
center_tensor = to_tensor(center_roi).unsqueeze(0).to(device)
with torch.no_grad():
    center_output = model(center_tensor)

center_output_image = center_output.squeeze(0).cpu().clamp(0, 1).numpy()
center_output_image = np.transpose(center_output_image, (1, 2, 0))  # [roi_h, roi_w, 3]

# 6. 다시 합성: 중심 영역만 교체
output_image[y1:y2, x1:x2, :] = center_output_image

# 7. 저장
final_output = (output_image * 255).astype(np.uint8)
final_output = cv2.cvtColor(final_output, cv2.COLOR_RGB2BGR)
cv2.imwrite(output_path, final_output)

print(f"[✔] 디헤이징 완료 (중앙 영역 추가 처리 포함): {output_path}")
