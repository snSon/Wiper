#!/bin/bash

# Jetson GPU 성능 최대로 설정
# nvpmodel -m 0
# jetson_clocks

set -e # error: 스크립트 실행 중 오류 발생 시 즉시 종료

# setting path and config =========================================================
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$SCRIPT_DIR/.."

VAL_PY="$ROOT_DIR/val.py"
DATA_YAML="$ROOT_DIR/data/coco128.yaml" # kitti
PT_WEIGHTS="$ROOT_DIR/models/yolov5s.pt"  # PyTorch
TRT_WEIGHTS="$ROOT_DIR/models/yolov5s.engine" # TensorRT

IMG=640
DEVICE=0
WORKERS=0
REPEAT=1  # 원하는 반복 횟수로 수정 가능
BATCH=1

# 실험 조합 정의
declare -A experiments=(
  [pt]="$PT_WEIGHTS"
  [pt_dehaze]="$PT_WEIGHTS"
  [trt_engine]="$TRT_WEIGHTS"
  [trt_engine_dehaze]="$TRT_WEIGHTS"
)

# exec test ======================================================================

for key in "${!experiments[@]}"; do
  WEIGHT_PATH="${experiments[$key]}"
  DEHAZE_FLAG=""

  # 이름에 _dehaze 가 붙어 있으면 플래그 추가
  [[ $key == *_dehaze ]] && DEHAZE_FLAG="--dehaze"

  for ((i=1; i<=REPEAT; i++)); do
    echo "[$key] run $i / $REPEAT  (dehaze=${DEHAZE_FLAG:+on})"

    python3 "$VAL_PY" \
      --weights "$WEIGHT_PATH" \
      --data "$DATA_YAML" \
      --imgsz "$IMG" \
      --task val \
      --device "$DEVICE" \
      --workers "$WORKERS" \
      --verbose \
      --half \
      --batch-size "$BATCH" \
      $DEHAZE_FLAG

    echo "[$key] run $i 완료"
    echo
  done
done

# ### 
# #      옵션	        설명
# # --weights	사용할 모델 가중치
# # --workers 0 # map 등에 대한 성능에는 문제 x <- 임시 해결 방법
# # --data	평가용 데이터셋 YAML
# # --img	입력 이미지 크기
# # --task test	검증만 수행
# # --half	FP16으로 빠르게 (TensorRT처럼 mixed precision)
# # --device 0	GPU 사용 (CUDA 장치)
# # --verbose	클래스별 정밀도/재현율 출력
# ###