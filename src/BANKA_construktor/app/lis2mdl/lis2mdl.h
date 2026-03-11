/*
 * lis2mdl.h
 *
 *  Created on: Mar 7, 2026
 *      Author: nikit
 */

#ifndef LIS2MDL_LIS2MDL_H_
#define LIS2MDL_LIS2MDL_H_

#include "lis2mdl_reg.h"
#include "stm32f1xx.h"

typedef struct lis_data{
	uint8_t ADDR;
	I2C_HandleTypeDef *hi2c;
}lis_data_t;

int32_t lis_write_reg (void *handle, uint8_t sub, uint8_t *data, uint16_t len);
int32_t lis_read_reg (void *handle, uint8_t sub, uint8_t *data, uint16_t len);

#endif /* LIS2MDL_LIS2MDL_H_ */
