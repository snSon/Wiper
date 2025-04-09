#include "FreeRTOS.h"
#include "bluetooth.h"
#include "usart.h"
#include "queue.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

extern QueueHandle_t motorQueueHandle;
extern UART_HandleTypeDef huart2; // debug

#define RX_BUFFER_SIZE 64

static char rx_buffer[RX_BUFFER_SIZE];
static uint8_t rx_index = 0;
static uint8_t rx_byte;

static uint16_t global_motor_speed = 800;  // 기본 속도

void Bluetooth_Init(void)
{
    HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
}

void Bluetooth_RxCallback(void)
{
    if (rx_byte == '\n')
    {
        rx_buffer[rx_index] = '\0';
        Parse_Command(rx_buffer);
        rx_index = 0;
    }
    else if (rx_index < RX_BUFFER_SIZE - 1)
    {
        rx_buffer[rx_index++] = rx_byte;
    }

    HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
}

void Parse_Command(const char* cmd)
{
	// debug
	char debug[64];
	sprintf(debug, "CMD: [%s]\r\n", cmd);
	HAL_UART_Transmit(&huart2, (uint8_t*)debug, strlen(debug), HAL_MAX_DELAY);

    uint8_t msg;
    if (strcmp(cmd, "F") == 0)       msg = 'F';
    else if (strcmp(cmd, "B") == 0)  msg = 'B';
    else if (strcmp(cmd, "L") == 0)  msg = 'L';
    else if (strcmp(cmd, "R") == 0)  msg = 'R';
    else if (strcmp(cmd, "S") == 0)  msg = 'S';
    else if (cmd[0] == 'V')  // V800
    {
        int speed = atoi(&cmd[1]);
        if (speed >= 0 && speed <= 1000)
        {
            global_motor_speed = (uint16_t)speed;
            return;
        }
        else
        {
            const char* err = "Invalid Speed\n";
            HAL_UART_Transmit(&huart1, (uint8_t*)err, strlen(err), HAL_MAX_DELAY);
            return;
        }
    }
    else
    {
        const char* err = "Unknown Command\n";
        HAL_UART_Transmit(&huart1, (uint8_t*)err, strlen(err), HAL_MAX_DELAY);
        return;
    }

    xQueueSendFromISR(motorQueueHandle, &msg, NULL);
}

uint16_t Bluetooth_GetSpeed(void)
{
    return global_motor_speed;
}
