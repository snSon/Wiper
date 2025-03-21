# 차량 제어

## 1. MPC 기반 차량 제어

MPC(Model Predictive Control)는 Optimal Control의 한 방법으로, 속도 및 가속력, 주변 환경 조건을 입력값으로 받아 최적의 제어 명령을 생성하는 방식이다. 상태 변수를 기반으로 최적의 제어 명령을 출력하는 것이 특징이다.

[MPC기반 자율주행 논문.pdf] 

## 2. 학생 수준의 차량 제어

### 2.1 센서 기반 제어

- OpenCV의 라인트레이싱  
- 초음파 센서를 이용한 장애물 감지  

### 2.2 PID 제어

**정의:** PID(Proportional-Integral-Derivative) 제어는 차량의 속도 및 조향을 조절하는 데 사용된다.

- **P (비례 제어)**: 현재의 편차(error)에 비례하여 조향을 조절  
- **I (적분 제어)**: 오차가 누적되지 않도록 보정  
- **D (미분 제어)**: 급격한 변화에 대응하여 부드럽게 조정  

---

# 주행 및 회피 알고리즘

## 1. 주행 알고리즘 (차선 유지)

### (1) 차선 인식 및 주행 경로 설정

- OpenCV를 활용하여 차선의 기울기를 계산하고 조향 각도를 결정  
- PID 제어를 통해 차선 유지 및 속도 조절  
- LFA (Lane Following Assist, 차로 유지 보조)  

#### 1. 카메라 데이터 사전 처리
- BEV(Birds-Eye View) 변환, 색상 필터링  
- Opening/Closing, Bilateral 필터링 적용  

#### 2. 관심 영역(ROI) 지정 후 필터링
- Gaussian Blur 및 노이즈 제거  
- Canny Edge Detection을 활용하여 차선 추출  

#### 3. 차선 중앙값 계산
- RANSAC 알고리즘을 통해 차선 구간 추출  
- 평균값 계산을 통해 차선 중앙값 확보  

#### 4. 차선 중앙 유지 주행
- 확보된 차선 중앙값과 차량의 현재 위치 차이를 계산하여 차선 중앙 유지 주행  

---

## 2. 회피 알고리즘

### 1. 레이더 기반 회피
레이더를 통해서 먼 거리의 물체와의 거리를 계산하여 감속 혹은 회피

#### 관련 논문
- **자율 주행을 위한 레이더 기반 인지 알고리즘의 정량적 분석**  
  [논문 링크](https://scienceon.kisti.re.kr/commons/util/originalView.do?cn=JAKO201821142173836&oCn=JAKO201821142173836&dbt=JAKO&journal=NJOU00550557)

- **차량의 자율 주행을 위한 레이더 기술**  
  [논문 링크](https://scienceon.kisti.re.kr/commons/util/originalView.do?cn=JAKO201418960400848&oCn=JAKO201418960400848&dbt=JAKO&journal=NJOU00290673)

### 2. 초음파 센서 기반 회피
초음파 센서를 활용하여 가까운 거리의 장애물이 나타났을 때 급정거  

### 3. 라이다 기반 회피
라이다를 이용한 장애물 감지 및 회피 알고리즘 적용  
