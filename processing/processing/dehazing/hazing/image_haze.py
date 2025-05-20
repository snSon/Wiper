import cv2
import os
import numpy as np
import torch
from pathlib import Path
from haze_filter import apply_fog  # 안개 효과 함수 (Tensor 입력, GPU 연산)

# 경로 설정
input_video_path = 'test_drive_30.mp4'
output_dir = Path('output')
output_dir.mkdir(parents=True, exist_ok=True)
save_path = str(output_dir / 'depth100_foggy_video.mp4')


def process_video():
    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
    cap = cv2.VideoCapture(input_video_path)

    if not cap.isOpened():
        print(f"[✘] Failed to open video: {input_video_path}")
        return

    # 영상 속성 가져오기
    fps = cap.get(cv2.CAP_PROP_FPS)
    w = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
    h = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
    print(f"Resolution: {w}x{h}, FPS: {fps}")

    # YOLO 스타일: 첫 프레임에서 VideoWriter 생성
    vid_writer = None
    frame_count = 0

    while True:
        ret, frame = cap.read()
        if not ret:
            break

        # BGR -> RGB 변환 및 정규화
        rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        rgb_tensor = torch.from_numpy(rgb_frame).float().to(device) / 255.0  # [H, W, 3]
        rgb_tensor = rgb_tensor.permute(2, 0, 1).unsqueeze(0)  # [1, 3, H, W]

        with torch.no_grad():
            foggy_tensor = apply_fog(rgb_tensor, beta=2.3, A=0.8)

        foggy_tensor = torch.clamp(foggy_tensor.squeeze(0), 0.0, 1.0)  # 안정적 변환
        foggy_frame = (foggy_tensor.permute(1, 2, 0).cpu().numpy() * 255).astype(np.uint8)
        foggy_frame_bgr = cv2.cvtColor(foggy_frame, cv2.COLOR_RGB2BGR)

        # 첫 프레임에서 VideoWriter 초기화
        if vid_writer is None:
            fourcc = cv2.VideoWriter_fourcc(*'mp4v')
            vid_writer = cv2.VideoWriter(save_path, fourcc, fps, (w, h))

        vid_writer.write(foggy_frame_bgr)
        frame_count += 1
        print(frame_count)

    cap.release()
    if vid_writer:
        vid_writer.release()

    print(f"[✔] Foggy video saved at {save_path} ({frame_count} frames)")


if __name__ == '__main__':
    print("Start processing with CUDA...")
    process_video()
