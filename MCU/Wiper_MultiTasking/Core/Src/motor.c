#include "motor.h"
#include "tim.h"   // htim1, htim3 사용
#include "main.h"
#include "cmsis_os.h"

// 핀 매핑
#define IN1_PORT GPIOC // GPIOA 0 (기존)
#define IN1_PIN  GPIO_PIN_10
#define IN2_PORT GPIOA
#define IN2_PIN  GPIO_PIN_12
#define IN3_PORT GPIOB
#define IN3_PIN  GPIO_PIN_14
#define IN4_PORT GPIOB
#define IN4_PIN  GPIO_PIN_12

// PWM 핸들 (좌: TIM1_CH1, 우: TIM3_CH4)
#define LEFT_PWM_TIMER    htim1
#define LEFT_PWM_CHANNEL  TIM_CHANNEL_1
#define RIGHT_PWM_TIMER   htim3
#define RIGHT_PWM_CHANNEL TIM_CHANNEL_4

static uint16_t current_speed = 0;  // 가속 제어용

void Motor_Init(void)
{
    HAL_TIM_PWM_Start(&LEFT_PWM_TIMER, LEFT_PWM_CHANNEL);
    HAL_TIM_PWM_Start(&RIGHT_PWM_TIMER, RIGHT_PWM_CHANNEL);

    __HAL_TIM_MOE_ENABLE(&htim1);
    Motor_Stop();
}

// 내부 함수: 방향 설정
static void SetMotorDirection
(GPIO_PinState in1, GPIO_PinState in2, GPIO_PinState in3, GPIO_PinState in4)
{
    HAL_GPIO_WritePin(IN1_PORT, IN1_PIN, in1);
    HAL_GPIO_WritePin(IN2_PORT, IN2_PIN, in2);
    HAL_GPIO_WritePin(IN3_PORT, IN3_PIN, in3);
    HAL_GPIO_WritePin(IN4_PORT, IN4_PIN, in4);
}

// 내부 함수: PWM 출력
static void SetMotorSpeed(uint16_t left_speed, uint16_t right_speed)
{
    __HAL_TIM_SET_COMPARE(&LEFT_PWM_TIMER, LEFT_PWM_CHANNEL, left_speed);
    __HAL_TIM_SET_COMPARE(&RIGHT_PWM_TIMER, RIGHT_PWM_CHANNEL, right_speed);
}

// 정지
void Motor_Stop(void)
{
    SetMotorDirection(GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET);
    SetMotorSpeed(0, 0);
    current_speed = 0;
}

// 전진
void Motor_Forward(uint16_t speed)
{
    //SetMotorDirection(GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_RESET);
	SetMotorDirection(GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_SET);
    SetMotorSpeed(speed, speed);
    current_speed = speed;
}

// 후진
void Motor_Backward(uint16_t speed)
{
    //SetMotorDirection(GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_SET);
	SetMotorDirection(GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_RESET);
    SetMotorSpeed(speed, speed);
    current_speed = speed;
}

// 좌회전 (좌측 정지, 우측 전진)
void Motor_Left(uint16_t speed)
{
    //SetMotorDirection(GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_RESET);
	SetMotorDirection(GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_SET);
    SetMotorSpeed(0, speed);
}

// 우회전 (우측 정지, 좌측 전진)
void Motor_Right(uint16_t speed)
{
    // SetMotorDirection(GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET);
    SetMotorDirection(GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET);
    SetMotorSpeed(speed, 0);
}

// 가속
void Motor_Accelerate(uint16_t target_speed, uint16_t step_delay_ms)
{
    while (current_speed < target_speed)
    {
        current_speed += 100;
        if (current_speed > target_speed) current_speed = target_speed;
        SetMotorSpeed(current_speed, current_speed);
        osDelay(step_delay_ms);
    }
}

// 감속
void Motor_Decelerate(uint16_t step_delay_ms)
{
    while (current_speed > 0)
    {
        current_speed -= 100;
        SetMotorSpeed(current_speed, current_speed);
        osDelay(step_delay_ms);
    }
    Motor_Stop();
}
