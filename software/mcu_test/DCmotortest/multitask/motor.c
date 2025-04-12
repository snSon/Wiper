#include "motor.h"
#include "tim.h"
#include "gpio.h"
#include "usart.h"
#include <stdio.h>
#include <string.h>

extern UART_HandleTypeDef huart2;

void motor_init(void)
{
    // PWM Start
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);  // ENA (Left)
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);  // ENB (Right)

    // Default direction pins to LOW
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);  // IN1
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);   // IN2
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);   // IN3
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);   // IN4
}

void motor_forward(void)
{
    // Left forward
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);    // IN1
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);   // IN2

    // Right forward
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);     // IN3
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);   // IN4

    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 800);  // ENA
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 800);  // ENB

    char msg[] = "Motor: forward\r\n";
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

void motor_backward(void)
{
    // Left backward
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);  // IN1
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);     // IN2

    // Right backward
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);   // IN3
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);     // IN4

    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 800);  // ENA
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 800);  // ENB

    char msg[] = "Motor: backward\r\n";
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

void motor_left(void)
{
    // Right motor forward only
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);  // IN1
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);   // IN2

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);     // IN3
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);   // IN4

    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);    // ENA off
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 800);  // ENB on

    char msg[] = "Motor: turn left\r\n";
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

void motor_right(void)
{
    // Left motor forward only
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);    // IN1
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);   // IN2

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);   // IN3
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);   // IN4

    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 800);  // ENA on
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);    // ENB off

    char msg[] = "Motor: turn right\r\n";
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

void motor_stop(void)
{
    // PWM은 유지하고 듀티만 0으로
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);  // ENA
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);  // ENB

    // 방향 핀 LOW로 모두 정지
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET); // IN1
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);  // IN2
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);  // IN3
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);  // IN4

    char msg[] = "Motor: stop\r\n";
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}
