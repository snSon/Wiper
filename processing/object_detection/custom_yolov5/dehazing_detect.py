# 수정해야 함. 출력 결과.
import argparse
import os
import time
import cv2
import torch
import psutil
import numpy as np
from pathlib import Path
from models.common import DetectMultiBackend
from utils.general import non_max_suppression, scale_boxes
from utils.dataloaders import LoadImages, LoadStreams
from utils.torch_utils import select_device

# 리소스 측정 함수들
def get_ram_usage():
    return psutil.virtual_memory().percent

def get_gpu_usage():
    try:
        with os.popen('tegrastats --interval 1000 --count 1') as f:
            line = f.read()
            return line.strip()
    except Exception:
        return "tegrastats not available"

def get_incremented_dir(base_dir: str, name_prefix: str) -> Path:
    base = Path(base_dir)
    base.mkdir(parents=True, exist_ok=True)
    i = 1
    while (base / f"{name_prefix}_run{i}").exists():
        i += 1
    result_path = base / f"{name_prefix}_run{i}"
    (result_path / "labels").mkdir(parents=True, exist_ok=True)
    return result_path

# 객체 탐지 함수
def detect(opt):
    if isinstance(opt.imgsz, int):
        opt.imgsz = (opt.imgsz, opt.imgsz)

    device = select_device(opt.device)
    model = DetectMultiBackend(opt.weights, device=device)
    stride, names = model.stride, model.names

    if opt.source == "0":
        dataset = LoadStreams(opt.source, img_size=opt.imgsz, stride=stride)
    else:
        dataset = LoadImages(opt.source, img_size=opt.imgsz, stride=stride)

    save_dir = Path(opt.save_dir)
    save_dir = get_incremented_dir("runs/test_dehazing_detect", "trt_engine")
    label_dir = save_dir / "labels"
    vid_writer = None
    total_frames = 0
    total_time = 0

    for path, img, im0s, _, _ in dataset:
        img = torch.from_numpy(img).to(device)
        img = img.float() / 255.0
        if img.ndimension() == 3:
            img = img.unsqueeze(0)
        
        img = torch.nn.functional.interpolate(img, size=(640, 640), 
                                                mode='bilinear', align_corners=False)

        t1 = time.time()
        pred = model(img, augment=False)
        pred = non_max_suppression(pred, opt.conf_thres, opt.iou_thres)
        t2 = time.time()

        inference_time = t2 - t1
        total_time += inference_time
        total_frames += 1

        log_msg = f"[{total_frames:04d}] {inference_time * 1000:.2f}ms"
        if len(pred[0]):
            counts = {}
            for *_, conf, cls in pred[0]:
                cls_name = names[int(cls)]
                counts[cls_name] = counts.get(cls_name, 0) + 1
            log_msg += " | Detected: " + ", ".join([f"{k} x{v}" for k, v in counts.items()])
        else:
            log_msg += " | No detections"

        print(log_msg)

        # 로그 파일에도 기록
        with open(save_dir / "results_log.txt", "a") as log_f:
            log_f.write(log_msg + "\n")

        for i, det in enumerate(pred):
            im0 = im0s.copy() if isinstance(im0s, np.ndarray) else im0s[i].copy()
            im0 = cv2.resize(im0, (640, 640))

            label_path = label_dir / f"{Path(path).stem}_{total_frames}.txt"
            with open(label_path, "w") as lf:
                if len(det):
                    det[:, :4] = scale_boxes(img.shape[2:], det[:, :4], im0.shape).round()
                    for *xyxy, conf, cls in det:
                        label = f'{names[int(cls)]} {conf:.2f}'
                        cv2.rectangle(im0, (int(xyxy[0]), int(xyxy[1])), (int(xyxy[2]), int(xyxy[3])), (0,255,0), 2)
                        cv2.putText(im0, label, (int(xyxy[0]), int(xyxy[1])-10), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0,255,0), 2)

                        # Save label in YOLO format
                        xywh = (torch.tensor(xyxy).view(1, 4) / 640).view(-1).tolist()
                        lf.write(f"{int(cls)} {' '.join(f'{x:.6f}' for x in xywh)} {conf:.4f}\n")

            if opt.save_video:
                if vid_writer is None:
                    fourcc = cv2.VideoWriter_fourcc(*'mp4v')
                    h, w = im0.shape[:2]
                    vid_writer = cv2.VideoWriter(str(save_dir / 'result.mp4'), fourcc, 30, (w, h))
                vid_writer.write(im0)

    fps = total_frames / total_time if total_time > 0 else 0
    with open(save_dir / "results.txt", "w") as f:
        f.write(f"Frames: {total_frames}\n")
        f.write(f"Total Inference Time: {total_time:.2f} sec\n")
        f.write(f"FPS: {fps:.2f}\n")
        f.write(f"RAM Usage: {get_ram_usage()}%\n")
        f.write(f"GPU Usage: {get_gpu_usage()}\n")

    if vid_writer:
        vid_writer.release()

    print("Inference Done.")
    print(f"Results saved to: {save_dir}")


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--weights', type=str, default='models/yolov5s.engine', help='weights path')
    parser.add_argument('--source', type=str, default='0', help='input source (image/video/webcam)')
    parser.add_argument('--imgsz', type=int, default=640, help='image size')
    parser.add_argument('--conf-thres', type=float, default=0.25, help='confidence threshold')
    parser.add_argument('--iou-thres', type=float, default=0.45, help='NMS IoU threshold')
    parser.add_argument('--device', default='', help='device to run on, e.g. 0 or cpu')
    parser.add_argument('--save-dir', type=str, default='runs/test_dehazing_detect', help='base directory to save results')
    parser.add_argument('--save-video', action='store_true', help='save output video')
    opt = parser.parse_args()
    detect(opt)