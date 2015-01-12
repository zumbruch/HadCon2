/*
   Licensed under the EUPL V.1.1, Lizenziert unter EUPL V.1.1 
*/
#ifndef WAVEFORM_GENERATOR_REGISTERS_H
#define WAVEFORM_GENERATOR_REGISTERS_H
#include "api_define.h"
#include "api_global.h"

#include "api_debug.h"
#include "api.h"

#define UCSR1C_register_of_ATMEL_address 0xCA
#define UBRR1L_register_of_ATMEL_address 0xCC
#define UBRR1H_register_of_ATMEL_address 0xCD
#define UCSR1B_register_of_ATMEL_address 0xC9
#define UCSR1A_register_of_ATMEL_address 0xC8
#define UDR1_register_of_ATMEL_address   0xCE

#define UCSR1C_register_of_ATMEL_value 0x06
#define UBRR1L_register_of_ATMEL_value 0x00
#define UBRR1H_register_of_ATMEL_value 0x00
#define UCSR1B_register_of_ATMEL_value_transmit 0x08
#define UCSR1B_register_of_ATMEL_value_receive 0x10
#define UCSR1A_register_of_ATMEL_value 0x22

#define CONTROL_REGISTER       0x00
#define DELAY1_MSB_REGISTER    0x01
#define DELAY1_MIDDLE_REGISTER 0x02
#define DELAY1_LSB_REGISTER    0x03
#define DELAY2_MSB_REGISTER    0x04
#define DELAY2_MIDDLE_REGISTER 0x05
#define DELAY2_LSB_REGISTER    0x06
#define SHOW_ALL               0xFF

#define DEBUG 0x00
#define RESET 0x40
#define READ  0xa0
#define PSEUDORANDOM 0x81
#define SQUARE 0x82
#define PULSE 0x83
#define PSEUDORAND_TIME_PULSE 0x84

enum registerIndex {
	DELAY1_MSB_REGISTER_INDEX = 0,
	DELAY1_MIDDLE_REGISTER_INDEX,
	DELAY1_LSB_REGISTER_INDEX,
	DELAY2_MSB_REGISTER_INDEX,
	DELAY2_MIDDLE_REGISTER_INDEX,
	DELAY2_LSB_REGISTER_INDEX,
	CONTROL_REGISTER_INDEX,
	MAX_REGISTER_INDEX
};

void waveformGeneratorReadRegister( struct uartStruct *ptr_uartStruct ); /*this function reads the status of a register with the of his adress */
void waveformGeneratorWriteRegister( struct uartStruct *ptr_uartStruct );/* this function writes a value the status in the register with the of his adress */
void waveformGeneratorPrintSingleRegister( uint8_t registerId, uint8_t registers[], uint8_t size );
void waveformGeneratorDeclareUARTtoSendData(void);
void waveformGeneratorDeclareUARTtoReceiveData(void);

#endif
