# Custom YOLOv5 Object Detection

본 레포지토리는 [YOLOv5](https://github.com/ultralytics/yolov5)를 기반으로 객체 탐지 기능에 특화된 경량화 버전입니다.

## 주요 기능
- `detect.py`: 객체 탐지 실행
- `val.py`: 검증 수행
- `export.py`: 모델 변환 (.onnx, .torchscript 등)
- `convert_model.sh`: ONNX → TensorRT 엔진 자동 변환
- `download_resources.sh`: 외부 리소스 자동 다운로드

## 환경 구성
```bash
pip install -r requirements.txt