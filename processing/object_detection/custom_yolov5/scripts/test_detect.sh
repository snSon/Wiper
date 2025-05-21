#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$SCRIPT_DIR/.."

# Jetson GPU 성능 최대로 설정
# nvpmodel -m 0
# jetson_clocks

VIDEO_PATH="$ROOT_DIR/videos/test_drive_30.mp4"
IMG_SIZE=640
CONF=0.25
REPEAT=1  # 반복 횟수 변수 설정

# 실험 조합 정의 
declare -A experiments
# experiments["pt_fp32"]=""           # PyTorch FP32
experiments["pt_half"]="--half"     # PyTorch FP16
# experiments["trt_engine"]=""        # TensorRT 엔진

for name in "${!experiments[@]}"; do
    for ((i = 1; i <= REPEAT; i++)); do
        echo "[$name] 실험 $i 번째 실행 중..."

        if [[ "$name" == "trt_engine" ]]; then
            WEIGHTS="$ROOT_DIR/models/yolov5s.engine"
        else
            WEIGHTS="$ROOT_DIR/models/yolov5s.pt"
        fi
        ## 반복문 
        ## 이미지 디헤이징 하기

        python3 $ROOT_DIR/detect_runner.py \
            --weights "$WEIGHTS" \
            --source "$VIDEO_PATH" \
            --conf $CONF \
            --img $IMG_SIZE \
            --save-txt \
            --save-conf \
            --device 0 \
            --project runs/test_detect \
            --name "${name}_run${i}" \
            ${experiments[$name]}
    done
done