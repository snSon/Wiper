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
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
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
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
    SetSensorLogCallback(SensorLogPrinter);  // 로그 콜백 등록
    Sensors_Init();  // 타이머 시작

    if (MPU6050_Init())
        HAL_UART_Transmit(&huart2, (uint8_t*)"MPU6050 Init OK\r\n", 18, HAL_MAX_DELAY);
    else
        HAL_UART_Transmit(&huart2, (uint8_t*)"MPU6050 Init FAIL\r\n", 20, HAL_MAX_DELAY);

    uint8_t temp = 0, humi = 0;
    uint16_t light = 0;
    char msg[512];  // 출력 확장을 위해 버퍼 크기 증가

    for (;;)
    {
        // (1) 센서 데이터 읽기
        MPU6050_Read_Accel();
        MPU6050_Read_Gyro();  // 자이로도 읽기

        float pitch = MPU6050_CalcPitch();
        float roll  = MPU6050_CalcRoll();
        float yaw = MPU6050_CalcYaw(0.02f); // 20 ms 기준

        int16_t ax = MPU6050_GetAccelX();
        int16_t ay = MPU6050_GetAccelY();
        int16_t az = MPU6050_GetAccelZ();

        int16_t gx = MPU6050_GetGyroX();
        int16_t gy = MPU6050_GetGyroY();
        int16_t gz = MPU6050_GetGyroZ();

        light = ReadCDS();
        uint8_t ok = ReadDHT11(&temp, &humi);

        // (2) 출력 메시지 구성
        if (ok)
        {
            snprintf(msg, sizeof(msg),
                "[DHT11] Temp: %d°C, Humi: %d%%\r\n"
                "[CDS]   Light: %d\r\n"
                "[MPU6050]\r\n"
                "  Accel -> X: %d, Y: %d, Z: %d\r\n"
                "  Gyro  -> X: %d, Y: %d, Z: %d\r\n"
                "  Pitch: %.2f°, Roll: %.2f°, YAW: %.2f°\r\n"
                "-------------------------------\r\n",
                temp, humi, light,
                ax, ay, az,
                gx, gy, gz,
                pitch, roll, yaw
            );
        }
        else
        {
            snprintf(msg, sizeof(msg),
                "[DHT11] Read Fail\r\n"
                "[CDS]   Light: %d\r\n"
                "[MPU6050]\r\n"
                "  Accel -> X: %d, Y: %d, Z: %d\r\n"
                "  Gyro  -> X: %d, Y: %d, Z: %d\r\n"
                "  Pitch: %.2f°, Roll: %.2f°, YAW: %.2f°\r\n"
                "-------------------------------\r\n",
                light,
                ax, ay, az,
                gx, gy, gz,
                pitch, roll, yaw
            );
        }

        // (3) UART 전송
        HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

        // (4) 주기 대기
        osDelay(2000);  // 2초
    }
}

  /* USER CODE END StartDefaultTask */
/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
