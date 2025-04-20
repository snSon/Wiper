/*
 * lineTracer.c
 *
 *  Created on: Apr 20, 2025
 *      Author: jiwan
 */


#include "LineTracer.h"
#include "gpio.h"
#include "usart.h"
#include "cmsis_os.h"
#include "cmsis_os2.h"
#include <string.h>
#include <stdio.h>

// 센서 입력 핀 정의
#define SENSOR_LEFT_PIN     GPIO_PIN_13
#define SENSOR_LEFT_PORT    GPIOC

#define SENSOR_CENTER_PIN   GPIO_PIN_4
#define SENSOR_CENTER_PORT  GPIOB

#define SENSOR_RIGHT_PIN    GPIO_PIN_5
#define SENSOR_RIGHT_PORT   GPIOB

// 라인트레이서 태스크 함수 정의
void StartLineTracerTask(void *argument)
{
    char msg[32];
    for (;;)
    {
      // 센서값 읽기
      uint8_t left_raw   = HAL_GPIO_ReadPin(SENSOR_LEFT_PORT, SENSOR_LEFT_PIN);
      uint8_t center_raw = HAL_GPIO_ReadPin(SENSOR_CENTER_PORT, SENSOR_CENTER_PIN);
      uint8_t right_raw  = HAL_GPIO_ReadPin(SENSOR_RIGHT_PORT, SENSOR_RIGHT_PIN);
      // 센서 해석 (0: 흰색, 1: 검정)
	  uint8_t left   = (left_raw == GPIO_PIN_RESET)   ? 1 : 0;
	  uint8_t center = (center_raw == GPIO_PIN_RESET) ? 1 : 0;
	  uint8_t right  = (right_raw == GPIO_PIN_RESET)  ? 1 : 0;

      snprintf(msg, sizeof(msg), "[Line] L:%d C:%d R:%d\r\n", left, center, right);
      HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

      osDelay(1000); // 100ms 주기
    }
}
