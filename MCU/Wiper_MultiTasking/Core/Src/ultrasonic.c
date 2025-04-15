/*
 * ultrasonic.c
 *
 *  Created on: Apr 12, 2025
 *      Author: JunYeong Lee
 */

#include "ultrasonic.h"
#include "tim.h"

extern TIM_HandleTypeDef htim4;

void USdelay_us(uint32_t us)
{
    uint32_t start = __HAL_TIM_GET_COUNTER(&htim4);
    while ((uint32_t)(__HAL_TIM_GET_COUNTER(&htim4) - start) < us);
}


uint32_t read_ultrasonic_distance_cm(GPIO_TypeDef* trigPort, uint16_t trigPin,
                                     GPIO_TypeDef* echoPort, uint16_t echoPin)
{
    HAL_GPIO_WritePin(trigPort, trigPin, GPIO_PIN_RESET);
    USdelay_us(2);
    HAL_GPIO_WritePin(trigPort, trigPin, GPIO_PIN_SET);
    USdelay_us(10);
    HAL_GPIO_WritePin(trigPort, trigPin, GPIO_PIN_RESET);

    uint32_t timeout = 100000;
    while (HAL_GPIO_ReadPin(echoPort, echoPin) == GPIO_PIN_RESET && timeout--);
    if (timeout == 0) return 0;

    uint32_t start = __HAL_TIM_GET_COUNTER(&htim4);

    timeout = 100000;
    while (HAL_GPIO_ReadPin(echoPort, echoPin) == GPIO_PIN_SET && timeout--);
    if (timeout == 0) return 0;

    uint32_t end = __HAL_TIM_GET_COUNTER(&htim4);
    uint32_t duration = (end >= start) ? (end - start) : (0xFFFF - start + end);
    uint32_t distance_cm = duration * 0.034 / 2;

    return distance_cm;
}
