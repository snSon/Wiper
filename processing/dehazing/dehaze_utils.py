import torch
import numpy as np
import torchvision.transforms as transforms

to_tensor = transforms.ToTensor()

def dehaze_image_tensor(model, image_tensor):
    """
    모델과 텐서 이미지로부터 디헤이징된 결과 생성

    Args:
        model: 학습된 디헤이징 모델
        image_tensor (torch.Tensor): (1, 3, H, W) 크기의 텐서

    Returns:
        numpy.ndarray: 디헤이징된 (H, W, 3) 이미지 (0~1)
    """
    with torch.no_grad():
        output = model(image_tensor)
    output_image = output.squeeze(0).cpu().clamp(0, 1).numpy().transpose(1, 2, 0)
    return output_image

def dehaze_image_np(model, rgb_image_np, device):
    """
    numpy RGB 이미지 입력 → 디헤이징된 numpy 출력

    Args:
        model: 학습된 디헤이징 모델
        rgb_image_np (np.ndarray): (H, W, 3) RGB 이미지 (0~255)
        device: torch.device

    Returns:
        numpy.ndarray: 디헤이징된 (H, W, 3) 이미지 (0~1)
    """
    input_tensor = to_tensor(rgb_image_np).unsqueeze(0).to(device)
    return dehaze_image_tensor(model, input_tensor)
