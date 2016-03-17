/*
   Licensed under the EUPL V.1.1, Lizenziert unter EUPL V.1.1
*/
/*
 * VERSION 1.0 Januar 11th 2015 LATE  File: 'api_dac.c'
 * Author: Martin Mitkov
 * Modified: March 2016, Peter Zumbruch, P.Zumbruch@gsi.de
 */

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <stddef.h>
#define __DELAY_BACKWARD_COMPATIBLE__
#include <util/delay.h>
#include <util/atomic.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/iocan128.h>
#include <avr/iocanxx.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h>

#include "api.h"
#include "api_debug.h"
#include "api_show.h"
#include "api_help.h"
#include "api_debug.h"
#include "twi_master.h"

#include "api_define.h"
#include "api_global.h"
#include "api.h"
#ifdef TESTING_ENABLE
#include "testing.h"
#endif
#include "api_dac.h"

/*
 Parameters:
 Port: 0-7,
 Vout: 0mV - 3300mV

 The aim of the function is to set up fast the DAC output.
 */

uint16_t DacPacket[8];

uint8_t DAC_Write(uint8_t port, uint16_t Vout)
{
	uint8_t status = 0;
	// range check
	if (port > 7)
	{
		return 0; //
	}

	// uint8_t Twim_Write_Data(uint8_t Address, uint8_t twi_bytes_to_transceive, uint16_t twi_data[TWI_MAX_DATA_ELEMENTS])
	// reset the MUX channel in case of change
	DacPacket[0] = DAC_I2C_MUX_DAC_ADDRESS;
	status = Twim_Write_Data(DAC_I2C_MUX_ADDRESS, 1, DacPacket); // connect with DAC

	if (status)
	{
		return 0; //
	}

	// Build DacPacket
	DacPacket[0] = (DAC_I2C_DAC5574_CONTROL_BYTE | ((port & 0x03) << 1));	// Control Byte A,B,C,D
	DacPacket[1] = Vout;	// calculated Vout
	DacPacket[2] = 0x00;	// dummy byte

	// Send command
	// 0 for channel [0-3]
	// 1 for channel [4-7]
	if (port & 0x04)		// check carry bit
	{
		status = Twim_Write_Data(DAC_I2C_DAC2_ADDRESS, 3, DacPacket); // connect with DAC
	}
	else
	{
		status = Twim_Write_Data(DAC_I2C_DAC1_ADDRESS, 3, DacPacket); // connect with DAC
	}

	//Status

	if (!status)
	{
		if (DacValueUndefinedByteArray & (0x1 << port))
		{
			DacValueUndefinedByteArray &= (uint8_t) ~(0x1 << port);
		}
		return 1; // true
	}
	else
	{
		return 0; // false
	}

}

// ======================================================================
/*
 Parameter:
 port: 0 - 7;

 With the help of this function, we are able to get the current value on a specific output port.
 */
// ======================================================================
uint8_t DAC_PortRead(uint8_t port, uint16_t *data)
{
	uint8_t status = 0;

	// range check
	if (port > 7)
	{
		return 0; //
	}

	// if value hasn't been set before, return undefined
	if (DacValueUndefinedByteArray & (0x1 << port))
	{
		*data = (uint16_t) DAC_UNDEFINED_DATA;
		return 1;
	}

	// reset the MUX channel in case of change

	DacPacket[0] = DAC_I2C_MUX_DAC_ADDRESS;
	status = Twim_Write_Data(DAC_I2C_MUX_ADDRESS, 1, DacPacket); // connect with DAC
	if (status)
	{
		return 0; //
	}

	//  select channel
	DacPacket[0] = (DAC_I2C_DAC5574_CONTROL_BYTE | ((port & 0x03) << 1));	// select channel

	// echo "I2C 0 4D 1 20">/dev/ttyUSB0			choose channel
	// echo "I2C 1 4D 1">/dev/ttyUSB0			read channel value
	// read selected buffer
	if (port & 0x04)
	{	//0 for [0-3], 1 for [4-7] channel
		status = Twim_Write_Data(DAC_I2C_DAC2_ADDRESS, 1, DacPacket); // set channel
		status = Twim_Read_Data(DAC_I2C_DAC2_ADDRESS, 1, DacPacket); //read channel
	}
	else
	{
		status = Twim_Write_Data(DAC_I2C_DAC1_ADDRESS, 1, DacPacket); // set channel
		status = Twim_Read_Data(DAC_I2C_DAC1_ADDRESS, 1, DacPacket); //read channel
	}

	*data = (uint16_t) DacPacket[0];

	if (!status)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

uint8_t DAC_Init(void)
{
	// if POWER_ON then all values are set to zero, otherwise undefined
	if (resetSource_POWER_ON == resetSource)
	{
		DacValueUndefinedByteArray = 0;
	}
	else
	{
		DacValueUndefinedByteArray = 0xFF;
	}
	return apiCommandResult_SUCCESS_QUIET;
}

void DAC(struct uartStruct *ptrUART)
{
	uint8_t counter = 0;
	uint8_t channel = 0;
	uint32_t mVin;
	uint32_t dac_value;

	// We are using the current arguments from the UART Structure
	//uint32_t Uart_Message_ID;	// 1 argument // show us the length of the command
	//uint32_t Uart_Mask;		// 2 argument // give use the port
	//uint8_t Uart_Rtr;			// 3 argument // If a value exist, we are settings up the current port with this value

	switch (ptrUART->number_of_arguments)
	{
		case 0:
			// Command DAC. Print all channels output value.
			// Read Channel
			// send the receive data through UART

			for (counter = 0; counter < DAC_CHANNEL_COUNT; counter++)
			{
				DAC_Read(counter);
			}
			break;

		case 1:
			// Command DAC, channel[0-8]. Print the output value of the current channel
			channel = ptrUART->Uart_Message_ID;
			DAC_Read(channel);
			break;

		case 2:
			// Command DAC, channel[0-8], value[0-3300]. Set the current channel to the value
			channel = ptrUART->Uart_Message_ID;
			mVin = (uint32_t) strtoul(setParameter[2], &ptr_setParameter[2], 10);

			/* 		 dac_value = ((mVin/DAC_Vcc)*256)		*/

			dac_value = mVin * 256;
			dac_value /= DAC_Vcc;

			if (dac_value > DAC_MAX_Vin)
			{
				mVin = DAC_Vcc;
				dac_value = DAC_MAX_Vin;
			}

			//set_status = DAC_Write(channel, dac_value);
			if (DAC_Write(channel, dac_value))
			{
				DAC_Read(channel);
			}

			break;
	}
}

void DAC_Read(uint8_t channel)
{
	uint16_t rcv_data;
	float Vout;

	if (channel > DAC_MAX_CH)
	{
		CommunicationError(ERRA, -1, 1, PSTR("wrong channel number %i [%i,%i]"), channel, 0, DAC_MAX_CH);
		return;
	}

	if (DAC_PortRead(channel, &rcv_data))
	{
	    printDebug_p(debugLevelEventDebug, debugSystemMain, __LINE__, PSTR(""), PSTR("DAC_Read"));
		if ( DAC_UNDEFINED_DATA <= rcv_data)
		{
			Vout = -1.0F;
		}
		else
		{
			Vout = DAC_Vper_bit * rcv_data;
		}
		createReceiveHeader(NULL, NULL, 0);
		snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s%i %x %.0f"), uart_message_string, channel, rcv_data, roundf(Vout));
		if ( DAC_UNDEFINED_DATA <= rcv_data)
		{
			strncat_P(uart_message_string, PSTR(" undefined"),BUFFER_SIZE -1);
		}
		UART0_Send_Message_String_p(NULL, 0);
	}

}
