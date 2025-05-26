import os
import cv2
import torch
import numpy as np
import torch.nn.functional as F
import time
import psutil
import GPUtil

from processing.dehazing.JetDehaze.model import JetDehazeNet
from processing.dehazing.MSBDN.MSBDN import Net as MSBDN
from processing.dehazing.FFA.ffa_net import FFA
from processing.dehazing.AOD.net import dehaze_net as DehazeNet

# Device setting
device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

# Model constructors
models = {
    "JetDehazeNet": lambda: JetDehazeNet(),
    "MSBDN": lambda: MSBDN(),
    "FFA": lambda: FFA(3, 19),
    "DehazeNet": lambda: DehazeNet(),
}

# Corresponding weight paths
model_weights = {
    "JetDehazeNet": "JetDehaze/JetDehaze.pth",
    "MSBDN": "MSBDN/MSBDN.pth",
    "FFA": "FFA/FFA_NET.pth",
    "DehazeNet": "AOD/AOD_NET.pth",
}

input_dir = "test_images"

def pad_to_32(tensor):
    _, _, h, w = tensor.shape
    pad_h = (32 - h % 32) % 32
    pad_w = (32 - w % 32) % 32
    tensor = F.pad(tensor, (0, pad_w, 0, pad_h), mode='reflect')
    return tensor, pad_h, pad_w

def get_gpu_usage():
    if torch.cuda.is_available():
        gpus = GPUtil.getGPUs()
        return gpus[0].memoryUsed, gpus[0].memoryUtil * 100  # MB, %
    return 0, 0

def get_cpu_memory():
    return psutil.Process(os.getpid()).memory_info().rss / (1024 ** 2)  # MB

image_files = [
    fname for fname in os.listdir(input_dir)
    if fname.lower().endswith((".jpg", ".jpeg", ".png", ".bmp"))
]

# Store final summary
summary = {}

for model_name, model_fn in models.items():
    print(f"\n[INFO] Starting test for model: {model_name}")
    output_dir = f"results_{model_name}"
    os.makedirs(output_dir, exist_ok=True)

    model = model_fn().to(device)
    weight_path = model_weights[model_name]

    if not os.path.exists(weight_path):
        print(f"[WARN] Weight file not found: {weight_path}")
        continue

    try:
        model.load_state_dict(torch.load(weight_path, map_location=device))
        print(f"[INFO] Loaded weights for {model_name}")
    except Exception as e:
        print(f"[ERROR] Failed to load weights: {e}")
        continue

    model.eval()

    total_time = 0
    total_cpu_mem = 0
    total_gpu_mem = 0
    total_gpu_util = 0

    for idx, fname in enumerate(image_files):
        img_path = os.path.join(input_dir, fname)
        img = cv2.imread(img_path)
        if img is None:
            print(f"[WARN] Failed to load image: {fname}")
            continue

        img_rgb = img[:, :, ::-1].astype(np.float32) / 255.0
        img_tensor = torch.from_numpy(img_rgb).permute(2, 0, 1).unsqueeze(0).to(device)
        img_tensor, pad_h, pad_w = pad_to_32(img_tensor)

        start = time.time()
        with torch.no_grad():
            output = model(img_tensor)
        elapsed = time.time() - start

        if pad_h > 0 or pad_w > 0:
            output = output[:, :, :output.shape[2] - pad_h, :output.shape[3] - pad_w]

        output_img = output.squeeze().cpu().numpy().transpose(1, 2, 0)
        output_img = (output_img * 255).clip(0, 255).astype(np.uint8)
        output_img_bgr = output_img[:, :, ::-1]

        out_path = os.path.join(output_dir, fname)
        cv2.imwrite(out_path, output_img_bgr)

        cpu_mem = get_cpu_memory()
        gpu_mem, gpu_util = get_gpu_usage()

        total_time += elapsed
        total_cpu_mem += cpu_mem
        total_gpu_mem += gpu_mem
        total_gpu_util += gpu_util

        print(f"[INFO] {fname}: {elapsed:.3f}s | CPU Mem: {cpu_mem:.1f}MB | GPU Mem: {gpu_mem:.1f}MB | GPU Util: {gpu_util:.1f}%")

    num_imgs = len(image_files)
    summary[model_name] = {
        "Total Time (s)": total_time,
        "Avg Time (s)": total_time / num_imgs if num_imgs else 0,
        "Avg CPU Mem (MB)": total_cpu_mem / num_imgs if num_imgs else 0,
        "Avg GPU Mem (MB)": total_gpu_mem / num_imgs if num_imgs else 0,
        "Avg GPU Util (%)": total_gpu_util / num_imgs if num_imgs else 0,
    }

# Print final summary
print("\n=== SUMMARY ===")
for model_name, stats in summary.items():
    print(f"\n{model_name}")
    for k, v in stats.items():
        print(f"{k}: {v:.2f}")
