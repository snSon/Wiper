#include "stm32f4xx_hal.h"
#include "usart.h"
#include <stdio.h>

int __io_putchar(int ch) {
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
