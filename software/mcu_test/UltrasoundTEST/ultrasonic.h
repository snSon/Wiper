// lnc 폴더에 넣기
#ifndef __ULTRASONIC_H
#define __ULTRASONIC_H

#include "stm32f4xx_hal.h"

void delay_us(uint16_t us);
uint32_t read_ultrasonic_distance_cm(GPIO_TypeDef* trigPort, uint16_t trigPin,
                                     GPIO_TypeDef* echoPort, uint16_t echoPin);
void UltrasonicTask1(void *argument);
void UltrasonicTask2(void *argument);
void UltrasonicTask3(void *argument);

#endif
