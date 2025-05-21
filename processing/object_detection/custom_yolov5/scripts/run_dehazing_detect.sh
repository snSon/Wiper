#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(dirname "$SCRIPT_DIR")"
PYFILE="$ROOT_DIR/dehazing_detect.py"

SOURCE_TYPE=$1                   # 0 | video path | image dir path

# 🔍 소스 유형 설명
if [ -z "$SOURCE_TYPE" ]; then
    echo "❗ 실행 방법: ./scripts/run_detect.sh [source]"
    echo "   예시: ./scripts/run_detect.sh 0                  # 웹캠"
    echo "   예시: ./scripts/run_detect.sh videos/video.mp4  # 영상"
    echo "   예시: ./scripts/run_detect.sh hazing_frames/    # 이미지 폴더"
    exit 1
fi

# 기본 실행 설정
WEIGHTS="$ROOT_DIR/models/yolov5s.engine"  # 또는 yolov5s.pt
SAVE_DIR="$ROOT_DIR/runs/test_detect"
IMG_SIZE=640

echo "🚀 객체 인식 시작: 입력 소스 = $SOURCE_TYPE"
python3 "$PYFILE" \
    --weights "$WEIGHTS" \
    --source "$ROOT_DIR/$SOURCE_TYPE" \
    --imgsz "$IMG_SIZE" \
    --save-dir "$SAVE_DIR" \
    --save-video

echo "결과 영상 및 로그가 $SAVE_DIR 에 저장되었습니다."
