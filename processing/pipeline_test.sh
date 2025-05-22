#!/bin/bash

VIDEO="input/test_drive_30.mp4"
PT_MODEL="processing/object_detection/custom_yolov5/models/yolov5s.pt"
ENGINE_MODEL="processing/object_detection/custom_yolov5/models/yolov5s.engine"

echo "[INFO] 확장된 테스트 케이스 실행 시작"

# hazing, dehazing, ROI 등 처리 옵션 조합
declare -a options=(
    ""
    "--enable_haze"
    "--enable_aod"
    "--enable_haze --enable_aod"
    "--enable_aod --enable_roi"
    "--enable_aod --enable_roi --enable_blend"
    "--enable_haze --enable_aod --enable_roi"
    "--enable_haze --enable_aod --enable_roi --enable_blend"
)

# haze 파라미터 조합
declare -a haze_betas=(1.5 2.3 3.0)
declare -a haze_As=(0.7 0.8 0.9)
declare -a haze_layers=(50 100)

# 모델 루프 (PT, ENGINE)
for model in "$PT_MODEL" "$ENGINE_MODEL"; do
    suffix_base=$(basename "$model" | cut -d. -f1)  # yolov5s

    for i in "${!options[@]}"; do
        opt="${options[$i]}"

        for beta in "${haze_betas[@]}"; do
            for A in "${haze_As[@]}"; do
                for layers in "${haze_layers[@]}"; do

                    suffix="${suffix_base}_case${i}_b${beta}_A${A}_l${layers}"

                    echo "[RUN] $suffix : $opt"
                    python3 processing/hazing_yolo_pipeline.py \
                        --weights "$model" \
                        $opt \
                        --haze_beta "$beta" \
                        --haze_A "$A" \
                        --haze_layers "$layers" \
                        --output_suffix "$suffix"

                done
            done
        done
    done
done

echo "[✔] 모든 테스트 케이스 완료"
