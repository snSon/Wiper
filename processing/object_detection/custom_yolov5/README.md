# Custom YOLOv5 Object Detection

이 레포지토리는 Jetson에 최적화된 경량화된 YOLOv5 버전입니다. [YOLOv5](https://github.com/ultralytics/yolov5)를 기반으로 합니다.

## 주요 기능
- `detect.py`: 지정된 모델 및 입력 소스를 사용하여 객체 탐지를 실행합니다. (수정됨)
  → 프레임당 추론 시간 로그가 `results.txt`에 저장됩니다.

- `val.py`: 지정된 모델의 객체 탐지 성능을 mAP, Precision, Recall 등의 지표로 정량적으로 평가합니다. (수정됨)
  → Inference 수행 전, Dehazing을 수행하는 기능이 추가되었습니다.

- `detect_runner.py`: `detect.py`를 감싸서 추론을 실행하고, GPU/RAM 사용량(tegrastats를 통해)을 모니터링하며, 성능 로그를 생성하는 실험 드라이버입니다.
  → FPS, 추론 시간, GPU/RAM 사용량을 측정하고 실행별로 결과를 정리합니다.

- `scripts/test_detect.sh`: `detect_runner.py`를 사용하여 여러 조건의 객체 실험을 자동화하는 쉘 스크립트입니다.
  → `.pt`, `.engine`, FP16 등과 같은 구성을 쉽게 반복하고 비교할 수 있습니다.

- `scripts/test_val.sh`: 다양한 설정으로 val.py를 일괄 실행하는 쉘 스크립트입니다.
  → 실행 모델, 비디오, 데이터셋과 같은 설정을 변경하며 유효성 검사 벤치마킹에 유용합니다.

- `scripts/convert_model.sh`: `.pt` -> `.onnx` `.engine` 변환을 한 번의 명령으로 자동 수행합니다.

- `scripts/download_resources.sh`: 사전 훈련된 모델과 테스트 영상을 다운로드합니다.

- `scripts/jetson_env.sh`: JetPack, PyTorch, CUDA, TensorRT, OpenCV를 포함한 Jetson 하드웨어 및 소프트웨어 스택 정보를 출력합니다.

- 모듈식 디렉토리 구조 (예: `models/`, `scripts/`, `metadata/`)

## Directory Overview
레포지토리는 다음과 같이 구성되어 있습니다.
<pre>
project-root/
├── data/                     # 데이터셋 파일
├── metadata/                 # 원본 오픈소스 라이선스 및 인용 파일
│   ├── CITATION.cff
│   └── CONTRIBUTING.md
├── models/                   # 훈련된 모델 (.pt)
├── runs/                     # 추론 및 평가 결과
├── scripts/                  # 설정, 변환, 평가를 위한 유틸리티 쉘 스크립트
│   ├── convert_model.sh
│   ├── download_resources.sh
│   ├── jetson_env.sh
│   ├── test_detect.sh
│   └── test_val.sh
├── utils/                   # Python 유틸리티 모음 (사용된 경우)
├── detect.py                # 수정된 "detect.py"
├── detect_runner.py         # detect.py를 실행하고 성능 지표를 로깅합니다.
├── export.py                # Model export script (to ONNX, etc.)
├── hubconf.py               # PyTorch Hub interface
├── LICENSE                  # MIT License file
├── pyproject.toml           # 프로젝트 구성 메타데이터
├── README.md                # 프로젝트 문서
├── requirements.txt         # Python dependencies
├── val.py                   # 수정된 유효성 검사 스크립트트
</pre>

## Metadata and Licensing

이 프로젝트는 Ultralytics YOLOv5를 기반으로 하며 원본 MIT 라이선스를 따릅니다.

다음 오픈소스 관련 문서는 `metadata/` 디렉토리로 보존 및 재배치되었습니다:

- `metadata/CITATION.cff`: 학술적 참조를 위한 인용 가이드
- `metadata/CONTRIBUTING.md`: 원본 저장소의 기여 가이드라인

라이선스 파일 (`LICENSE`)은 루트 디렉토리에 남아 있습니다.

> 이 저장소의 모든 수정 사항은 원본 MIT 라이선스의 조건에 따릅니다.
> 원본 저작권 및 라이선스 정보는 유지되어야 합니다.

## Environment Setup

```bash
pip install -r requirements.txt
```
## 실행 방법

### 1. 사전 훈련된 모델 및 테스트 비디오 다운로드

```bash
./scripts/download_resources.sh
```

### 2. `.pt` → `.onnx` → `.engine` (TensorRT) 변환

```bash
./scripts/convert_model.sh
```

### 3. 추론 실행

```bash
./scripts/test_detect.sh
```

### 4. 모델 평가 (mAP, precision, recall)

```bash
./scripts/test_val.sh
```