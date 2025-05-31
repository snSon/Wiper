# Dehazing Model for jetson

제작한 모델명: JetDehaze

<br>

# Fog/Haze Module

CUDA 기반으로 이미지·비디오에 안개(Fog) 효과를 빠르게 적용할 수 있는 PyTorch 모듈입니다.

<br>

# Usage

## 1. 환경 세팅
```text
환경 세팅

[모델 pth파일 다운로드](https://drive.google.com/file/d/1pGPN4S6kOlZ8R6Febkrq5UJICjDRVBgX/view?usp=sharing)

압축 해제 후 각각의 모델명에 맞는 폴더에 위치
```

## 2. Hazing
```text
i.
hazing_input 폴더에 hazing을 적용할 이미지 또는 비디오 넣기

ii.
python haze_apply.py [--beta BETA] [--A A] [--layers LAYERS]
beta : 안개 밀도 계수 (float, 기본값 1.0)
A : 대기광 강도 (0~1, float, 기본값 0.9)
layers : 안개 레이어 수 (int, 기본값 100)
```

## 3. Dehazing
```text
dehazing_utils.py에서 모델 선택하기

# JetDehaze 가져오기
# model = JetDehazeNet().to(device)
# MODEL_PATH = os.path.join(os.path.dirname(__file__), "JetDehaze/JetDehaze.pth")

# AOD 가져오기
model = AODNet().to(device)
MODEL_PATH = os.path.join(os.path.dirname(__file__), "AOD/AOD_NET.pth")
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
dehazing/
├── AOD/
│   ├── AOD_NET.pth
│   └── AOD.py
├── JetDehaze/
│   ├── JetDehaze.pth
│   └── JetDehaze.py
├── hazing_input/
│   ├── 0586.jpg
│   └── subfolder/
│       └── (이미지/비디오 파일들)
├── hazing_output/
│   ├── 0586.jpg
│   └── subfolder/
│       └── (이미지/비디오 파일들)
├── dehazing_utils.py
├── haze_apply.py
├── haze_filter.py
└── README.md
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

<br>

## `dehazing_utils.py`

### 설명
- GPU 혹은 CPU 환경을 감지하여 모델을 한 번만 로드하고 초기화합니다.
- AODNet (or JetDehazeNet) 모델 가중치를 불러와 `eval()` 모드로 설정합니다.
- 하나의 OpenCV BGR 프레임을 입력받아:
  1. 크기를 `640×360`으로 리사이즈
  2. BGR → RGB 변환
  3. Tensor 변환 (`ToTensor`)
  4. 모델 추론 (Dehazing)
  5. 결과를 다시 NumPy 배열로 변환하여 `[0,1]` → `[0,255]`로 스케일링
  6. RGB → BGR으로 재변환  
  최종적으로 `(360, 640, 3)` 크기의 `uint8 BGR` 이미지를 반환합니다.