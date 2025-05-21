#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LOG_FILE="$SCRIPT_DIR/../log/jetson_env.log"

get_jetpack_version() {
  case "$1" in
    32.7.1) echo "JetPack 4.6.1" ;;
    35.1.0) echo "JetPack 5.0.1" ;;
    35.2.1) echo "JetPack 5.1" ;;
    35.3.1) echo "JetPack 5.1.1" ;;
    35.6.0) echo "JetPack 5.1.2" ;;
    *) echo "Unknown (L4T $1)" ;;
  esac
}

L4T_VERSION=$(dpkg -l | grep nvidia-l4t-core | awk '{print $3}' | cut -d '-' -f1)
JETPACK_VERSION=$(get_jetpack_version "$L4T_VERSION")

echo "[OS & JetPack Info]" > $LOG_FILE
echo "JetPack Version   : $JETPACK_VERSION" >> $LOG_FILE
echo "OS Version        : $(head -n 1 /etc/nv_tegra_release)" >> $LOG_FILE
echo "Ubuntu Version    : $(lsb_release -ds)" >> $LOG_FILE
echo "L4T Version       : $L4T_VERSION" >> $LOG_FILE
echo "" >> $LOG_FILE

echo "[Python & PyTorch]" >> $LOG_FILE
echo "Python Version    : $(python3 --version | awk '{print $2}')" >> $LOG_FILE
echo "PyTorch Version   : $(python3 -c "import torch; print(torch.__version__)")" >> $LOG_FILE
echo "TorchVision Version: $(python3 -c "import torchvision; print(torchvision.__version__)")" >> $LOG_FILE

echo "" >> $LOG_FILE

echo "[CUDA & cuDNN]" >> $LOG_FILE
echo "CUDA Version      : $(nvcc --version | grep release | sed 's/.*release //' | awk '{print $1}')" >> $LOG_FILE
CUDNN_MAJOR=$(grep "#define CUDNN_MAJOR" /usr/include/cudnn_version.h | awk '{print $3}')
CUDNN_MINOR=$(grep "#define CUDNN_MINOR" /usr/include/cudnn_version.h | awk '{print $3}')
CUDNN_PATCH=$(grep "#define CUDNN_PATCHLEVEL" /usr/include/cudnn_version.h | awk '{print $3}')
echo "cuDNN Version     : ${CUDNN_MAJOR}.${CUDNN_MINOR}.${CUDNN_PATCH}" >> $LOG_FILE
echo "" >> $LOG_FILE

echo "[OpenCV]" >> $LOG_FILE
echo "OpenCV Version    : $(python3 -c "import cv2; print(cv2.__version__)")" >> $LOG_FILE
echo "" >> $LOG_FILE

echo "[TensorRT]" >> $LOG_FILE
echo "TensorRT Version  : $(dpkg -l | grep tensorrt | head -n 1 | awk '{print $3}')" >> $LOG_FILE
echo "TensorRT Available: $(python3 -c "import torch; print(torch.backends.cudnn.enabled)")" >> $LOG_FILE
echo "" >> $LOG_FILE

echo "[GPU Status Snapshot]" >> $LOG_FILE
tegrastats --interval 1000 --logfile _tmp_gpu.log & sleep 3 && kill $!
head -n 1 _tmp_gpu.log >> $LOG_FILE
rm _tmp_gpu.log

echo "Environment check complete. Results saved to '$LOG_FILE'."