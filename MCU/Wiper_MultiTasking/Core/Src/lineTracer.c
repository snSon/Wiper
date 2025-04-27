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
#define SENSOR_LEFT_PIN     GPIO_PIN_5 // 실제는 x4
#define SENSOR_LEFT_PORT    GPIOB

#define SENSOR_CENTER_LEFT_PIN   GPIO_PIN_4
#define SENSOR_CENTER_LEFT_PORT  GPIOB

#define SENSOR_CENTER_RIGHT_PIN    GPIO_PIN_2 // 실제는 x1
#define SENSOR_CENTER_RIGHT_PORT   GPIOD

#define SENSOR_RIGHT_PIN    GPIO_PIN_13
#define SENSOR_RIGHT_PORT   GPIOC


void ReadLineSensor(uint8_t* left, uint8_t* center, uint8_t* right)
{
    uint8_t left_raw   = HAL_GPIO_ReadPin(SENSOR_LEFT_PORT, SENSOR_LEFT_PIN);
    uint8_t center_left_raw  = HAL_GPIO_ReadPin(SENSOR_CENTER_LEFT_PORT, SENSOR_CENTER_LEFT_PIN);
    uint8_t center_right_raw = HAL_GPIO_ReadPin(SENSOR_CENTER_RIGHT_PORT, SENSOR_CENTER_RIGHT_PIN);
    uint8_t right_raw  = HAL_GPIO_ReadPin(SENSOR_RIGHT_PORT, SENSOR_RIGHT_PIN);

    *left   = (left_raw == GPIO_PIN_RESET)   ? 1 : 0;
    // 중앙은 좌측 또는 우측 센서가 LOW이면 감지된 것으로 판단
    *center = ((center_left_raw == GPIO_PIN_RESET) || (center_right_raw == GPIO_PIN_RESET)) ? 1 : 0;
    *right  = (right_raw == GPIO_PIN_RESET)  ? 1 : 0;
}

LinePosition DecideLineDirection(uint8_t left, uint8_t center, uint8_t right)
{
	if (left && center && right) return LINE_ALL;
	if (left && center && !right) return LINE_CENTER;
	if (right && center && !left) return LINE_CENTER;
	if (left && !center && !right) return LINE_LEFT;
	if (right && !center && !left) return LINE_RIGHT;
	if (center && !left && !right) return LINE_CENTER;
    return LINE_NONE;
}
