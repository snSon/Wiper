# 이번주 진행사항 - 이준영 (팀원)

## 🎯 목표
- Jetson Orin Nano에서 OpenCV + YOLOv5 객체 인식 구동

---

## ⚠️ 문제 발생

1. **CSI 카메라 구동 시 USB 포트 deadlock 문제**
2. **GStreamer에서의 CSI 카메라 구동 시 TLS block 문제**
3. **YOLOv5 모델을 GPU로 구동할 수 없는 문제**

---

## 🛠️ 해결 및 진행 과정

### 1. Deadlock 문제 해결
- 원인: 커널 충돌
- 조치: 커널 변경을 통해 문제 해결 완료

---

### 2. TLS Block 문제 분석
- 문제 현상:
  - YOLOv5 실행 시 GStreamer 관련 플러그인 로드 실패
  - 에러 로그:
    ```
    /lib/aarch64-linux-gnu/libGLdispatch.so.0: cannot allocate memory in static TLS block
    ```
  - GStreamer 파이프라인 관련 CRITICAL 오류 다수 발생
  - OpenCV GStreamer 사용 시 파이프라인 생성 실패

---

### 3. TLS Block 문제 해결
- 조치:
  - `LD_PRELOAD=/lib/aarch64-linux-gnu/libGLdispatch.so <your_app>` 설정 시도로 문제 해결
  - Torch 버전 변경 및 종속성 재설정 → 문제 해결 완료

---

### 4. Torch 및 종속성 버전 정리
- JetPack 버전: **5.1.2**
- Torch: **2.1.0**
- Torchvision: **0.16.1**
- Jetson Orin Nano에 최적화된 버전 및 종속성 설정 완료

---

✅ 현재까지의 문제 해결 완료 및 정상 구동 확인
