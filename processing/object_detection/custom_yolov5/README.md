# Custom YOLOv5 Object Detection

This repository is a lightweight and Jetson-optimized version of [YOLOv5](https://github.com/ultralytics/yolov5), specialized for object detection tasks in edge AI environments.

## Main Features
- `detect.py`: Run object detection (**modified**)
- `val.py`: Perform model evaluation (**modified**)
- `export.py`: Export models to other formats such as ONNX (**original**)
- `scripts/convert_model.sh`: Automatically convert `.pt` → `.onnx` → TensorRT engine
- `scripts/download_resources.sh`: Automatically download external resources (e.g., pretrained weights and test video)
- `scripts/jetson_env.sh`: Check Jetson environment status and dependencies
- Modular directory structure (e.g., `log/`, `models/`, `scripts/`, `metadata/`)

## Directory Overview
The repository is organized as follows:
<pre>
project-root/
├── data/                     # Dataset or annotation files (if applicable)
├── metadata/                 # Original open-source license and citation files
│   ├── CITATION.cff
│   └── CONTRIBUTING.md
├── models/                   # Trained models (.pt, .onnx, .engine)
├── runs/                     # Inference and evaluation results
├── scripts/                  # Utility shell scripts
│   ├── convert_model.sh
│   ├── download_resources.sh
│   └── jetson_env.sh
├── utils/                    # Utility Python modules
├── videos/                   # Input videos for testing
├── detect.py
├── export.py
├── hubconf.py
├── LICENSE
├── pyproject.toml
├── README.md
├── requirements.txt
├── test_detect_summary.py
├── test_detect.sh
├── test_val.sh
└── val.py
</pre>

## Metadata and Licensing

This project is based on [Ultralytics YOLOv5](https://github.com/ultralytics/yolov5) and follows the original MIT License.

The following open-source related documents are preserved and relocated to the `metadata/` directory:

- `metadata/CITATION.cff`: Citation guide for academic referencing  
- `metadata/CONTRIBUTING.md`: Contribution guidelines from the original repository

The license file (`LICENSE`) remains in the root directory.

> All modifications in this repository are under the terms of the original MIT License.  
> The original copyright and license information must be retained.

## Environment Setup

```bash
pip install -r requirements.txt
```
## How to Run? 

### 1. Download pretrained weights and test video

```bash
./scripts/download_resources.sh
```

### 2. Convert .pt → .onnx → .engine (TensorRT)

```bash
./scripts/convert_model.sh
```

### 3. Run inference

```bash
./test_detect.sh
```

### 4. Evaluate the model (mAP, precision, recall)

```bash
./test_val.sh
```