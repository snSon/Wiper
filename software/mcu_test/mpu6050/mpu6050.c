#include "mpu6050.h"
#include "main.h"
#include "i2c.h"
#include <math.h>

#define MPU6050_ADDR         (0x68 << 1)
#define WHO_AM_I_REG         0x75
#define PWR_MGMT_1           0x6B
#define ACCEL_XOUT_H         0x3B
#define GYRO_XOUT_H          0x43

extern I2C_HandleTypeDef hi2c1;

static int16_t accel_x_raw = 0;
static int16_t accel_y_raw = 0;
static int16_t accel_z_raw = 0;
static int16_t gyro_x_raw = 0;
static int16_t gyro_y_raw = 0;
static int16_t gyro_z_raw = 0;

uint8_t MPU6050_Init()
{
    uint8_t check, data = 0;
    HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR, WHO_AM_I_REG, 1, &check, 1, 100);
    if (check != 0x68) return 0;
    HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, PWR_MGMT_1, 1, &data, 1, 100);
    return 1;
}

void MPU6050_Read_Accel()
{
    uint8_t accel_data[6];
    if (HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR, ACCEL_XOUT_H, 1, accel_data, 6, 100) != HAL_OK)
    {
        accel_x_raw = 0;
        accel_y_raw = 0;
        accel_z_raw = 0;
        return;
    }
    accel_x_raw = (int16_t)(accel_data[0] << 8 | accel_data[1]);
    accel_y_raw = (int16_t)(accel_data[2] << 8 | accel_data[3]);
    accel_z_raw = (int16_t)(accel_data[4] << 8 | accel_data[5]);
}

void MPU6050_Read_Gyro()
{
    uint8_t gyro_data[6];
    if (HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR, GYRO_XOUT_H, 1, gyro_data, 6, 100) != HAL_OK)
    {
        gyro_x_raw = 0;
        gyro_y_raw = 0;
        gyro_z_raw = 0;
        return;
    }
    gyro_x_raw = (int16_t)(gyro_data[0] << 8 | gyro_data[1]);
    gyro_y_raw = (int16_t)(gyro_data[2] << 8 | gyro_data[3]);
    gyro_z_raw = (int16_t)(gyro_data[4] << 8 | gyro_data[5]);
}

int16_t MPU6050_GetAccelX() { return accel_x_raw; }
int16_t MPU6050_GetAccelY() { return accel_y_raw; }
int16_t MPU6050_GetAccelZ() { return accel_z_raw; }

int16_t MPU6050_GetGyroX() { return gyro_x_raw; }
int16_t MPU6050_GetGyroY() { return gyro_y_raw; }
int16_t MPU6050_GetGyroZ() { return gyro_z_raw; }

float MPU6050_CalcPitch()
{
    float ax = accel_x_raw / 16384.0f;
    float ay = accel_y_raw / 16384.0f;
    float az = accel_z_raw / 16384.0f;

    float denom = sqrtf(ay * ay + az * az);
    if (isnan(denom) || denom == 0 || isnan(ax)) return 0.0f;

    return atan2f(-ax, denom) * 180.0f / M_PI;
}

float MPU6050_CalcRoll()
{
    float ay = accel_y_raw / 16384.0f;
    float az = accel_z_raw / 16384.0f;

    if (isnan(az) || az == 0) return 0.0f;

    return atan2f(ay, az) * 180.0f / M_PI;
}
