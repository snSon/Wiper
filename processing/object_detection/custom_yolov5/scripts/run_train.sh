#!/bin/bash
# train_kitti.sh
# KITTI 데이터셋으로 YOLOv5 파인튜닝 실행 스크립트
# Jetson Orin Nano 8GB 환경에 최적화

set -e

# 스크립트 위치를 기준으로 프로젝트 루트 자동 설정
# $(dirname "$0")는 이 스크립트가 있는 폴더(custom_yolov5/scripts)
# 상위 폴더로 올라가 custom_yolov5 디렉토리를 ROOT_DIR로 지정
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

DATA_YAML="$ROOT_DIR/data/kitti.yaml"
CFG_YAML="$ROOT_DIR/models/yolov5s_kitti.yaml"
WEIGHTS="$ROOT_DIR/models/yolov5s_origin.pt"

# 학습 설정
IMG_SIZE=640
BATCH_SIZE=4        # VRAM 8GB 대비 적정 값
EPOCHS=50
DEVICE=0
CACHE="--cache"

cd "$ROOT_DIR"

echo "Starting fine-tuning on KITTI dataset..."
python3 train.py \
  --data "$DATA_YAML" \
  --cfg "$CFG_YAML" \
  --weights "$WEIGHTS" \
  --img "$IMG_SIZE" \
  --batch-size "$BATCH_SIZE" \
  --epochs "$EPOCHS" \
  --device "$DEVICE" \
  $CACHE

echo "Training completed. Best model saved at runs/train/exp/weights/best.pt"
