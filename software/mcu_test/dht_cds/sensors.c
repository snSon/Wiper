#include "sensors.h"
#include "main.h"
#include <string.h>
#include <stdio.h>

// ==== 외부 핸들 ====
extern UART_HandleTypeDef huart2;
extern ADC_HandleTypeDef hadc1;  // ADC1 핸들 선언
extern TIM_HandleTypeDef htim2;  // DHT11용 타이머

// ==== DHT11 정의 ====
#define DHT11_PORT GPIOA
#define DHT11_PIN GPIO_PIN_10

// GPIO 읽기 최적화 매크로
#define DHT11_INPUT() ((DHT11_PORT->IDR & DHT11_PIN) ? GPIO_PIN_SET : GPIO_PIN_RESET)

// ==== 내부 함수 정의 ====

static void DHT11_SetPinOutput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}

static void DHT11_SetPinInput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP; // 내부 풀업 적용
    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}

static uint8_t DHT11_WaitForPinState(GPIO_PinState state, uint32_t timeout_us)
{
    uint32_t start = __HAL_TIM_GET_COUNTER(&htim2);
    while (DHT11_INPUT() != state)
    {
        if ((uint32_t)(__HAL_TIM_GET_COUNTER(&htim2) - start) >= timeout_us)
            return 0;
    }
    return 1;
}


void delay_us(uint32_t us)
{
    uint32_t start = __HAL_TIM_GET_COUNTER(&htim2);
    while ((uint32_t)(__HAL_TIM_GET_COUNTER(&htim2) - start) < us);
}

// ==== DHT11 데이터 읽기 ====
uint8_t ReadDHT11(uint8_t *temperature, uint8_t *humidity)
{
    uint8_t bits[5] = {0};

    // Start signal
    DHT11_SetPinOutput();
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_RESET);
    delay_us(22000); // 최소 18ms
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET);
    delay_us(60); // 기존 30 -> 50 -> 60
    DHT11_SetPinInput();

    // 응답 신호 체크
    if (!DHT11_WaitForPinState(GPIO_PIN_RESET, 200))
    {
        HAL_UART_Transmit(&huart2, (uint8_t*)"Step1 Fail: No LOW from DHT\r\n", 30, HAL_MAX_DELAY);
        return 0;
    }

    if (!DHT11_WaitForPinState(GPIO_PIN_SET, 200))
    {
        HAL_UART_Transmit(&huart2, (uint8_t*)"Step2 Fail: No HIGH from DHT\r\n", 31, HAL_MAX_DELAY);
        return 0;
    }
    else
    {
        HAL_UART_Transmit(&huart2, (uint8_t*)"DHT11 Read OK\r\n", 16, HAL_MAX_DELAY);
    }

    // 데이터 수신 (40bit: 5bytes)
    for (uint8_t j = 0; j < 5; j++)
    {
        for (uint8_t i = 0; i < 8; i++)
        {
            if (!DHT11_WaitForPinState(GPIO_PIN_RESET, 500))
            {
                char failmsg[64];
                sprintf(failmsg, "Fail LOW at Bit[%d][%d]\r\n", j, i);
                HAL_UART_Transmit(&huart2, (uint8_t*)failmsg, strlen(failmsg), HAL_MAX_DELAY);
                return 0;
            }
            uint32_t t_start = __HAL_TIM_GET_COUNTER(&htim2);

            if (!DHT11_WaitForPinState(GPIO_PIN_SET, 500))
            {
                char failmsg[64];
                sprintf(failmsg, "Fail HIGH at Bit[%d][%d]\r\n", j, i);
                HAL_UART_Transmit(&huart2, (uint8_t*)failmsg, strlen(failmsg), HAL_MAX_DELAY);
                return 0;
            }
            uint32_t t_duration = __HAL_TIM_GET_COUNTER(&htim2) - t_start;

            // 디버깅 메시지
            char msg[64];
            sprintf(msg, "Bit[%d][%d] = %lu us (%d)\r\n", j, i, t_duration, (t_duration >= 40));
            HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

            // 비트 조합
            bits[j] <<= 1;
            if (t_duration >= 40) bits[j] |= 1;
        }
    }

    // 디버깅: 수신된 5바이트 출력 -------------------------------------------------------------------
    for (int k = 0; k < 5; k++)
    {
        char byte_buf[64];
        sprintf(byte_buf, "bits[%d] = %02X\r\n", k, bits[k]);
        HAL_UART_Transmit(&huart2, (uint8_t*)byte_buf, strlen(byte_buf), HAL_MAX_DELAY);
    }

    // 체크섬 확인
    if ((uint8_t)(bits[0] + bits[1] + bits[2] + bits[3]) != bits[4])
    {
        HAL_UART_Transmit(&huart2, (uint8_t*)"Checksum Fail\r\n", 16, HAL_MAX_DELAY);
        return 0;
    }

    // 값 반환
    *humidity = bits[0];
    *temperature = bits[2];
    return 1;
}

// ==== CDS 조도 센서 ADC 읽기 ====
uint16_t ReadCDS(void)
{
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 10);
    uint16_t val = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    return val;
}

// ==== 센서 초기화 ====
void Sensors_Init(void)
{
    HAL_TIM_Base_Start(&htim2);  // DHT11 타이머 시작
}
