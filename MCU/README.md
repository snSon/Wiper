# 🛠️ Wiper_STM 프로젝트 구조

STM32F446RE 기반 FreeRTOS 프로젝트로, 자율주행 RC카에서 각종 센서와 모터를 제어하는 코드 구조

---

## 📁 사용자 정의 헤더 파일

| 파일명         | 설명                                 |
|----------------|--------------------------------------|
| `bluetooth.h`  | 블루투스 통신 제어 함수 선언        |
| `cds.h`        | 조도 센서(CDS) 값을 읽기 위한 헤더  |
| `dht.h`        | DHT 온습도 센서 제어 헤더           |
| `motor.h`      | DC 모터 제어용 함수 정의             |
| `mpu6050.h`    | MPU6050 IMU 센서 제어 헤더          |
| `ultrasonic.h` | HC-SR04 초음파 센서 제어 헤더       |

---

## 📄 사용자 정의 소스 파일

| 파일명         | 주요 기능 설명 |
|----------------|----------------|
| `bluetooth.c`  | UART1 기반 BLE 초기화, Rx 콜백 처리, 명령어 파싱, 속도 제어 |
| `cds.c`        | 조도 센서 값 읽기 (`ReadCDS`) |
| `dht.c`        | 온습도 센서 초기화, 읽기, 디버깅 로그 함수 포함 |
| `motor.c`      | 모터 초기화, 정방향/역방향/회전 제어, 가감속 구현 |
| `mpu6050.c`    | IMU 초기화, 가속도/자이로 데이터 처리, Pitch/Roll/Yaw 계산 |
| `ultrasonic.c` | 초음파 거리 측정 및 FreeRTOS Task 예제 구현 |

---

## ⚙️ 자동 생성 코드

STM32CubeMX에서 자동 생성된 코드들로, 시스템 및 주변 장치 초기화 포함

- `main.c/h`
- `adc.c/h`, `i2c.c/h`, `spi.c/h`, `tim.c/h`, `usart.c/h`, `gpio.c/h`
- `FreeRTOSConfig.h`

---

## 📌 주요 코드 설명

### `main.c`

- `HAL_Init()` 및 각종 주변 장치 초기화 수행
- `MX_FREERTOS_Init()` 호출로 FreeRTOS Task 및 Queue 설정
- `osKernelStart()` 호출 이후, CPU 제어권은 FreeRTOS 커널로 이관됨

### `freertos.c`

- RTOS의 핵심 Task 및 메시지 큐 정의
- 각 센서 Task, UART 로그 출력 Task, BLE 명령 처리 Task를 병렬로 생성
- 실제 애플리케이션 로직은 모두 여기서 실행됨

---

## ✅ 전체 흐름 요약

```text
main.c
 ├── HAL 및 주변 장치 초기화
 ├── MX_FREERTOS_Init() → Task, Queue 설정
 └── osKernelStart() → Task 실행 시작 (FreeRTOS가 CPU 제어)

freertos.c
 ├── 각종 Task (MPU6050, CDS, Ultrasonic, UART, Motor 등) 생성
 ├── 각 Task는 개별적으로 동작하며, 병렬 처리 가능
 └── 메시지 큐를 통해 로그 및 명령 제어 수행
