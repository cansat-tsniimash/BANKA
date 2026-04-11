/*
 * lsm6ds3.c
 *
 *  Created on: Mar 4, 2026
 *      Author: nikit
 */
#include "stm32f1xx.h"
#include "lsm6ds3.h"
#include "i2c-crutch/i2c-crutch.h"

int32_t lsm_write_reg (void *handle, uint8_t sub, const uint8_t *data, uint16_t len)
{
	lsm_data_t *ptr = (lsm_data_t *)handle;
	uint8_t LSM_MASSIV[2];
	HAL_StatusTypeDef read;

	for( uint32_t i = 0; i < len; i++ )
	{
		LSM_MASSIV [0] = sub + i;
		LSM_MASSIV [1] = data[i];
		read = HAL_I2C_Master_Transmit(ptr->hi2c, ptr->ADDR, LSM_MASSIV, 2, 100);
		if (read != HAL_OK)
		{
			if (read == HAL_BUSY)
			{
				I2C_ClearBusyFlagErratum(ptr->hi2c, 100);
			}
			return read;
		}
	}
	return HAL_OK;
}

int32_t lsm_read_reg (void *handle, uint8_t sub, uint8_t *data, uint16_t len)
{
	lsm_data_t *ptr = (lsm_data_t *)handle;
	HAL_StatusTypeDef read = HAL_I2C_Master_Transmit (ptr->hi2c, ptr->ADDR, &sub, 1, 100);
	if (read != HAL_OK)
	{
		if (read == HAL_BUSY)
		{
			I2C_ClearBusyFlagErratum(ptr->hi2c, 100);
		}
		return read;
	}
	read = HAL_I2C_Master_Receive (ptr->hi2c, ptr->ADDR, data, len, 150);
	if (read != HAL_OK)
	{
		if (read == HAL_BUSY)
		{
			I2C_ClearBusyFlagErratum(ptr->hi2c, 100);
		}
		return read;
	}
	return HAL_OK;
}
