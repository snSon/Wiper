# Fog/Haze Module

CUDA 기반으로 이미지·비디오에 안개(Fog) 효과를 빠르게 적용할 수 있는 PyTorch 모듈입니다.

<br>

# Usage

```text
1. Input
- hazing_input 폴더에 hazing을 적용할 이미지 또는 비디오 넣기

2. Run
- python haze_apply.py [--beta BETA] [--A A] [--layers LAYERS]
beta : 안개 밀도 계수 (float, 기본값 1.0)
A : 대기광 강도 (0~1, float, 기본값 0.9)
layers : 안개 레이어 수 (int, 기본값 100)
```

<br>

## Example
### 기본 파라미터로 실행
python haze_apply.py

### 안개를 더 짙게(β=1.5), 레이어 80개로
python haze_apply.py --beta 1.5 --layers 80

### 대기광을 조금 더 높여서(0.95)
python haze_apply.py --A 0.95

<br>

## 파일 구조
```text
├── haze_filter.py
├── haze_apply.py
├── hazing_input/
│   ├── img1
│   └── subfolder/
│       └── video1.mp4
└── hazing_output/
    ├── hazed_img1
    └── subfolder/
        └── hazed_video1.mp4
```

<br>

## `haze_filter.py`

### 설명  
`apply_fog` 함수는 입력된 4D 이미지 텐서([B, 3, H, W], 0~1 정규화)에 대기광 모델 기반의 안개 효과를 적용합니다.  
GPU(CUDA) 가속이 가능하도록 작성되어 있으며, `beta`, `A`, `layers` 파라미터로 안개 특성을 제어할 수 있습니다.

```python
apply_fog(
    image_tensor: torch.Tensor,  # [B,3,H,W], float32, 0~1 정규화, GPU 상에 있어야 함
    beta: float = 1.0,           # 안개 밀도 계수
    A: float = 0.9,              # 대기광 강도 (0~1)
    layers: int = 100            # 안개 레이어 수
) -> torch.Tensor               # same shape, fog applied
```

<br>

## `haze_apply.py`

### 설명
커맨드라인에서 단일 이미지 또는 비디오 파일을 입력받아 apply_fog 를 적용하고, 결과를 파일로 저장하는 스크립트입니다.
JPEG/PNG/BMP/TIFF 이미지는 물론 MP4/AVI/MOV/MKV 비디오도 지원합니다.