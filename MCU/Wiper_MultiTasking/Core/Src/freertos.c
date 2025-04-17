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
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>

#include "mpu6050.h"
#include "motor.h"
#include "queue.h"
#include "bluetooth.h"
#include "ultrasonic.h"  // 초음파 헤더 추가
#include "cds.h"
#include "dht.h"
#include "spi.h"

extern TIM_HandleTypeDef htim4;   // TIM4 핸들
extern UART_HandleTypeDef huart2; // UART2 핸들
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
// ---- 기존 센서/모터 관련 태스크 핸들 ----
osThreadId_t mpuTaskHandle;
osThreadId_t cdsTaskHandle;
osMessageQueueId_t uartQueueHandle; // 센서 로그 전용
QueueHandle_t motorQueueHandle;     // 모터 명령 큐

// ---- 초음파 태스크 핸들 ----
osThreadId_t ultrasonicTask1Handle;
osThreadId_t ultrasonicTask2Handle;
osThreadId_t ultrasonicTask3Handle;

osThreadId_t spiTaskHandle;
/* USER CODE END Variables */

/* Definitions for defaultTask */

// ---- 태스크 속성들 ----
/* USER CODE BEGIN RTOS_THREADS */
/* MPU6050 태스크 속성 */
const osThreadAttr_t mpuTask_attributes = {
  .name = "mpuTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* CDS 태스크 속성 */
const osThreadAttr_t cdsTask_attributes = {
  .name = "cdsTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* UARTTask 속성 */
const osThreadAttr_t uartTask_attributes = {
  .name = "uartTask",
  .stack_size = 512 * 4,
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
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

const osThreadAttr_t ultrasonicTask2_attributes = {
  .name = "ultrasonicTask2",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

const osThreadAttr_t ultrasonicTask3_attributes = {
  .name = "ultrasonicTask3",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

const osThreadAttr_t spiTask_attributes = {
  .name = "ultrasonicTask2",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* USER CODE END RTOS_THREADS */

/* Private function prototypes -----------------------------------------------*/
void StartDefaultTask(void *argument);
void MX_FREERTOS_Init(void);

/* USER CODE BEGIN FunctionPrototypes */
void StartMPUTask(void *argument);
void StartCDSTask(void *argument);
void StartUARTTask(void *argument);
void StartMotorTask(void *argument);
void StartSPITask(void* argument);

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

  // ---- BLE UART 초기화 ----
  Bluetooth_Init();

  // ---- 메시지 큐(센서 로그) 생성 ----
  uartQueueHandle = osMessageQueueNew(8, sizeof(SensorMessage_t), NULL);

  // ---- 모터 큐 생성 ----
  motorQueueHandle = xQueueCreate(8, sizeof(uint8_t));

  // ---- 타이머 4 베이스 스타트 (초음파 측정용) ----
  // 모터 쪽에서는 TIM1/TIM4 등 PWM으로만 쓰고 있다면, 초음파 전용으로 TIM4를 이 형태(프리런)로 사용한다고 가정
  HAL_TIM_Base_Start(&htim4);

  // ---- 태스크 생성들 ----
  // MPU 태스크
  mpuTaskHandle = osThreadNew(StartMPUTask, NULL, &mpuTask_attributes);

  // CDS 태스크
  cdsTaskHandle = osThreadNew(StartCDSTask, NULL, &cdsTask_attributes);

  // 센서 로그(UART) 출력 태스크
  osThreadNew(StartUARTTask, NULL, &uartTask_attributes);

  // 모터 제어 태스크
  osThreadNew(StartMotorTask, NULL, &motorTask_attributes);

  // ---- 초음파 태스크 3개 생성 ----
  ultrasonicTask1Handle = osThreadNew(UltrasonicTask1, NULL, &ultrasonicTask1_attributes);
  ultrasonicTask2Handle = osThreadNew(UltrasonicTask2, NULL, &ultrasonicTask2_attributes);
  ultrasonicTask3Handle = osThreadNew(UltrasonicTask3, NULL, &ultrasonicTask3_attributes);

  spiTaskHandle = osThreadNew(StartSPITask, NULL, &spiTask_attributes);
  /* USER CODE END init */
}

/* USER CODE BEGIN Header_StartMPUTask */
/**
  * @brief  Function implementing the MPU 태스크
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartMPUTask */
void StartMPUTask(void *argument)
{
  /* Infinite loop */
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

    osDelay(2000);
  }
}

/* USER CODE BEGIN Header_StartCDSTask */
/**
  * @brief  Function implementing the CDS 태스크
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartCDSTask */
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
    osDelay(2000);
  }
}

/* USER CODE BEGIN Header_StartUARTTask */
/**
  * @brief  Function implementing the 센서 로그 처리 태스크
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartUARTTask */
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

/* USER CODE BEGIN Header_StartMotorTask */
/**
  * @brief  모터 제어 태스크 (BLE 명령 수신 후 큐로 전달)
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartMotorTask */

uint8_t current_motor_cmd = 'S'; // 초기 정지 상태
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
/* USER CODE BEGIN 1 */
// 초음파 센서 테스크
void UltrasonicTask1(void *argument)
{
	SensorMessage_t msg_out;
    for (;;) {
        uint32_t d = read_ultrasonic_distance_cm(GPIOC, GPIO_PIN_7, GPIOC, GPIO_PIN_6);
        snprintf(msg_out.message, sizeof(msg_out.message), "[Ultrasonic1] Distance: %lu cm\r\n", d);
        osMessageQueuePut(uartQueueHandle, &msg_out, 0, 0);
        osDelay(1000);
    }
}

void UltrasonicTask2(void *argument)
{
	SensorMessage_t msg_out;
    for (;;) {
        uint32_t d = read_ultrasonic_distance_cm(GPIOB, GPIO_PIN_0, GPIOC, GPIO_PIN_8);
        snprintf(msg_out.message, sizeof(msg_out.message), "[Ultrasonic2] Distance: %lu cm\r\n", d);
        osMessageQueuePut(uartQueueHandle, &msg_out, 0, 0);
        osDelay(1000);
    }
}

void UltrasonicTask3(void *argument)
{
	SensorMessage_t msg_out;
    for (;;) {
        uint32_t d = read_ultrasonic_distance_cm(GPIOC, GPIO_PIN_9, GPIOB, GPIO_PIN_2);
        snprintf(msg_out.message, sizeof(msg_out.message), "[Ultrasonic3] Distance: %lu cm\r\n", d);
        osMessageQueuePut(uartQueueHandle, &msg_out, 0, 0);
        osDelay(1000);
    }
}

void StartSPITask(void *argument)
{
    uint8_t rx_val = 0;
    uint8_t tx_val = 0x5A;
    char msg[64];

    for(;;)
    {
        if (HAL_SPI_TransmitReceive(&hspi1, &tx_val ,&rx_val, 1, HAL_MAX_DELAY) == HAL_OK)
        {
            snprintf(msg, sizeof(msg), "[SPI] 수신: 0x%02X, 응답: 0x%02X\r\n", rx_val, tx_val);
        }
        else
        {
            snprintf(msg, sizeof(msg), "[SPI] 통신 실패\r\n");
        }

        HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
        osDelay(1);  // 1ms 주기 (또는 원하시는 주기로 조절)
    }
}

/* USER CODE END 1 */
/* USER CODE BEGIN Application */
/* USER CODE END Application */
