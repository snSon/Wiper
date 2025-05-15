import cv2
import os
import numpy as np
from haze_filter import apply_fog

# 경로 설정
input_image_path = 'input/image.png'
input_video_path = 'input/test_drive_30.mp4'
output_image_path = 'output/foggy_image.png'
output_video_path = 'output/foggy_video.mp4'

# ✅ 이미지에 안개 효과 적용
def process_image():
    image = cv2.imread(input_image_path)
    foggy_image = apply_fog(image, 1.0, 0.9)
    cv2.imwrite(output_image_path, foggy_image)
    print(f"[✔] Foggy image saved at {output_image_path}")

# ✅ 영상에 안개 효과 적용
def process_video():
    cap = cv2.VideoCapture(input_video_path)
    fourcc = cv2.VideoWriter_fourcc(*'mp4v')
    fps = cap.get(cv2.CAP_PROP_FPS)
    w = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
    h = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
    print(w, h)
    
    out = cv2.VideoWriter(output_video_path, fourcc, fps, (w, h))

    while cap.isOpened():
        ret, frame = cap.read()
        if not ret or frame is None:
            break
        print(frame)
        
        foggy_frame = apply_fog(frame, 1.0, A=0.9)
        foggy_frame = np.clip(foggy_frame, 0, 255).astype(np.uint8)
        out.write(foggy_frame)


    cap.release()
    out.release()
    print(f"[✔] Foggy video saved at {output_video_path}")

if __name__ == '__main__':
    os.makedirs('output', exist_ok=True)
    print("hello")
    process_image()
    process_video()
