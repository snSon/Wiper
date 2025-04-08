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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "sensors.h"
#include "mpu6050.h"
#include <string.h>
#include <stdio.h>

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

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t mpuTaskHandle;
osThreadId_t dht11TaskHandle;
osThreadId_t cdsTaskHandle;
osMessageQueueId_t uartQueueHandle;


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


/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void StartMPUTask(void *argument);
void StartDHT11Task(void *argument);
void StartCDSTask(void *argument);
void StartUARTTask(void *argument);  // 🔧 누락된 선언

// ==== 센서 로그 출력 함수 ====
void SensorLogPrinter(const char* msg)
{
    char buf[128];
    snprintf(buf, sizeof(buf), "[SENSOR LOG] %s\r\n", msg);
    HAL_UART_Transmit(&huart2, (uint8_t*)buf, strlen(buf), HAL_MAX_DELAY);
}
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
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
}


/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
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
                 "[MPU6050] Accel: X=%d Y=%d Z=%d | Gyro: X=%d Y=%d Z=%d | Pitch=%.2f Roll=%.2f Yaw=%.2f\r\n",
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

  /* USER CODE END StartDefaultTask */
/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
