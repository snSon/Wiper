import cv2
import numpy as np

def apply_fog(image, beta=1.0, A=0.9, layers=3):
    h, w = image.shape[:2]
    center_x, center_y = w / 2, h / 2

    # 거리 기반 깊이 맵 생성
    y, x = np.indices((h, w))
    distance_from_center = np.sqrt((x - center_x) ** 2 + (y - center_y) ** 2)
    max_distance = np.sqrt(center_x ** 2 + center_y ** 2)

    image = image.astype(np.float32)
    total_transmission = np.zeros((h, w), dtype=np.float32)

    # 여러 안개 레이어를 평균적으로 적용
    for i in range(1, layers + 1):
        weight = i / layers
        local_depth = (1 - distance_from_center / max_distance) ** (1.5 * weight)
        transmission = np.exp(-beta * local_depth)
        total_transmission += transmission

    # 평균 transmission 계산
    avg_transmission = total_transmission / layers
    avg_transmission = np.stack([avg_transmission] * 3, axis=-1)

    foggy = image * avg_transmission + A * 255 * (1 - avg_transmission)
    return np.clip(foggy, 0, 255).astype(np.uint8)
