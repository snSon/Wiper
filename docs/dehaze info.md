# 🚗 이미지 디헤이징 기법을 활용한 자율주행 시스템 개선

> 자율주행 환경에서 안개와 같은 악천후로 인해 저하되는 시야 문제를 이미지 디헤이징 기술로 해결할 수 있을까?

---

## 📌 프로젝트 개요

현재 상용 자율주행 시스템에서는 **LiDAR, Radar, Ultrasonic 등 다양한 센서 융합**을 통해 악천후 상황을 대응합니다.  
하지만 **카메라 기반 자율주행**에서는 시야 저하에 따른 객체 인식 정확도 감소 문제가 여전히 존재합니다.

이 문제를 해결하기 위한 대안으로, **이미지 디헤이징(dehazing)** 기술을 활용한 연구가 활발히 진행되고 있습니다.  
본 프로젝트는 대표적인 디헤이징 모델인 **Dehaze-GAN**을 중심으로, 자율주행 시스템에 적용 가능한 흐름과 예시를 정리합니다.

---

## 🔬 연구 기반 사례

- **📂 GitHub:** [Dehaze-GAN (thatbrguy)](https://github.com/thatbrguy/Dehaze-GAN)
- **🧪 Frameworks:** TensorFlow, NumPy, Matplotlib, Scikit-Image
- **🗂️ 사용 데이터셋:**
  - NYU Depth V2
  - Make3D Dataset
  - → RGB + Depth 정보를 포함한 실내/실외 이미지
- **📷 처리 개념:**
  - RGB + Depth 데이터를 이용해 **Foggy / Clear 이미지 페어**를 생성하고 학습

---

## ⚠️ 주요 이슈

- 최신 **NumPy** 버전에서 VGG-19 weight 로딩 오류 발생
- **TensorFlow 2.x** 및 **Eager Execution** 미지원
- Jetson 등 엣지 디바이스를 활용하면 고해상도 실시간 출력 가능성 존재

---

## 🧭 자율주행 적용 전체 흐름 예시

```python
# 1. 흐릿한 이미지 수신 (카메라 입력)
frame = get_camera_frame()

# 2. 전처리
input_tensor = preprocess(frame)

# 3. 디헤이징 수행 (CNN or GAN)
if use_cnn:
    transmission = dehaze_net(input_tensor)
    restored_img = restore_with_physical_model(input_tensor, transmission)
elif use_gan:
    restored_img = generator(input_tensor)

# 4. 후속 처리: 객체 탐지
detections = object_detector(restored_img)

# 5. 판단 및 주행 제어
control_car(detections)


