/*
 * twi_master.c
 *
 *  Created on: Oct, 2010
 *      Author: fouedjio
 */
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h> 
#include <stddef.h>
#include <math.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/iocan128.h>
#include <avr/iocanxx.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include "api_define.h"
#include "api_global.h"
#include "twi_master.h"
#include "delay.h"
#include "api.h"


/****************************************************************************
 TWI State codes
 ****************************************************************************/
// General TWI Master status codes
#define TWI_START				    0x08  // START has been transmitted
#define TWI_REP_START				0x10  // Repeated START has been transmitted
#define TWI_ARB_LOST				0x38  // Arbitration lost
// TWI Master Transmitter status codes
#define TWI_MTX_ADR_ACK				0x18  // SLA+W has been transmitted and ACK received
#define TWI_MTX_ADR_NACK			0x20  // SLA+W has been transmitted and NACK received
#define TWI_MTX_DATA_ACK			0x28  // Data byte has been transmitted and ACK received
#define TWI_MTX_DATA_NACK			0x30  // Data byte has been transmitted and NACK received
// TWI Master Receiver status codes
#define TWI_MRX_ADR_ACK				0x40  // SLA+R has been transmitted and ACK received
#define TWI_MRX_ADR_NACK			0x48  // SLA+R has been transmitted and NACK received
#define TWI_MRX_DATA_ACK			0x50  // Data byte has been received and ACK transmitted
#define TWI_MRX_DATA_NACK			0x58  // Data byte has been received and NACK transmitted
// TWI Slave Transmitter status codes
#define TWI_STX_ADR_ACK				0xA8  // Own SLA+R has been received; ACK has been returned
#define TWI_STX_ADR_ACK_M_ARB_LOST	0xB0  // Arbitration lost in SLA+R/W as Master; own SLA+R has been received; ACK has been returned
#define TWI_STX_DATA_ACK			0xB8  // Data byte in TWDR has been transmitted; ACK has been received
#define TWI_STX_DATA_NACK			0xC0  // Data byte in TWDR has been transmitted; NOT ACK has been received
#define TWI_STX_DATA_ACK_LAST_BYTE	0xC8  // Last data byte in TWDR has been transmitted (TWEA = �0�); ACK has been received
// TWI Slave Receiver status codes
#define TWI_SRX_ADR_ACK				0x60  // Own SLA+W has been received ACK has been returned
#define TWI_SRX_ADR_ACK_M_ARB_LOST	0x68  // Arbitration lost in SLA+R/W as Master; own SLA+W has been received; ACK has been returned
#define TWI_SRX_GEN_ACK				0x70  // General call address has been received; ACK has been returned
#define TWI_SRX_GEN_ACK_M_ARB_LOST	0x78  // Arbitration lost in SLA+R/W as Master; General call address has been received; ACK has been returned
#define TWI_SRX_ADR_DATA_ACK		0x80  // Previously addressed with own SLA+W; data has been received; ACK has been returned
#define TWI_SRX_ADR_DATA_NACK		0x88  // Previously addressed with own SLA+W; data has been received; NOT ACK has been returned
#define TWI_SRX_GEN_DATA_ACK		0x90  // Previously addressed with general call; data has been received; ACK has been returned
#define TWI_SRX_GEN_DATA_NACK		0x98  // Previously addressed with general call; data has been received; NOT ACK has been returned
#define TWI_SRX_STOP_RESTART		0xA0  // A STOP condition or repeated START condition has been received while still addressed as Slave
// TWI Miscellaneous status codes
#define TWI_NO_STATE		        0xF8  // No relevant state information available; TWINT = �0�
#define TWI_BUS_ERROR			    0x00  // Bus error due to an illegal START or STOP condition

void twiMaster(struct uartStruct *ptr_uartStruct)
{
	int status = -1;
	if ( eventDebugVerbose <= debug && ( ( debugMask >> debugTWI ) & 0x1 ) )
	{
		snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) entered twiMaster"), __LINE__, __FILE__);
		UART0_Send_Message_String(NULL,0);
	}

	status = twiMasterParseUartStruct(ptr_uartStruct);
	if ( 0 != status )
	{
		if ( eventDebug <= debug && ( ( debugMask >> debugTWI ) & 0x1 ) )
		{
			snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) argument parsing failed"), __LINE__, __FILE__);
			UART0_Send_Message_String(NULL,0);
		}

		Check_Error(ptr_uartStruct);

		return;
	    /* exit */
	}

	/* command :TWIS:Read/write  Adresse  length  Data[0...7] */
    switch(ptr_uartStruct->Uart_Message_ID)
    {
    case TWIM_READ:
		if ( eventDebug <= debug && ( ( debugMask >> debugTWI ) & 0x1 ) )
		{
			snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) going to read from twi/i2c"), __LINE__, __FILE__);
			UART0_Send_Message_String(NULL,0);
		}
		Read_from_slave(ptr_uartStruct);
    	break;
    case TWIM_WRITE:
		if ( eventDebug <= debug && ( ( debugMask >> debugTWI ) & 0x1 ) )
		{
			snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) going to write to twi/i2c"), __LINE__, __FILE__);
			UART0_Send_Message_String(NULL,0);
		}
    	Write_to_slave(ptr_uartStruct);
    	break;
    default:
		twi_errorCode = CommunicationError(ERRT, TWI_ERROR_unknown_command, 0, NULL, 0);
    	break;
    }

    if ( eventDebugVerbose <= debug && ( ( debugMask >> debugTWI ) & 0x1 ) )
	{
		snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) leaving twiMaster"), __LINE__, __FILE__);
		UART0_Send_Message_String(NULL,0);
	}
    return;
}

uint8_t twiMasterParseUartStruct(struct uartStruct *ptr_uartStruct)
{
    if ( eventDebugVerbose <= debug && ( ( debugMask >> debugTWI ) & 0x1 ) )
	{
		snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) parsing UartStruct"), __LINE__, __FILE__);
		UART0_Send_Message_String(NULL,0);
	}
	/* convert char in hexadecimal*/

    if ( eventDebugVerbose <= debug && ( ( debugMask >> debugTWI ) & 0x1 ) )
	{
		snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) number of arguments %i"), __LINE__, __FILE__, ptr_uartStruct->number_of_arguments);
		UART0_Send_Message_String(NULL,0);
	}

	if (
			( TWIM_WRITE == ptr_uartStruct->Uart_Message_ID && TWIS_MIN_NARGS > ptr_uartStruct->number_of_arguments )
			||
			( TWIM_READ == ptr_uartStruct->Uart_Message_ID && TWIS_MIN_NARGS -1 > ptr_uartStruct->number_of_arguments )
		)
	{
		if ( eventDebugVerbose <= debug && ( ( debugMask >> debugTWI ) & 0x1 ) )
		{
			snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) too few arguments (%i) %i"), __LINE__, __FILE__, TWIS_MIN_NARGS, ptr_uartStruct->number_of_arguments);
			UART0_Send_Message_String(NULL,0);
		}
#warning TODO: implement check on 1st argument if it is a number or a sub-keyword
#warning TODO: ... to change/read e.g. the proporties of the interface (speed, timeouts, etc ...)
		twi_errorCode = CommunicationError(ERRT, TWI_ERROR_too_few_numeric_arguments, 0, NULL, 0);
		return 2;
	}

	//   ptr_uartStruct->Name = setParameter[0];
	ptr_uartStruct->Uart_Message_ID = (uint32_t) strtoul(setParameter [1], &ptr_setParameter[1], 16);
	ptr_uartStruct->Uart_Mask       = (uint32_t) strtoul(setParameter [2], &ptr_setParameter[2], 16);
	ptr_uartStruct->Uart_Rtr        = 0; /*dummy value*/
	ptr_uartStruct->Uart_Length     = (uint8_t)  strtoul(setParameter [3], &ptr_setParameter[3], 16);
	ptr_uartStruct->Uart_Data[0]    = (uint16_t) strtoul(setParameter [4], &ptr_setParameter[4], 16);
	ptr_uartStruct->Uart_Data[1]    = (uint16_t) strtoul(setParameter [5], &ptr_setParameter[5], 16);
	ptr_uartStruct->Uart_Data[2]    = (uint16_t) strtoul(setParameter [6], &ptr_setParameter[6], 16);
	ptr_uartStruct->Uart_Data[3]    = (uint16_t) strtoul(setParameter [7], &ptr_setParameter[7], 16);
	ptr_uartStruct->Uart_Data[4]    = (uint16_t) strtoul(setParameter [8], &ptr_setParameter[8], 16);
	ptr_uartStruct->Uart_Data[5]    = (uint16_t) strtoul(setParameter [9], &ptr_setParameter[9], 16);
	ptr_uartStruct->Uart_Data[6]    = (uint16_t) strtoul(setParameter[10], &ptr_setParameter[10],16);
	ptr_uartStruct->Uart_Data[7]    = (uint16_t) strtoul(setParameter[11], &ptr_setParameter[11],16);

	if ( TRUE == twiMasterCheckParameterFormatAndRanges(ptr_uartStruct) )
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

uint8_t twiMasterCheckParameterFormatAndRanges(struct uartStruct *ptr_uartStruct)
{
	   int8_t nargs = ptr_uartStruct->number_of_arguments;
	   static int8_t dataIndex = 0;

	   /* maximum checks (minimum not needed since unsigned variables */
	   if ( 0 < nargs && 0x1        < ptr_uartStruct->Uart_Message_ID ){ return FALSE;}
	   if ( 1 < nargs && 0x7F       < ptr_uartStruct->Uart_Mask       ){ return FALSE;}
	   if ( 2 < nargs && 8          < ptr_uartStruct->Uart_Length     ){ return FALSE;}

	   for ( dataIndex = 0 ; dataIndex < (nargs - 3) && TWI_MAX_DATA_ELEMENTS > dataIndex ; dataIndex++ )
	   {
	       if ( 0XFF < ptr_uartStruct->Uart_Data[dataIndex]    ){ return FALSE;}
	   }

	   /* consistency checks for given number of data members when writing data*/
	   if ( TWIM_WRITE == ptr_uartStruct->Uart_Message_ID &&
			   2 < nargs &&
			   nargs - 3 != ptr_uartStruct->Uart_Length)
	   {
		   return FALSE;
	   }

	   /* set number of data == TWI_MAX_DATA_ELEMENTS if nargs = 2 and ID = TWIM_READ */
	   if ( 2 == nargs && TWIM_READ == ptr_uartStruct->Uart_Message_ID)
	   {
		   ptr_uartStruct->Uart_Length = TWI_MAX_DATA_ELEMENTS;
		   ptr_uartStruct->number_of_arguments = 3 + TWI_MAX_DATA_ELEMENTS;
	   }

	   return TRUE;
}

uint8_t twiMasterCheckError(struct uartStruct *ptr_uartStruct)
{
	/* type check */
	uint8_t error = FALSE;
	uint8_t parameterIndex;
    for ( parameterIndex = 0 ; parameterIndex <= ptr_uartStruct->number_of_arguments; parameterIndex++ )
	{
		switch (parameterIndex)
		{
		case 1:
			if ( (0 != (ptr_uartStruct->Uart_Message_ID)) &&
					(1 != (ptr_uartStruct->Uart_Message_ID))  )
			{
				uart_errorCode = CommunicationError(ERRT, TWI_ERROR_unknown_command, 0, NULL, 0);
				error = TRUE;
				break;
			}
			break;
		case 2:
			if ( ( 0x7F ) < ptr_uartStruct->Uart_Mask )
			{
				uart_errorCode = CommunicationError(ERRT, TWI_ERROR_address_is_too_long, 0, NULL, 0);
				error = TRUE;
				break;
			}
			break;

		case 3:
			if ( ( 8 ) < ptr_uartStruct->Uart_Length )
			{
				uart_errorCode = CommunicationError(ERRT, TWI_ERROR_data_length_is_too_long, 0, NULL, 0);
				error = TRUE;
				break;
			}
			break;
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
			for ( uint8_t dataIndex = 0 ; dataIndex < TWI_MAX_DATA_ELEMENTS ; dataIndex++ )
			{
				if ( ( 0XFF ) < ptr_uartStruct->Uart_Data[dataIndex] )
				{
					uart_errorCode = CommunicationError(ERRT, TWI_ERROR_data_0_is_too_long + dataIndex, 0, NULL, 0);
					ptr_uartStruct->Uart_Data[dataIndex] = 0;
					error = TRUE;
					break;
				}
			}
			break;
		}
	}

	if (TWIM_WRITE == ptr_uartStruct->Uart_Message_ID &&  3 <= ptr_uartStruct->number_of_arguments )
	{
		if ( ptr_uartStruct->Uart_Length + 3 != ptr_uartStruct->number_of_arguments )
		{
			uart_errorCode = CommunicationError(ERRT, TWI_ERROR_wrong_length_or_number_of_data_bytes, 0, NULL, 0);
			error = TRUE;
		}
	}

	return error;
}

/*
 * Public Function: Twim_Init
 * Purpose: Initialise the TWI Master Interface
 * Input Parameter:
 * uint16_t	TWI_Bitrate (Hz)
 * Return Value: uint8_t
 *	- FALSE:	Bitrate too high
 *	- TRUE:		Bitrate OK
*/
uint8_t Twim_Init (uint32_t TWI_Bitrate)
{
/* Set TWI bitrate, if bitrate is too high, then error return*/
//	TWBR = ((F_CPU/TWI_Bitrate)-16)/2;
	TWBR = 42;	// flo
	
	if (TWBR < 11) return FALSE;

/* Hardware tweak needed for hadcon2 */
#if ( HADCON_VERSION == 2)
	PORTB = (1<<PB5); // i2cmultilpexer: i2creset (aktive low) set to high flo
#endif

	TWCR = ((1<<TWEN)|(0<<TWIE)); // enable i2c interface, disable i2c interrupts flo

	return TRUE;
}

/*
 * Public Function: Twim_Stop
 * Purpose: Stop the Twi Master
 * Input Parameter: None
 * Return Value: None
 */

void Twim_Stop(void)
{
	/*Send stop condition*/

	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);

	/* Wait until stop condition is executed and bus released*/
	while (TWCR & (1 << TWINT)) {;}
}


/*

 * Public Function: Write_to_slave
 * Purpose: Write byte(s) to the slave.
 * It is implicitely assumed, that the slave will
 * accepts 8 bytes
 * Input Parameter: structure
 * Return Value: None

 */

void Write_to_slave(struct uartStruct *ptr_uartStruct)
{
	uint8_t status = FALSE;
    if ( eventDebugVerbose <= debug && ( ( debugMask >> debugTWI ) & 0x1 ) )
	{
		snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) entering 'Write to Slave'"), __LINE__, __FILE__);
		UART0_Send_Message_String(NULL,0);
	}

    if ( eventDebugVerbose <= debug && ( ( debugMask >> debugTWI ) & 0x1 ) )
	{
		snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) address is:0x%x"), __LINE__, __FILE__, ptr_uartStruct->Uart_Mask);
		UART0_Send_Message_String(NULL,0);
	}

    status = Twim_Write_Data(ptr_uartStruct->Uart_Mask, ptr_uartStruct->Uart_Length, &(ptr_uartStruct->Uart_Data[0]));

	if ( TWI_success == status )
	{
    	if ( eventDebugVerbose <= debug && ( ( debugMask >> debugTWI ) & 0x1 ) )
    	{
    		snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) write success"), __LINE__, __FILE__);
    		UART0_Send_Message_String(NULL,0);
    	}
    	twiCreateRecvString(ptr_uartStruct, ptr_uartStruct->Uart_Length, ptr_uartStruct->Uart_Data);
	}
	else
	{
		TWI_errorAnalysis(status);
	}
    if ( eventDebugVerbose <= debug && ( ( debugMask >> debugTWI ) & 0x1 ) )
	{
		snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) leaving 'Write to Slave'"), __LINE__, __FILE__);
		UART0_Send_Message_String(NULL,0);
	}
}

uint8_t twiCreateRecvString(struct uartStruct *ptr_uartStruct, uint32_t length, uint16_t array[TWI_MAX_DATA_ELEMENTS])
{
	if (NULL == ptr_uartStruct) { return 1; }
	if (NULL == array) { return 1; }

	createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
	snprintf_P(uart_message_string, BUFFER_SIZE - 1,
			PSTR("%s%x"),
			uart_message_string,
			ptr_uartStruct->Uart_Message_ID);
	snprintf(uart_message_string, BUFFER_SIZE - 1,
			"%s %02X %02X",
			uart_message_string,
			(unsigned int) (ptr_uartStruct->Uart_Mask),
			(unsigned int) (length) );
	for (uint8_t i = 0; i < length; i++)
	{
		snprintf(uart_message_string, BUFFER_SIZE - 1, "%s %02X", uart_message_string, array[i]);
	}
	if (0 == ptr_uartStruct->Uart_Message_ID) /* write */
	{
		snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s -OK-"), uart_message_string);
	}
	UART0_Send_Message_String(NULL, 0);
	return 0;
}


/*
 * Public Function: read_to_slave
 * Purpose: Read byte(s) from the slave
 * It is implicitly assumed, that the slave will send
 * 8 bytes
 * Input Parameter: structure
 * Return Value: None
 */

void Read_from_slave(struct uartStruct *ptr_uartStruct)
{
    if ( eventDebugVerbose <= debug && ( ( debugMask >> debugTWI ) & 0x1 ) )
	{
		snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) entering 'Read from Slave'"), __LINE__, __FILE__);
		UART0_Send_Message_String(NULL,0);
	}

    uint8_t status;

	status = Twim_Read_Data(ptr_uartStruct->Uart_Mask, ptr_uartStruct->Uart_Length, twi_data);


	if ( TWI_success == status )
	{

    	if ( eventDebugVerbose <= debug && ( ( debugMask >> debugTWI ) & 0x1 ) )
    	{
    		snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) read success"), __LINE__, __FILE__);
    		UART0_Send_Message_String(NULL,0);
    	}
		twiCreateRecvString(ptr_uartStruct, ptr_uartStruct->Uart_Length, twi_data);
	}
	else
	{
		TWI_errorAnalysis(status);
	}

	if ( eventDebugVerbose <= debug && ( ( debugMask >> debugTWI ) & 0x1 ) )
	{
		snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) leaving 'Read from Slave'"), __LINE__, __FILE__);
		UART0_Send_Message_String(NULL,0);
	}
}

void TWI_errorAnalysis(uint8_t status)
{
	switch (status)
	{
	case TWI_success:
    	if ( eventDebugVerbose <= debug && ( ( debugMask >> debugTWI ) & 0x1 ) )
    	{
    		snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) read/write success"), __LINE__, __FILE__);
    		UART0_Send_Message_String(NULL,0);
    	}
		break;
	case TWI_start_condition_read_fail:
		twi_errorCode = CommunicationError(ERRT, TWI_ERROR_Could_not_start_TWI_Bus_for_READ, 0, NULL, 0);
		break;
	case TWI_start_condition_write_fail:
		twi_errorCode = CommunicationError(ERRT, TWI_ERROR_Could_not_start_TWI_Bus_for_WRITE, 0, NULL, 0);
		break;
	case TWI_data_write_transfer_fail:
		twi_errorCode = CommunicationError(ERRT, TWI_ERROR_failed_writing_TWI_Bus, 0, NULL, 0);
		break;
	case TWI_data_read_transfer_fail:
		twi_errorCode = CommunicationError(ERRT, TWI_ERROR_failed_reading_TWI_Bus, 0, NULL, 0);
		break;
	default:
		twi_errorCode = CommunicationError(ERRT, -1, 0, PSTR("unknown TWI error condition"), 0);
		break;
	}
}

/*
 * Public Function: TWIM_Start
 * Purpose: Start the TWI Master Interface
 * Input Parameter:
 - uint8_t	Device address
 - uint8_t	Type of required Operation:
            Twim_READ : Read  data from the slave
            Twim_WRITE: Write data to the slave
 * Return Value: uint8_t
 - TRUE:	OK, TWI Master accessible
 - FALSE:	Error in starting TWI Master
 */

uint8_t Twim_Start(uint8_t Address, uint8_t TWIM_Type)
{
	uint8_t twst;
	/*Send START condition*/
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN) | (0 << TWIE);

	/*Wait until transmission completed*/
	while (!(TWCR & (1 << TWINT)));

	/* Check value of TWI Status Register. Mask prescaler bits.*/
	twst = TWSR & 0xF8;
	if ((twst != TWI_START) && (twst != TWI_REP_START))
		return FALSE;
	/* Send device address*/
	TWDR = (Address << 1) + TWIM_Type;
	TWCR = (1 << TWINT) | (1 << TWEN);

	/*Wait until transmission completed and ACK/NACK has been received*/
	while (!(TWCR & (1 << TWINT))) ;

	/*Check value of TWI Status Register. Mask prescaler bits.*/
	twst = TWSR & 0xF8;
	if ((twst != TWI_MTX_ADR_ACK) && (twst != TWI_MRX_ADR_ACK)) {
		return FALSE;
	}

	return TRUE;
}

/*
 * Public Function: Twim_Write
 * Purpose: Write a byte to the slave
 * Input Parameter:
 - uint8_t	Byte to be sent
 * Return Value: uint8_t
 - TRUE:		OK, Byte sent
 - FALSE:	Error in byte transmission

 */

uint8_t Twim_Write(uint8_t byte)
{
	uint8_t twst;

	/* Send data to the previously addressed device*/
	TWDR = byte;
	TWCR = (1 << TWINT) | (1 << TWEN);

	/*Wait until transmission completed*/
	while (!(TWCR & (1 << TWINT)));

	/* Check value of TWI Status Register. Mask prescaler bits*/
	twst = TWSR & 0xF8;
	if (twst != TWI_MTX_DATA_ACK) {
		return FALSE;
	}

	return TRUE;
}

/*
 * Public Function: Twim_ReadAck
 * Purpose: Read a byte from the slave and request next byte
 * Input Parameter: None
 * Return Value: uint8_t
 - uint8_t	Read byte
 */

uint8_t Twim_ReadAck(void)
{
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
	while (!(TWCR & (1 << TWINT))) {};

	return TWDR;
}

/*
 * Public Function: Twim_ReadAck
 * Purpose: Read the last byte from the slave
 * Input Parameter: None
 * Return Value: uint8_t
 - uint8_t	Read byte
 */
uint8_t Twim_ReadNack(void)
{
	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT))) {;}

	return TWDR;
}

// flo

uint8_t Twim_Write_Data(uint8_t Address, uint8_t twi_bytes_to_transceive, uint16_t twi_data[TWI_MAX_DATA_ELEMENTS])
{
	uint8_t status = FALSE;

	if (!Twim_Start(Address, TWIM_WRITE))
	{
		Twim_Stop();
		return TWI_start_condition_write_fail;
	}
	else
	{
		for(uint8_t i = 0; i < twi_bytes_to_transceive; i++)
		{
			status = Twim_Write(twi_data[i]);
			if ( FALSE == status)
			{
				break;
			}
		}
		Twim_Stop();
		if (FALSE == status) { return TWI_data_write_transfer_fail; }
		return TWI_success;
	}
}

uint8_t Twim_Read_Data(uint8_t Address, uint8_t twi_bytes_to_transceive, uint16_t twi_data[TWI_MAX_DATA_ELEMENTS])
{
	uint8_t dataByteIndex;
	if (!Twim_Start(Address, TWIM_READ))
	{
		Twim_Stop();
		return TWI_start_condition_read_fail;

	}
	else
	{
		for (dataByteIndex = 0; dataByteIndex < twi_bytes_to_transceive -1 && dataByteIndex < TWI_MAX_DATA_ELEMENTS -1 ; dataByteIndex++)
		{
			twi_data[dataByteIndex] = Twim_ReadAck();
		}
		twi_data[dataByteIndex] = Twim_ReadNack();
		Twim_Stop();
		return TWI_success;
	}
}
