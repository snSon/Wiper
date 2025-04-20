/*
 * task_manage.h
 *
 *  Created on: Apr 20, 2025
 *      Author: jiwan
 */

#ifndef INC_TASK_MANAGE_H_
#define INC_TASK_MANAGE_H_

#include "cmsis_os.h"
#include "queue.h"

typedef struct
{
	char message[128];
} SensorMessage_t;

// Task Handler 외부 사용
extern osThreadId_t mpuTaskHandle;
extern osThreadId_t cdsTaskHandle;
extern osThreadId_t lineTracerTaskHandle;
extern osThreadId_t ultrasonicTask1Handle;
extern osThreadId_t ultrasonicTask2Handle;
extern osThreadId_t ultrasonicTask3Handle;
extern osThreadId_t spiTaskHandle;
extern QueueHandle_t motorQueueHandle;     // 모터 명령 큐
extern osMessageQueueId_t uartQueueHandle; // 센서 로그 전용
extern uint8_t current_motor_cmd;

// Task 함수 선언
void StartMPUTask(void *argument);
void StartCDSTask(void *argument);
void StartLineTracerTask(void *argument);
void UltrasonicTask1(void *argument);
void UltrasonicTask2(void *argument);
void UltrasonicTask3(void *argument);
void StartSPITask(void* argument);
void StartMotorTask(void *argument);
void StartUARTTask(void *argument);

#endif /* INC_TASK_MANAGE_H_ */
