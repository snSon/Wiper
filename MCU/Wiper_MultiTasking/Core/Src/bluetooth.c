#include "FreeRTOS.h"
#include "bluetooth.h"
#include "usart.h"
#include "queue.h"
#include "main.h"
#include "cmsis_os2.h"
#include "motor.h"
#include "ultrasonic.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>

extern QueueHandle_t motorQueueHandle;
extern UART_HandleTypeDef huart2;
extern uint8_t current_motor_cmd;
extern volatile uint32_t ultrasonic_center_distance_cm;

static uint8_t rx_byte;
static uint16_t global_motor_speed = 700;  // 기본 속도

static bool obstacle_detected = false;      // 장애물 감지 상태 플래그
static uint8_t last_motor_command = 'S';

void Bluetooth_Init(void)
{
    HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
}

void Bluetooth_RxCallback(void)
{
    if (rx_byte >= 32 && rx_byte <= 126)  // 유효한 문자
    {
        char command[2] = {0};
        command[0] = tolower((char)rx_byte);
        Parse_Command(command);  // 단일 문자로 파싱
    }
    HAL_UART_Receive_IT(&huart1, &rx_byte, 1);  // 다음 수신 대기
}

static void SendMotorCommand(char direction, const char* message)
{
	xQueueSendFromISR(motorQueueHandle, (uint8_t*)&direction, NULL);
	HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
}

void Parse_Command(const char* cmd)
{
    char c = cmd[0];
	char debug_msg[64];
	snprintf(debug_msg, sizeof(debug_msg), "[DEBUG] Set speed: %d\r\n", global_motor_speed);
	HAL_UART_Transmit(&huart2, (uint8_t*)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);

    switch (c)
    {
	   case 'f': SendMotorCommand('F', "[BLE_CMD]: Forward\r\n"); break;
	   case 'b': SendMotorCommand('B', "[BLE_CMD]: Backward\r\n"); break;
	   case 'l': SendMotorCommand('L', "[BLE_CMD]: Left\r\n"); break;
	   case 'r': SendMotorCommand('R', "[BLE_CMD]: Right\r\n"); break;
	   case 's': SendMotorCommand('S', "[BLE_CMD]: Stop\r\n"); break;

	   case 'a': global_motor_speed = 500; break;
	   case 'e': global_motor_speed = 650; break;
	   case 'i': global_motor_speed = 800; break;

	   case 'x':
			HAL_UART_Transmit(&huart2, (uint8_t*)"[BLE_CMD]: MCU Reset\r\n", strlen("[BLE_CMD]: MCU Reset\r\n"), HAL_MAX_DELAY);
			osDelay(100); // 전송 시간 확보
			NVIC_SystemReset();  // 리셋
			break;
	   default:
       {
            char err_msg[64];
            snprintf(err_msg, sizeof(err_msg), "[BLE_ERROR] '%s' was not defined\r\n", cmd);
            HAL_UART_Transmit(&huart2, (uint8_t*)err_msg, strlen(err_msg), HAL_MAX_DELAY);
            return;
       }
    }

    if (c == 'a' || c == 'e' || c == 'i')
    { // 속도 설정 메시지 출력 (속도 명령어일 때만)
    	switch (current_motor_cmd)
    	{
			case 'F': Motor_Forward(global_motor_speed); break;
			case 'B': Motor_Backward(global_motor_speed); break;
			case 'L': Motor_Left(global_motor_speed); break;
			case 'R': Motor_Right(global_motor_speed); break;
    	}

        char ok_msg[64];
        snprintf(ok_msg, sizeof(ok_msg), "[BLE] Speed set: %d (CMD: %c)\r\n", global_motor_speed, c);
        HAL_UART_Transmit(&huart2, (uint8_t*)ok_msg, strlen(ok_msg), HAL_MAX_DELAY);
    }
}
// 블루투스 기반 초음파 센서 정지
void Bluetooth_CheckObstacle(void)
{

    uint32_t dist = ultrasonic_center_distance_cm;
    if (dist < 30)
    {
        if (!obstacle_detected)
        {
            // 장애물 새로 감지
            obstacle_detected = true;
            uint8_t stop_cmd = 'S';
            xQueueSendFromISR(motorQueueHandle, &stop_cmd, NULL);
            HAL_UART_Transmit(&huart2, (uint8_t*)"[Obstacle] 장애물 감지, 정지\r\n", strlen("[Obstacle] 장애물 감지, 정지\r\n"), HAL_MAX_DELAY);
        }
    }
    else
    {
        if (obstacle_detected)
        {
            // 장애물 해제
            obstacle_detected = false;
            if (last_motor_command != 'S')
            {
                xQueueSendFromISR(motorQueueHandle, &last_motor_command, NULL);
                HAL_UART_Transmit(&huart2, (uint8_t*)"[Obstacle] 장애물 해제, 주행 재개\r\n", strlen("[Obstacle] 장애물 해제, 주행 재개\r\n"), HAL_MAX_DELAY);
            }
        }
    }
}

uint16_t Bluetooth_GetSpeed(void)
{
    return global_motor_speed;
}
