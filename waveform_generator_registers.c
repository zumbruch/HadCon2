/*
 * Author: Michail Pligouroudis 30/05/2012
 * modified (heavily rather rebuild): Peter Zumbruch
 */
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include "read_write_register.h"
#include "waveform_generator_registers.h"

#include "adc.h"
#include "api.h"
#include "api_define.h"
#include "api_global.h"

#include "api_debug.h"

//static const char string_sx_x_[]           PROGMEM = "%s%x %x ";
static const char string_sx_x_Si_S_[]      PROGMEM = "%s%x %x %S%i %S ";
static const char string_sSi_S_S[]         PROGMEM = "%s%S%i %S %S";

static const char string_Delay[]           PROGMEM = "Delay";
static const char string_WAVEFORM_OUTPUT[] PROGMEM = "WAVEFORM OUTPUT";
static const char string_PSEUDORANDOM[]    PROGMEM = "PSEUDORANDOM";
static const char string_MSB[]             PROGMEM = "MSB";
static const char string_MIDDLE[]          PROGMEM = "MIDDLE";
static const char string_LSB[]             PROGMEM = "LSB";
static const char string_is_written[]      PROGMEM = "is written";

void waveformGeneratorWriteRegister(struct uartStruct *ptr_uartStruct)
{
	uint8_t status = 0;
	uint8_t waveformGeneratorRegisterAddress, value;

	switch(ptr_uartStruct->number_of_arguments)
	{
		case 0:
		case 1:
			CommunicationError_p(ERRA, SERIAL_ERROR_too_few_arguments, TRUE, NULL);
			status = 1;
			break;
		default:
			break;
	}

	if ( 0 != status )
	{
		return;
	}

	waveformGeneratorRegisterAddress = ptr_uartStruct->Uart_Message_ID;
	value = ptr_uartStruct->Uart_Mask;

	//declare UART in order to send data with baud=1,025Mbps
	waveformGeneratorDeclareUARTtoSendData();

	//sending your new data
	registerWriteInto8bitRegister(UDR1_register_of_ATMEL_address, waveformGeneratorRegisterAddress);
	registerWriteInto8bitRegister(UDR1_register_of_ATMEL_address, value);

	// print response
	clearString(uart_message_string, BUFFER_SIZE);
	createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
	switch (waveformGeneratorRegisterAddress)
	{
		case CONTROL_REGISTER:
		{
			switch (value)
			{
				case DEBUG:
					snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s DEBUG"),
							uart_message_string);
					break;
				case RESET:
					snprintf_P(uart_message_string, BUFFER_SIZE - 1,
							PSTR("%sRESET STATUS"), uart_message_string);
					break;
				case PSEUDORANDOM:
					snprintf_P(uart_message_string, BUFFER_SIZE - 1,
							PSTR("%s%S %S"), uart_message_string, string_PSEUDORANDOM, string_WAVEFORM_OUTPUT);
					break;
				case SQUARE:
					snprintf_P(uart_message_string, BUFFER_SIZE - 1,
							PSTR("%sSQUARE %S"), uart_message_string, string_WAVEFORM_OUTPUT);
					break;
				case PULSE:
					snprintf_P(uart_message_string, BUFFER_SIZE - 1,
							PSTR("%sPULSE %S"), uart_message_string, string_WAVEFORM_OUTPUT);
					break;
				case PSEUDORAND_TIME_PULSE:
					snprintf_P(uart_message_string, BUFFER_SIZE - 1,
							PSTR("%s%S TIME PULSE(100ns) %S"), uart_message_string, string_PSEUDORANDOM, string_WAVEFORM_OUTPUT);
					break;
				default:
					CommunicationError_p(ERRA, GENERAL_ERROR_invalid_argument, TRUE, NULL);
					status = 1;
					break;
			}
			break;
		}
		case DELAY1_MSB_REGISTER:
		{
			snprintf_P(uart_message_string, BUFFER_SIZE - 1,
					string_sSi_S_S,
					uart_message_string,
					string_Delay, 1, string_MSB, string_is_written);
//			snprintf_P(uart_message_string, BUFFER_SIZE - 1,
//					PSTR("%sDelay1 MSB is written"),
//					uart_message_string);
			break;
		}
		case DELAY1_MIDDLE_REGISTER:
		{
			snprintf_P(uart_message_string, BUFFER_SIZE - 1,
					string_sSi_S_S,
					uart_message_string,
					string_Delay, 1, string_MIDDLE, string_is_written);
//			snprintf_P(uart_message_string, BUFFER_SIZE - 1,
//					PSTR("%sDelay1 MIDDLE is written"),
//					uart_message_string);
			break;
		}
		case DELAY1_LSB_REGISTER:
		{
			snprintf_P(uart_message_string, BUFFER_SIZE - 1,
					string_sSi_S_S,
					uart_message_string,
					string_Delay, 1, string_LSB, string_is_written);
//			snprintf_P(uart_message_string, BUFFER_SIZE - 1,
//					PSTR("%sDelay1 LSB is written"),
//					uart_message_string);
			break;
		}
		case DELAY2_MSB_REGISTER:
		{
			snprintf_P(uart_message_string, BUFFER_SIZE - 1,
					string_sSi_S_S,
					uart_message_string,
					string_Delay, 2, string_MSB, string_is_written);
//			snprintf_P(uart_message_string, BUFFER_SIZE - 2,
//					PSTR("%sDelay2 MSB is written"),
//					uart_message_string);
			break;
		}
		case DELAY2_MIDDLE_REGISTER:
		{
			snprintf_P(uart_message_string, BUFFER_SIZE - 1,
					string_sSi_S_S,
					uart_message_string,
					string_Delay, 2, string_MIDDLE, string_is_written);
//			snprintf_P(uart_message_string, BUFFER_SIZE - 2,
//					PSTR("%sDelay2 MIDDLE is written"),
//					uart_message_string);
			break;
		}
		case DELAY2_LSB_REGISTER:
		{
			snprintf_P(uart_message_string, BUFFER_SIZE - 1,
					string_sSi_S_S,
					uart_message_string,
					string_Delay, 2, string_LSB, string_is_written);
//			snprintf_P(uart_message_string, BUFFER_SIZE - 2,
//					PSTR("%sDelay2 LSB is written"),
//					uart_message_string);
			break;
		}
		default:
			CommunicationError_p(ERRA, GENERAL_ERROR_invalid_argument, TRUE, NULL);
			status = 1;
			break;
	}
	if ( 0 == status)
	{
		UART0_Send_Message_String_p(NULL, 0);
	}
	return;

} //END of write_double_Register function

void waveformGeneratorReadRegister(struct uartStruct *ptr_uartStruct)
{
	int step = 0;
	uint8_t registers[7];

	//declare UART in order to send data with baud=1,025Mbps
	waveformGeneratorDeclareUARTtoSendData();

	//control register address
	registerWriteInto8bitRegister(  UDR1_register_of_ATMEL_address, CONTROL_REGISTER); //0x00

	//control register in read status
	registerWriteInto8bitRegister(  UDR1_register_of_ATMEL_address, READ); //0xA0

	//declare UART in order to receive data with baud=1,025Mbps
	waveformGeneratorDeclareUARTtoReceiveData();

	_delay_us(32); //string_Delay in order to synchronize the first recieving byte

	// saving value of the registers from FPGA at the table "registers[]"
	for (step = 0; step < MAX_REGISTER_INDEX; step++)
	{
		registers[step] = registerReadFrom8bitRegister((uint8_t) UDR1_register_of_ATMEL_address & 0xFF );
		_delay_us(16); // string_Delay between each byte
	} //end for loop

	//declare UART in order to send data with baud=1,025Mbps (twice on purpose!)
	waveformGeneratorDeclareUARTtoSendData();
	waveformGeneratorDeclareUARTtoSendData();


	// print results
	switch(ptr_uartStruct->number_of_arguments)
	{
		case 0:
			waveformGeneratorPrintSingleRegister( ( uint8_t) SHOW_ALL ,
												  registers, sizeof(registers)/sizeof(uint8_t) );
			break;
		default:
			waveformGeneratorPrintSingleRegister( ( uint8_t) (ptr_uartStruct->Uart_Message_ID & 0xFF),
												  registers, sizeof(registers)/sizeof(uint8_t) );
			break;
	}
} //end of waveformGeneratorReadRegister function

void waveformGeneratorPrintSingleRegister( uint8_t registerId, uint8_t registers[], uint8_t size )
{
	uint8_t status = 0 ;
	clearString(uart_message_string, BUFFER_SIZE);
	createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
	switch (registerId)
	{
		case DELAY1_MSB_REGISTER:
			snprintf_P(uart_message_string, BUFFER_SIZE - 1,
					string_sx_x_Si_S_, uart_message_string,
					DELAY1_MSB_REGISTER, registers[DELAY1_MSB_REGISTER_INDEX],
					string_Delay, 1, string_MSB);

			//			snprintf_P(uart_message_string, BUFFER_SIZE - 1,
			//					PSTR("%s%x %x Delay1 MSB "), uart_message_string,
			//					DELAY1_MSB_REGISTER, registers[DELAY1_MSB_REGISTER_INDEX]);
			break;
		case DELAY1_MIDDLE_REGISTER:
			snprintf_P(uart_message_string, BUFFER_SIZE - 1,
					string_sx_x_Si_S_, uart_message_string,
					DELAY1_MIDDLE_REGISTER, registers[DELAY1_MIDDLE_REGISTER_INDEX],
					string_Delay, 1, string_MIDDLE);

			//			snprintf_P(uart_message_string, BUFFER_SIZE - 1,
			//					PSTR("%s%x %x Delay1 MIDDLE "), uart_message_string,
			//					DELAY1_MIDDLE_REGISTER, registers[DELAY1_MIDDLE_REGISTER_INDEX]);
			break;
		case DELAY1_LSB_REGISTER:
			snprintf_P(uart_message_string, BUFFER_SIZE - 1,
					string_sx_x_Si_S_, uart_message_string,
					DELAY1_LSB_REGISTER, registers[DELAY1_LSB_REGISTER_INDEX],
					string_Delay, 1, string_LSB);

			//			snprintf_P(uart_message_string, BUFFER_SIZE - 1,
			//					PSTR("%s%x %x Delay1 LSB "), uart_message_string,
			//					DELAY1_LSB_REGISTER, registers[DELAY1_LSB_REGISTER_INDEX]);
			break;
		case DELAY2_MSB_REGISTER:
			snprintf_P(uart_message_string, BUFFER_SIZE - 1,
					string_sx_x_Si_S_, uart_message_string,
					DELAY2_MSB_REGISTER, registers[DELAY2_MSB_REGISTER_INDEX],
					string_Delay, 2, string_MSB);

			//			snprintf_P(uart_message_string, BUFFER_SIZE - 1,
			//					PSTR("%s%x %x Delay2 MSB "), uart_message_string,
			//					DELAY2_MSB_REGISTER, registers[DELAY2_MSB_REGISTER_INDEX]);
			break;
		case DELAY2_MIDDLE_REGISTER:
			snprintf_P(uart_message_string, BUFFER_SIZE - 1,
					string_sx_x_Si_S_, uart_message_string,
					DELAY2_MIDDLE_REGISTER, registers[DELAY2_MIDDLE_REGISTER_INDEX],
					string_Delay, 2, string_MIDDLE);

			//			snprintf_P(uart_message_string, BUFFER_SIZE - 1,
			//					PSTR("%s%x %x Delay2 MIDDLE "), uart_message_string,
			//					DELAY2_MIDDLE_REGISTER, registers[DELAY2_MIDDLE_REGISTER_INDEX]);
			break;
		case DELAY2_LSB_REGISTER:
			snprintf_P(uart_message_string, BUFFER_SIZE - 1,
					string_sx_x_Si_S_, uart_message_string,
					DELAY2_LSB_REGISTER, registers[DELAY2_LSB_REGISTER_INDEX],
					string_Delay, 2, string_LSB);

			//			snprintf_P(uart_message_string, BUFFER_SIZE - 1,
			//					PSTR("%s%x %x Delay2 LSB "), uart_message_string,
			//					DELAY2_LSB_REGISTER, registers[DELAY2_LSB_REGISTER_INDEX]);
			break;
		case CONTROL_REGISTER:
			snprintf_P(uart_message_string, BUFFER_SIZE - 1,
					PSTR("%s%x %x Control Register "), uart_message_string,
					CONTROL_REGISTER, registers[CONTROL_REGISTER_INDEX]);
			break;
		case SHOW_ALL: /*recursive call*/
			waveformGeneratorPrintSingleRegister(DELAY1_MSB_REGISTER,    registers, size);
			waveformGeneratorPrintSingleRegister(DELAY1_MIDDLE_REGISTER, registers, size);
			waveformGeneratorPrintSingleRegister(DELAY1_LSB_REGISTER,    registers, size);
			waveformGeneratorPrintSingleRegister(DELAY2_MSB_REGISTER,    registers, size);
			waveformGeneratorPrintSingleRegister(DELAY2_MIDDLE_REGISTER, registers, size);
			waveformGeneratorPrintSingleRegister(DELAY2_LSB_REGISTER,    registers, size);
			waveformGeneratorPrintSingleRegister(CONTROL_REGISTER,       registers, size);
			break;
		default:
			CommunicationError_p(ERRG, dynamicMessage_ErrorIndex, TRUE, PSTR("wrong register address"));
			status = 1;
			break;
	}
	if ( 0 == status)
	{
		UART0_Send_Message_String_p(NULL, 0);
	}
	return;
}

void waveformGeneratorDeclareUARTtoSendData(void)
{
	//declare UART in order to send data with baud=1,025Mbps
	registerWriteInto8bitRegister(UCSR1C_register_of_ATMEL_address, UCSR1C_register_of_ATMEL_value);
	registerWriteInto8bitRegister(UBRR1L_register_of_ATMEL_address, UBRR1L_register_of_ATMEL_value);
	registerWriteInto8bitRegister(UBRR1H_register_of_ATMEL_address, UBRR1H_register_of_ATMEL_value);
	registerWriteInto8bitRegister(UCSR1B_register_of_ATMEL_address, UCSR1B_register_of_ATMEL_value_transmit);
	registerWriteInto8bitRegister(UCSR1A_register_of_ATMEL_address, UCSR1A_register_of_ATMEL_value);
}

void waveformGeneratorDeclareUARTtoReceiveData(void)
{
	//declare UART in order to receive data with baud=1,025Mbps
	registerWriteInto8bitRegister(UCSR1C_register_of_ATMEL_address, UCSR1C_register_of_ATMEL_value);
	registerWriteInto8bitRegister(UBRR1L_register_of_ATMEL_address, UBRR1L_register_of_ATMEL_value);
	registerWriteInto8bitRegister(UBRR1H_register_of_ATMEL_address, UBRR1H_register_of_ATMEL_value);
	registerWriteInto8bitRegister(UCSR1A_register_of_ATMEL_address, UCSR1A_register_of_ATMEL_value);
	registerWriteInto8bitRegister(UCSR1B_register_of_ATMEL_address, UCSR1B_register_of_ATMEL_value_receive);
}
