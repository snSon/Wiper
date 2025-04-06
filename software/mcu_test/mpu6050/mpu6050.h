#ifndef MPU6050_H_
#define MPU6050_H_

#include <stdint.h>

// ==== 초기화 및 데이터 갱신 함수 ====
uint8_t MPU6050_Init(void);
void MPU6050_Read_Accel(void);
void MPU6050_Read_Gyro(void);

// ==== 가속도 데이터 접근 ====
int16_t MPU6050_GetAccelX(void);
int16_t MPU6050_GetAccelY(void);
int16_t MPU6050_GetAccelZ(void);

// ==== 자이로 데이터 접근 ====
int16_t MPU6050_GetGyroX(void);
int16_t MPU6050_GetGyroY(void);
int16_t MPU6050_GetGyroZ(void);

// ==== 계산 함수 ====
float MPU6050_CalcPitch(void);
float MPU6050_CalcRoll(void);

#endif /* MPU6050_H_ */
