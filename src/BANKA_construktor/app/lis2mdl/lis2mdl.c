/*
 * lis2mdl.c
 *
 *  Created on: Mar 7, 2026
 *      Author: nikit
 */


#include "stm32f1xx.h"
#include "lis2mdl.h"

int32_t lis_write_reg (void *handle, uint8_t sub, uint8_t *data, uint16_t len)
{
	lis_data_t *ptr = (lis_data_t *)handle;
	uint8_t LSM_MASSIV[2];
	HAL_StatusTypeDef read;

	for( uint32_t i = 0; i < len; i++ )
	{
		LSM_MASSIV [0] = sub + i;
		LSM_MASSIV [1] = data[i];
		read = HAL_I2C_Master_Transmit(ptr->hi2c, ptr->ADDR, LSM_MASSIV, 2, 100);
		if (read != HAL_OK)// TODO: Дописать проверку результата как в бмп
		{
			return read;
		}
	}
	return HAL_OK;
}

int32_t lis_read_reg (void *handle, uint8_t sub, uint8_t *data, uint16_t len)
{
	lis_data_t *ptr = (lis_data_t *)handle;
	HAL_StatusTypeDef read = HAL_I2C_Master_Transmit (ptr->hi2c, ptr->ADDR, &sub, 1, 100);
	if (read != HAL_OK)// TODO: Дописать проверку результата как в бмп
	{
		return read;
	}
	read = HAL_I2C_Master_Receive (ptr->hi2c, ptr->ADDR, data, len, 150);
	if (read != HAL_OK)// TODO: Дописать проверку результата как в бмп
	{
		return read;
	}
	return HAL_OK;
}

