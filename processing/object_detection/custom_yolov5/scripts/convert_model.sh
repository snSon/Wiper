#!/bin/bash

# [0] 현재 스크립트 경로 기준으로 custom_yolov5 경로 설정
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$SCRIPT_DIR/.."

# PYTHONPATH 설정
export PYTHONPATH="$ROOT_DIR:$PYTHONPATH"

# [1] 저장 경로 및 파일 이름 설정
WEIGHTS_PT="$ROOT_DIR/models/yolov5s.pt"
ENGINE_PATH="$ROOT_DIR/models/yolov5s.engine"
LOG_PATH="$ROOT_DIR/log/export.log"

# [2] 디렉토리 없으면 생성
mkdir -p "$ROOT_DIR/log"

# [3] export.py 실행하여 .engine 변환 (onnx 포함)
echo "Step 1: .pt → .engine 변환 중 ..."
	python3 "$ROOT_DIR/export.py" \
	  --weights "$WEIGHTS_PT" \
	  --imgsz 640 \
	  --batch 1 \
	  --device 0 \
	  --half \
	  --workspace 4 \
	  --include onnx engine | tee "$LOG_PATH"

  # [4] 결과 확인
if [ ! -f "$ENGINE_PATH" ]; then
    echo "TensorRT 엔진 변환 실패: $ENGINE_PATH 파일 없음"
    exit 1
fi

echo "✅ TensorRT 엔진 변환 완료: $ENGINE_PATH"
echo "로그: $LOG_PATH"