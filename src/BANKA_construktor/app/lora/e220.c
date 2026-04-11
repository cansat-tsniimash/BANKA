/*
 * e220.c
 *
 *  Created on: Mar 18, 2026
 *      Author: nikit
 */
#include "e220.h"
#include "stm32f1xx.h"

void e220_mode_switch (e220_connect_t* lora, lora_mode_t mode)
{
	switch(mode)
	{
	case E220_MODE_TM:
		HAL_GPIO_WritePin(lora->m0_port, lora->m0_pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(lora->m1_port, lora->m1_pin, GPIO_PIN_RESET);
		break;
	case E220_MODE_WOR:
		HAL_GPIO_WritePin(lora->m0_port, lora->m0_pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(lora->m1_port, lora->m1_pin, GPIO_PIN_RESET);
		break;
	case E220_MODE_WORRC:
		HAL_GPIO_WritePin(lora->m0_port, lora->m0_pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(lora->m1_port, lora->m1_pin, GPIO_PIN_SET);
		break;
	case E220_MODE_DSM:
		HAL_GPIO_WritePin(lora->m0_port, lora->m0_pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(lora->m1_port, lora->m1_pin, GPIO_PIN_SET);
		break;
	}
}
void e220_write_reg(e220_connect_t* lora, uint8_t addr, uint8_t *data, uint8_t size)
{
    uint8_t buff_reg[12] = {0};

    buff_reg[0] = 0xC0;
    buff_reg[1] = addr;
    buff_reg[2] = size;

    for (uint8_t i = 0; i < size; i++)
    {
        buff_reg[3 + i] = data[i];
    }
    HAL_UART_Transmit(lora->uart, buff_reg, size + 3, HAL_MAX_DELAY);
}

void e220_set_channel(e220_connect_t* lora, uint8_t ch)
{
    if (ch > 83)
        return;

    e220_write_reg(lora, 0x04, &ch, 1);
}

void e220_set_add(e220_connect_t* lora, uint16_t addr)
{
	uint8_t data[2];
	data[0] = (uint8_t)((addr >> 8) & 0xFF);
	data[1] = (uint8_t)(addr & 0xFF);
	e220_write_reg(lora, 0x00, data, 2);
}

void e220_reg_0(e220_connect_t* bus, e220_air_rate_t adr, e220_serial_parity_bit_t bdr, e220_serial_port_rate_t cdr)
{
	uint8_t data = 0;
	data = data | adr;
	data = data | (bdr << 3);
	data = data | (cdr << 5);
	e220_write_reg(bus, 0x02, &data, 1);
}

void e220_reg_1(e220_connect_t* bus, e220_sub_packet_setting_t ddr, e220_rssi_ambient_noice_mode_t edr, e220_transmitting_power_t fdr)
{
	uint8_t data = 0;
	data = data | fdr;
	data = data | (edr << 5);
	data = data | (ddr << 6);
	e220_write_reg(bus, 0x03, &data, 1);
}


void e220_send_packet(e220_connect_t *bus, uint8_t *reg_data, uint16_t len)
{
	uint16_t try = 0;
	HAL_UART_Transmit(bus->uart, reg_data, len, 100);
	while ( (HAL_GPIO_ReadPin(bus->aux_port, bus->aux_pin) == GPIO_PIN_RESET) && (try <20))
	{
		try++;
		HAL_Delay(1);
	}
}
