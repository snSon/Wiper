#!/bin/bash

# setting path and config =========================================================

USE_MONITORING=true # detect_runner 사용 여부
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$SCRIPT_DIR/.."
DATA_PATH="$ROOT_DIR/data/coco128.yaml"

# Jetson GPU 성능 최대로 설정
# nvpmodel -m 0
# jetson_clocks

VIDEO_PATH="$ROOT_DIR/videos/foggy_1st_padding.mp4"
IMG_SIZE=640
CONF=0.25
REPEAT=1  # 반복 횟수 변수 설정

# 실험 조합 정의 
declare -A experiments
# experiments["pt_fp32"]=""           # PyTorch FP32
# experiments["pt_half"]="--half"     # PyTorch FP16
experiments["trt_engine"]=""        # TensorRT 엔진

# exec test ======================================================================

if $USE_MONITORING; then
    RUN_SCRIPT="$ROOT_DIR/detect_runner.py"
else
    RUN_SCRIPT="$ROOT_DIR/detect.py"
fi

for name in "${!experiments[@]}"; do
    for ((i = 1; i <= REPEAT; i++)); do
        echo "[$name] 실험 $i 번째 실행 중..."

        if [[ "$name" == "trt_engine" ]]; then
            WEIGHTS="$ROOT_DIR/models/yolov5s.engine"
        else
            WEIGHTS="$ROOT_DIR/models/yolov5s.pt"
        fi

        python3 $RUN_SCRIPT \
            --weights "$WEIGHTS" \
            --source "$VIDEO_PATH" \
            --conf $CONF \
            --img $IMG_SIZE \
            --save-txt \
            --save-conf \
            --device 0 \
            --project $ROOT_DIR/runs/test_detect \
            --name "${name}_run${i}" \
            --data $DATA_PATH \
            ${experiments[$name]}
    done
done
