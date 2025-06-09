#!/bin/bash

# [0] 현재 스크립트 기준 루트 경로 설정
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && cd .. && pwd)"
VIDEO_DIR="$SCRIPT_DIR/videos"
VIDEO_PATH1="$VIDEO_DIR/test_drive.mp4"
VIDEO_PATH2="$VIDEO_DIR/test_drive_640.mp4"
VIDEO_PATH3="$VIDEO_DIR/foggy_1st_padding.mp4"

# Google Drive 파일 ID
VIDEO_ID1="1iNMtI-X5bhbP7aOyfMDur5eOHAvCnaxM"
VIDEO_ID2="1y6Q7gwTLlk5Q1HYjnfkdaEGrVaPTkwdY"
VIDEO_ID3="17c2b9AVaR7ARy6ZTV6xw3vrU9vjFqos9"

# gdown 설치 확인 및 설치 시도
if ! command -v gdown &> /dev/null; then
    echo "gdown이 설치되지 않았습니다. 설치 중..."
    if command -v pip3 &> /dev/null; then
        pip3 install gdown
    elif command -v pip &> /dev/null; then
        pip install gdown
    else
        echo "pip3 또는 pip가 시스템에 설치되어 있지 않습니다. 수동으로 설치해 주세요."
        exit 1
    fi
fi

# 영상 디렉토리 없으면 생성
mkdir -p "$VIDEO_DIR"

# 각 영상 파일이 없을 경우에만 다운로드
if [ ! -f "$VIDEO_PATH1" ]; then
    echo "test_drive.mp4 다운로드 중..."
    gdown https://drive.google.com/uc?id=$VIDEO_ID1 -O "$VIDEO_PATH1"
else
    echo "✅ 이미 존재: $VIDEO_PATH1"
fi

if [ ! -f "$VIDEO_PATH2" ]; then
    echo "test_drive_640.mp4 다운로드 중..."
    gdown https://drive.google.com/uc?id=$VIDEO_ID2 -O "$VIDEO_PATH2"
else
    echo "✅ 이미 존재: $VIDEO_PATH2"
fi

if [ ! -f "$VIDEO_PATH3" ]; then
    echo "foggy_1st_padding.mp4 다운로드 중..."
    gdown https://drive.google.com/uc?id=$VIDEO_ID3 -O "$VIDEO_PATH3"
else
    echo "✅ 이미 존재: $VIDEO_PATH3"
fi

echo "모든 리소스가 준비되었습니다."