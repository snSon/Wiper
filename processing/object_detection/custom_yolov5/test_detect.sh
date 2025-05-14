#!/bin/bash

# Jetson GPU 성능 최대로 설정
# nvpmodel -m 0
# jetson_clocks

VIDEO_PATH="/home/wiper/jiwan/videos/test_drive_30.mp4"
IMG_SIZE=640
CONF=0.25
REPEAT=5  # 반복 횟수 변수 설정

# 실험 조합 정의 
declare -A experiments
# experiments["pt_fp32"]=""           # PyTorch FP32
# experiments["pt_half"]="--half"     # PyTorch FP16
experiments["trt_engine"]=""        # TensorRT 엔진

for name in "${!experiments[@]}"; do
    for ((i = 1; i <= REPEAT; i++)); do
        echo "🔁 [$name] 실험 $i 번째 실행 중..."

        if [[ "$name" == "trt_engine" ]]; then
            WEIGHTS="yolov5s.engine"
        else
            WEIGHTS="yolov5s.pt"
        fi

        python test_detect_summary.py \
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