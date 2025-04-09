#ifndef __SENSORS_H__
#define __SENSORS_H__

#include "stm32f4xx_hal.h"

uint8_t ReadDHT11(uint8_t *temperature, uint8_t *humidity); // DHT11
uint16_t ReadCDS(void); // CDS (조도센서)
void SetSensorLogCallback(void (*callback)(const char* msg));
uint8_t ReadDHT22_Debug(uint8_t *temperature, uint8_t *humidity);

// bluetooth
void Bluetooth_SendString(const char* str);

void Sensors_Init(void);  // 타이머 등 필요한 초기화

#endif  /* SENSORS_H_ */
