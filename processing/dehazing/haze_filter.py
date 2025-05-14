import cv2
import numpy as np

def apply_fog(image, beta=0.1, A=0.9):
    h, w = image.shape[:2]
    depth = np.linspace(1, 3, w)  # 단순 깊이맵 시뮬레이션
    depth_map = np.tile(depth, (h, 1))

    transmission = np.exp(-beta * depth_map)
    transmission = np.stack([transmission]*3, axis=-1)

    foggy = image * transmission + A * 255 * (1 - transmission)
    foggy = np.clip(foggy, 0, 255).astype(np.uint8)
    return foggy
