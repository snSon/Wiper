import os
import cv2
import torch
import numpy as np
from model import JetDehazeNet  # 사용자 정의 모델

# 디바이스 설정
device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

# 모델 로드
model = JetDehazeNet().to(device)
model.load_state_dict(torch.load("jetDehaze_epoch_1.pth", map_location=device))
model.eval()

# 디렉토리 설정
input_dir = "test_images"
output_dir = "results"
os.makedirs(output_dir, exist_ok=True)

# 이미지 파일 반복 처리
for fname in os.listdir(input_dir):
    if not fname.lower().endswith((".jpg", ".png", ".jpeg", ".bmp")):
        continue  # 이미지 아닌 파일은 스킵

    # 1. 이미지 로드
    img_path = os.path.join(input_dir, fname)
    img = cv2.imread(img_path)
    if img is None:
        print(f"[경고] 이미지 로드 실패: {fname}")
        continue

    # 2. RGB 및 정규화
    img_rgb = img[:, :, ::-1].astype(np.float32) / 255.0  # BGR → RGB
    img_tensor = torch.from_numpy(img_rgb).permute(2, 0, 1).unsqueeze(0).to(device)

    # 3. 디헤이징 수행
    with torch.no_grad():
        output = model(img_tensor)

    # 4. 후처리 및 저장
    output_img = output.squeeze().cpu().numpy().transpose(1, 2, 0)
    output_img = (output_img * 255).clip(0, 255).astype(np.uint8)
    output_img_bgr = output_img[:, :, ::-1]  # RGB → BGR

    out_path = os.path.join(output_dir, fname)
    cv2.imwrite(out_path, output_img_bgr)

    print(f"[✔️ 완료] {fname} → {out_path}")
