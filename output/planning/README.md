# Obstacle Stop Planning

## 모듈 개요

### 모듈 소개

- `obstacle_stop_planning.cpp`는 자율주행 차량 환경에서 객체 인식 결과 및 거리 데이터를 기반으로 제어 판단을 수행하는 모듈입니다.
- 주행 중 해당 차선에 인식된 객체의 거리와 위치, 날씨 조건(일반/비)에 따라 제동 안전 거리를 계산하여 "감속 후 정지" 또는 "대상 속도에 맞춰 추종" 제어 판단을 수행하며, 그 결과를 로그로 저장합니다.

### 주요 기능

- 객체 인식 로그(`.txt`) 파일에서 인식된 객체 정보를 읽음  
- 수동 거리 매핑(`.hpp`) 파일을 통해 객체 거리 주입  
- 날씨 조건(일반/비)에 따른 제동 안전 거리 계산  
- 대상 객체 정지 → 감속 후 정지  
- 대상 객체 이동 중 → 대상 속도 추종 결정  
- 판단 결과 및 거리 정보를 로그로 저장  



## 디렉토리 구조

```plaintext
project_root/
├── planning/
│   ├── obstacle_stop_planning.cpp
│   ├── frame_distances.hpp          # 프레임별 수동 거리 데이터
│   └── test/
│       ├── detect_log/              # 프레임별 객체 인식 로그(txt)
│       └── control_log/
│           └── control_log.txt   # 판단 결과 로그 파일
```

# 핵심 구현 내용


## 주요 함수 구현 내용


### control_signal()

- 메인 함수에서 객체 구조체, 사용자 속도 그리고 날씨 여부를  받아 안전거리 계산 함수를 통해 최종 제어 신호를 출력합니다.

| **매개변수** | **유형** | **설명** |
| --- | --- | --- |
| `obj` | DetectedObject 구조체 | 객체 인식 데이터 |
| `ego_velocity_kmph` | double | 사용자 차량의 속도 [km/h] |
| `weather` | int | 일반 날씨 기준 = 0, 비오는 날 기준 = 1 |
| `frame` | int | 프레임 번호 |

- 안전 거리 계산 함수 `calculate_braking_distance`를 이용하여 제동 안전 거리를  받아 속도 제어 및 정지 명령을 출력합니다.

<aside>

정지 제어 명령 생성 : 객체와의 거리 ≤ 제동 안전거리 + 5.0m

if (distance_m <= (safe_distance + 5.0))

</aside>

- 전방 객체가 주행 중이라 판단되면 해당 속도에 맞춰 추종 주행 알고리즘 `match_object_speed`실행

### calculate_braking_distance()

- 날씨에 따른 제동 안전 거리를 계산하여 반환하는 함수

1. 차량 속도를 m/s 단위로 변환

$$
v = \frac{v_{\text{km/h}} \times 1000}{3600} \quad \text{(m/s)}
$$

2. 제동 거리 계산

$$
d_{\text{brake}} =  \frac{v^2}{2 \mu g}
$$

- μ : 노면 마찰 계수 (0.8 - 일반, 0.4 - 비)
- g : 중력 가속도 (9.8 m/s²)

3. 반응 거리 계산

$$
d_{\text{reaction}} = v \cdot t_{\text{reaction}}
$$

 시스템 상 반응 시간이 0.0000.. 이지만 안전을 위하여 0.1 값을 넣었습니다. 

4. 최종 계산식

$$
d_{\text{total}} = d_{\text{brake}} + d_{\text{reaction}} = \frac{v^2}{2 \mu g} + v \cdot t_{\text{reaction}}
$$

5. 적용 예시

| 속도 (km/h) | 날씨 | 마찰 계수 μ\mu | 제동 거리 (m) | 반응 거리 (m) | 총 제동 거리 (m) |
| --- | --- | --- | --- | --- | --- |
| 60 | 맑음 | 0.8 | 19.15 | 1.67 | 20.82 |
| 60 | 비 | 0.4 | 38.30 | 1.67 | 39.97 |

### match_object_speed()

- 전방 주행 중인 객체의 속도를 매개변수로 받아 차량의 속도를 변경합니다.
- `calculate_braking_distance`에서 받아온 안전 거리 + 5.0m 안에서 사용자 차량의 속도를 대상 객체 차량의 속도와 동일하게 변경 제어 신호를 출력합니다.

### is_target_object()

- bbox x, y값을 이용하여 대상 객체인지 bool 형식으로 리턴하는 함수
- 대상 객체는 사용자 차량의 차선을 기준으로 정합니다.

```cpp
std::abs(obj.x_center - ref_x) < tolerance &&
std::abs(obj.y_center - ref_y) < tolerance;
```

- 허용 오차 : tolerance = 0.03f

## main() 동작 과정

1. 로그 파일에서 프레임마다 DetectedObject 구조체 형식으로 저장
2. 대상 객체 판별 후 거리 삽입
3. 현재 날씨를 토글로 값 입력 
    - 맑은 날씨 : 0
    - 비오는 날 : 1
4. 판단 수행 함수 호출



# 실행 가이드


## 1. 수동 거리 데이터 입력

- 대상 거리 frame_distances.hpp를 아래의 형식에 맞추어 넣습니다.
```cpp
#pragma once
#include <unordered_map>

const std::unordered_map<int, double> frame_to_distance = {
    {0, 180.000000},
    ...
}
```
- {프레임 번호, 거리값}

## 2. 프레임 별 객체 인식 로그 입력

- yolo.py 에서 입력 영상에 객체에 대한 프레임 별 객체 인식 로그 생성 
```bash
python3 yolo.py
```
- 생성한 로그는 detect_log 폴더에 프레임 별로 텍스트 파일로 저장됩니다.
- 객체 인식 결과 값은 해당 구조체에 맞게 저장됩니다.

```cpp
struct DetectedObject {
	float x_center;     // 인식 객체 박스 가운데 x 
    float y_center;     // 인식 객체 박스 가운데 y
    float bbox_width;   // 바운딩 박스 폭 (픽셀)
    float bbox_height;  // 바운딩 박스 높이 (픽셀)
    float confidence;   // 객체 인식 정확도
    double manual_distance = 100.0; // 디폴트 거리값
}
```
- obstacle_stop_planning.cpp 파일에 폴더 위치가 명시되어있습니다.
```cpp
const std::string LOG_DIR = "/detect_log";
```

## 3. 실행

- 모든 파일이 준비되었다면 아래의 명령어를 통해 실행시킵니다.
```bash
g++ -o obstacle_stop_planning obstacle_stop_planning.cpp -lrt && ./obstacle_stop_planning
```
