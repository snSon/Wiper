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

/**
  * @brief  FreeRTOS Initialization
  * @retval None
  */
void MX_FREERTOS_Init(void)
{
  /* USER CODE BEGIN init */

  // ---- MPU6050 센서 초기화 ----
  if (MPU6050_Init())
    HAL_UART_Transmit(&huart2, (uint8_t*)"MPU6050 Init OK\r\n", 18, HAL_MAX_DELAY);
  else
    HAL_UART_Transmit(&huart2, (uint8_t*)"MPU6050 Init FAIL\r\n", 20, HAL_MAX_DELAY);

  // ---- 블루투스 초기화 ----
  Bluetooth_Init();

  // ---- 큐 생성 ----
  uartQueueHandle = osMessageQueueNew(8, sizeof(SensorMessage_t), NULL);  // 센서 UART 출력용 큐
  motorQueueHandle = xQueueCreate(8, sizeof(uint8_t));                    // 모터 명령 큐

  // ---- 타이머4 시작 (초음파 거리 측정용) ----
  HAL_TIM_Base_Start(&htim4);

  // ---- 태스크 생성 ----
  mpuTaskHandle         = osThreadNew(StartMPUTask, NULL, &mpuTask_attributes);
  cdsTaskHandle         = osThreadNew(StartCDSTask, NULL, &cdsTask_attributes);
  osThreadNew(StartUARTTask, NULL, &uartTask_attributes);
  osThreadNew(StartMotorTask, NULL, &motorTask_attributes);
  ultrasonicTaskHandle  = osThreadNew(UltrasonicTask, NULL, &ultrasonicTask_attributes);
  spiTaskHandle         = osThreadNew(StartSPITask, NULL, &spiTask_attributes);
  lineTracerTaskHandle = osThreadNew(StartLineTracerTask, NULL, &lineTracerTask_attributes); // 필요시 활성화

  /* USER CODE END init */
}

/* USER CODE BEGIN Application */
/* USER CODE END Application */
