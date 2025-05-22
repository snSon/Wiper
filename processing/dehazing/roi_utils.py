import numpy as np

def create_alpha_mask(h, w):
    """
    중심에서 바깥으로 갈수록 알파값이 줄어드는 원형 마스크 생성

    Args:
        h (int): ROI 높이
        w (int): ROI 너비

    Returns:
        np.ndarray: (h, w, 1) 알파 마스크 (0~1 float32)
    """
    alpha = np.zeros((h, w, 1), dtype=np.float32)
    for i in range(h):
        for j in range(w):
            dy = abs(i - h / 2) / (h / 2)
            dx = abs(j - w / 2) / (w / 2)
            dist = np.sqrt(dx ** 2 + dy ** 2)
            alpha[i, j, 0] = np.clip(1.0 - dist, 0, 1)
    return alpha

def apply_roi_blend(base_image, roi_image, x1, y1, alpha_mask):
    """
    ROI 영역을 블렌딩하여 원본 이미지에 반영

    Args:
        base_image (np.ndarray): (H, W, 3) 원본 이미지 (0~1)
        roi_image (np.ndarray): (roi_h, roi_w, 3) ROI 디헤이징 이미지 (0~1)
        x1, y1 (int): ROI 시작 좌표
        alpha_mask (np.ndarray): (roi_h, roi_w, 1) 알파 마스크

    Returns:
        np.ndarray: 블렌딩 적용된 base_image
    """
    y2, x2 = y1 + roi_image.shape[0], x1 + roi_image.shape[1]
    base_image[y1:y2, x1:x2, :] = (
        roi_image * alpha_mask + base_image[y1:y2, x1:x2, :] * (1 - alpha_mask)
    )
    return base_image
