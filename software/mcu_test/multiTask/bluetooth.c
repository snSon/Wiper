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

//void Bluetooth_RxCallback(void)
//{
//    // rx_byte가 32~126(가시문자) 범위일 때만 버퍼에 쌓는다.
//    // 나머지(\n,\r 등)는 무시.
//    if (rx_byte >= 32 && rx_byte <= 126)
//    {
//        rx_buffer[rx_index++] = rx_byte;
//
//        // 4글자 받으면 바로 파싱
//        if (rx_index == 4)
//        {
//            // 문자열 끝에 널문자 추가
//            rx_buffer[4] = '\0';
//
//            // 디버그 출력 (원하면 생략 가능)
//            HAL_UART_Transmit(&huart2, (uint8_t*)"RX BUFFER (4 chars): ", 21, HAL_MAX_DELAY);
//            HAL_UART_Transmit(&huart2, (uint8_t*)rx_buffer, 4, HAL_MAX_DELAY);
//            HAL_UART_Transmit(&huart2, (uint8_t*)"\r\n", 2, HAL_MAX_DELAY);
//
//            // 명령어 파싱
//            Parse_Command(rx_buffer);
//
//            // 버퍼를 깨끗이 지우고 인덱스 0으로 되돌림
//            memset(rx_buffer, 0, RX_BUFFER_SIZE);
//            rx_index = 0;
//        }
//        // 만약 4글자를 초과로 입력해버리면 (rx_index==5~64) 무시되거나
//        // 뒤섞일 수 있으니, 가급적 앱에서 '정확히 4글자'씩만 전송하는 게 중요.
//    }
//
//    // 다음 바이트 수신 준비 (이 콜백 맨 끝에서 항상 필요)
//    HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
//}

void Bluetooth_RxCallback(void)
{
    // 만약 수신된 바이트가 개행 문자('\n' 또는 '\r')라면,
    // 지금까지 받은 문자열을 파싱한다.
    if (rx_byte == '\n' || rx_byte == '\r')
    {
        // 문자열 끝에 널 문자 붙여서 C-스트링 완성
        rx_buffer[rx_index] = '\0';

        // 버퍼에 뭔가 들어있다면 파싱
        if (rx_index > 0)
        {
            Parse_Command(rx_buffer);
        }

        // 버퍼 비우기
        memset(rx_buffer, 0, RX_BUFFER_SIZE);
        rx_index = 0;
    }
    // 가시 문자(스페이스(32)~'~'(126))라면 버퍼에 쌓는다.
    else if (rx_byte >= 32 && rx_byte <= 126)
    {
        // 버퍼 오버플로우 방지
        if (rx_index < RX_BUFFER_SIZE - 1)
        {
            rx_buffer[rx_index++] = rx_byte;
        }
        else
        {
            // 버퍼 초과 시 그냥 초기화
            rx_index = 0;
            memset(rx_buffer, 0, RX_BUFFER_SIZE);
        }
    }

    // 다음 바이트 수신 대기
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
    else if (strcmp(command, "righ") == 0)
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
    else if (strcmp(command, "norm") == 0)
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
