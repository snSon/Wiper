#!/bin/bash

# [0] 현재 스크립트 경로 기준으로 custom_yolov5 경로 설정
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$SCRIPT_DIR"

# PYTHONPATH 설정
export PYTHONPATH="$ROOT_DIR:$PYTHONPATH"

# [1] 저장 경로 및 파일 이름 설정
WEIGHTS="$ROOT_DIR/models/yolov5s.pt"
ONNX_PATH="$ROOT_DIR/models/yolov5s.onnx"
ENGINE_PATH="$ROOT_DIR/models/yolov5s.engine"
LOG_PATH="$ROOT_DIR/log/trtexec.log"

# [2] 디렉토리 없으면 생성
mkdir -p "$ROOT_DIR"

# [3] export.py 실행하여 ONNX 변환
echo "Step 1: .pt → .onnx 변환 중..."
python3 "$ROOT_DIR/export.py" --weights "$WEIGHTS" --img 640 --batch 1 --include onnx

# 변환 결과 확인
if [ ! -f "$ONNX_PATH" ]; then
    echo "❌ ONNX 변환 실패: $ONNX_PATH 파일 없음"
    exit 1
fi
echo "ONNX 변환 완료: $ONNX_PATH"

# [4] 사용자 입력: 백그라운드 실행 여부
read -p "백그라운드에서 TensorRT 변환을 실행하시겠습니까? (Y/N): " run_bg
run_bg=$(echo "$run_bg" | tr '[:lower:]' '[:upper:]')  # 소문자 입력도 처리

# [5] TensorRT 변환
echo "Step 2: ONNX → TensorRT 엔진 변환 시작..."
if [ "$run_bg" = "Y" ]; then
    nohup /usr/src/tensorrt/bin/trtexec \
      --onnx="$ONNX_PATH" \
      --saveEngine="$ENGINE_PATH" \
      --fp16 \
      --verbose > "$LOG_PATH" 2>&1 &
    echo "로그는 $LOG_PATH 에 저장됩니다 (백그라운드 실행 중)"
    echo "엔진 생성 완료까지 수 분 ~ 30분 소요될 수 있습니다."
else
    /usr/src/tensorrt/bin/trtexec \
      --onnx="$ONNX_PATH" \
      --saveEngine="$ENGINE_PATH" \
      --fp16 \
      --verbose | tee "$LOG_PATH"
    echo "변환 완료. 로그는 $LOG_PATH 에 저장되었습니다."
fi

