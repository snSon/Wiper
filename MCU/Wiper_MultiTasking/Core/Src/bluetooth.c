#include "FreeRTOS.h"
#include "bluetooth.h"
#include "usart.h"
#include "queue.h"
#include "main.h"
#include "cmsis_os2.h"
#include "motor.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

extern QueueHandle_t motorQueueHandle;
extern UART_HandleTypeDef huart2; // debug
extern uint8_t current_motor_cmd;

#define RX_BUFFER_SIZE 64

static char rx_buffer[RX_BUFFER_SIZE];
static uint8_t rx_index = 0;
static uint8_t rx_byte;

static uint16_t global_motor_speed = 450;  // 기본 속도

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

    // 다음 수신 대기
    HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
}



void Parse_Command(const char* cmd)
{
	char msgs[64];
	snprintf(msgs, sizeof(msgs), "[DEBUG] Set speed: %d\r\n", global_motor_speed);
	HAL_UART_Transmit(&huart2, (uint8_t*)msgs, strlen(msgs), HAL_MAX_DELAY);

    char c = cmd[0];
    uint8_t msg;

    switch (c)
    {
    case 'f':
           msg = 'F';
           xQueueSendFromISR(motorQueueHandle, &msg, NULL);
           HAL_UART_Transmit(&huart2, (uint8_t*)"[BLE_CMD]: Forward\r\n", 18, HAL_MAX_DELAY);
           break;
       case 'b':
           msg = 'B';
           xQueueSendFromISR(motorQueueHandle, &msg, NULL);
           HAL_UART_Transmit(&huart2, (uint8_t*)"[BLE_CMD]: Backward\r\n", 19, HAL_MAX_DELAY);
           break;
       case 'l':
           msg = 'L';
           xQueueSendFromISR(motorQueueHandle, &msg, NULL);
           HAL_UART_Transmit(&huart2, (uint8_t*)"[BLE_CMD]: Left\r\n", 16, HAL_MAX_DELAY);
           break;
       case 'r':
           msg = 'R';
           xQueueSendFromISR(motorQueueHandle, &msg, NULL);
           HAL_UART_Transmit(&huart2, (uint8_t*)"[BLE_CMD]: Right\r\n", 17, HAL_MAX_DELAY);
           break;
       case 's':
           msg = 'S';
           xQueueSendFromISR(motorQueueHandle, &msg, NULL);
           HAL_UART_Transmit(&huart2, (uint8_t*)"[BLE_CMD]: Stop\r\n", 16, HAL_MAX_DELAY);
           break;
        case 'a':
            global_motor_speed = 200;
            break;
        case 'e':
            global_motor_speed = 450;
            break;
        case 'i':
            global_motor_speed = 600;
            break;
        default:
        {
            char err_msg[64];
            snprintf(err_msg, sizeof(err_msg), "[BLE_ERROR] '%s' was not defined\r\n", cmd);
            HAL_UART_Transmit(&huart2, (uint8_t*)err_msg, strlen(err_msg), HAL_MAX_DELAY);
            return;
        }

    }

    // 속도 설정 메시지 출력 (속도 명령어일 때만)
    if (c == 'a' || c == 'e' || c == 'i')
    {
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


uint16_t Bluetooth_GetSpeed(void)
{
    return global_motor_speed;
}
