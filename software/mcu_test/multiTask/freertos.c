/* USER CODE BEGIN Header */
/*
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
*/
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include <string.h>
#include <stdio.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "sensors.h"
#include "mpu6050.h"
#include "motor.h"
#include "queue.h"
#include "bluetooth.h"

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern SPI_HandleTypeDef hspi2;
extern TIM_HandleTypeDef htim2; // timer handle
extern uint8_t rx_data;

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
osThreadId_t mpuTaskHandle;
osThreadId_t dht11TaskHandle;
osThreadId_t cdsTaskHandle;
osMessageQueueId_t uartQueueHandle; // 센서 전용
QueueHandle_t motorQueueHandle; // 모터 명령 큐

/* USER CODE END Variables */
/* Definitions for defaultTask */


const osThreadAttr_t mpuTask_attributes = {
  .name = "mpuTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

const osThreadAttr_t dht11Task_attributes = {
  .name = "dht11Task",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

const osThreadAttr_t cdsTask_attributes = {
  .name = "cdsTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

const osMessageQueueAttr_t uartQueue_attributes = {
    .name = "uartQueue"
};

const osThreadAttr_t uartTask_attributes = {
  .name = "uartTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

const osThreadAttr_t motorTask_attributes = {
  .name = "motorTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void StartMPUTask(void *argument);
void StartDHT11Task(void *argument);
void StartCDSTask(void *argument);
void StartUARTTask(void *argument);
void StartMotorTask(void *argument);

// ==== 센서 로그 출력 함수 ====
void SensorLogPrinter(const char* msg)
{
    char buf[128];
    snprintf(buf, sizeof(buf), "[SENSOR LOG] %s\r\n", msg);
    HAL_UART_Transmit(&huart2, (uint8_t*)buf, strlen(buf), HAL_MAX_DELAY);
}
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void MX_FREERTOS_Init(void);

void MX_FREERTOS_Init(void) {
  /* 기본 초기화 */
  SetSensorLogCallback(SensorLogPrinter);
  Sensors_Init();

  if (MPU6050_Init())
    HAL_UART_Transmit(&huart2, (uint8_t*)"MPU6050 Init OK\r\n", 18, HAL_MAX_DELAY);
  else
    HAL_UART_Transmit(&huart2, (uint8_t*)"MPU6050 Init FAIL\r\n", 20, HAL_MAX_DELAY);

  /* Task 생성 */
  mpuTaskHandle    = osThreadNew(StartMPUTask, NULL, &mpuTask_attributes);
  dht11TaskHandle  = osThreadNew(StartDHT11Task, NULL, &dht11Task_attributes);
  cdsTaskHandle    = osThreadNew(StartCDSTask, NULL, &cdsTask_attributes);
  uartQueueHandle = osMessageQueueNew(8, sizeof(SensorMessage_t), &uartQueue_attributes);
  osThreadNew(StartUARTTask, NULL, &uartTask_attributes);
  motorQueueHandle = xQueueCreate(8, sizeof(uint8_t)); // 모터 큐 생성
  osThreadNew(StartMotorTask, NULL, &motorTask_attributes); // 모터 제어 Task 생성
  Bluetooth_Init();  // BLE UART1 수신 시작
}

void StartMPUTask(void *argument)
{
    while (1)
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
                 " Gyro: X=%d Y=%d Z=%d\r\n"
                 " Pitch=%.2f Roll=%.2f Yaw=%.2f\r\n",
                 ax, ay, az, gx, gy, gz, pitch, roll, yaw);
        osMessageQueuePut(uartQueueHandle, &msg_out, 0, 0);

        osDelay(2000);
    }
}

void StartDHT11Task(void *argument)
{
    uint8_t temp = 0, humi = 0;
    while (1)
    {
        SensorMessage_t msg_out;
        uint8_t ok = ReadDHT11(&temp, &humi);
        if (ok)
        {
            snprintf(msg_out.message, sizeof(msg_out.message),
                     "[DHT11] Temp: %d°C, Humi: %d%%\r\n", temp, humi);
        }
        else
        {
            snprintf(msg_out.message, sizeof(msg_out.message),
                     "[DHT11] Read Fail\r\n");
        }
        osMessageQueuePut(uartQueueHandle, &msg_out, 0, 0);

        osDelay(2000);
    }
}

void StartCDSTask(void *argument)
{
    while (1)
    {
        SensorMessage_t msg_out;
        uint16_t light = ReadCDS();
        snprintf(msg_out.message, sizeof(msg_out.message),
                 "[CDS] Light Intensity: %d\r\n", light);
        osMessageQueuePut(uartQueueHandle, &msg_out, 0, 0);

        osDelay(2000);
    }
}


void StartUARTTask(void *argument)
{
    SensorMessage_t recv_msg;
    for (;;)
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
    while (1)
    {
        if (xQueueReceive(motorQueueHandle, &cmd, portMAX_DELAY) == pdTRUE)
        {
        	uint16_t speed = Bluetooth_GetSpeed();
            switch (cmd)
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

  /* USER CODE END StartDefaultTask */
/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
