import cv2
import torch
import torchvision.transforms as transforms
import numpy as np
from net import dehaze_net

# 경로 설정
input_path = "input/foggy_image.png"
output_path = "output/aod_dehazed_image.png"
model_path = "processing/dehazing/dehazer.pth"

# 1. 모델 로딩
device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
model = dehaze_net().to(device)
model.load_state_dict(torch.load(model_path, map_location=device))
model.eval()

# 2. 이미지 불러오기 및 전처리
image = cv2.imread(input_path)
image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
h, w, _ = image.shape

transform = transforms.Compose([
    transforms.ToTensor(),                    # [H, W, C] -> [C, H, W] + [0, 1] 정규화
])

input_tensor = transform(image).unsqueeze(0).to(device)  # [1, 3, H, W]

# 3. 모델 추론
with torch.no_grad():
    output_tensor = model(input_tensor)

# 4. 후처리 및 저장
output_image = output_tensor.squeeze(0).cpu().clamp(0, 1).numpy()
output_image = np.transpose(output_image, (1, 2, 0)) * 255
output_image = output_image.astype(np.uint8)
output_image = cv2.cvtColor(output_image, cv2.COLOR_RGB2BGR)
cv2.imwrite(output_path, output_image)

print(f"[✔] 디헤이징 완료: {output_path}")
