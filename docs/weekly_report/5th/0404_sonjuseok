# A111 RADAR 테스트 진행 보고

## 테스트 개요

- SparkFun A111 RADAR 모듈을 **Jetson Orin Nano**에서 구동하기 위한 테스트 수행
- 공식 지원 플랫폼은 **Raspberry Pi**이므로, Jetson에서 직접 구동하려면 아키텍처 및 GPIO 체계에 맞는 커스터마이징 필요
- 두 가지 방식으로 접근:
  1. **Acconeer SDK 수동 빌드**
  2. **Exploration Tool (GUI 기반 툴) 설치 및 실행**

---

## 진행 내용 및 문제 해결

### 1. SDK 빌드

- 문제: SDK는 Raspberry Pi 전용 라이브러리 및 하드웨어 API 사용
- 해결:
  - `libgpiod`, `spidev` 등 Jetson 호환 드라이버로 교체
  - ARMv7 기반 일부 코드 → Jetson Orin Nano(ARMv8 Cortex-A78) 호환 형태로 수정
  - GPIO 및 SPI 제어 방식 맞춤 리팩토링 후 빌드

### 2. Exploration Tool 실행

- 문제: 기본적으로 FT4222 USB-SPI 브릿지를 사용하는 구조 → Jetson 환경과 맞지 않음
- 해결:
  - Tool 설정 파일을 수정해 SPI 디바이스 직접 지정 (`spidev0.0` 등)
  - Jetson GPIO 핀 번호 및 gpiod 방식에 맞춰 **Interrupt/Enable 핀 설정 수정**
  - Device Tree 수정 중 USB 장치(마우스/키보드) 충돌 문제 발생 → 현재는 기본 FDT로 복귀한 상태

---

## 결론 및 향후 방향

- A111은 Raspberry Pi 전용 RADAR로, Jetson 환경에서 사용하기에는 비효율적
- Jetson Orin Nano에 적합한 RADAR 센서 도입 필요
  - 후보: **Infineon BGT60TR13C**
  - 일부 `.whl` 파일 및 DTS 수정으로 Jetson 대응 가능성 존재 → 분석 진행 중

---

# Image Dehazing 진행

- Image Dehazing 테스트 환경 구축을 위해 주요 데이터셋 조사
- 대표 벤치마크인 **RESIDE** 데이터셋 확인 및 해당 기반 논문 분석 완료
- 기초적인 **Single Image Dehazing** 알고리즘 관련 논문 검토
- 향후 목표는 **Real-time Video Dehazing 구현**이며, 관련 논문 셀렉 및 기술 분석 진행 중
