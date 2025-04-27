/*
 * task_manage.c
 *
 *  Created on: Apr 20, 2025
 *  Modified on: Apr 21, 2025
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

#define DURATION 1000
#define LINE_TRACE_PERIOD 250 // 0.25초

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

uint8_t current_motor_cmd = 'S'; // BLE 용
volatile uint32_t ultrasonic_center_distance_cm = 1000;  // 기본값: 멀리 있음
extern uint16_t current_speed;

void StartMPUTask(void *argument)
{
  for(;;)
  {
    MPU6050_Read_Accel();
    MPU6050_Read_Gyro();

    SensorMessage_t msg_out;
    snprintf(msg_out.message, sizeof(msg_out.message),
             "[MPU6050]\r\n"
             " Accel: X=%d Y=%d Z=%d\r\n"
             " Gyro:  X=%d Y=%d Z=%d\r\n"
             " Pitch=%.2f Roll=%.2f Yaw=%.2f\r\n",
			 MPU6050_GetAccelX(), MPU6050_GetAccelY(), MPU6050_GetAccelZ(),
			 MPU6050_GetGyroX(), MPU6050_GetGyroY(), MPU6050_GetGyroZ(),
			 MPU6050_CalcPitch(), MPU6050_CalcRoll(), MPU6050_CalcYaw(0.02f));

    osMessageQueuePut(uartQueueHandle, &msg_out, 0, 0);

    osDelay(DURATION);
  }
}

void StartCDSTask(void *argument)
{
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
    for (;;)
    {
//        float d1 = read_ultrasonic_distance_cm(GPIOC, GPIO_PIN_7, GPIOC, GPIO_PIN_6);
        float d2 = read_ultrasonic_distance_cm(GPIOB, GPIO_PIN_0, GPIOC, GPIO_PIN_4);
//        float d3 = read_ultrasonic_distance_cm(GPIOC, GPIO_PIN_9, GPIOB, GPIO_PIN_2);
        float d1=9999, d3=9999;

        ultrasonic_center_distance_cm = (uint32_t)d2; // 초음파 거리

        if(d2 < 40.0)
        {
        	Motor_Stop();
        }
        else
        {
        	uint16_t speed = Bluetooth_GetSpeed();
        	Motor_Forward(speed);
        }


        snprintf(msg_out.message, sizeof(msg_out.message),
        		"Dist1: %.2f cm\r\n"
        		"Dist2: %.2f cm\r\n"
        		"Dist3: %.2f cm\r\n",
				d1, d2, d3);
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
            if(human || red_light || car)
            {
            	Motor_Stop();
            	snprintf(msg, sizeof(msg), "[SPI 수신] Stop - 사람:%d, 신호:%d, 차량:%d\r\n", human, red_light, car);
            }
            else
            {
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
    SensorMessage_t msg_out1, msg_out2;

    for (;;)
    {
        ReadLineSensor(&left, &center, &right);
        LinePosition dir = DecideLineDirection(left, center, right);

        snprintf(msg_out1.message, sizeof(msg_out1.message),
                 "[Line] L:%d C:%d R:%d -> Dir:%d\r\n", left, center, right, dir);
        osMessageQueuePut(uartQueueHandle, &msg_out1, 0, 0);

        LineTracerDriveDecision(dir, &msg_out2);

        osMessageQueuePut(uartQueueHandle, &msg_out2, 0, 0);
        osDelay(LINE_TRACE_PERIOD);
    }
}
