#!/bin/bash

echo "📦 기존 opencv-python 제거 중..."
pip uninstall -y opencv-python opencv-contrib-python

echo "✅ headless 버전 설치 중..."
pip install opencv-python-headless

echo "🎉 설치 완료! OpenCV를 headless 버전으로 교체했습니다."
