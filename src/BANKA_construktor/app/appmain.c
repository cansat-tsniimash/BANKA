/*
 * appmain.c
 *
 *  Created on: Feb 1000-7, 2026
 *      Author: nikit
 */
#include "stm32f1xx.h"
#include "delay/dwt_delay.h"
#include "neo6mv2/neo6mv2.h"


extern UART_HandleTypeDef huart1;

void app_main(void)
{
	/*dwt_delay_init();
	uint32_t ds_stert_time = HAL_GetTick();
	ds18b20_conv();*/

	__HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_ERR);
	neo6mv2_Init();
	GPS_Data gps_data;
	while(1)
	{
		for (int i = 0; i <= 10; i++)
		{
			if (neo6mv2_work() == 1)
				break;
		}
		gps_data = neo6mv2_GetData();
		printf("cookie = %d\n", gps_data.cookie);
		printf("%f %f\n",gps_data.latitude, gps_data.longitude);
		printf("%d\n",gps_data.fixQuality);
		//if ((HAL_GetTick() - ds_stert_time) > 750)
		//{
		//	volatile float temp = ds18b20_readtemp();
		//	ds18b20_conv();
		//	ds_stert_time = HAL_GetTick();
		//}




		/*HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);*/
	}
	return;

}
