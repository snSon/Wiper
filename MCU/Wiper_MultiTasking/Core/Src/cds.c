/*
 * cds.c
 *
 *  Modified on: Apr 15, 2025
 *      Author: wldhks1959
 */


#include "cds.h"
#include "adc.h"

extern ADC_HandleTypeDef hadc1;

uint16_t ReadCDS(void)
{
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 10);
    uint16_t val = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    return val;
}
