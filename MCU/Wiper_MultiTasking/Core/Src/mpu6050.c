#include "mpu6050.h"
#include "main.h"
#include "i2c.h"
#include <math.h>

#define MPU6050_ADDR         (0x68 << 1)
#define WHO_AM_I_REG         0x75
#define PWR_MGMT_1           0x6B
#define ACCEL_XOUT_H         0x3B
#define GYRO_XOUT_H          0x43
#define GYRO_SENS            131.0f
#define ACCEL_SCALE_FACTOR   16384.0f
#define GRAVITY              9.81f   // 중력 가속도 (m/s²)

extern I2C_HandleTypeDef hi2c1;

static int16_t accel_raw[3] = {0};
static int16_t gyro_raw[3] = {0};
static float accel_mps2[3] = {0}; // 가속도 (m/s²)
static float gyro_dps[3] = {0};   // 자이로 (°/s)
static float yaw_angle = 0.0f;

// Read 6 bytes from MPU6050 starting reg_addr and store into raw_data array
static void ReadSensorData(uint8_t reg_addr, int16_t *raw_data)
{
	uint8_t buffer[6];
	if (HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR, reg_addr, 1, buffer, 6, 100) == HAL_OK)
	{
		raw_data[0] = (int16_t)(buffer[0] << 8 | buffer[1]);
		raw_data[1] = (int16_t)(buffer[2] << 8 | buffer[3]);
		raw_data[2] = (int16_t)(buffer[4] << 8 | buffer[5]);
	}
	else
	{
		raw_data[0] = raw_data[1] = raw_data[2] = 0;
	}
}

uint8_t MPU6050_Init()
{
    uint8_t check, data = 0;
    if (HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR, WHO_AM_I_REG, 1, &check, 1, 200) != HAL_OK || check != 0x68)
           return 0;

    HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, PWR_MGMT_1, 1, &data, 1, 200);
    return 1;
}

void MPU6050_Read_Accel()
{
    ReadSensorData(ACCEL_XOUT_H, accel_raw);

    // 변환: raw -> m/s²
   accel_mps2[0] = (accel_raw[0] / ACCEL_SCALE_FACTOR) * GRAVITY;
   accel_mps2[1] = (accel_raw[1] / ACCEL_SCALE_FACTOR) * GRAVITY;
   accel_mps2[2] = (accel_raw[2] / ACCEL_SCALE_FACTOR) * GRAVITY;
}

void MPU6050_Read_Gyro()
{
	ReadSensorData(GYRO_XOUT_H, gyro_raw);
	// 변환: raw -> °/s
	gyro_dps[0] = gyro_raw[0] / GYRO_SENS;
	gyro_dps[1] = gyro_raw[1] / GYRO_SENS;
	gyro_dps[2] = gyro_raw[2] / GYRO_SENS;
}

float MPU6050_GetAccelX() { return accel_mps2[0]; }
float MPU6050_GetAccelY() { return accel_mps2[1]; }
float MPU6050_GetAccelZ() { return accel_mps2[2]; }

float MPU6050_GetGyroX() { return gyro_dps[0]; }
float MPU6050_GetGyroY() { return gyro_dps[1]; }
float MPU6050_GetGyroZ() { return gyro_dps[2]; }

float MPU6050_CalcPitch()
{
	float ax = accel_mps2[0] / GRAVITY; // 다시 g로 환산
	float ay = accel_mps2[1] / GRAVITY;
	float az = accel_mps2[2] / GRAVITY;

    float denom = sqrtf(ay * ay + az * az);
    if (isnan(denom) || denom == 0 || isnan(ax)) return 0.0f;
    return atan2f(-ax, denom) * 180.0f / M_PI;
}

float MPU6050_CalcRoll()
{
	float ay = accel_mps2[1] / GRAVITY;
	float az = accel_mps2[2] / GRAVITY;

    if (isnan(az) || az == 0) return 0.0f;

    return atan2f(ay, az) * 180.0f / M_PI;
}

float MPU6050_CalcYaw(float dt)
{
	// 변환된 gyro_dps를 사용
	yaw_angle += gyro_dps[2] * dt; // z축 각속도

	// 필요 시 -180 ~ +180 범위로 고정
	if (yaw_angle > 180.0f) yaw_angle -= 360.0f;
	if (yaw_angle < -180.0f) yaw_angle += 360.0f;

	return yaw_angle;
}
