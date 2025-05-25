import struct
import os
from multiprocessing import shared_memory

# 공유 메모리 이름 설정
SHM_NAME = "detection_buffer_3d"

# 최대 프레임 수와 프레임당 최대 객체 수
MAX_FRAMES = 932
MAX_OBJECTS = 20

# 객체 구조: class_id, x_center, y_center, width, height, distance (총 6개의 float)
OBJECT_SIZE = 6 * 4  # float32 6개, 1개당 4바이트 = 24 바이트
FRAME_SIZE = MAX_OBJECTS * OBJECT_SIZE
TOTAL_SIZE = MAX_FRAMES * FRAME_SIZE

# 중앙 영역 필터링 기준 (비율)
X_MIN_RATIO, X_MAX_RATIO = 1/3, 2/3
Y_MIN_RATIO, Y_MAX_RATIO = 1/3, 2/3

# .txt 라벨 파일이 위치한 디렉토리 경로
LABEL_DIR = "./labels_3d"

# 공유 메모리 생성 (존재 시 연결)
try:
    shm = shared_memory.SharedMemory(name=SHM_NAME, create=True, size=TOTAL_SIZE)
    print(f"[✓] 공유 메모리 '{SHM_NAME}' 생성 완료.")
except FileExistsError:
    shm = shared_memory.SharedMemory(name=SHM_NAME, create=False)
    print(f"[!] 공유 메모리 '{SHM_NAME}' 이미 존재, 연결 성공.")

# 프레임 0 ~ 931까지 반복
for frame_id in range(MAX_FRAMES):
    label_path = os.path.join(LABEL_DIR, f"{frame_id}.txt")
    offset = frame_id * FRAME_SIZE

    # 해당 프레임의 라벨 파일이 없으면 건너뜀
    if not os.path.exists(label_path):
        continue

    with open(label_path, 'r') as f:
        lines = f.readlines()

    object_count = 0

    for line in lines:
        if object_count >= MAX_OBJECTS:
            break

        parts = line.strip().split()
        if len(parts) != 6:
            continue

        cls, x, y, w, h, dist = map(float, parts)

        # 중앙 영역 필터링 (x, y가 1/3 ~ 2/3 범위 내에 있어야 기록)
        if not (X_MIN_RATIO <= x <= X_MAX_RATIO and Y_MIN_RATIO <= y <= Y_MAX_RATIO):
            continue

        packed = struct.pack('6f', cls, x, y, w, h, dist)
        start = offset + object_count * OBJECT_SIZE
        shm.buf[start:start+OBJECT_SIZE] = packed
        object_count += 1

    print(f"[✓] Frame {frame_id}: {object_count} objects written to memory.")

# 공유 메모리 닫기
shm.close()
print("[✓] 모든 프레임 라벨 기록 완료. 공유 메모리 닫음.")
