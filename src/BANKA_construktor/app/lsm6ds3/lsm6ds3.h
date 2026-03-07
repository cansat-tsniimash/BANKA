/*
 * lsm6ds3.h
 *
 *  Created on: Mar 4, 2026
 *      Author: nikit
 */

#ifndef LSM6DS3_LSM6DS3_H_
#define LSM6DS3_LSM6DS3_H_
#include "lsm6ds3_reg.h"
#include "stm32f1xx.h"

typedef struct lsm_data{
	uint8_t ADDR;
	I2C_HandleTypeDef *hi2c;
}lsm_data_t;

int32_t lsm_write_reg (void *handle, uint8_t sub, const uint8_t *data, uint16_t len);
int32_t lsm_read_reg (void *handle, uint8_t sub, uint8_t *data, uint16_t len);

#endif /* LSM6DS3_LSM6DS3_H_ */
