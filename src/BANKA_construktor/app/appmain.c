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
#include "lsm6ds3/lsm6ds3.h"
#include "lis2mdl/lis2mdl.h"
#include "../Middlewares/Third_Party/FatFs/src/ff.h"

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

	lsm_data_t lsm_bus;
	lsm_bus.ADDR = 0x6A << 1;
	lsm_bus.hi2c = &hi2c1;

	stmdev_ctx_t lsm6ds3;
	lsm6ds3.handle = &lsm_bus;
	lsm6ds3.write_reg = lsm_write_reg;
	lsm6ds3.read_reg = lsm_read_reg;

	lsm6ds3_reset_set(&lsm6ds3, 1);
	lsm6ds3_xl_full_scale_set(&lsm6ds3, LSM6DS3_16g);
	lsm6ds3_xl_data_rate_set(&lsm6ds3, LSM6DS3_XL_ODR_104Hz);
	lsm6ds3_gy_full_scale_set(&lsm6ds3, LSM6DS3_2000dps);
	lsm6ds3_gy_data_rate_set(&lsm6ds3, LSM6DS3_GY_ODR_208Hz);

	int16_t buf_lsm_gy[3] = {0};
	int16_t buf_lsm_xl[3] = {0};
	volatile float gyro[3] = {0};
	volatile float acc[3] = {0};

	lis_data_t lis_bus;
	lis_bus.ADDR = 0x1E << 1;
	lis_bus.hi2c = &hi2c1;

	lis2mdl_ctx_t lis;
	lis.handle = &lis_bus;
	lis.write_reg = lis_write_reg;
	lis.read_reg = lis_read_reg;

	lis2mdl_reset_set(&lis, 1);
	lis2mdl_operating_mode_set(&lis, LIS2MDL_CONTINUOUS_MODE);
	lis2mdl_data_rate_set(&lis, LIS2MDL_ODR_50Hz);
	lis2mdl_power_mode_set(&lis, LIS2MDL_HIGH_RESOLUTION);


	int16_t buf_lis[3] = {0};
	volatile float magn[3] = {0};

	FATFS sd;
	FIL pocket1;
	char pocket1_path[] = "pocket1.bin";
	FRESULT result_mount = f_mount(&sd, "", 1);
	FRESULT rezult_pocket1 = 255;
	UINT byte_count;

	if (result_mount == FR_OK)
	{
		f_open(&pocket, &pocket1_path, FA_WRITE);
	}
	if (rezult_pocket1 == FR_OK);
	{
		f_write(&pocket1, buff, btw, &byte_count);
	}



	while(1)
	{
		bme280_get_sensor_data(BME280_TEMP | BME280_PRESS, &bmp_data, &bmp280);

		lsm6ds3_acceleration_raw_get(&lsm6ds3, buf_lsm_xl);
		lsm6ds3_angular_rate_raw_get(&lsm6ds3, buf_lsm_gy);

		for (int i = 0; i < 3; i++)
		{
			acc[i] = lsm6ds3_from_fs16g_to_mg(buf_lsm_xl[i]) /1000.0;
			gyro[i] = lsm6ds3_from_fs125dps_to_mdps(buf_lsm_gy[i]) /1000.0;
		}

		lis2mdl_magnetic_raw_get(&lis, buf_lis);
		for (int i = 0; i < 3; i++)
		{
			magn[i] = lis2mdl_from_lsb_to_mgauss(buf_lis[i]) /1000.0;
		}

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

