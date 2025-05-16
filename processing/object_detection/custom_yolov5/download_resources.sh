#!/bin/bash

# [0] 현재 스크립트 기준 루트 경로 설정
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VIDEO_DIR="$SCRIPT_DIR/videos"
MODEL_PATH="$SCRIPT_DIR/yolov5s.pt"
VIDEO_PATH="$VIDEO_DIR/test_drive_30.mp4"

# Google Drive 파일 ID
VIDEO_ID="1iNMtI-X5bhbP7aOyfMDur5eOHAvCnaxM"

# gdown 설치 확인
if ! command -v gdown &> /dev/null
then
    echo "⚠️  gdown이 설치되지 않았습니다. 설치 중..."
    pip install gdown
fi

# 영상 디렉토리 없으면 생성
mkdir -p "$VIDEO_DIR"

# YOLOv5s 모델 다운로드 (PyTorch Hub 사용)
if [ ! -f "$MODEL_PATH" ]; then
    echo "✅ yolov5s.pt 모델을 PyTorch Hub에서 다운로드합니다..."
    python3 - <<EOF
import torch
model = torch.hub.load('ultralytics/yolov5', 'yolov5s', trust_repo=True)
torch.save(model.state_dict(), "$MODEL_PATH")
EOF
else
    echo " 모델 파일 이미 존재: $MODEL_PATH"
fi

# 영상 파일 다운로드
if [ ! -f "$VIDEO_PATH" ]; then
    echo " 영상 파일 다운로드 중..."
    gdown https://drive.google.com/uc?id=$VIDEO_ID -O $VIDEO_PATH
else
    echo " 영상 파일 이미 존재: $VIDEO_PATH"
fi

echo "모든 리소스가 준비되었습니다."