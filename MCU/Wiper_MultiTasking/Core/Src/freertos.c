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

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* USER CODE BEGIN RTOS_THREADS */
/* MPU6050 태스크 속성 */
const osThreadAttr_t mpuTask_attributes = {
  .name = "mpuTask",
  .stack_size = 384 * 4,
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

/* 초음파 태스크 속성 */
const osThreadAttr_t ultrasonicTask_attributes = {
  .name = "ultrasonicTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* SPI 통신 태스크 속성 */
const osThreadAttr_t spiTask_attributes = {
  .name = "spiTask",
  .stack_size = 192 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* 라인트레이서 태스크 속성 */
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
/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

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
  // lineTracerTaskHandle = osThreadNew(StartLineTracerTask, NULL, &lineTracerTask_attributes); // 필요시 활성화

  /* USER CODE END init */
}

/* USER CODE BEGIN Application */
/* USER CODE END Application */
