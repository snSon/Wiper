/*
 * ultrasonic.c
 *
 *  Created on: Apr 12, 2025
 *      Author: JunYeong Lee
 */

#include "ultrasonic.h"
#include "tim.h"

extern TIM_HandleTypeDef htim4;

#define TIMEOUT_COUNT 100000
#define SPEED_OF_SOUND_CM_PER_US 0.034f
#define TRIGGER_PULSE_US 10
#define SETTLING_TIME_US 2
#define MAX_TIMER_COUNT 0xFFFF

void USdelay_us(uint32_t us)
{
    uint32_t start = __HAL_TIM_GET_COUNTER(&htim4);
    while ((uint32_t)(__HAL_TIM_GET_COUNTER(&htim4) - start) < us);
}


float read_ultrasonic_distance_cm(GPIO_TypeDef* trigPort, uint16_t trigPin,
                                     GPIO_TypeDef* echoPort, uint16_t echoPin)
{
	// Trigger pulse
    HAL_GPIO_WritePin(trigPort, trigPin, GPIO_PIN_RESET);
    USdelay_us(SETTLING_TIME_US);
    HAL_GPIO_WritePin(trigPort, trigPin, GPIO_PIN_SET);
    USdelay_us(TRIGGER_PULSE_US);
    HAL_GPIO_WritePin(trigPort, trigPin, GPIO_PIN_RESET);

    // Wait for echo start
    uint32_t timeout = TIMEOUT_COUNT;
    while (HAL_GPIO_ReadPin(echoPort, echoPin) == GPIO_PIN_RESET && timeout--);
    if (timeout == 0) return 0;

    uint32_t start = __HAL_TIM_GET_COUNTER(&htim4);

    // Wait for echo end
    timeout = TIMEOUT_COUNT;
    while (HAL_GPIO_ReadPin(echoPort, echoPin) == GPIO_PIN_SET && timeout--);
    if (timeout == 0) return 0;

    uint32_t end = __HAL_TIM_GET_COUNTER(&htim4);
    uint32_t duration = (end >= start) ? (end - start) : (MAX_TIMER_COUNT - start + end);

    // Calculate distance in cm
    float distance_cm = (((float)duration * SPEED_OF_SOUND_CM_PER_US) / 2.0f);
    return distance_cm;
}
