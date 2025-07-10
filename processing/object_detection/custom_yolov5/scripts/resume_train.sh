#!/bin/bash
# resume_train.sh

# $(dirname "$0")는 이 스크립트가 있는 폴더(custom_yolov5/scripts)
# 상위 폴더로 올라가 custom_yolov5 디렉토리를 ROOT_DIR로 지정
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
set -e

cd "$ROOT_DIR"

DATA_YAML="$ROOT_DIR/data/kitti.yaml"
CFG_YAML="$ROOT_DIR/models/yolov5s_kitti.yaml"
WEIGHTS="$ROOT_DIR/runs/train/exp3/weights/last.pt"

echo "현재 디렉토리: $(pwd)"

# 학습 설정
IMG_SIZE=640
BATCH_SIZE=4        # VRAM 8GB 대비 적정 값
EPOCHS=50
DEVICE=0
CACHE="--cache"

# 1) runs/train/exp3 폴더 안의 last.pt 로드 
# 2) 나머지 하이퍼파라미터 그대로 이어감
nohup python3 train.py \
  --data "$DATA_YAML" \
  --cfg "$CFG_YAML" \
  --weights "$WEIGHTS" \
  --img "$IMG_SIZE" \
  --batch-size "$BATCH_SIZE" \
  --epochs "$EPOCHS" \
  --device "$DEVICE" \
  $CACHE \
  --resume \
  > resume.log 2>&1 &
echo "Resume training in background (pid: $!)"
