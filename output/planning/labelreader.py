# labelreader.py
# --------------------------------------------------
# 목적: labelgun.py에서 작성한 공유 메모리로부터 실시간 객체 정보를 프레임 단위로 읽어와 출력
# 조건: 중앙 영역(640x640의 3등분 중앙) 안에 있는 객체만 출력
# --------------------------------------------------

import time
import struct
from multiprocessing import shared_memory

# 공유 메모리 이름 정의
SHM_NAME = "detection_buffer"

# 각 프레임당 객체 수 및 1객체당 데이터 구조 (class, x, y, w, h, conf)
MAX_OBJECTS = 20
OBJECT_SIZE = 6 * 4  # 6개의 float32 (24 바이트)
FRAME_SIZE = MAX_OBJECTS * OBJECT_SIZE

# 이미지 해상도 및 중앙 3등분 기준 (정규화된 좌표 기준)
CENTER_X_MIN = 1/3
CENTER_X_MAX = 2/3
CENTER_Y_MIN = 1/3
CENTER_Y_MAX = 2/3

# 마지막 접근 시간 저장
last_access_time = time.time()

try:
    # 공유 메모리 연결 시도
    shm = shared_memory.SharedMemory(name=SHM_NAME)
    print("[✓] 공유 메모리 연결 성공.")
except FileNotFoundError:
    print("[!] 공유 메모리가 존재하지 않습니다. 먼저 labelgun.py를 실행하세요.")
    exit(1)

frame_id = 0

try:
    while True:
        # 현재 프레임 데이터의 시작 위치 계산
        offset = frame_id * FRAME_SIZE

        # 메모리 범위를 초과하면 종료
        if offset + FRAME_SIZE > len(shm.buf):
            print(f"[!] 프레임 {frame_id}는 메모리 범위를 초과합니다.")
            break

        objects = []

        # 객체 20개 반복 처리
        for i in range(MAX_OBJECTS):
            start = offset + i * OBJECT_SIZE
            end = start + OBJECT_SIZE
            data = shm.buf[start:end]

            # 객체 데이터 unpack (6개의 float32)
            cls, x, y, w, h, conf = struct.unpack('6f', data)

            # confidence가 0이면 빈 슬롯으로 간주
            if conf == 0.0:
                break

            # 중앙 영역 필터링 조건: 1/3~2/3 내의 x, y
            if CENTER_X_MIN <= x <= CENTER_X_MAX and CENTER_Y_MIN <= y <= CENTER_Y_MAX:
                objects.append((cls, x, y, w, h, conf))

        # 출력
        print(f"[Frame {frame_id}] 중앙 영역 객체 {len(objects)}개")
        for obj in objects:
            print(f"  → Class: {int(obj[0])}, x: {obj[1]:.3f}, y: {obj[2]:.3f}, w: {obj[3]:.3f}, h: {obj[4]:.3f}, conf: {obj[5]:.2f}")

        last_access_time = time.time()  # 접근 시간 갱신
        frame_id += 1                   # 다음 프레임으로 이동

        time.sleep(0.033)  # 약 30 FPS 처리 속도

        # 10초간 입력이 없으면 종료 및 메모리 해제
        if time.time() - last_access_time > 10:
            print("[!] 10초간 입력이 없어 공유 메모리를 해제합니다.")
            break

except KeyboardInterrupt:
    print("[!] 사용자 인터럽트로 종료합니다.")

# 공유 메모리 닫기
shm.close()
print("[✓] 공유 메모리 해제 완료.")
