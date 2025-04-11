/* main.c
 * 프로젝트: 3초음파 센서를 FreeRTOS Task로 분리하여 동작
 * 작성일: 2025년 4월 11일
 * 작성자: dlwns
 */

#include "main.h"
#include "cmsis_os.h"
#include "gpio.h"
#include "tim.h"
#include "usart.h"
#include <stdio.h>
#include <string.h>

/* 외부 핸들러 선언 */
extern TIM_HandleTypeDef htim1;
extern UART_HandleTypeDef huart2;

/* 타이머 기반 delay 함수 (us 단위) */
void delay_us(uint16_t us)
{
    __HAL_TIM_SET_COUNTER(&htim1, 0);
    while (__HAL_TIM_GET_COUNTER(&htim1) < us);
}

/*
 * 초음파 센서 거리 측정 함수 (cm 단위)
 * 매개변수로 센서의 Trig/Echo 핀 포트 및 번호를 전달함
 */
uint32_t read_ultrasonic_distance_cm(GPIO_TypeDef* trigPort, uint16_t trigPin,
                                       GPIO_TypeDef* echoPort, uint16_t echoPin)
{
    /* 트리거 펄스 생성: LOW(2us) → HIGH(10us) → LOW */
    HAL_GPIO_WritePin(trigPort, trigPin, GPIO_PIN_RESET);
    delay_us(2);
    HAL_GPIO_WritePin(trigPort, trigPin, GPIO_PIN_SET);
    delay_us(10);
    HAL_GPIO_WritePin(trigPort, trigPin, GPIO_PIN_RESET);

    /* 확인용 메시지 (원한다면 UART 출력) */
    HAL_UART_Transmit(&huart2, (uint8_t*)"Trig sent\r\n", 11, HAL_MAX_DELAY);

    /* Echo 신호 대기 (Timeout 처리 포함) */
    uint32_t timeout = 100000;
    while (HAL_GPIO_ReadPin(echoPort, echoPin) == GPIO_PIN_RESET && timeout--) ;
    if (timeout == 0)
    {
        HAL_UART_Transmit(&huart2, (uint8_t*)"Echo timeout\r\n", 14, HAL_MAX_DELAY);
        return 0;
    }

    uint32_t start = __HAL_TIM_GET_COUNTER(&htim1);

    timeout = 100000;
    while (HAL_GPIO_ReadPin(echoPort, echoPin) == GPIO_PIN_SET && timeout--) ;
    if (timeout == 0)
    {
        HAL_UART_Transmit(&huart2, (uint8_t*)"Echo stuck HIGH\r\n", 18, HAL_MAX_DELAY);
        return 0;
    }

    uint32_t end = __HAL_TIM_GET_COUNTER(&htim1);
    uint32_t duration = (end >= start) ? (end - start) : (0xFFFF - start + end);
    /* 음파 속도: 0.034 cm/µs, 왕복이므로 /2 */
    uint32_t distance_cm = duration * 0.034 / 2;

    return distance_cm;
}

/* FreeRTOS Task: Sensor 1 (Trig: PA9, Echo: PA1) */
void UltrasonicTask1(void *argument)
{
    char msg[64];
    for (;;)
    {
        uint32_t distance = read_ultrasonic_distance_cm(GPIOA, GPIO_PIN_9, GPIOA, GPIO_PIN_1);
        snprintf(msg, sizeof(msg), "Sensor1: %lu cm\r\n", distance);
        HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(1000));  // 1초마다 측정
    }
}

/* FreeRTOS Task: Sensor 2 (Trig: PB0, Echo: PB1) */
void UltrasonicTask2(void *argument)
{
    char msg[64];
    for (;;)
    {
        uint32_t distance = read_ultrasonic_distance_cm(GPIOB, GPIO_PIN_0, GPIOB, GPIO_PIN_1);
        snprintf(msg, sizeof(msg), "Sensor2: %lu cm\r\n", distance);
        HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(1000));  // 1초마다 측정
    }
}

/* FreeRTOS Task: Sensor 3 (Trig: PC7, Echo: PC8) */
void UltrasonicTask3(void *argument)
{
    char msg[64];
    for (;;)
    {
        uint32_t distance = read_ultrasonic_distance_cm(GPIOC, GPIO_PIN_7, GPIOC, GPIO_PIN_8);
        snprintf(msg, sizeof(msg), "Sensor3: %lu cm\r\n", distance);
        HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(1000));  // 1초마다 측정
    }
}

/* main 함수 */
int main(void)
{
    /* HAL 초기화 및 시스템 클럭 설정 */
    HAL_Init();
    SystemClock_Config();

    /* Peripheral 초기화 */
    MX_GPIO_Init();
    MX_USART2_UART_Init();
    MX_TIM1_Init();
    // FreeRTOS 초기화 (CMSIS-OS 설정에 따라 자동 생성된 파일 포함)

    /* 타이머 시작 (delay_us()에서 사용) */
    HAL_TIM_Base_Start(&htim1);

    /* FreeRTOS Task 생성 */
    xTaskCreate(UltrasonicTask1, "UltraTask1", 128, NULL, 1, NULL);
    xTaskCreate(UltrasonicTask2, "UltraTask2", 128, NULL, 1, NULL);
    xTaskCreate(UltrasonicTask3, "UltraTask3", 128, NULL, 1, NULL);

    /* 스케줄러 시작 */
    vTaskStartScheduler();

    /* 여기로 절대 도달하지 않음 */
    while (1) {}
}

/* System Clock 설정 (사용 중인 MCU에 맞게 조정) */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = 16;
    RCC_OscInitStruct.PLL.PLLN = 336;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
    RCC_OscInitStruct.PLL.PLLQ = 2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
}

void Error_Handler(void)
{
    __disable_irq();
    while (1) {}
}
