#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$SCRIPT_DIR/.."

VAL_PY="$ROOT_DIR/val.py"
WEIGHTS="$ROOT_DIR/models/yolov5s.engine"
DATA_YAML="$ROOT_DIR/data/coco128.yaml"

repeat=1  # 원하는 반복 횟수로 수정 가능

for ((i=1; i<=repeat; i++)); do
    echo " $i 번째 실행 중..."
    python3 "$VAL_PY" \
        --weights "$WEIGHTS" \
        --data "$DATA_YAML" \
        --img 640 \
        --task val \
        --half \
        --device 0 \
        --verbose
    echo " $i 번째 실행 완료"
done

### 
#      옵션	        설명
# --weights	사용할 모델 가중치
# --data	평가용 데이터셋 YAML
# --img	입력 이미지 크기
# --task val	검증(validation)만 수행
# --half	FP16으로 빠르게 (TensorRT처럼 mixed precision)
# --device 0	GPU 사용 (CUDA 장치)
# --save-json	COCO 평가용 JSON 저장
# --verbose	클래스별 정밀도/재현율 출력
###