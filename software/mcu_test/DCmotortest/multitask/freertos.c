#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "motor.h"
#include <string.h>

/* UART 핸들 외부 참조 */
extern UART_HandleTypeDef huart2;

/* ==== Task 핸들 정의 ==== */
osThreadId_t sensorTaskHandle;
const osThreadAttr_t sensorTask_attributes = {
  .name = "sensorTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t motorTaskHandle;
const osThreadAttr_t motorTask_attributes = {
  .name = "motorTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};

osThreadId_t uartTaskHandle;
const osThreadAttr_t uartTask_attributes = {
  .name = "uartTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

/* ==== Queue 정의 (사용 중이면 유지) ==== */
osMessageQueueId_t distanceQueueHandle;
const osMessageQueueAttr_t distanceQueue_attributes = {
  .name = "distanceQueue"
};

osMessageQueueId_t uartQueueHandle;
const osMessageQueueAttr_t uartQueue_attributes = {
  .name = "uartQueue"
};

/* UART 메시지 전송 함수 */
void send_uart(const char* msg)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

/* ==== Task 함수 선언 ==== */
void StartSensorTask(void *argument);
void StartMotorTask(void *argument);
void StartUARTTask(void *argument);

/* ==== FreeRTOS 초기화 함수 ==== */
void MX_FREERTOS_Init(void) {
  /* Queue 생성 */
  distanceQueueHandle = osMessageQueueNew(4, sizeof(uint16_t), &distanceQueue_attributes);
  uartQueueHandle = osMessageQueueNew(8, sizeof(uint16_t), &uartQueue_attributes);

  /* Task 생성 */
  sensorTaskHandle = osThreadNew(StartSensorTask, NULL, &sensorTask_attributes);
  motorTaskHandle  = osThreadNew(StartMotorTask,  NULL, &motorTask_attributes);
  uartTaskHandle   = osThreadNew(StartUARTTask,   NULL, &uartTask_attributes);
}

/* ==== Task 정의 ==== */

/* 센서 Task - 현재 미사용 */
void StartSensorTask(void *argument)
{
  for (;;)
  {
    osDelay(100);
  }
}

/* 모터 Task */
void StartMotorTask(void *argument)
{
  char msg[32];

  for (;;)
  {
    send_uart("Motor: forward\r\n");
    motor_forward();
    vTaskDelay(pdMS_TO_TICKS(2000));

    send_uart("Motor: stop\r\n");
    motor_stop();
    vTaskDelay(pdMS_TO_TICKS(1000));

    send_uart("Motor: backward\r\n");
    motor_backward();
    vTaskDelay(pdMS_TO_TICKS(2000));

    send_uart("Motor: stop\r\n");
    motor_stop();
    vTaskDelay(pdMS_TO_TICKS(1000));

    send_uart("Motor: left\r\n");
    motor_left();
    vTaskDelay(pdMS_TO_TICKS(1500));

    send_uart("Motor: right\r\n");
    motor_right();
    vTaskDelay(pdMS_TO_TICKS(1500));

    send_uart("Motor: stop\r\n");
    motor_stop();
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

/* UART Task - 현재 미사용 */
void StartUARTTask(void *argument)
{
  for (;;)
  {
    osDelay(100);
  }
}
