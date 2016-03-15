/*
 * VERSION 1.0 Januar 11th 2015 LATE  File: 'dac_api.c'
 * Author: Martin Mitkov
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

//#include "one_wire.h"
//#include "one_wire_adc.h"
//#include "one_wire_dualSwitch.h"
//#include "one_wire_octalSwitch.h"
//#include "one_wire_simpleSwitch.h"
//#include "read_write_register.h"
//#include "waveform_generator_registers.h"
//#include "one_wire_temperature.h"
//#include "one_wire_api_settings.h"
//#include "one_wire_octalSwitch.h"
//#include "relay.h"



#include "api.h"
#include "api_debug.h"
#include "api_show.h"
#include "api_help.h"
#include "api_debug.h"
//#include "jtag.h"
//#include "can.h"
//#include "mem-check.h"
//#include "api_version.h"
//#include "api_identification.h"
#include "twi_master.h"

//#include "adc.h"
#include "api_define.h"
#include "api_global.h"
#include "api.h"
#ifdef TESTING_ENABLE
#include "testing.h"
#endif
#include "dac_api.h"


//avarice -c 0,1,0,8 --jtag /dev/ttyUSB1 -B 1000000 --erase --program --file api_hadcon2.elf;

/*
Paramepers:
	Port: 0-7,
	Vout: 0mV - 3300mV

The aim of the function is to set up fast the DAC output.
 */
uint16_t packet[8];

uint8_t DAC_SetUP(uint8_t port, uint16_t Vout){
	uint8_t status = 0;


	//uint8_t Twim_Write_Data(uint8_t Address, uint8_t twi_bytes_to_transceive, uint16_t twi_data[TWI_MAX_DATA_ELEMENTS])
	// reset the MUX channel in case of change
	packet[0] = MUX_DAC_ADDRESS;
	status = Twim_Write_Data(MUX_ADDRESS, 1, packet); // connect with DAC

	// Build packet
	packet[0] = (DAC_CHANNEL | ((port & 0x03)<<1));	// A,B,C,D
	//packet[1] = (Vout/DAC_VCC)*256;		//Calculate the Vout
	packet[1] = Vout;	//Calculate the Vout
	packet[2] = 0x00;	// dummy byte

	// Send command
	// 0 for channel [0-3]
	// 1 for channel [4-7]
	if(port & 0x04)		// check carry bit
		status = Twim_Write_Data(DAC2_ADDRESS, 3, packet); // connect with DAC
	else
		status = Twim_Write_Data(DAC1_ADDRESS, 3, packet); // connect with DAC

	//Status

	if(!status)
		return 1; // true
	else
		return 0; // false



}

// ======================================================================
/*
Parameter:
 	 port: 0 - 7;

With the help of this function, we are able to get the current value on a specific output port.
 */
// ======================================================================
uint8_t DAC_PortRead(uint8_t port, uint8_t *data){
	//uint8_t Twim_Read_Data(uint8_t Address, uint8_t twi_bytes_to_transceive, uint16_t twi_data[TWI_MAX_DATA_ELEMENTS])
	uint8_t status = 0;
	//uint8_t tmp_data = 0;
	// reset the MUX channel in case of change

	packet[0] = MUX_DAC_ADDRESS;
	status = Twim_Write_Data(MUX_ADDRESS, 1, packet); // connect with DAC
	if(status)
		return 0;//

	//  select channel
	packet[0] = (DAC_CHANNEL | ((port & 0x03)<<1));	// select channel

	//echo "I2C 0 4D 1 20">/dev/ttyUSB0			choose channel
	//echo "I2C 1 4D 1">/dev/ttyUSB0			read channel value
	// read selected buffer
	if(port & 0x04){	//0 for [0-3], 1 for [4-7] channel
		status = Twim_Write_Data(DAC2_ADDRESS, 1, packet); // set channel
		status = Twim_Read_Data(DAC2_ADDRESS, 1, packet); //read channel
	}else{
		status = Twim_Write_Data(DAC1_ADDRESS, 1, packet);// set channel
		status = Twim_Read_Data(DAC1_ADDRESS, 1, packet); //read channel
	}

	*data = (uint8_t)packet[0];

	if(!status)
		return 1;
	else
		return 0;

//	return (uint8_t)packet[0];// ((data[0]/256)*DAC_VCC)/100;
}


void DAC(struct uartStruct *ptrUART){
	uint8_t counter = 0;
	uint8_t channel = 0;
	uint32_t mVin;
	uint32_t dac_value;

	// We are using the current arguments from the UART Structure
    //uint32_t Uart_Message_ID;	// 1 argument // show us the length of the command
    //uint32_t Uart_Mask;		// 2 argument // give use the port
    //uint8_t Uart_Rtr;			// 3 argument // If a value exist, we are settings up the current port with this value


	switch(ptrUART->number_of_arguments){	//
	case 0: // Command DAC. Print all channels output value.
		// Read Channel
		// send the receive data through UART

		for(counter = 0; counter < CHANNEL_COUNT; counter++)
		{
			DAC_Read(counter);
		}

		break;

	case 1:	// Command DAC, channel[0-8]. Print the output value of the current channel
			channel = ptrUART->Uart_Message_ID;
			DAC_Read(channel);
		break;

	case 2: // Command DAC, channel[0-8], value[0-3300]. Set the current channel to the value
		channel = ptrUART->Uart_Message_ID;
		mVin = (uint32_t) strtoul(setParameter [2], &ptr_setParameter[2], 10);

		/* 		 dac_value = ((mVin/DAC_Vcc)*256)		*/

		dac_value =  mVin * 256;
		dac_value /= DAC_Vcc;

		if(dac_value>MAX_Vin)
		{
			mVin = DAC_Vcc;
			dac_value = MAX_Vin;
		}


		//set_status = DAC_SetUP(channel, dac_value);
		if(DAC_SetUP(channel, dac_value))
		{
			DAC_Read(channel);
		}

		break;
	}
};

void DAC_Read(uint8_t channel){
	uint8_t rcv_data;
	float Vout;

	if(channel>DAC_MAX_CH)
	{
		CommunicationError(ERRA,-1, 1, PSTR("wrong channel number %i [%i,%i]"), channel, 0, DAC_MAX_CH);
		return;
	}

	if(DAC_PortRead(channel, &rcv_data))
	{
		Vout = DAC_Vper_bit * rcv_data;

		//printDebug(debugLevelNoDebug, debugSystemMain, __LINE__, PSTR(""), PSTR("%x %i"), rcv_data, Vout);

		createReceiveHeader(NULL,NULL,0);
		snprintf_P(uart_message_string,BUFFER_SIZE -1, PSTR("%s%i %x %.0f"),uart_message_string, channel, rcv_data, roundf(Vout));
		//snprintf_P(uart_message_string,BUFFER_SIZE -1, PSTR("%s%i %.0f"),uart_message_string, channel, Vout);
		UART0_Send_Message_String_p(NULL,0);
	}

}
