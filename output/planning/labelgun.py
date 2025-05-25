import os
import time
import numpy as np
from multiprocessing import shared_memory

# ------------------------------------------
# [1] 설정 상수
# ------------------------------------------
FRAME_COUNT = 932              # 전체 프레임 수 (0 ~ 931)
MAX_OBJECTS = 20               # 프레임당 최대 객체 수를 20개로 설정
FEATURE_SIZE = 6               # YOLO 형식: class, cx, cy, w, h, conf → 6개

# ------------------------------------------
# [2] 공유 메모리 생성
#    → 3차원 배열 형태로 (프레임 수, 객체 수, 객체 정보)
#    → float32이므로 요소 하나당 4바이트
# ------------------------------------------
buffer_shape = (FRAME_COUNT, MAX_OBJECTS, FEATURE_SIZE)
buffer_size = np.prod(buffer_shape) * 4  # 전체 크기(바이트) 계산

try:
    # 이미 있으면 에러 → 새로 생성
    shm = shared_memory.SharedMemory(name='detection_buffer', create=True, size=buffer_size)
    print("[✓] Shared memory 'detection_buffer' created.")
except FileExistsError:
    # 이미 존재하면 연결만 수행
    shm = shared_memory.SharedMemory(name='detection_buffer', create=False)
    print("[!] Shared memory already exists. Overwriting.")

# 공유 메모리 버퍼를 numpy 배열로 매핑
buffer = np.ndarray(buffer_shape, dtype=np.float32, buffer=shm.buf)

# -----------------------------------   -------
# [3] YOLO 라벨 파일 경로 설정
# ------------------------------------------
# 필요 시 수정 가능
label_dir = '/home/hanul/jiwan_yolov5/Wiper/processing/object_detection/custom_yolov5/runs/test_detect/trt_engine_run1/labels'  # 라벨 .txt 파일이 저장된 경로

# ------------------------------------------
# [4] 각 프레임마다 라벨 파일 읽어서 공유 메모리에 저장
# ------------------------------------------
for frame_idx in range(FRAME_COUNT):
    label_path = os.path.join(label_dir, f'test_drive_30_{frame_idx}.txt')  # 예: 0.txt ~ 931.txt

    if not os.path.exists(label_path):
        print(f"[!] Frame {frame_idx}: label file not found.")
        continue  # 없으면 건너뜀

    with open(label_path, 'r') as f:
        lines = f.readlines()

    # 객체 리스트 초기화
    objects = []
    for line in lines:
        if len(objects) >= MAX_OBJECTS:
            break  # 최대 20개까지만 허용
        data = list(map(float, line.strip().split()))
        if len(data) == FEATURE_SIZE:
            objects.append(data)

    # 객체 정보를 numpy 배열로 변환 (없는 부분은 0으로 패딩)
    frame_array = np.zeros((MAX_OBJECTS, FEATURE_SIZE), dtype=np.float32)
    frame_array[:len(objects), :] = np.array(objects, dtype=np.float32)
    buffer[frame_idx] = frame_array  # 공유 메모리에 저장

    print(f"[✓] Frame {frame_idx}: {len(objects)} objects written to shared memory.")

# ------------------------------------------
# [5] 모든 프레임 처리 완료 후 공유 메모리 유지
#     → Ctrl+C로 직접 종료해야 메모리 유지됨
# ------------------------------------------
print("[✓] 모든 프레임 처리 완료. 메모리 유지 중... (Ctrl+C to exit)")

try:
    while True:
        time.sleep(1)
except KeyboardInterrupt:
    print("[!] 종료됨. 메모리는 reader가 접근 후 자동 해제하거나 직접 unlink 해주세요.")
    shm.close()
    shm.unlink()
