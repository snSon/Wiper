import argparse
import subprocess, time, re, statistics, os, threading
import pandas as pd
import sys

# [0] 경로 설정
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))  
ROOT_DIR = SCRIPT_DIR 
DETECT_PY = os.path.join(ROOT_DIR, "detect.py")
BASE_DIR = os.path.join(ROOT_DIR, "runs", "test_detect")

# [1] PYTHONPATH 설정 (detect.py에서 models, utils 참조 가능하도록)
sys.path.insert(0, ROOT_DIR)

# Argument 파싱
parser = argparse.ArgumentParser()
parser.add_argument('--name', required=True)
parser.add_argument('--source', required=True)
parser.add_argument('--weights', required=True)
args, unknown = parser.parse_known_args()
exp_name = args.name

weights_path = os.path.abspath(args.weights)
source_path = os.path.abspath(args.source)
data_path = os.path.abspath(os.path.join(ROOT_DIR, "data", "coco128.yaml"))

# GPU & RAM 모니터링
gpu_avg = ram_avg = gpu_max = ram_max = "N/A"
monitoring = True

def monitor_resources(interval=0.2):
    global gpu_avg, ram_avg, gpu_max, ram_max, monitoring

    gpu_usage = []
    ram_usage = []

    proc = subprocess.Popen(['tegrastats'], stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)

    try:
        while monitoring:
            line = proc.stdout.readline().decode('utf-8').strip()
            gpu_match = re.search(r'GR3D_FREQ\s+(\d+)%', line) or re.search(r'GPU\s+(\d+)%', line)
            if gpu_match:
                gpu_usage.append(int(gpu_match.group(1)))
            ram_match = re.search(r'RAM\s+(\d+)/\d+MB', line)
            if ram_match:
                ram_usage.append(int(ram_match.group(1)))
            time.sleep(interval)
    except Exception as e:
        print("[ERROR] monitor_resources:", e)
    finally:
        proc.kill()

    if gpu_usage:
        gpu_avg = round(statistics.mean(gpu_usage), 2)
        gpu_max = max(gpu_usage)
    if ram_usage:
        ram_avg = round(statistics.mean(ram_usage), 2)
        ram_max = max(ram_usage)

# detect.py 실행 명령어
cmd = [
    "python3", DETECT_PY,
    "--weights",  weights_path,
    "--source", source_path,
    "--project", BASE_DIR,
    "--name", exp_name,
    "--data", data_path,  # 이 줄 추가
    "--save-txt",
    "--save-conf",
    "--device", "0"
] + unknown

print(f"[INFO] Running: {' '.join(cmd)}")

# 디렉토리 미리 생성
os.makedirs(BASE_DIR, exist_ok=True)

# 1️⃣ 리소스 측정 시작
monitor_thread = threading.Thread(target=monitor_resources)
monitor_thread.start()

# 2️⃣ detect.py 실행
proc = subprocess.Popen(cmd, cwd=ROOT_DIR)
proc.wait()

# 3️⃣ 리소스 측정 종료
monitoring = False
monitor_thread.join()

# 🔍 생성된 exp 디렉토리 확인
matched_dirs = sorted(
    [d for d in os.listdir(BASE_DIR) if d.startswith(exp_name)],
    key=lambda d: os.path.getmtime(os.path.join(BASE_DIR, d))
)
if matched_dirs:
    exp_dir = os.path.join(BASE_DIR, matched_dirs[-1])
    print(f"[INFO] 사용된 디렉토리: {exp_dir}")
else:
    print(f"[ERROR] '{exp_name}'로 시작하는 디렉토리를 찾지 못함.")
    exit(1)

# 추론 시간 파싱
log_file = os.path.join(exp_dir, "results.txt")
times = []
if os.path.exists(log_file):
    with open(log_file, 'r') as f:
        for line in f:
            match = re.search(r"inference:\s*(\d+\.\d+)s", line)
            if match:
                times.append(float(match.group(1)))

avg_time = statistics.mean(times) if times else 0
fps = 1 / avg_time if avg_time else 0

# 리소스 로그 저장
log_path = os.path.join(exp_dir, "resource_log.txt")
with open(log_path, "w") as log_file:
    log_file.write("🔍 GPU 사용률 로그:\n")
    log_file.write(f"gpu_avg = {gpu_avg}, gpu_max = {gpu_max}\n\n")
    log_file.write("💾 RAM 사용량 로그:\n")
    log_file.write(f"ram_avg = {ram_avg}, ram_max = {ram_max}\n")

# 누적 로그 저장
log_txt = os.path.join(BASE_DIR, "detect_log.txt")
with open(log_txt, "a") as f:
    f.write(f"\n--- Experiment: {exp_name} ---\n")
    f.write(f"FPS:                   {round(fps, 2)}\n")
    f.write(f"Inference Time(ms):   {avg_time * 1000:.2f}\n")
    f.write(f"GPU 사용률 평균(%):     {gpu_avg}\n")
    f.write(f"GPU 최대 사용률(%):     {gpu_max}\n")
    f.write(f"RAM 평균 사용량(MB):    {ram_avg}\n")
    f.write(f"RAM 최대 사용량(MB):    {ram_max}\n")
    f.write(f"-------------------------------\n")