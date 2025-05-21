# Custom YOLOv5 Object Detection

This repository is a lightweight and Jetson-optimized version of [YOLOv5](https://github.com/ultralytics/yolov5), specialized for object detection tasks in edge AI environments.

## Main Features
- `detect.py`: Run object detection using specified weights and input sources (**modified**)  
  → Inference speed logging to `results.txt` has been added.

- `detect_runner.py`: Experiment driver that wraps `detect.py` to run inference, monitor GPU/RAM usage (via `tegrastats`), and generate performance logs  
  → Measures FPS, inference time, GPU/RAM usage, and organizes results per run.

- `scripts/test_detect.sh`: Shell script to automate multiple detection experiments using `detect_runner.py`  
  → Easily repeat experiments and compare configurations like `.pt`, `.engine`, FP16, etc.

- `scripts/test_val.sh`: Shell script to batch-run `val.py` with various settings  
  → Useful for validation benchmarking.

- `scripts/convert_model.sh`: Automatically converts `.pt` → `.onnx` → `.engine` with one command.

- `scripts/download_resources.sh`: Download pretrained weights and test video

- `scripts/jetson_env.sh`: Prints Jetson hardware and software stack info including JetPack, PyTorch, CUDA, TensorRT, and OpenCV.

- Modular directory structure (e.g., `models/`, `scripts/`, `metadata/`)

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
├── scripts/                  # Utility shell scripts for setup, conversion, and evaluation
│   ├── convert_model.sh
│   ├── download_resources.sh
│   ├── jetson_env.sh
│   ├── test_detect.sh
│   └── test_val.sh
├── utils/                   # Python utility modules (if used)
├── detect.py                # Modified detection script
├── detect_runner.py         # Runs detect.py and logs performance metrics
├── export.py                # Model export script (to ONNX, etc.)
├── hubconf.py               # PyTorch Hub interface
├── LICENSE                  # MIT License file
├── pyproject.toml           # Project configuration metadata
├── README.md                # Project documentation
├── requirements.txt         # Python dependencies
├── val.py                   # Modified validation script
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
./scripts/test_detect.sh
```

### 4. Evaluate the model (mAP, precision, recall)

```bash
./scripts/test_val.sh
```