/*
 * spiApi.c
 *
 *  Created on: 22.05.2013
 *      Author: Peter Zumbruch, GSI, P.Zumbruch@gsi.de
 */

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "api_global.h"
#include "api_define.h"
#include "api.h"
#include "spiApi.h"
#include "spi.h"

#warning TODO: optimize memory management of big data array
spiByteDataArray spiWriteData;

static const char spiCommandKeyword00[] PROGMEM = "status"; 			 /* show spi status of control bits and operation settings*/
static const char spiCommandKeyword01[] PROGMEM = "write"; 				 /* write <args>, directly setting automatically CS */
static const char spiCommandKeyword02[] PROGMEM = "add"; 				 /* add <args> to write buffer*/
static const char spiCommandKeyword03[] PROGMEM = "transmit"; 			 /* transmit write buffer auto set of CS controlled via cs_auto_enable*/
static const char spiCommandKeyword04[] PROGMEM = "read"; 				 /* read read Buffer*/
static const char spiCommandKeyword05[] PROGMEM = "cs"; 				 /* set chip select: cs <0|1|H(IGH)|L(OW)|T(RUE)|F(ALSE)>, CS is low active */
static const char spiCommandKeyword06[] PROGMEM = "cs_bar"; 			 /* same as cs but inverse logic*/
static const char spiCommandKeyword07[] PROGMEM = "cs_select"; 			 /* chip select output pin byte mask */
static const char spiCommandKeyword08[] PROGMEM = "cs_auto_enable"; 	 /* cs_auto_enable [<a(ll)|w(rite)|t(ransmit)> <0|1 ..>: enable/disable automatic setting for write and transmit actions*/
static const char spiCommandKeyword09[] PROGMEM = "cs_pins"; 			 /* set hardware addresses of multiple CS outputs*/
static const char spiCommandKeyword10[] PROGMEM = "control_bits"; 		 /* get/set SPI control bits*/
static const char spiCommandKeyword11[] PROGMEM = "spi_enable"; 		 /* get/set enable SPI*/
static const char spiCommandKeyword12[] PROGMEM = "data_order"; 		 /* get/set bit endianess*/
static const char spiCommandKeyword13[] PROGMEM = "master"; 			 /* get/set master mode*/
static const char spiCommandKeyword14[] PROGMEM = "clock_polarity"; 	 /* get/set clock polarity*/
static const char spiCommandKeyword15[] PROGMEM = "clock_phase"; 		 /* get/set clock phase*/
static const char spiCommandKeyword16[] PROGMEM = "speed"; 				 /* get/set speed */
static const char spiCommandKeyword17[] PROGMEM = "speed_divisor"; 		 /* get/set speed divider*/
static const char spiCommandKeyword18[] PROGMEM = "double_speed"; 		 /* get/set double speed*/
static const char spiCommandKeyword19[] PROGMEM = "show_write_buffer"; 	 /* show content of write buffer, detailed when increasing DEBG level */
static const char spiCommandKeyword20[] PROGMEM = "show_read_buffer"; 	 /* show content of read buffer, detailed when increasing DEBG level */
static const char spiCommandKeyword21[] PROGMEM = "purge"; 				 /* purge write/read buffers*/
static const char spiCommandKeyword22[] PROGMEM = "purge_write_buffer";  /* purge write buffer*/
static const char spiCommandKeyword23[] PROGMEM = "purge_read_buffer";   /* purge read buffer*/
static const char spiCommandKeyword24[] PROGMEM = "transmit_byte_order"; /* MSB/LSB, big endian */
static const char spiCommandKeyword25[] PROGMEM = "complete_byte"; 		 /* completing byte by zero at the end or the beginning, due to odd hex digit*/

const char* spiCommandKeywords[] PROGMEM = {
        spiCommandKeyword00, spiCommandKeyword01, spiCommandKeyword02, spiCommandKeyword03, spiCommandKeyword04,
		spiCommandKeyword05, spiCommandKeyword06, spiCommandKeyword07, spiCommandKeyword08, spiCommandKeyword09,
		spiCommandKeyword10, spiCommandKeyword11, spiCommandKeyword12, spiCommandKeyword13, spiCommandKeyword14,
		spiCommandKeyword15, spiCommandKeyword16, spiCommandKeyword17, spiCommandKeyword18, spiCommandKeyword19,
		spiCommandKeyword20, spiCommandKeyword21, spiCommandKeyword22, spiCommandKeyword23, spiCommandKeyword24,
		spiCommandKeyword25 };


void spiApi(struct uartStruct *ptr_uartStruct)
{
	switch(ptr_uartStruct->number_of_arguments)
	{
		case 0:
#warning TODO: show status ???			/*status*/

			break;
		default:
			if (isNumericArgument(setParameter[1], MAX_LENGTH_PARAMETER)) /*write*/
			{
				/*uart and uart_remainder to spi data */

				if ( 0 < spiApiFillWriteArray(ptr_uartStruct, 1) )
				{
					//spiWrite(spiWriteData,temp.length);
				}
			}
			else /*sub command*/
			{
				// find matching command keyword: -1
				spiApiSubCommands(ptr_uartStruct, -1);
			}
			break;
	}
	return;
}


size_t spiApiFillWriteArray(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex)
{
	/* reset array length*/
	spiWriteData.length = 0;

	spiApiAddToWriteArray(ptr_uartStruct, parameterIndex);

	spiApiShowWriteBufferContent();
	return spiWriteData.length;
}

size_t spiApiAddToWriteArray(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex)
{
	int8_t result;

	while( spiWriteData.length < ptr_uartStruct->Uart_Length &&
			spiWriteData.length < sizeof(spiWriteData.data) -1 )
	{
		if ( parameterIndex < MAX_PARAMETER)
		{
			/* get contents from parameter container */
			printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, __FILE__,
						 PSTR("setParameter[%i]: \"%s\""), parameterIndex, setParameter[parameterIndex]);

			result = spiAddNumericParameterToByteArray(NULL, &spiWriteData, parameterIndex);

			if ( 0 > result )
			{
				break;
			}

			parameterIndex++;
		}
		else
		{
			/* get contents from remainder container */


		result = spiAddNumericParameterToByteArray(&resultString[0], &spiWriteData, -1);
		if ( 0 > result )
		{
			break;
		}
		}
	}
	return spiWriteData.length;

}//END of function

int8_t spiAddNumericParameterToByteArray(const char string[], spiByteDataArray* data, uint8_t index)
{
	int8_t result;

	if ( NULL == data)
	{
		CommunicationError_p(ERRG, dynamicMessage_ErrorIndex, TRUE, PSTR("NULL pointer received"));
		return -1;
	}

	/* get string string[] */
	if ( 0 > index || MAX_PARAMETER <= index  )
	{
		if ( NULL == string)
		{
			CommunicationError_p(ERRG, dynamicMessage_ErrorIndex, TRUE, PSTR("NULL pointer received"));
			return -1;
		}
		result = spiAddNumericStringToByteArray( string , data );
		if (-1 == result)
		{
			return -1;
		}
	}
	/* get string from setParameter at the index */
	else
	{
		result = spiAddNumericStringToByteArray( setParameter[index] , data );
		if (-1 == result)
		{
			return -1;
		}
	}
	return 0;
}

/*
 * int8_t spiAddNumericStringToByteArray(const char string[], spiByteDataArray* data)
 *
 *
 */

int8_t spiAddNumericStringToByteArray(const char string[], spiByteDataArray* data)
{
	uint64_t value;
	size_t numberOfDigits;

	if ( NULL == data)
	{
		CommunicationError_p(ERRG, dynamicMessage_ErrorIndex, TRUE, PSTR("NULL pointer received"));
		return -1;
	}

	if ( NULL == string)
	{
		CommunicationError_p(ERRG, dynamicMessage_ErrorIndex, TRUE, PSTR("NULL pointer received"));
		return -1;
	}

	if ( isNumericArgument(string, MAX_LENGTH_COMMAND - MAX_LENGTH_KEYWORD - 2))
	{
		/* get number of digits*/
		numberOfDigits = getNumberOfHexDigits(string, MAX_LENGTH_COMMAND - MAX_LENGTH_KEYWORD - 2 );

		/* check if new element, split into bytes, MSB to LSB,
		 * to be added would exceed the maximum size of data array */

		size_t numberOfBytes = ((numberOfDigits + numberOfDigits%2) >> 1);
		if ( data->length + numberOfBytes < sizeof(data->data) -1 )
		{
			if (numberOfDigits <= 16) /*64 bit*/
			{
				if ( 0 == getUnsignedNumericValueFromParameterString(string, &value) )
				{
					/* byte order: MSB to LSB / big endian*/
					for (uint8_t byteIndex = numberOfBytes - 1; byteIndex >= 0; byteIndex--)
					{
						/* 0x0 					.. 0xFF*/
						/* 0x000 				.. 0xFFFF*/
						/* 0x00000 				.. 0xFFFFFF*/
						/* 0x0000000 			.. 0xFFFFFFFF*/
						/* 0x000000000	 		.. 0xFFFFFFFFFF*/
						/* 0x00000000000		.. 0xFFFFFFFFFFFF*/
						/* 0x0000000000000		.. 0xFFFFFFFFFFFFFF*/
						/* 0x000000000000000	.. 0xFFFFFFFFFFFFFFFF*/
						/* ... */
#warning TODO add case add leading 0 at end for odd number of digits
						data->data[data->length] = 0xFF & (value >> (8 * byteIndex));
						data->length++;
					}
				}
				else
				{
					CommunicationError_p(ERRA, SERIAL_ERROR_argument_has_invalid_type, TRUE, PSTR("%s"), string);
					return -1;
				}
			} /* > 64 bit*/
			else
			{
				char byte[2]= "00";
				size_t charIndex = 0;

				/* in case of odd number of digits*/
#warning TODO add case add leading 0 at end for odd number of digits
				/* add leading 0 in front
				 */
				if (numberOfDigits%2 )
				{
					byte[0]='0';
					byte[1]=string[charIndex];
					charIndex++;
					spiAddNumericStringToByteArray(byte, data);
				}
				else
				{
					charIndex = 0;
				}
				while (charIndex + 1 < numberOfDigits)
				{
					byte[0]=string[charIndex];
					charIndex++;
					byte[1]=string[charIndex];
					charIndex++;
					spiAddNumericStringToByteArray(byte, data);
				}
			}
		}
		else
		{
			CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, TRUE, PSTR("too many bytes to add"));
			return -1;
		}

	}
	else
	{
		CommunicationError_p(ERRA, SERIAL_ERROR_argument_has_invalid_type, TRUE, PSTR("%s"), string);
		return -1;
	}

	return 0;
}

void spiApiShowWriteBufferContent(void)
{
	/* header */
	createExtendedSubCommandReceiveResponseHeader(ptr_uartStruct, commandKeyNumber_SPI, spiCommandKeyNumber_SHOW_WRITE_BUFFER, spiCommandKeywords);
	snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s write buffer - elements: %i"), uart_message_string, spiWriteData.length );
	UART0_Send_Message_String(NULL,0);

	/* data */
	clearString(message, BUFFER_SIZE);
	for (size_t byteIndex = 0; byteIndex < spiWriteData.length; ++byteIndex)
	{
		snprintf(message, BUFFER_SIZE -1, "%s %02X", message, spiWriteData.data[byteIndex]);
	}

	printDebug_p(debugLevelEventDebug, debugSystemSPI, -1, NULL, PSTR("wb:%s"), message);

	createExtendedSubCommandReceiveResponseHeader(ptr_uartStruct, commandKeyNumber_SPI, spiCommandKeyNumber_SHOW_WRITE_BUFFER, spiCommandKeywords);

	strncat(uart_message_string, message, BUFFER_SIZE -1 );
	UART0_Send_Message_String(NULL,0);

}

#warning TODO: add function to display write buffer content
void spiApiSubCommands(struct uartStruct *ptr_uartStruct, int16_t subCommandIndex)
{
	subCommandIndex = 0;
	// find matching command keyword
/*	while ( subCommandIndex < commandDebugKeyNumber_MAXIMUM_NUMBER )
	{
		if ( 0 == strncmp_P(setParameter[1], (const char*) ( pgm_read_word( &(commandDebugKeywords[subCommandIndex])) ), MAX_LENGTH_PARAMETER) )
		{
				printDebug_p(debugLevelEventDebug, debugSystemDEBUG, __LINE__, PSTR(__FILE__), PSTR("keyword %s matches"), &setParameter[1][0]);
			break;
		}
		else
		{
				printDebug_p(debugLevelEventDebug, debugSystemDEBUG, __LINE__, PSTR(__FILE__), PSTR("keyword %s doesn't match"), &setParameter[1][0]);
		}
		subCommandIndex++;
	}
	*/
}

//spiWrite()
//spiRead() ?
//spiInit()
//spiEnable()
