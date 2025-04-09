#include "FreeRTOS.h"
#include "bluetooth.h"
#include "usart.h"
#include "queue.h"
#include "main.h"
#include "cmsis_os2.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

extern QueueHandle_t motorQueueHandle;
extern UART_HandleTypeDef huart2; // debug

#define RX_BUFFER_SIZE 64

static char rx_buffer[RX_BUFFER_SIZE];
static uint8_t rx_index = 0;
static uint8_t rx_byte;

static uint16_t global_motor_speed = 400;  // 기본 속도

void Bluetooth_Init(void)
{
    HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
}

void Bluetooth_RxCallback(void)
{
    // 디버그 로그
    char debug_buf[32];
    snprintf(debug_buf, sizeof(debug_buf), "[BLE] CMD: %c\r\n", rx_byte);
    // HAL_UART_Transmit(&huart2, (uint8_t*)debug_buf, strlen(debug_buf), HAL_MAX_DELAY); // debug

    if (rx_byte == '\n' || rx_byte == '\r')
	{
		rx_buffer[rx_index] = '\0';
		// HAL_UART_Transmit(&huart2, (uint8_t*)"RX BUFFER: ", 11, HAL_MAX_DELAY);
		// HAL_UART_Transmit(&huart2, (uint8_t*)rx_buffer, strlen(rx_buffer), HAL_MAX_DELAY);
		// HAL_UART_Transmit(&huart2, (uint8_t*)"\r\n", 2, HAL_MAX_DELAY);

		Parse_Command(rx_buffer);
		memset(rx_buffer, 0, RX_BUFFER_SIZE);
		rx_index = 0;
	}
	else if (rx_index < RX_BUFFER_SIZE - 1 && rx_byte >= 32 && rx_byte <= 126)
	{
		rx_buffer[rx_index++] = rx_byte;

		// 4글자 이상 수신 시 강제 파싱
		if (rx_index >= 4)
		{
			rx_buffer[rx_index] = '\0';
			HAL_UART_Transmit(&huart2, (uint8_t*)"RX BUFFER (auto): ", 20, HAL_MAX_DELAY);
			HAL_UART_Transmit(&huart2, (uint8_t*)rx_buffer, strlen(rx_buffer), HAL_MAX_DELAY);
			HAL_UART_Transmit(&huart2, (uint8_t*)"\r\n", 2, HAL_MAX_DELAY);

			Parse_Command(rx_buffer);
			memset(rx_buffer, 0, RX_BUFFER_SIZE);
			rx_index = 0;
		}
	}
    HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
}



void Parse_Command(const char* cmd)
{
    char command[RX_BUFFER_SIZE];
    strncpy(command, cmd, RX_BUFFER_SIZE);
    for (int i = 0; command[i]; i++)
        command[i] = tolower((unsigned char)command[i]);

    if (strcmp(command, "gogo") == 0)
    {
        uint8_t msg = 'F';
        xQueueSendFromISR(motorQueueHandle, &msg, NULL);
        return;
    }
    else if (strcmp(command, "back") == 0)
    {
        uint8_t msg = 'B';
        xQueueSendFromISR(motorQueueHandle, &msg, NULL);
        return;
    }
    else if (strcmp(command, "left") == 0)
    {
        uint8_t msg = 'L';
        xQueueSendFromISR(motorQueueHandle, &msg, NULL);
        return;
    }
    else if (strcmp(command, "right") == 0)
    {
        uint8_t msg = 'R';
        xQueueSendFromISR(motorQueueHandle, &msg, NULL);
        return;
    }
    else if (strcmp(command, "stop") == 0)
    {
        uint8_t msg = 'S';
        xQueueSendFromISR(motorQueueHandle, &msg, NULL);
        return;
    }
    else if (strcmp(command, "slow") == 0)
    {
        global_motor_speed = 100;
    }
    else if (strcmp(command, "normal") == 0)
    {
        global_motor_speed = 400;
    }
    else if (strcmp(command, "high") == 0)
    {
        global_motor_speed = 800;
    }
    else
    {
        const char* err = "Unknown Command\r\n";
        HAL_UART_Transmit(&huart2, (uint8_t*)err, strlen(err), HAL_MAX_DELAY);
        return;
    }

    char ok_msg[64];
	snprintf(ok_msg, sizeof(ok_msg), "Speed set: %d (CMD: %s)\r\n", global_motor_speed, command);
	HAL_UART_Transmit(&huart2, (uint8_t*)ok_msg, strlen(ok_msg), HAL_MAX_DELAY);
}


uint16_t Bluetooth_GetSpeed(void)
{
    return global_motor_speed;
}
