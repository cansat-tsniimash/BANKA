/*
 * bmp280.c
 *
 *  Created on: Feb 25, 2026
 *      Author: nikit
 */
#include "stm32f1xx.h"
#include "delay/dwt_delay.h"
#include "bmp280.h"




BME280_INTF_RET_TYPE bmp280_read_reg (uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
	bmp280_bus_t *ptr = (bmp280_bus_t *) intf_ptr;
	HAL_StatusTypeDef read = HAL_I2C_Master_Transmit(ptr->hi2c, ptr->ADDR, &reg_addr, 1, 100);
	if (read != HAL_OK)
		return read;
	read = HAL_I2C_Master_Receive(ptr->hi2c, ptr->ADDR, reg_data, len, 150);
	if (read != HAL_OK)
		return read;
	return HAL_OK;
}

BME280_INTF_RET_TYPE bmp280_write_reg (uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
	bmp280_bus_t *ptr = (bmp280_bus_t *) intf_ptr;
	uint8_t BMP_MASSIV[2];
	HAL_StatusTypeDef read;

	for( uint32_t i = 0; i < len; i++ )
	{
		BMP_MASSIV [0] = reg_addr + i;
		BMP_MASSIV [1] = reg_data[i];
		read = HAL_I2C_Master_Transmit(ptr->hi2c, ptr->ADDR, BMP_MASSIV, 2, 100);
		if (read != HAL_OK)
		{
			return read;
		}
	}
	return HAL_OK;
}

void bmp280_delay_us (uint32_t period, void *intf_ptr)
{
	dwt_delay_us(period);
}
