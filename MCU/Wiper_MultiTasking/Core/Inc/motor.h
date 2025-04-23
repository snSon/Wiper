#ifndef __MOTOR_H__
#define __MOTOR_H__

#include "stm32f4xx_hal.h"

extern uint16_t current_speed;

// 모터 방향
typedef enum {
    MOTOR_STOP = 0,
    MOTOR_FORWARD,
    MOTOR_BACKWARD,
    MOTOR_LEFT,
    MOTOR_RIGHT
} MotorDirection;

// 초기화 함수
void Motor_Init(void);

// 기본 제어
void Motor_Stop(void);
void Motor_Forward(uint16_t speed);    // speed: 0~1000
void Motor_Backward(uint16_t speed);
void Motor_Left(uint16_t speed);
void Motor_Right(uint16_t speed);

// 가속 / 감속
void Motor_Accelerate(uint16_t target_speed, uint16_t step_delay_ms);
void Motor_Decelerate(uint16_t step_delay_ms);

// 최저 속도
uint16_t SafeSpeed(uint16_t desired, uint16_t min_required);
#endif
