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
#include "ultrasonic.h"  // 초음파 헤더 추가

extern TIM_HandleTypeDef htim4;   // TIM4 핸들
extern UART_HandleTypeDef huart2; // UART2 핸들
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
osThreadId_t dht11TaskHandle;  // 사용한다면
osThreadId_t cdsTaskHandle;
osMessageQueueId_t uartQueueHandle; // 센서 로그 전용
QueueHandle_t motorQueueHandle;     // 모터 명령 큐

// ---- 초음파 태스크 핸들 ----
osThreadId_t ultrasonicTask1Handle;
osThreadId_t ultrasonicTask2Handle;
osThreadId_t ultrasonicTask3Handle;
/* USER CODE END Variables */

/* Definitions for defaultTask */
// (자동생성된 코드에 의해 생성되는 부분 - 여기선 생략)

// ---- 태스크 속성들 ----
/* USER CODE BEGIN RTOS_THREADS */
/* MPU6050 태스크 속성 */
const osThreadAttr_t mpuTask_attributes = {
  .name = "mpuTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

// DHT11 태스크 속성 (필요시 사용)
// const osThreadAttr_t dht11Task_attributes = {
//   .name = "dht11Task",
//   .stack_size = 512 * 4,
//   .priority = (osPriority_t) osPriorityNormal,
// };

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
  .priority = (osPriority_t) osPriorityLow,
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
/* USER CODE END RTOS_THREADS */

/* Private function prototypes -----------------------------------------------*/
void StartDefaultTask(void *argument);
void MX_FREERTOS_Init(void);

/* USER CODE BEGIN FunctionPrototypes */
void StartMPUTask(void *argument);
//void StartDHT11Task(void *argument);
void StartCDSTask(void *argument);
void StartUARTTask(void *argument);
void StartMotorTask(void *argument);

// 센서 로그 콜백 함수
void SensorLogPrinter(const char* msg)
{
    char buf[128];
    snprintf(buf, sizeof(buf), "[SENSOR LOG] %s\r\n", msg);
    HAL_UART_Transmit(&huart2, (uint8_t*)buf, strlen(buf), HAL_MAX_DELAY);
}
/* USER CODE END FunctionPrototypes */

/* Function implementing the defaultTask thread. */
/* (자동생성된 코드 있을 수 있음, 여기선 생략) */

/**
  * @brief  FreeRTOS Initialization
  * @retval None
  */
void MX_FREERTOS_Init(void)
{
  /* USER CODE BEGIN init */
  // ---- 센서 로그 콜백 설정 + 센서 초기화 ----
  SetSensorLogCallback(SensorLogPrinter);
  Sensors_Init();

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

  // DHT11 태스크 (필요하면 주석 해제)
  // dht11TaskHandle = osThreadNew(StartDHT11Task, NULL, &dht11Task_attributes);

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

/* USER CODE BEGIN Header_StartDHT11Task */
// DHT11 태스크가 필요하다면 참고:
/*
void StartDHT11Task(void *argument)
{
  uint8_t temp = 0, humi = 0;
  for(;;)
  {
    SensorMessage_t msg_out;
    uint8_t ok = ReadDHT11(&temp, &humi);
    if (ok)
      snprintf(msg_out.message, sizeof(msg_out.message),
               "[DHT11] Temp: %d°C, Humi: %d%%\r\n", temp, humi);
    else
      snprintf(msg_out.message, sizeof(msg_out.message),
               "[DHT11] Read Fail\r\n");

    osMessageQueuePut(uartQueueHandle, &msg_out, 0, 0);
    osDelay(2000);
  }
}
*/
/* USER CODE END Header_StartDHT11Task */

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
void StartMotorTask(void *argument)
{
  Motor_Init();
  uint8_t cmd;
  for(;;)
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

/* USER CODE BEGIN Header_UltrasonicTasks */
/**
  * @brief  초음파 태스크 (ultrasonic.c에 구현된 함수들)
  * @note   UltrasonicTask1,2,3는 각각 초음파 센서 핀 다르게 설정
  *         1초 간격 vTaskDelay(1000)로 거리 측정 후 UART2로 출력
  */
/* USER CODE END Header_UltrasonicTasks */

// UltrasonicTask1,2,3 함수는 ultrasonic.c에 구현되어 있다고 가정.
// 여기서는 extern 선언만 해줘도 됨. (이미 #include "ultrasonic.h"에 있으므로 생략)

/* USER CODE BEGIN Application */
/* USER CODE END Application */
