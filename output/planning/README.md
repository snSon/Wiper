# Obstacle Stop Planning

---

## Role

---

이 obstacle_stop_planning 모듈은 객체 인식 기반 자율주행 차량의 속도 제어 및 정지 판단 알고리즘을 구현한 것 입니다.

## **Activation**

---

이 모듈은 객체 인식 코드에 입력 영상을 넣어 작동합니다.

## **Structure**

---

### DetectedObject

객체 인식 결과 값을 해당 구조로 저장합니다.

```cpp
		float x_center;     // 인식 객체 박스 가운데 x 
    float y_center;     // 인식 객체 박스 가운데 y
    float bbox_width;   // 바운딩 박스 폭 (픽셀)
    float bbox_height;  // 바운딩 박스 높이 (픽셀)
    double manual_distance = -1.0; // 거리값
```

## **Inner-workings / Algorithms**

---

### control_signal()

메인 함수에서 객체 구조체, 사용자 속도 그리고 날씨 여부를  받아 안전거리 계산 함수를 통해 최종 제어 신호를 출력합니다.

| **매개변수** | **유형** | **설명** |
| --- | --- | --- |
| `obj` | DetectedObject 구조체 | 객체 인식 데이터 |
| `ego_velocity_kmph` | double | 사용자 차량의 속도 [km/h] |
| `weather` | int | 일반 날씨 기준 = 0, 비오는 날 기준 = 1 |

안전 거리 계산 함수 `calculate_braking_distance`를 이용하여 제동 안전 거리를  받아 속도 제어 및 정지 명령을 출력합니다.

<aside>

정지 제어 명령 생성 : 객체와의 거리 ≤ 제동 안전거리 + 5.0m

if (distance_m <= (safe_distance + 5.0))

</aside>

전방 객체가 주행 중이라 판단되면 해당 속도에 맞춰 추종 주행 알고리즘 `match_object_speed`실행

### calculate_braking_distance()

날씨에 따른 제동 안전 거리를 계산하여 반환하는 함수

1. 차량 속도를 m/s 단위로 변환

$$
v = \frac{v_{\text{km/h}} \times 1000}{3600} \quad \text{(m/s)}
$$

1. 제동 거리 계산

$$
d_{\text{brake}} =  \frac{v^2}{2 \mu g}
$$

- μ : 노면 마찰 계수 (0.8 - 일반, 0.4 - 비)
- g : 중력 가속도 (9.8 m/s²)

1. 반응 거리 계산

$$
d_{\text{reaction}} = v \cdot t_{\text{reaction}}
$$

 시스템 상 반응 시간이 0.0000.. 이지만 안전을 위하여 0.1 값을 넣었습니다. 

1. 최종 계산식

$$
d_{\text{total}} = d_{\text{brake}} + d_{\text{reaction}} = \frac{v^2}{2 \mu g} + v \cdot t_{\text{reaction}}
$$

1. 적용 예시

| 속도 (km/h) | 날씨 | 마찰 계수 μ\mu | 제동 거리 (m) | 반응 거리 (m) | 총 제동 거리 (m) |
| --- | --- | --- | --- | --- | --- |
| 60 | 맑음 | 0.8 | 19.15 | 1.67 | 20.82 |
| 60 | 비 | 0.4 | 38.30 | 1.67 | 39.97 |

### match_object_speed()

전방 주행 중인 객체의 속도를 매개변수로 받아 차량의 속도를 변경합니다.

### is_target_object()

bbox x, y값을 이용하여 대상 객체인지 bool 형식으로 리턴하는 함수

```cpp
std::abs(obj.x_center - ref_x) < tolerance &&
std::abs(obj.y_center - ref_y) < tolerance;
```

허용 오차 : tolerance = 0.03f

## main() 동작 과정

1. 공유 메모리에서 프레임 별로 DetectedObject 구조체에 저장
2. 대상 객체 판별 후 거리 삽입
3. 현재 날씨를 토글로 값 입력 
    - 맑은 날씨 : 0
    - 비오는 날 : 1
4. 판단 수행 함수 호출

---