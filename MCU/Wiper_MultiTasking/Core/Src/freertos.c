/* USER CODE BEGIN Header */
/*
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
*/
/* for commit*/
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "task_manage.h"

extern UART_HandleTypeDef huart2;
extern SPI_HandleTypeDef hspi1;
extern TIM_HandleTypeDef htim4;
extern uint8_t current_motor_cmd;

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

// ---- 태스크 속성들 ----
/* USER CODE BEGIN RTOS_THREADS */
/* MPU6050 태스크 속성 */
const osThreadAttr_t mpuTask_attributes = {
  .name = "mpuTask",
  .stack_size = 384 * 4, // 1.5KB
  .priority = (osPriority_t) osPriorityNormal,
};

/* CDS 태스크 속성 */
const osThreadAttr_t cdsTask_attributes = {
  .name = "cdsTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* UARTTask 속성 */
const osThreadAttr_t uartTask_attributes = {
  .name = "uartTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* MotorTask 속성 */
const osThreadAttr_t motorTask_attributes = {
  .name = "motorTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* 초음파 태스크 속성들 */
const osThreadAttr_t ultrasonicTask1_attributes = {
  .name = "ultrasonicTask1",
  .stack_size = 192 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

const osThreadAttr_t ultrasonicTask2_attributes = {
  .name = "ultrasonicTask2",
  .stack_size = 192 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

const osThreadAttr_t ultrasonicTask3_attributes = {
  .name = "ultrasonicTask3",
  .stack_size = 192 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

const osThreadAttr_t spiTask_attributes = {
  .name = "spiTask",
  .stack_size = 192 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

// 라인트레이서 속성
const osThreadAttr_t lineTracerTask_attributes = {
  .name = "lineTracerTask",
  .stack_size = 384 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* USER CODE END RTOS_THREADS */

/* Private function prototypes -----------------------------------------------*/
void MX_FREERTOS_Init(void);

/* USER CODE BEGIN FunctionPrototypes */

// 센서 로그 콜백 함수
void SensorLogPrinter(const char* msg)
{
    char buf[128];
    snprintf(buf, sizeof(buf), "[SENSOR LOG] %s\r\n", msg);
    HAL_UART_Transmit(&huart2, (uint8_t*)buf, strlen(buf), HAL_MAX_DELAY);
}
/* USER CODE END FunctionPrototypes */

/* Function implementing the defaultTask thread. */

/**
  * @brief  FreeRTOS Initialization
  * @retval None
  */
void MX_FREERTOS_Init(void)
{
  /* USER CODE BEGIN init */

  // ---- MPU6050 초기화 ----
  if (MPU6050_Init())
    HAL_UART_Transmit(&huart2, (uint8_t*)"MPU6050 Init OK\r\n", 18, HAL_MAX_DELAY);
  else
    HAL_UART_Transmit(&huart2, (uint8_t*)"MPU6050 Init FAIL\r\n", 20, HAL_MAX_DELAY);

  Bluetooth_Init();   // ---- BLE UART 초기화 ----
  uartQueueHandle = osMessageQueueNew(8, sizeof(SensorMessage_t), NULL);  // ---- 메시지 큐(센서 로그) 생성 ----
  motorQueueHandle = xQueueCreate(8, sizeof(uint8_t)); // ---- 모터 큐 생성 ----
  HAL_TIM_Base_Start(&htim4);   // ---- 타이머 4 베이스 스타트 (초음파 측정용) ----

  // ---- 태스크 생성
  mpuTaskHandle = osThreadNew(StartMPUTask, NULL, &mpuTask_attributes);
  cdsTaskHandle = osThreadNew(StartCDSTask, NULL, &cdsTask_attributes);
  osThreadNew(StartUARTTask, NULL, &uartTask_attributes);
  osThreadNew(StartMotorTask, NULL, &motorTask_attributes);
  ultrasonicTask1Handle = osThreadNew(UltrasonicTask1, NULL, &ultrasonicTask1_attributes);
  ultrasonicTask2Handle = osThreadNew(UltrasonicTask2, NULL, &ultrasonicTask2_attributes);
  ultrasonicTask3Handle = osThreadNew(UltrasonicTask3, NULL, &ultrasonicTask3_attributes);
  spiTaskHandle = osThreadNew(StartSPITask, NULL, &spiTask_attributes);
  lineTracerTaskHandle = osThreadNew(StartLineTracerTask, NULL, &lineTracerTask_attributes);
  /* USER CODE END init */
}

/* USER CODE BEGIN Application */
/* USER CODE END Application */
