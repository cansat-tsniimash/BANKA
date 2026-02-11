/*
 * ds18b20.c
 *
 *  Created on: Feb 7, 2026
 *      Author: nikit
 */


#include "stm32f1xx.h"
#include "main.h"
#include "delay/dwt_delay.h"
void one_wire_force_down()
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = ds18b20_Pin;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	HAL_GPIO_Init(ds18b20_GPIO_Port, &GPIO_InitStruct);
	HAL_GPIO_WritePin(ds18b20_GPIO_Port, ds18b20_Pin, GPIO_PIN_RESET);
}


void one_wire_force_release()
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = ds18b20_Pin;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	HAL_GPIO_Init(ds18b20_GPIO_Port, &GPIO_InitStruct);
	HAL_GPIO_WritePin(ds18b20_GPIO_Port, ds18b20_Pin, GPIO_PIN_RESET);
}

void one_vire_write_bit(uint8_t bit)
{
	if (bit == 0)
	{
		one_wire_force_down();
		dwt_delay_us(60);
		one_wire_force_release();

	}
	else
	{
		one_wire_force_down();
		dwt_delay_us(1);
		one_wire_force_release();
		dwt_delay_us(59);
	}
}


uint8_t one_wire_read_bit(void)
{
    uint8_t bit;

    one_wire_force_down();
    dwt_delay_us(1);
    one_wire_force_release();

    dwt_delay_us(14);
    bit = HAL_GPIO_ReadPin(ds18b20_GPIO_Port, ds18b20_Pin);
    dwt_delay_us(45);

    return bit;
}

uint8_t one_wire_read_byte(void)
{
    uint8_t byte = 0;

    for (uint8_t i = 0; i < 8; i++)
    {
        byte |= (one_wire_read_bit() << i);
    }

    return byte;
}
