# Custom YOLOv5 Object Detection

This repository is a lightweight and Jetson-optimized version of [YOLOv5](https://github.com/ultralytics/yolov5), specialized for object detection tasks in edge AI environments.

## 🔧 Main Features
- `detect.py`: Run object detection (**modified**)
- `val.py`: Perform model evaluation (**modified**)
- `export.py`: Export models to other formats such as ONNX (**original**)
- `convert_model.sh`: Automatically convert `.pt` → `.onnx` → TensorRT engine
- `download_resources.sh`: Automatically download external resources (e.g., pretrained weights and test video)

## Environment Setup

```bash
pip install -r requirements.txt

## how to Run 

### 1. Download pretrained weights and test video

```bash
./custom_yolov5/scripts/download_resources.sh
```

### 2. Convert .pt → .onnx → .engine (TensorRT)

```bash
./custom_yolov5/scripts/convert_model.sh
```

### 3. Run inference

```bash
./custom_yolov5/scripts/test_detect.sh
```

### 4. Evaluate the model (mAP, precision, recall)

```bash
./custom_yolov5/scripts/test_val.sh
```