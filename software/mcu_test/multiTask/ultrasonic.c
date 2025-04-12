/*
 * ultrasonic.c
 *
 *  Created on: Apr 12, 2025
 *      Author: JunYeong Lee
 */


#include "ultrasonic.h"
#include "tim.h"
#include "usart.h"
#include <stdio.h>
#include <string.h>
#include "cmsis_os.h"
#include "ultrasonic.h"

extern TIM_HandleTypeDef htim4;
extern UART_HandleTypeDef huart2;

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

/* FreeRTOS Tasks */
void UltrasonicTask1(void *argument)
{
    char msg[64];
    for (;;) {
        uint32_t d = read_ultrasonic_distance_cm(GPIOC, GPIO_PIN_7, GPIOC, GPIO_PIN_6);
        snprintf(msg, sizeof(msg), "Sensor1: %lu cm\r\n", d);
        HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void UltrasonicTask2(void *argument)
{
    char msg[64];
    for (;;) {
        uint32_t d = read_ultrasonic_distance_cm(GPIOB, GPIO_PIN_0, GPIOC, GPIO_PIN_8);
        snprintf(msg, sizeof(msg), "Sensor2: %lu cm\r\n", d);
        HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void UltrasonicTask3(void *argument)
{
    char msg[64];
    for (;;) {
        uint32_t d = read_ultrasonic_distance_cm(GPIOC, GPIO_PIN_9, GPIOB, GPIO_PIN_2);
        snprintf(msg, sizeof(msg), "Sensor3: %lu cm\r\n", d);
        HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
