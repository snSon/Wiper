/*
 * ultrasonic.h
 *
 *  Created on: Apr 12, 2025
 *      Author: JunYeong Lee
 *
 *  Modified on : Arr 22, 2025
 *      - Intergrate Sensor task
 *      Author : Juseok Son
 */

#ifndef INC_ULTRASONIC_H_
#define INC_ULTRASONIC_H_

#include "stm32f4xx_hal.h"
#include "main.h"

float read_ultrasonic_distance_cm(GPIO_TypeDef* trigPort, uint16_t trigPin,
                                     GPIO_TypeDef* echoPort, uint16_t echoPin);
void UltrasonicTask(void *argument);
void USdelay_us(uint32_t us);

#endif /* INC_ULTRASONIC_H_ */
