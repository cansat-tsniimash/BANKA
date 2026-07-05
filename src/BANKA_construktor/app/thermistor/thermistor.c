/*
 * thermistor.c
 *
 *  Created on: Jun 12, 2026
 *      Author: nikit
 */
#include "stm32f1xx.h"
extern ADC_HandleTypeDef hadc1;

float thermistor_read_data()
{
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_SET);
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 100);
	uint32_t data = HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop(&hadc1);
	return 3.3 / 4095.0 * data;
}
