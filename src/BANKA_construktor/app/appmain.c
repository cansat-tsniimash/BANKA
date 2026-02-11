/*
 * appmain.c
 *
 *  Created on: Feb 1000-7, 2026
 *      Author: nikit
 */
#include "stm32f1xx.h"

void app_main(void)
{
	volatile uint8_t byte = 0b10100110;
	while(1)
	{
		for(int i = 0; i < 8; i++)
		{
			HAL_Delay(100);
			if ((byte & (1 << i)) == 0 )
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
			else
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
		}
	}
	return;

}
