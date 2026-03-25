/*
 * e220.h
 *
 *  Created on: Mar 18, 2026
 *      Author: nikit
 */

#ifndef LORA_E220_H_
#define LORA_E220_H_

#include "stm32f1xx.h"

typedef struct
{
	UART_HandleTypeDef *uart;
	GPIO_TypeDef *m0_port;
	GPIO_TypeDef *m1_port;
	uint16_t m0_pin;
	uint16_t m1_pin;
	GPIO_TypeDef *aux_port;
	uint16_t aux_pin;

} e220_connect_t;

typedef enum
{
	E220_MODE_TM = 0,
	E220_MODE_WOR = 1,
	E220_MODE_WORRC = 2,
	E220_MODE_DSM = 3,
}lora_mode_t;

typedef enum
{
	E220_AIR_RATE_2P4 = 2,
	E220_AIR_RATE_4P8 = 3,
	E220_AIR_RATE_9P6 = 4,
	E220_AIR_RATE_19P2 = 5,
	E220_AIR_RATE_38P4 = 6,
	E220_AIR_RATE_62P5 = 7,
}e220_air_rate_t;

typedef enum
{
	E220_SERIAL_PARITY_BIT_8N1 = 0,
	E220_SERIAL_PARITY_BIT_8O1 = 1,
	E220_SERIAL_PARITY_BIT_8E1 = 2,
}e220_serial_parity_bit_t;

typedef enum
{
	E220_SERIAL_PORT_RATE_1200 = 0,
	E220_SERIAL_PORT_RATE_2400 = 1,
	E220_SERIAL_PORT_RATE_4800 = 2,
	E220_SERIAL_PORT_RATE_9600 = 3,
	E220_SERIAL_PORT_RATE_19200 = 4,
	E220_SERIAL_PORT_RATE_38400 = 5,
	E220_SERIAL_PORT_RATE_57600 = 6,
	E220_SERIAL_PORT_RATE_115200 = 7,
}e220_serial_port_rate_t;

typedef enum
{
	E220_SUB_PACKET_SETTING_200B = 0,
	E220_SUB_PACKET_SETTING_128B = 1,
	E220_SUB_PACKET_SETTING_64B = 2,
	E220_SUB_PACKET_SETTING_32B = 3,
}e220_sub_packet_setting_t;

typedef enum
{
	E220_RSSI_AMBIENT_NOICE_ENABLE = 0,
	E220_RSSI_AMBIENT_NOICE_DISABLE = 1,
}e220_rssi_ambient_noice_mode_t;

typedef enum
{
	E220_TRANSMITTING_POWER_22DBM = 0,
	E220_TRANSMITTING_POWER_17DBM = 1,
	E220_TRANSMITTING_POWER_13DBM = 2,
	E220_TRANSMITTING_POWER_10DBM = 3,
}e220_transmitting_power_t;

void e220_set_channel(e220_connect_t* lora, uint8_t ch);
void e220_set_add(e220_connect_t* lora, uint16_t addr);
void e220_reg_0(e220_connect_t* bus, e220_air_rate_t adr, e220_serial_parity_bit_t bdr, e220_serial_port_rate_t cdr);
void e220_reg_1(e220_connect_t* bus, e220_sub_packet_setting_t ddr, e220_rssi_ambient_noice_mode_t edr, e220_transmitting_power_t fdr);
void e220_mode_switch (e220_connect_t* lora, lora_mode_t mode);
//E220_SERIAL_PARITY_BIT = 4,
	///E220_SERIAL_PORT_RATE =  7,///

#endif /* LORA_E220_H_ */
