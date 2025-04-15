/*
 * dht.c
 *
 *  Modified on: Apr 15, 2025
 *      Author: wldhks1959
 */

#include "dht.h"
#include <string.h>
#include <stdio.h>
#include "main.h"
#include "tim.h"

extern UART_HandleTypeDef huart2;
extern TIM_HandleTypeDef htim2;

#define DHT11_PORT GPIOB
#define DHT11_PIN  GPIO_PIN_3

static void (*SensorLogCallback)(const char* msg) = NULL;

void SetDHTLogCallback(void (*callback)(const char* msg))
{
    SensorLogCallback = callback;
}

#define DHT11_INPUT() ((DHT11_PORT->IDR & DHT11_PIN) ? GPIO_PIN_SET : GPIO_PIN_RESET)

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
    GPIO_InitStruct.Pull = GPIO_PULLUP;
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

uint8_t ReadDHT11(uint8_t *temperature, uint8_t *humidity)
{
    uint8_t bits[5] = {0};

    DHT11_SetPinOutput();
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_RESET);
    delay_us(1500);
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET);
    delay_us(30);
    DHT11_SetPinInput();

    if (!DHT11_WaitForPinState(GPIO_PIN_RESET, 200))
    {
        if (SensorLogCallback) SensorLogCallback("[DEBUG] Step1 Fail: No LOW from DHT");
        return 0;
    }

    if (!DHT11_WaitForPinState(GPIO_PIN_SET, 200))
    {
        if (SensorLogCallback) SensorLogCallback("[DEBUG] Step2 Fail: No HIGH from DHT");
        return 0;
    }
    else
    {
        if (SensorLogCallback) SensorLogCallback("[DEBUG] DHT11 Read OK");
    }

    for (uint8_t j = 0; j < 5; j++)
    {
        for (uint8_t i = 0; i < 8; i++)
        {
            if (!DHT11_WaitForPinState(GPIO_PIN_RESET, 1000))
            {
                if (SensorLogCallback) SensorLogCallback("Fail LOW while reading bit");
                return 0;
            }

            uint32_t t_start = __HAL_TIM_GET_COUNTER(&htim2);
            if (!DHT11_WaitForPinState(GPIO_PIN_SET, 1000)) {
                if (SensorLogCallback) SensorLogCallback("Fail HIGH while reading bit");
                return 0;
            }
            uint32_t t_duration = __HAL_TIM_GET_COUNTER(&htim2) - t_start;
            bits[j] <<= 1;
            if (t_duration >= 30) bits[j] |= 1;
        }
    }

    if ((uint8_t)(bits[0] + bits[1] + bits[2] + bits[3]) != bits[4])
    {
        if (SensorLogCallback) SensorLogCallback("Checksum Fail");
        return 0;
    }

    *temperature = bits[2];
    *humidity = bits[0];
    return 1;
}
