import time
import struct
from multiprocessing import shared_memory

# 공유 메모리 이름과 구조 정의
SHM_NAME = "detection_buffer_3d"
MAX_OBJECTS = 20
OBJECT_SIZE = 6 * 4  # class, x, y, w, h, distance
FRAME_SIZE = MAX_OBJECTS * OBJECT_SIZE

frame_id = 0
last_access_time = time.time()

# 공유 메모리 연결 시도
try:
    shm = shared_memory.SharedMemory(name=SHM_NAME)
    print("[✓] 공유 메모리 연결 성공.")
except FileNotFoundError:
    print("[!] 공유 메모리가 존재하지 않습니다. 먼저 labelgun_3d.py를 실행하세요.")
    exit(1)

try:
    while True:
        offset = frame_id * FRAME_SIZE

        # 공유 메모리 범위 초과 시 종료
        if offset + FRAME_SIZE > len(shm.buf):
            print(f"[!] Frame {frame_id}는 메모리 범위를 초과합니다.")
            break

        objects = []
        for i in range(MAX_OBJECTS):
            start = offset + i * OBJECT_SIZE
            end = start + OBJECT_SIZE
            data = shm.buf[start:end]
            cls, x, y, w, h, dist = struct.unpack("6f", data)

            if dist == 0.0:
                break

            # 중앙 영역만 출력 (640x640 기준 1/3 ~ 2/3)
            if not (1/3 <= x <= 2/3 and 1/3 <= y <= 2/3):
                continue

            objects.append((cls, x, y, w, h, dist))

        print(f"[Frame {frame_id}] {len(objects)} objects (center only):")
        for obj in objects:
            print(f"  → Class: {int(obj[0])}, x: {obj[1]:.3f}, y: {obj[2]:.3f}, w: {obj[3]:.3f}, h: {obj[4]:.3f}, dist: {obj[5]:.2f}m")

        last_access_time = time.time()
        frame_id += 1
        time.sleep(0.033)  # 약 30FPS 출력

        if time.time() - last_access_time > 10:
            print("[!] 10초간 접근 없음. 공유 메모리 해제.")
            break

except KeyboardInterrupt:
    print("[!] 사용자 중단 감지. 종료합니다.")

# 공유 메모리 해제
shm.close()
print("[✓] 공유 메모리 해제 완료.")
