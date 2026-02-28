/*
 * appmain.c
 *
 *  Created on: Feb 1000-7, 2026
 *      Author: nikit
 */
#include "stm32f1xx.h"
#include "ds18b20/ds18b20.h"
#include "neo6mv2/neo6mv2.h"
#include "bmp280/bmp280.h"

extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart1;

#define BMP280_ADDR (0x76 << 1)

void app_main(void)
{
	ds18b20_init(DS18B20_12_BIT);
	uint32_t ds_start_time = HAL_GetTick();
	ds18b20_conv();

	__HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_ERR);
	neo6mv2_Init();
	GPS_Data gps_data;


	struct bme280_dev bmp280;
	bmp280.intf = BME280_I2C_INTF;
	bmp280.read = bmp280_read_reg;
	bmp280.write = bmp280_write_reg;
	bmp280.delay_us = bmp280_delay_us;

	bmp280.settings.osr_p = BME280_OVERSAMPLING_16X;
	bmp280.settings.osr_t = BME280_OVERSAMPLING_16X;
	bmp280.settings.filter = BME280_OVERSAMPLING_16X;
	bmp280.settings.standby_time = BME280_STANDBY_TIME_20_MS;

	bmp280_bus_t bmp_bus;
	bmp_bus.ADDR = 0x76 << 1;
	bmp_bus.hi2c = &hi2c1;
	bmp280.intf_ptr = &bmp_bus;

	bme280_init(&bmp280);
	bme280_set_sensor_settings(BME280_ALL_SETTINGS_SEL, &bmp280);
	bme280_set_sensor_mode(BME280_NORMAL_MODE, &bmp280);
	struct bme280_data bmp_data;


	while(1)
	{
		bme280_get_sensor_data(BME280_TEMP | BME280_PRESS, &bmp_data, &bmp280);

		for (int i = 0; i <= 10; i++)
		{
			if (neo6mv2_work() == 1)
				break;
		}
		gps_data = neo6mv2_GetData();
		printf("cookie = %d\n", gps_data.cookie);
		printf("%f %f\n",gps_data.latitude, gps_data.longitude);
		printf("%d\n",gps_data.fixQuality);

		if ((HAL_GetTick() - ds_start_time) > 750)
		{
			volatile float temp = ds18b20_readtemp();
			ds18b20_conv();
			ds_start_time = HAL_GetTick();
		}


		/*HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);*/
	}
	return;

}
