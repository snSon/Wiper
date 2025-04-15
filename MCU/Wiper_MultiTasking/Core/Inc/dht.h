/*
 * dht.h
 *
 *  Modified on: Apr 15, 2025
 *      Author: wldhks1959
 */

#ifndef INC_DHT_H_
#define INC_DHT_H_

#include "stm32f4xx_hal.h"
#include <stdint.h>

void SetDHTLogCallback(void (*callback)(const char* msg));
uint8_t ReadDHT11(uint8_t *temperature, uint8_t *humidity);
void delay_us(uint32_t us);

#endif /* INC_DHT_H_ */
