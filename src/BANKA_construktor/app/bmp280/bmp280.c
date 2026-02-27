/*
 * bmp280.c
 *
 *  Created on: Feb 25, 2026
 *      Author: nikit
 */
#include "stm32f1xx.h"
#include "bme280_defs.h"

extern I2C_HandleTypeDef hi2c1;

#define BMP280_ADDR (0x76)


BME280_INTF_RET_TYPE bmp280_read_reg (uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
	HAL_I2C_Master_Transmit(&hi2c1, BMP280_ADDR, &reg_addr, 1, 100);
	HAL_I2C_Master_Receive(&hi2c1, BMP280_ADDR, reg_data, len, 100);

	return 0;
}




BME280_INTF_RET_TYPE bmp280_write_reg (uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr)
{

	uint8_t BMP_MASSIV [2] = {reg_addr, reg_data[0]};

	for( int i=0; i < len; i++ )
	{
		BMP_MASSIV [0] = reg_addr + i;
		BMP_MASSIV [1] = reg_data[i];
		HAL_I2C_Master_Seq_Transmit(&hi2c, BMP280_ADDR, BMP_MASSIV, 2, 100);
	return 0;
	}

}


