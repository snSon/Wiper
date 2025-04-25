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

extern I2C_HandleTypeDef hi2c1;

static int16_t accel_raw[3] = {0};
static int16_t gyro_raw[3] = {0};
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
}

void MPU6050_Read_Gyro()
{
	ReadSensorData(GYRO_XOUT_H, gyro_raw);
}

int16_t MPU6050_GetAccelX() { return accel_raw[0]; }
int16_t MPU6050_GetAccelY() { return accel_raw[1]; }
int16_t MPU6050_GetAccelZ() { return accel_raw[2]; }

int16_t MPU6050_GetGyroX() { return gyro_raw[0]; }
int16_t MPU6050_GetGyroY() { return gyro_raw[1]; }
int16_t MPU6050_GetGyroZ() { return gyro_raw[2]; }

float MPU6050_CalcPitch()
{
    float ax = accel_raw[0] / ACCEL_SCALE_FACTOR;
    float ay = accel_raw[1] / ACCEL_SCALE_FACTOR;
    float az = accel_raw[2] / ACCEL_SCALE_FACTOR;

    float denom = sqrtf(ay * ay + az * az);
    if (isnan(denom) || denom == 0 || isnan(ax)) return 0.0f;
    return atan2f(-ax, denom) * 180.0f / M_PI;
}

float MPU6050_CalcRoll()
{
    float ay = accel_raw[1] / ACCEL_SCALE_FACTOR;
    float az = accel_raw[2] / ACCEL_SCALE_FACTOR;

    if (isnan(az) || az == 0) return 0.0f;

    return atan2f(ay, az) * 180.0f / M_PI;
}

float MPU6050_CalcYaw(float dt)
{
	// 각속도를 degree/s 단위로 변환
	float gyro_z = gyro_raw[2] / GYRO_SENS; // +- 250dps 범위라면 131
	yaw_angle += gyro_z * dt; // 시간(dt) 단위로 적분 -> yaw 각도 누적

	// 필요 시 -180 ~ +180 범위로 고정
	if (yaw_angle > 180.0f) yaw_angle -= 360.0f;
	if (yaw_angle < -180.0f) yaw_angle += 360.0f;

	return yaw_angle;
}
