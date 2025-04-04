#include "sensors.h"
#include "main.h"
#include <string.h>

extern UART_HandleTypeDef huart2;
extern ADC_HandleTypeDef hadc1;  // ADC1 핸들 선언
extern TIM_HandleTypeDef htim2;  // DHT11용 타이머

// ==== DHT11 정의 ====
#define DHT11_PORT GPIOA
#define DHT11_PIN GPIO_PIN_10

// 직접 GPIO 읽기 최적화 매크로 추가
#define DHT11_INPUT() ((DHT11_PORT->IDR & DHT11_PIN) ? GPIO_PIN_SET : GPIO_PIN_RESET)

static void DHT11_SetPinOutput(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}

static void DHT11_SetPinInput(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}

// 새로 수정한 함수 코드
static uint8_t DHT11_WaitForPinState(GPIO_PinState state, uint32_t timeout_us) {
    uint32_t start = __HAL_TIM_GET_COUNTER(&htim2);
    while (DHT11_INPUT() != state) {
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

uint8_t ReadDHT11(uint8_t *temperature, uint8_t *humidity)
{
    uint8_t bits[5] = {0};
    uint8_t i, j;

    DHT11_SetPinOutput();
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_RESET);
    delay_us(22000);
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET);
    delay_us(30);
    DHT11_SetPinInput();

    if (!DHT11_WaitForPinState(GPIO_PIN_RESET, 200)) {
        HAL_UART_Transmit(&huart2, (uint8_t*)"Step1 Fail: No LOW from DHT\r\n", 30, HAL_MAX_DELAY);
        return 0;
    }
    if (!DHT11_WaitForPinState(GPIO_PIN_SET, 200)) {
        HAL_UART_Transmit(&huart2, (uint8_t*)"Step2 Fail: No HIGH from DHT\r\n", 31, HAL_MAX_DELAY);
        return 0;
    } else {
        HAL_UART_Transmit(&huart2, (uint8_t*)"DHT11 Read OK\r\n", 16, HAL_MAX_DELAY);
    }

    for (j = 0; j < 5; j++) {
        for (i = 0; i < 8; i++) {
        	if (!DHT11_WaitForPinState(GPIO_PIN_RESET, 500)) {
        	            //HAL_UART_Transmit(&huart2, (uint8_t*)"Timeout Low\r\n", 14, HAL_MAX_DELAY);
        	            return 0;
        	   }
        	uint32_t t_start = __HAL_TIM_GET_COUNTER(&htim2);
        	if (!DHT11_WaitForPinState(GPIO_PIN_SET, 500)) {
        	            //HAL_UART_Transmit(&huart2, (uint8_t*)"Timeout High\r\n", 15, HAL_MAX_DELAY);
        	            return 0;
        	}
        	uint32_t t_duration = __HAL_TIM_GET_COUNTER(&htim2) - t_start;

//        	char msg[64];
//        	sprintf(msg, "Bit[%d][%d] = %lu us (%d)\r\n", j, i, t_duration, (t_duration >= 40));
//        	HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);


            bits[j] <<= 1;
            if (t_duration >= 40) bits[j] |= 1;
            else bits[j] &= ~1; // 명확하게 0 처리
        }
    }
    // 테스트-------------------------------------------------------------------------------------
//    for (j = 0; j < 1; j++) {  // 1바이트만 테스트
//        for (i = 0; i < 8; i++) {
//            if (!DHT11_WaitForPinState(GPIO_PIN_RESET, 250)) {
//                HAL_UART_Transmit(&huart2, (uint8_t*)"Timeout Low\r\n", 14, HAL_MAX_DELAY);
//                return 0;
//            }
//            uint32_t t_start = __HAL_TIM_GET_COUNTER(&htim2);
//            if (!DHT11_WaitForPinState(GPIO_PIN_SET, 250)) {
//                HAL_UART_Transmit(&huart2, (uint8_t*)"Timeout High\r\n", 15, HAL_MAX_DELAY);
//                return 0;
//            }
//            uint32_t t_duration = __HAL_TIM_GET_COUNTER(&htim2) - t_start;
//
//            char msg[64];
//            sprintf(msg, "Bit %d duration: %lu us\r\n", i, t_duration);
//            HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
//            osDelay(1); // 수신 간 안정화 목적
//        }
//    }
    // 테스트-------------------------------------------------------------------------------------

    // 테스트-------------------------------------------------------------------------------------
    for (int k = 0; k < 5; k++) {
        char byte_buf[64];
        sprintf(byte_buf, "bits[%d] = %02X\r\n", k, bits[k]);
        HAL_UART_Transmit(&huart2, (uint8_t*)byte_buf, strlen(byte_buf), HAL_MAX_DELAY);
    }
    // 테스트-------------------------------------------------------------------------------------

    if ((uint8_t)(bits[0] + bits[1] + bits[2] + bits[3]) != bits[4]){
        HAL_UART_Transmit(&huart2, (uint8_t*)"Checksum Fail\r\n", 16, HAL_MAX_DELAY);
        return 0;
    }

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

void Sensors_Init(void)
{
    HAL_TIM_Base_Start(&htim2);  // DHT11 타이머용
}
