/*
 * ds18b20.c
 *
 *  Created on: Feb 7, 2026
 *      Author: nikit
 */


#include "ds18b20.h"

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

void one_wire_write_bit(uint8_t bit)
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

void one_wire_write_byte(uint8_t byte)
{
	for (int i = 0; i < 8; i++)
	{
		//one_wire_write_bit(); 0xFF << 4  >>  4 0xFF & 0b00010000 = 0b00010000 (1 << i)
		if ((byte & (1 << i)) == 0)
			one_wire_write_bit(0);
		else
			one_wire_write_bit(1);
	}
}

GPIO_PinState one_wire_read_bit(void)
{
	GPIO_PinState bit;

    one_wire_force_down();
    dwt_delay_us(1);
    one_wire_force_release();

    //dwt_delay_us(14);
    bit = HAL_GPIO_ReadPin(ds18b20_GPIO_Port, ds18b20_Pin);
    dwt_delay_us(60);

    return bit;
}

uint8_t one_wire_read_byte(void)
{
    uint8_t byte = 0;

    for (uint8_t i = 0; i < 8; i++)
    {
    	if (one_wire_read_bit() == GPIO_PIN_SET)
        	byte |= (1 << i);
    }

    return byte;
}

GPIO_PinState one_wire_reset()
{
	one_wire_force_down();
	dwt_delay_us(500);
	one_wire_force_release();

	dwt_delay_us(50);
	GPIO_PinState bit = HAL_GPIO_ReadPin(ds18b20_GPIO_Port, ds18b20_Pin);
	dwt_delay_us(450);
	return bit;
}

void ds18b20_conv()
{
	one_wire_reset();
	one_wire_write_byte(0xCC);
	one_wire_write_byte(0x44);
}

float ds18b20_readtemp()
{
	one_wire_reset();
	one_wire_write_byte(0xCC);
	one_wire_write_byte(0xBE);

	uint8_t buf[8] = {0};
	for (uint8_t i = 0; i < 8; i++)
	{
		buf[i] = one_wire_read_byte();
	}

	return (int16_t)((buf[1] << 8) | buf[0]) / 16.0;

}

void ds18b20_init(DS18B20_BIT_t conf)
{
	dwt_delay_init();
	one_wire_reset();
	one_wire_write_byte(0xCC);
	one_wire_write_byte(0x4E);
	one_wire_write_byte(0);
	one_wire_write_byte(0);
	one_wire_write_byte(conf);
}


