/*
 * task_manage.c
 *
 *  Created on: Apr 20, 2025
 *      Author: jiwan
 *
 *  Modified on : Arr 22, 2025
 *      - Intergrate Sensor task
 *      Author : Juseok Son
 */

#include "task_manage.h"
#include "mpu6050.h"
#include "motor.h"
#include "cds.h"
#include "bluetooth.h"
#include "ultrasonic.h"
#include "spi.h"
#include "usart.h"
#include "tim.h"
#include "lineTracer.h"
#include <string.h>
#include <stdio.h>

#define DURATION 3000

extern UART_HandleTypeDef huart2;
extern SPI_HandleTypeDef hspi1;
extern TIM_HandleTypeDef htim4;

osThreadId_t mpuTaskHandle;
osThreadId_t cdsTaskHandle;
osThreadId_t lineTracerTaskHandle;
osThreadId_t ultrasonicTaskHandle;
osThreadId_t spiTaskHandle;

QueueHandle_t motorQueueHandle;
osMessageQueueId_t uartQueueHandle;

uint8_t current_motor_cmd = 'S';

void StartMPUTask(void *argument)
{
  for(;;)
  {
    MPU6050_Read_Accel();
    MPU6050_Read_Gyro();

    float pitch = MPU6050_CalcPitch();
    float roll  = MPU6050_CalcRoll();
    float yaw   = MPU6050_CalcYaw(0.02f);

    int16_t ax = MPU6050_GetAccelX();
    int16_t ay = MPU6050_GetAccelY();
    int16_t az = MPU6050_GetAccelZ();
    int16_t gx = MPU6050_GetGyroX();
    int16_t gy = MPU6050_GetGyroY();
    int16_t gz = MPU6050_GetGyroZ();

    SensorMessage_t msg_out;
    snprintf(msg_out.message, sizeof(msg_out.message),
             "[MPU6050]\r\n"
             " Accel: X=%d Y=%d Z=%d\r\n"
             " Gyro:  X=%d Y=%d Z=%d\r\n"
             " Pitch=%.2f Roll=%.2f Yaw=%.2f\r\n",
             ax, ay, az, gx, gy, gz, pitch, roll, yaw);

    osMessageQueuePut(uartQueueHandle, &msg_out, 0, 0);

    osDelay(DURATION);
  }
}

void StartCDSTask(void *argument)
{
  /* Infinite loop */
  for(;;)
  {
    SensorMessage_t msg_out;
    uint16_t light = ReadCDS();
    snprintf(msg_out.message, sizeof(msg_out.message),
             "[CDS] Light Intensity: %d\r\n", light);

    osMessageQueuePut(uartQueueHandle, &msg_out, 0, 0);
    osDelay(DURATION);
  }
}

void StartUARTTask(void *argument)
{
  SensorMessage_t recv_msg;
  for(;;)
  {
    if (osMessageQueueGet(uartQueueHandle, &recv_msg, NULL, osWaitForever) == osOK)
    {
      HAL_UART_Transmit(&huart2, (uint8_t*)recv_msg.message,
                        strlen(recv_msg.message), HAL_MAX_DELAY);
    }
  }
}


void StartMotorTask(void *argument)
{
  Motor_Init();
  uint8_t cmd;
  for(;;)
  {
    if (xQueueReceive(motorQueueHandle, &cmd, portMAX_DELAY) == pdTRUE)
    {
      current_motor_cmd = cmd;
      uint16_t speed = Bluetooth_GetSpeed();
      switch (current_motor_cmd)
      {
        case 'F': Motor_Forward(speed); break;
        case 'B': Motor_Backward(speed); break;
        case 'L': Motor_Left(speed); break;
        case 'R': Motor_Right(speed); break;
        case 'S': Motor_Stop(); break;
        default: break;
      }
    }
  }
}

void UltrasonicTask(void *argument)
{
	SensorMessage_t msg_out;
    for (;;) {
        uint32_t d1 = read_ultrasonic_distance_cm(GPIOC, GPIO_PIN_7, GPIOC, GPIO_PIN_6);
        uint32_t d2 = read_ultrasonic_distance_cm(GPIOB, GPIO_PIN_0, GPIOC, GPIO_PIN_8);
        uint32_t d3 = read_ultrasonic_distance_cm(GPIOC, GPIO_PIN_9, GPIOB, GPIO_PIN_2);
        snprintf(msg_out.message, sizeof(msg_out.message), "D1(LEFT) : %lu cm\r\nD2(MID) : %lu cm\r\nD3(RIGHT) : %lu cm\r\n", d1, d2, d3);
        osMessageQueuePut(uartQueueHandle, &msg_out, 0, 0);
        osDelay(DURATION);
    }
}

void StartSPITask(void *argument)
{
    uint8_t rx_val = 0;
    uint8_t tx_val = 0x5A;  // Jetson에게 줄 응답 (필요시 수정 가능)
    char msg[64];

    for(;;)
    {
        if (HAL_SPI_TransmitReceive(&hspi1, &tx_val ,&rx_val, 1, HAL_MAX_DELAY) == HAL_OK)
        {
            // 받은 데이터 해석
            uint8_t human     = (rx_val >> 2) & 0x01;
            uint8_t red_light = (rx_val >> 1) & 0x01;
            uint8_t car       =  rx_val       & 0x01;

            Motor_Forward(600);
            if(human || red_light || car){
            	Motor_Stop();
            	snprintf(msg, sizeof(msg), "[SPI 수신] Stop - 사람:%d, 신호:%d, 차량:%d\r\n", human, red_light, car);
            }
            else{
            	snprintf(msg, sizeof(msg), "[SPI 수신] 사람:%d, 신호:%d, 차량:%d\r\n", human, red_light, car);
            }
        }
        else
        {
            snprintf(msg, sizeof(msg), "[SPI] 통신 실패\r\n");
        }

        HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
        osDelay(100); // 100ms 주기
    }
}


void StartLineTracerTask(void *argument)
{
    uint8_t left, center, right;
    char msg[32];

    for (;;)
    {
        ReadLineSensor(&left, &center, &right);
        LinePosition dir = DecideLineDirection(left, center, right);

        snprintf(msg, sizeof(msg), "[Line] L:%d C:%d R:%d -> Dir:%d\r\n", left, center, right, dir);
        HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
        // UART를 바로 보내는게 아닌 메시지큐에 담아서 보낼 수 있도록 수정 필요 //
        osDelay(DURATION);
    }
}
