import torch

def apply_fog_tensor(image_tensor, beta=2.3, A=0.8, layers=100):
    """
    apply_fog()을 wrapping하여 인터페이스를 단순화

    Args:
        image_tensor (torch.Tensor): [1, 3, H, W] 정규화 RGB 텐서
    Returns:
        torch.Tensor: 안개 적용 결과
    """
    return apply_fog(image_tensor, beta=beta, A=A, layers=layers)


def apply_fog(image_tensor, beta=1.0, A=0.9, layers=100):
    """
    CUDA 기반 안개 효과 적용 함수

    Args:
        image_tensor (torch.Tensor): [B, 3, H, W] 형식의 0~1 정규화된 이미지 텐서 (GPU 상에 있어야 함)
        beta (float): 안개의 밀도 조절 계수
        A (float): 대기광 강도 (0~1)
        layers (int): 안개 레이어 수 (기본값: 3)

    Returns:
        torch.Tensor: 안개가 적용된 이미지 텐서 (같은 크기와 형식)
    """
    B, C, H, W = image_tensor.shape
    device = image_tensor.device

    # 중심점 계산
    center_x = W / 2
    center_y = H / 2

    # 거리 기반 깊이 맵 생성 (브로드캐스팅을 위해 meshgrid 대신 linspace + reshape 사용)
    y = torch.linspace(0, H - 1, H, device=device).view(H, 1).expand(H, W)
    x = torch.linspace(0, W - 1, W, device=device).view(1, W).expand(H, W)
    distance_from_center = torch.sqrt((x - center_x) ** 2 + (y - center_y) ** 2)
    max_distance = torch.sqrt(torch.tensor(center_x ** 2 + center_y ** 2, device=device))

    total_transmission = torch.zeros((H, W), device=device)

    for i in range(1, layers + 1):
        weight = i / layers
        local_depth = (1 - distance_from_center / max_distance) ** (1.5 * weight)
        transmission = torch.exp(-beta * local_depth)
        total_transmission += transmission

    # 평균 transmission
    avg_transmission = total_transmission / layers
    avg_transmission = avg_transmission.unsqueeze(0).repeat(C, 1, 1)  # [3, H, W]
    avg_transmission = avg_transmission.unsqueeze(0).repeat(B, 1, 1, 1)  # [B, 3, H, W]

    # A값도 동일한 형식으로 확장
    foggy = image_tensor * avg_transmission + A * (1 - avg_transmission)

    return foggy.clamp(0.0, 1.0)
