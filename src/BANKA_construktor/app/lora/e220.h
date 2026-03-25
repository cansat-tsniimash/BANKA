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
	E220_SERIAL_PARITY_BIT_8N1 = 3,
}e220_serial_parity_bit;

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
}e220_serial_port_rate;


//E220_SERIAL_PARITY_BIT = 4,
	///E220_SERIAL_PORT_RATE =  7,///

#endif /* LORA_E220_H_ */
