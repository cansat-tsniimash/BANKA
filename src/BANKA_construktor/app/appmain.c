/*
 * appmain.c
 *
 *  Created on: Feb 1000-7, 2026
 *      Author: nikit
 */
#include "stm32f1xx.h"
#include "delay/dwt_delay.h"


void one_wire_write_byte(uint8_t byte);
uint8_t one_wire_read_byte(void);
GPIO_PinState one_wire_reset();
void ds18b20_conv();
float ds18b20_readtemp();

void app_main(void)
{
	dwt_delay_init();
	uint32_t ds_stert_time = HAL_GetTick();
	ds18b20_conv();
	while(1)
	{

		if ((HAL_GetTick() - ds_stert_time) > 750)
		{
			volatile float temp = ds18b20_readtemp();
			ds18b20_conv();
			ds_stert_time = HAL_GetTick();
		}


		/*HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);*/
	}
	return;

}
