import cv2
import numpy as np

def apply_fog(image, beta=1.0, A=0.9):
    h, w = image.shape[:2]
    
    # 중심 좌표
    center_x, center_y = w / 2, h / 2

    # 각 픽셀의 중심으로부터 거리 계산
    y, x = np.indices((h, w))
    distance_from_center = np.sqrt((x - center_x)**2 + (y - center_y)**2)

    # 거리 정규화 (0~1)
    max_distance = np.sqrt(center_x**2 + center_y**2)
    normalized_depth = 1- distance_from_center / max_distance

    # 전송 맵 계산
    transmission = np.exp(-beta * normalized_depth)
    transmission = np.stack([transmission]*3, axis=-1)

    # 안개 합성
    foggy = image.astype(np.float32)
    layers = 3
    for i in range(1, layers + 1):
        # 단계적으로 더 얕은 안개 층을 얇게 누적
        weight = i / layers  # 0.33, 0.66, 1.0 등
        local_depth = (1 - (distance_from_center / max_distance)) ** (1.5 * weight)
        transmission = np.exp(-beta * local_depth)
        transmission = np.stack([transmission]*3, axis=-1)

        foggy = foggy * transmission + A * 255 * (1 - transmission)

    return foggy
