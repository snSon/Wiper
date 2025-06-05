#!/bin/bash

# [0] 현재 스크립트 경로 기준으로 경로 설정
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ONNX_PATH="$SCRIPT_DIR/JetDehaze.onnx"
ENGINE_PATH="$SCRIPT_DIR/JetDehaze.engine"
LOG_PATH="$SCRIPT_DIR/trt_conversion.log"

# [1] 사용자 입력: 백그라운드 실행 여부
read -p "백그라운드에서 TensorRT 변환을 실행하시겠습니까? (Y/N): " run_bg
run_bg=$(echo "$run_bg" | tr '[:lower:]' '[:upper:]')  # 소문자 입력도 처리

# [2] TensorRT 변환
echo "Step 1: ONNX → TensorRT 엔진 변환 시작..."
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

