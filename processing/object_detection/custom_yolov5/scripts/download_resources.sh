#!/bin/bash

set -e

# [0] 현재 스크립트 기준 루트 경로 설정
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && cd .. && pwd)"

# [1-1] 영상 파일 경로 설정
VIDEO_DIR="$SCRIPT_DIR/videos"
VIDEO_PATH1="$VIDEO_DIR/test_drive.mp4"
VIDEO_PATH2="$VIDEO_DIR/test_drive_640.mp4"
VIDEO_PATH3="$VIDEO_DIR/foggy_1st_padding.mp4"

# Google Drive 파일 ID
VIDEO_ID1="1oYrDZfQ7Mdgf944HwEZmz9FKRULrf_A9"
VIDEO_ID2="1vQOe5KEnM9IDuxf_Y8KUPNSOtu9BLTzV"
VIDEO_ID3="1zlwLbKzsWDUqry4bDz9spMxrolg38Y07"

mkdir -p $SCRIPT_DIR/../datasets

# [1-2] 데이터셋 디렉토리 및 경로 설정
DATASET_DIR="$(cd "$SCRIPT_DIR/../datasets" && pwd)"
DATASET_PATH1="$DATASET_DIR/foggy_driving"
DATASET_PATH2="$DATASET_DIR/rtts"
DATASET_PATH3="$DATASET_DIR/foggy_coco128"

# Google Drive 파일 ID
DATASET_ID1="1rckXGzzNfy09laXHOk2tf1u0NmbQiutk"
DATASET_ID2="1zRmBUI9iGz81dY9T6L5MfYfYepLqW9JW"
DATASET_ID3="1byTXhk0-khGOx6S3Ay5hNPIQ3N3Seis6"

foggy_driving_zip="$DATASET_DIR/foggy_driving.zip"
rtts_zip="$DATASET_DIR/rtts.zip"
foggy_coco128_zip="$DATASET_DIR/foggy_coco128.zip"

# [1-3] 데이터셋 yaml 파일 경로 설정
DATASET_YAML="$SCRIPT_DIR/data"

DATASET_YAML_ID1="1pW3SmiVgB1NorC7OXFS9J2Zcpz_4_Jko"
DATASET_YAML_ID2="1PTga5VDoXJZjlr793N4qhsSDtl7Y0A_b"

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

# unzip 설치 확인
if ! command -v unzip &> /dev/null; then
    echo "unzip이 설치되지 않았습니다. 설치 중..."
    sudo apt update && sudo apt install unzip -y
fi

# 영상/데이터셋 디렉터리 없으면 생성
mkdir -p "$VIDEO_DIR"
mkdir -p "$DATASET_YAML"

# [2] 영상 다운로드
for i in 1 2 3; do
    VIDEO_PATH_VAR="VIDEO_PATH$i"
    VIDEO_ID_VAR="VIDEO_ID$i"
    if [ ! -f "${!VIDEO_PATH_VAR}" ]; then
        echo "$(basename "${!VIDEO_PATH_VAR}") 다운로드 중..."
        gdown "https://drive.google.com/uc?id=${!VIDEO_ID_VAR}" -O "${!VIDEO_PATH_VAR}"
    else
        echo "✅ 이미 존재: ${!VIDEO_PATH_VAR}"
    fi
done

# [3] foggy_driving.zip 다운로드 및 압축 해제
if [ ! -d "$DATASET_PATH1" ]; then
    if [ ! -f "$foggy_driving_zip" ]; then
        echo "foggy_driving.zip 다운로드 중..."
        gdown "https://drive.google.com/uc?id=$DATASET_ID1" -O "$foggy_driving_zip"
    fi
    echo "압축 해제 중 (foggy_driving)..."
    unzip "$foggy_driving_zip" -d "$DATASET_DIR"
else
    echo "✅ 이미 압축 해제됨: $DATASET_PATH1"
fi

# [4] rtts.zip 다운로드 및 압축 해제
if [ ! -d "$DATASET_PATH2" ]; then
    if [ ! -f "$rtts_zip" ]; then
        echo "rtts.zip 다운로드 중..."
        gdown --fuzzy "https://drive.google.com/uc?id=$DATASET_ID2" -O "$rtts_zip"
    fi
    echo "압축 해제 중 (rtts)..."
    unzip "$rtts_zip" -d "$DATASET_DIR"
else
    echo "✅ 이미 압축 해제됨: $DATASET_PATH2"
fi

# [5] foggy_coco128.zip 다운로드 및 압축 해제
if [ ! -d "$DATASET_PATH3" ]; then
    if [ ! -f "$foggy_coco128_zip" ]; then
        echo "foggy_coco128.zip 다운로드 중..."
        gdown --fuzzy "https://drive.google.com/uc?id=$DATASET_ID3" -O "$foggy_coco128_zip"
    fi
    echo "압축 해제 중 (rtts)..."
    unzip "$foggy_coco128_zip" -d "$DATASET_DIR"
else
    echo "✅ 이미 압축 해제됨: $DATASET_PATH3"
fi

# [6] 데이터셋 yaml 파일 다운로드
declare -a YAML_NAMES=("foggy_driving.yaml" "rtts.yaml")
declare -a YAML_IDS=("$DATASET_YAML_ID1" "$DATASET_YAML_ID2")

for i in 0 1; do
    FILE_NAME="${YAML_NAMES[$i]}"
    FILE_ID="${YAML_IDS[$i]}"
    FILE_PATH="$DATASET_YAML/$FILE_NAME"

    if [ ! -f "$FILE_PATH" ]; then
        echo "$FILE_NAME 다운로드 중..."
        gdown --fuzzy "https://drive.google.com/uc?id=$FILE_ID" -O "$FILE_PATH"
    else
        echo "✅ 이미 존재: $FILE_PATH"
    fi
done

rm "$foggy_driving_zip"
rm "$rtts_zip"
rm "$foggy_coco128_zip"

echo "모든 리소스가 준비되었습니다."
