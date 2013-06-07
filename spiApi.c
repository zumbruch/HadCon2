/*
 * spiApi.c
 *
 *  Created on: 22.05.2013
 *      Author: Peter Zumbruch, GSI, P.Zumbruch@gsi.de
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "api_global.h"
#include "api_define.h"
#include "api.h"
#include "spiApi.h"
#include "spi.h"

static const char filename[] PROGMEM = __FILE__;
static const char dots[] PROGMEM = "...";
static const char empty[] PROGMEM = "";

#warning TODO: optimize memory management of big data array
static char byte[3]= "00";

static const char spiCommandKeyword00[] PROGMEM = "status"; 			 /* show spi status of control bits and operation settings*/
static const char spiCommandKeyword01[] PROGMEM = "s";
static const char spiCommandKeyword02[] PROGMEM = "write"; 				 /* write <args>, directly setting automatically CS */
static const char spiCommandKeyword03[] PROGMEM = "w";
static const char spiCommandKeyword04[] PROGMEM = "add"; 				 /* add <args> to write buffer*/
static const char spiCommandKeyword05[] PROGMEM = "a";
static const char spiCommandKeyword06[] PROGMEM = "transmit"; 			 /* transmit write buffer auto set of CS controlled via cs_auto_enable*/
static const char spiCommandKeyword07[] PROGMEM = "t";
static const char spiCommandKeyword08[] PROGMEM = "read"; 				 /* read read Buffer*/
static const char spiCommandKeyword09[] PROGMEM = "r";
static const char spiCommandKeyword10[] PROGMEM = "cs"; 				 /* set chip select: cs <0|1|H(IGH)|L(OW)|T(RUE)|F(ALSE)>, CS is low active */
static const char spiCommandKeyword11[] PROGMEM = "cs_bar"; 			 /* same as cs but inverse logic*/
static const char spiCommandKeyword12[] PROGMEM = "cs_set"; 		     /* releases cs, optionally only for <cs mask>*/
static const char spiCommandKeyword13[] PROGMEM = "cs_release"; 		 /* completing byte by zero at the end or the beginning, due to odd hex digit*/
static const char spiCommandKeyword14[] PROGMEM = "cs_select_mask";		 /* chip select output pin byte mask */
static const char spiCommandKeyword15[] PROGMEM = "cs_pins"; 			 /* set hardware addresses of multiple CS outputs*/
static const char spiCommandKeyword16[] PROGMEM = "cs_auto_enable"; 	 /* cs_auto_enable [<a(ll)|w(rite)|t(ransmit)> <0|1 ..>: enable/disable automatic setting for write and transmit actions*/
static const char spiCommandKeyword17[] PROGMEM = "purge"; 				 /* purge write/read buffers*/
static const char spiCommandKeyword18[] PROGMEM = "p";
static const char spiCommandKeyword19[] PROGMEM = "purge_write_buffer";  /* purge write buffer*/
static const char spiCommandKeyword20[] PROGMEM = "pw";
static const char spiCommandKeyword21[] PROGMEM = "purge_read_buffer";   /* purge read buffer*/
static const char spiCommandKeyword22[] PROGMEM = "pr";
static const char spiCommandKeyword23[] PROGMEM = "show_write_buffer"; 	 /* show content of write buffer, detailed when increasing DEBG level */
static const char spiCommandKeyword24[] PROGMEM = "sw";
static const char spiCommandKeyword25[] PROGMEM = "show_read_buffer"; 	 /* show content of read buffer, detailed when increasing DEBG level */
static const char spiCommandKeyword26[] PROGMEM = "sr";
static const char spiCommandKeyword27[] PROGMEM = "control_bits"; 		 /* get/set SPI control bits*/
static const char spiCommandKeyword28[] PROGMEM = "c";
static const char spiCommandKeyword29[] PROGMEM = "spi_enable"; 		 /* get/set enable SPI*/
static const char spiCommandKeyword30[] PROGMEM = "data_order"; 		 /* get/set bit endianess*/
static const char spiCommandKeyword31[] PROGMEM = "master"; 			 /* get/set master mode*/
static const char spiCommandKeyword32[] PROGMEM = "clock_polarity"; 	 /* get/set clock polarity*/
static const char spiCommandKeyword33[] PROGMEM = "clock_phase"; 		 /* get/set clock phase*/
static const char spiCommandKeyword34[] PROGMEM = "speed"; 				 /* get/set speed */
static const char spiCommandKeyword35[] PROGMEM = "speed_divider"; 		 /* get/set speed divider*/
static const char spiCommandKeyword36[] PROGMEM = "double_speed"; 		 /* get/set double speed*/
static const char spiCommandKeyword37[] PROGMEM = "transmit_byte_order"; /* MSB/LSB, big endian */
static const char spiCommandKeyword38[] PROGMEM = "complete_byte"; 		 /* completing byte by zero at the end or the beginning, due to odd hex digit*/
static const char spiCommandKeyword39[] PROGMEM = "reset";       		 /* reset to default*/

const char* spiCommandKeywords[] PROGMEM = {
        spiCommandKeyword00, spiCommandKeyword01, spiCommandKeyword02, spiCommandKeyword03, spiCommandKeyword04,
		spiCommandKeyword05, spiCommandKeyword06, spiCommandKeyword07, spiCommandKeyword08, spiCommandKeyword09,
		spiCommandKeyword10, spiCommandKeyword11, spiCommandKeyword12, spiCommandKeyword13, spiCommandKeyword14,
		spiCommandKeyword15, spiCommandKeyword16, spiCommandKeyword17, spiCommandKeyword18, spiCommandKeyword19,
		spiCommandKeyword20, spiCommandKeyword21, spiCommandKeyword22, spiCommandKeyword23, spiCommandKeyword24,
		spiCommandKeyword25, spiCommandKeyword26, spiCommandKeyword27, spiCommandKeyword28, spiCommandKeyword29,
		spiCommandKeyword30, spiCommandKeyword31, spiCommandKeyword32, spiCommandKeyword33, spiCommandKeyword34,
		spiCommandKeyword35, spiCommandKeyword36, spiCommandKeyword37, spiCommandKeyword38, spiCommandKeyword39  };


void spiApi(struct uartStruct *ptr_uartStruct)
{
	switch(ptr_uartStruct->number_of_arguments)
	{
		case 0:
			spiApiSubCommandShowStatus(ptr_uartStruct);
			break;
		default:
			if (isNumericArgument(setParameter[1], MAX_LENGTH_PARAMETER)) /*write*/
			{
				/*uart and uart_remainder to spi data */
				spiApiSubCommandWrite(ptr_uartStruct, 1);
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

void spiApiSubCommands(struct uartStruct *ptr_uartStruct, int16_t subCommandIndex)
{
	// find matching command keyword

	if ( 0 > subCommandIndex )
	{
		subCommandIndex = apiFindCommandKeywordIndex(setParameter[1], spiCommandKeywords, spiCommandKeyNumber_MAXIMUM_NUMBER);
	}

    /* generate message */
    createExtendedSubCommandReceiveResponseHeader(ptr_uartStruct, ptr_uartStruct->commandKeywordIndex, subCommandIndex, spiCommandKeywords);

	/* TODO: spiApiMiscSubCommandsChooseFunction(ptr_uartStruct, index)*/


	switch (subCommandIndex)
	{
		case spiCommandKeyNumber_STATUS:
		case spiCommandKeyNumber_S:
			/* show spi status of control bits and operation settings*/
			spiApiSubCommandShowStatus(ptr_uartStruct);
			break;
		case spiCommandKeyNumber_WRITE:
		case spiCommandKeyNumber_W:
			/* write <args>, directly setting automatically CS */
			spiApiSubCommandWrite(ptr_uartStruct, 2);
			break;
		case spiCommandKeyNumber_ADD:
		case spiCommandKeyNumber_A:
			/* add <args> to write buffer*/
			spiApiSubCommandAdd(ptr_uartStruct, 2);
			break;
		case spiCommandKeyNumber_TRANSMIT:
		case spiCommandKeyNumber_T:
			/* transmit write buffer auto set of CS controlled via cs_auto_enable*/
			spiApiSubCommandTransmit(ptr_uartStruct, 2);
			break;
		case spiCommandKeyNumber_READ:
		case spiCommandKeyNumber_R:
			/* read read Buffer*/
			spiApiSubCommandRead(ptr_uartStruct, 2);
			break;
		case spiCommandKeyNumber_CS:
			/* set chip select: cs <0|1|H(IGH)|L(OW)|T(RUE)|F(ALSE)>, CS is low active */
			spiApiSubCommandCs(ptr_uartStruct, 2);
			break;
		case spiCommandKeyNumber_CS_BAR:
			/* same as cs but inverse logic*/
			spiApiSubCommandCsBar(ptr_uartStruct, 2);
			break;
		case spiCommandKeyNumber_CS_SET:
			/* releases cs, optionally only for <cs mask>*/
			spiApiSubCommandCsSet(ptr_uartStruct, 2);
			break;
		case spiCommandKeyNumber_CS_RELEASE:
			/* completing byte by zero at the end or the beginning, due to odd hex digit*/
			spiApiSubCommandCsRelease(ptr_uartStruct, 2);
			break;
		case spiCommandKeyNumber_CS_SELECT_MASK:
			/* chip select output pin byte mask */
			spiApiSubCommandCsSelectMask(ptr_uartStruct, 2);
			break;
		case spiCommandKeyNumber_CS_PINS:
			/* set hardware addresses of multiple CS outputs*/
			spiApiSubCommandCsPins(ptr_uartStruct, 2);
			break;
		case spiCommandKeyNumber_CS_AUTO_ENABLE:
			/* cs_auto_enable [<a(ll)|w(rite)|t(ransmit)> <0|1 ..>: enable/disable automatic setting for write and transmit actions*/
			spiApiSubCommandCsAutoEnable(ptr_uartStruct, 2);
			break;
		case spiCommandKeyNumber_PURGE:
		case spiCommandKeyNumber_P:
			/* purge write/read buffers*/
			spiApiSubCommandPurge(ptr_uartStruct, 2);
			break;
		case spiCommandKeyNumber_PURGE_WRITE_BUFFER:
		case spiCommandKeyNumber_PW:
			/* purge write buffer*/
			spiApiSubCommandPurgeWriteBuffer(ptr_uartStruct, 2);
			break;
		case spiCommandKeyNumber_PURGE_READ_BUFFER:
		case spiCommandKeyNumber_PR:
			/* purge read buffer*/
			spiApiSubCommandPurgeReadBuffer(ptr_uartStruct, 2);
			break;
		case spiCommandKeyNumber_SHOW_WRITE_BUFFER:
		case spiCommandKeyNumber_SW:
			/* show content of write buffer, detailed when increasing DEBG level */
			spiApiSubCommandShowWriteBuffer(ptr_uartStruct, 2);
			break;
		case spiCommandKeyNumber_SHOW_READ_BUFFER:
		case spiCommandKeyNumber_SR:
			/* show content of read buffer, detailed when increasing DEBG level */
			spiApiSubCommandShowReadBuffer(ptr_uartStruct, 2);
			break;
		case spiCommandKeyNumber_CONTROL_BITS:
		case spiCommandKeyNumber_C:
			/* get/set SPI control bits*/
			spiApiSubCommandControlBits(ptr_uartStruct, 2);
			break;
		case spiCommandKeyNumber_SPI_ENABLE:
			/* get/set enable SPI*/
			spiApiSubCommandSpiEnable(ptr_uartStruct, 2);
			break;
		case spiCommandKeyNumber_DATA_ORDER:
			/* get/set bit endianess*/
			spiApiSubCommandDataOrder(ptr_uartStruct, 2);
			break;
		case spiCommandKeyNumber_MASTER:
			/* get/set master mode*/
			spiApiSubCommandMaster(ptr_uartStruct, 2);
			break;
		case spiCommandKeyNumber_CLOCK_POLARITY:
			/* get/set clock polarity*/
			spiApiSubCommandClockPolarity(ptr_uartStruct, 2);
			break;
		case spiCommandKeyNumber_CLOCK_PHASE:
			/* get/set clock phase*/
			spiApiSubCommandClockPhase(ptr_uartStruct, 2);
			break;
		case spiCommandKeyNumber_SPEED:
			/* get/set speed */
			spiApiSubCommandSpeed(ptr_uartStruct, 2);
			break;
		case spiCommandKeyNumber_SPEED_DIVIDER:
			/* get/set speed divider*/
			spiApiSubCommandSpeedDivider(ptr_uartStruct, 2);
			break;
		case spiCommandKeyNumber_DOUBLE_SPEED:
			/* get/set double speed*/
			spiApiSubCommandDoubleSpeed(ptr_uartStruct, 2);
			break;
		case spiCommandKeyNumber_TRANSMIT_BYTE_ORDER:
			/* MSB/LSB, big endian */
			spiApiSubCommandTransmitByteOrder(ptr_uartStruct, 2);
			break;
		case spiCommandKeyNumber_COMPLETE_BYTE:
			/* completing byte by zero at the end or the beginning, due to odd hex digit*/
			spiApiSubCommandCompleteByte(ptr_uartStruct, 2);
			break;
		case spiCommandKeyNumber_RESET:
			/* completing byte by zero at the end or the beginning, due to odd hex digit*/
			spiApiSubCommandReset(ptr_uartStruct);
			break;
		default:
			break;
	}
	UART0_Send_Message_String(uart_message_string, BUFFER_SIZE - 1);
}

void spiApiSubCommandShowStatus(struct uartStruct *ptr_uartStruct)
{
	// printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*)  ( filename ), (const char*) (empty));
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;
}

void spiApiSubCommandWrite(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex)
{
	// printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*)  ( filename ), (const char*) (empty));
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;

}

void spiApiSubCommandAdd(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex)
{
	// printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*)  ( filename ), (const char*) (empty));
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;

}

void spiApiSubCommandTransmit(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex)
{
	// printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*)  ( filename ), (const char*) (empty));
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;

}

void spiApiSubCommandRead(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex)
{
	// printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*)  ( filename ), (const char*) (empty));
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;

}

void spiApiSubCommandCs(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex)
{
	// printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*)  ( filename ), (const char*) (empty));
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;

}

void spiApiSubCommandCsBar(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex)
{
	// printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*)  ( filename ), (const char*) (empty));
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;

}

void spiApiSubCommandCsSet(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex)
{
	// printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*)  ( filename ), (const char*) (empty));
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;

}

void spiApiSubCommandCsRelease(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex)
{
	// printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*)  ( filename ), (const char*) (empty));
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;

}

void spiApiSubCommandCsSelectMask(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex)
{
	// printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*)  ( filename ), (const char*) (empty));
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;

}

void spiApiSubCommandCsPins(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex)
{
	// printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*)  ( filename ), (const char*) (empty));
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;

}

void spiApiSubCommandCsAutoEnable(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex)
{
	// printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*)  ( filename ), (const char*) (empty));
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;

}

void spiApiSubCommandPurge(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex)
{
	// printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*)  ( filename ), (const char*) (empty));
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;

}

void spiApiSubCommandPurgeWriteBuffer(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex)
{
	// printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*)  ( filename ), (const char*) (empty));
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;

}

void spiApiSubCommandPurgeReadBuffer(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex)
{
	// printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*)  ( filename ), (const char*) (empty));
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;

}

void spiApiSubCommandShowWriteBuffer(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex)
{
	// printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*)  ( filename ), (const char*) (empty));
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;
	spiApiShowWriteBufferContent();

}

void spiApiSubCommandShowReadBuffer(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex)
{
	// printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*)  ( filename ), (const char*) (empty));
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;

}

void spiApiSubCommandControlBits(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex)
{
	// printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*)  ( filename ), (const char*) (empty));
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;

}

void spiApiSubCommandSpiEnable(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex)
{
	// printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*)  ( filename ), (const char*) (empty));
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;

}

void spiApiSubCommandDataOrder(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex)
{
	// printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*)  ( filename ), (const char*) (empty));
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;

}

void spiApiSubCommandMaster(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex)
{
	// printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*)  ( filename ), (const char*) (empty));
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;

}

void spiApiSubCommandClockPolarity(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex)
{
	// printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*)  ( filename ), (const char*) (empty));
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;

}

void spiApiSubCommandClockPhase(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex)
{
	// printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*)  ( filename ), (const char*) (empty));
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;

}

void spiApiSubCommandSpeed(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex)
{
	// printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*)  ( filename ), (const char*) (empty));
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;

}

void spiApiSubCommandSpeedDivider(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex)
{
	// printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*)  ( filename ), (const char*) (empty));
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;

}

void spiApiSubCommandDoubleSpeed(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex)
{
	// printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*)  ( filename ), (const char*) (empty));
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;

}

void spiApiSubCommandTransmitByteOrder(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex)
{
	// printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*)  ( filename ), (const char*) (empty));
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;

}

void spiApiSubCommandCompleteByte(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex)
{
	// printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*)  ( filename ), (const char*) (empty));
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;

}

void spiApiSubCommandReset(struct uartStruct *ptr_uartStruct)
{
	// printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*)  ( filename ), (const char*) (empty));
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;

}

/* helpers */

size_t spiApiFillWriteArray(struct uartStruct *ptr_uartStruct, uint16_t parameterStartIndex)
{
	/* reset array length*/
	spiWriteData.length = 0;

	spiApiAddToWriteArray(ptr_uartStruct, parameterStartIndex);

	spiApiShowWriteBufferContent();
	return spiWriteData.length;
}

size_t spiApiAddToWriteArray(struct uartStruct *ptr_uartStruct, uint16_t argumentIndex)
{
	int8_t result;

    while( spiWriteData.length < sizeof(spiWriteData.data) -1 && (int) argumentIndex <= ptr_uartStruct->number_of_arguments )
    {
    	if ( argumentIndex < MAX_PARAMETER)
    	{
    		printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*)  ( filename ),
    				PSTR("setParameter[%i]: \"%s\""), argumentIndex, setParameter[argumentIndex]);

    		/* get contents from parameter container */
    		/* spiWriteData should be increased */
    		result = spiAddNumericParameterToByteArray(NULL, argumentIndex);

    		if ( 0 > result )
    		{
    			break;
    		}
    	}
    	else
    	{
    		/* get contents from remainder container */
#warning TODO: !!!! REMAINDER analysis

    		result = spiAddNumericParameterToByteArray(&resultString[0], -1);
    		if ( 0 > result )
    		{
    			break;
    		}
    	}
  		printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*)  ( filename ),
    				PSTR("------------------------"));
  		argumentIndex++;
    }

    return spiWriteData.length;

}//END of function

int8_t spiAddNumericParameterToByteArray(const char string[], uint8_t index)
{
	int8_t result;
	printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*) ( filename ), PSTR("para to byte array"));

	/* get string string[] */
	if ( 0 > index || MAX_PARAMETER <= index  )
	{
		if ( NULL == string)
		{
			CommunicationError_p(ERRG, dynamicMessage_ErrorIndex, TRUE, PSTR("NULL pointer received"));
			return -1;
		}
		result = spiAddNumericStringToByteArray( string );
		if (-1 == result)
		{
			return -1;
		}
	}
	/* get string from setParameter at the index */
	else
	{
		printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*) ( filename ), PSTR("para string to byte array"));
		result = spiAddNumericStringToByteArray( setParameter[index] );
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

int8_t spiAddNumericStringToByteArray(const char string[])
{
	uint64_t value;
	size_t numberOfDigits;

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
		printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*) ( filename ), PSTR("no. bytes/digits %i %i"), numberOfBytes, numberOfDigits);

		if ( spiWriteData.length + numberOfBytes < sizeof(spiWriteData.data) -1 )
		{
			if ( 16 >= numberOfDigits ) /*64 bit*/
			{
				if ( 0 == getUnsignedNumericValueFromParameterString(string, &value) )
				{
					printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*) ( filename ),
								PSTR("value %#08lx%08lx"), value >> 32, value && 0xFFFFFFFF);
					/* byte order: MSB to LSB / big endian*/
					for (int8_t byteIndex = numberOfBytes - 1; byteIndex >= 0; byteIndex--)
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
						spi_add_write_data( 0xFF & (value >> (8 * byteIndex)) );

						//						data->data[data->length] = 0xFF & (value >> (8 * byteIndex));
						//						data->length++;
						printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*) ( filename ),
								PSTR("data[%i]=%x"), spiWriteData.length - 1, spiWriteData.data[spiWriteData.length -1]);
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
				printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*) ( filename ), PSTR(">64 bit"));

				clearString(byte, sizeof(byte));
				strcat_P(byte, PSTR("00"));

				printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*) ( filename ), PSTR("init byte '%s'"), byte);
				size_t charIndex = 0;

				/* in case of odd number of digits*/
#warning TODO add case add leading 0 at end for odd number of digits
				/* add leading 0 in front
				 */
				{
					if (numberOfDigits%2 )
					{
						printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*) ( filename ), PSTR("odd"));
						byte[0]='0';
						byte[1]=string[charIndex];
						charIndex++;
					}
					else
					{
						printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*) ( filename ), PSTR("even"));
						charIndex = 0;
					}
					while (charIndex + 1 < numberOfDigits)
					{

						byte[0]=string[charIndex];
						byte[1]=string[charIndex + 1];
						charIndex+=2;
						printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__,
								(const char*) ( filename ), PSTR("strtoul '%s'"), byte);
						spi_add_write_data( strtoul(byte, NULL, 16) );
						printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*) ( filename ),
								PSTR("data[%i]=%x"), spiWriteData.length - 1, spiWriteData.data[spiWriteData.length -1]);
						//spiAddNumericStringToByteArray(&byte[0], data);
					}
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
	uint16_t ctr = 0;
	uint16_t maxMessageSize = 0;
	/* header */
	createExtendedSubCommandReceiveResponseHeader(ptr_uartStruct, commandKeyNumber_SPI, spiCommandKeyNumber_SHOW_WRITE_BUFFER, spiCommandKeywords);
	snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%swrite buffer - elements: %#x (%#i)"), uart_message_string, spiWriteData.length, spiWriteData.length );
	UART0_Send_Message_String(NULL,0);


	/* data */
	clearString(message, BUFFER_SIZE);
	createExtendedSubCommandReceiveResponseHeader(ptr_uartStruct, commandKeyNumber_SPI, spiCommandKeyNumber_SHOW_WRITE_BUFFER, spiCommandKeywords);
	maxMessageSize = BUFFER_SIZE - 1 - strlen_P(dots) - strlen(uart_message_string);

	for (size_t byteIndex = 0; byteIndex < spiWriteData.length; byteIndex++)
	{
		if ( (byteIndex)%8 == 0)
		{
			clearString(message, BUFFER_SIZE);
			ctr++;
			snprintf(message, maxMessageSize, "(#%i) ", ctr);
		}

		snprintf(message, maxMessageSize, "%s%02X ", message, spiWriteData.data[byteIndex]);

		if ( 0 == (byteIndex +1 )%8 || byteIndex == spiWriteData.length -1 )
		{
			createExtendedSubCommandReceiveResponseHeader(ptr_uartStruct, commandKeyNumber_SPI, spiCommandKeyNumber_SHOW_WRITE_BUFFER, spiCommandKeywords);

			/* attach "..." at the end, if there are more to come*/
			if (byteIndex < spiWriteData.length - 1)
			{
				strncat_P(message, dots, BUFFER_SIZE -1 );
			}

			strncat(uart_message_string, message, BUFFER_SIZE -1 );
			UART0_Send_Message_String(NULL,0);
		}
	}
}

