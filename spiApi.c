/*
   Licensed under the EUPL V.1.1, Lizenziert unter EUPL V.1.1 
*/
/*
 * spiApi.c
 *
 *  Created on: 22.05.2013
 *      Author: Peter Zumbruch, GSI, P.Zumbruch@gsi.de
 */

#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include "api_global.h"
#include "api_define.h"
#include "api.h"
#include "spiApi.h"
#include "spi.h"

/*eclipse specific setting, not used during build process*/
#ifndef __AVR_AT90CAN128__
#include <avr/iocan128.h>
#endif

static const char filename[] PROGMEM = __FILE__;

static const char string_3dots[] 	PROGMEM = "...";
//static const char string_empty[]  PROGMEM = "";
static const char string_PORT[]     PROGMEM = "PORT";

static const char string_sKommax[]  PROGMEM = "%s,%x";

static char byte[3]= "00";

static const char spiApiCommandKeyword00[] PROGMEM = "add"; 					/* add <args> to write buffer*/
static const char spiApiCommandKeyword01[] PROGMEM = "a";
static const char spiApiCommandKeyword02[] PROGMEM = "control_bits";       		/* get/set SPI control bits*/
static const char spiApiCommandKeyword03[] PROGMEM = "c";
static const char spiApiCommandKeyword04[] PROGMEM = "purge";					/* purge write/read buffer*/
static const char spiApiCommandKeyword05[] PROGMEM = "p";
static const char spiApiCommandKeyword06[] PROGMEM = "read";					/* read read Buffer*/
static const char spiApiCommandKeyword07[] PROGMEM = "r";
static const char spiApiCommandKeyword08[] PROGMEM = "status";					/* show spi status of control bits and operation settings*/
static const char spiApiCommandKeyword09[] PROGMEM = "s";
static const char spiApiCommandKeyword10[] PROGMEM = "transmit"; 				/* transmit write buffer auto set of CS controlled via cs_auto_enable*/
static const char spiApiCommandKeyword11[] PROGMEM = "t";
static const char spiApiCommandKeyword12[] PROGMEM = "write";					/* write <args>, directly setting automatically CS */
static const char spiApiCommandKeyword13[] PROGMEM = "w";
static const char spiApiCommandKeyword14[] PROGMEM = "write_buffer";		 	/* write_buffer <args>, directly setting automatically CS */
static const char spiApiCommandKeyword15[] PROGMEM = "wb";
static const char spiApiCommandKeyword16[] PROGMEM = "purge_write_buffer"; 		/* purge write buffer*/
static const char spiApiCommandKeyword17[] PROGMEM = "pw";
static const char spiApiCommandKeyword18[] PROGMEM = "purge_read_buffer";  		/* purge read buffer*/
static const char spiApiCommandKeyword19[] PROGMEM = "pr";
static const char spiApiCommandKeyword20[] PROGMEM = "show_write_buffer";  		/* show content of write buffer, detailed when increasing DEBG level */
static const char spiApiCommandKeyword21[] PROGMEM = "sw";
static const char spiApiCommandKeyword22[] PROGMEM = "show_read_buffer";   		/* show content of read buffer, detailed when increasing DEBG level */
static const char spiApiCommandKeyword23[] PROGMEM = "sr";
static const char spiApiCommandKeyword24[] PROGMEM = "cs";            	   		/* set chip select: cs <0|1|H(IGH)|L(OW)|T(RUE)|F(ALSE)>, CS is low active */
static const char spiApiCommandKeyword25[] PROGMEM = "cs_bar";        	   		/* same as cs but inverse logic*/
static const char spiApiCommandKeyword26[] PROGMEM = "csb";
static const char spiApiCommandKeyword27[] PROGMEM = "cs_set";        	     	/* set cs, optionally only for <cs mask>*/
static const char spiApiCommandKeyword28[] PROGMEM = "css";
static const char spiApiCommandKeyword29[] PROGMEM = "cs_release";    	     	/* releases cs, optionally only for <cs mask>*/
static const char spiApiCommandKeyword30[] PROGMEM = "csr";
static const char spiApiCommandKeyword31[] PROGMEM = "cs_select_mask";	   		/* chip select output pin byte mask */
static const char spiApiCommandKeyword32[] PROGMEM = "cs_pins";       	   		/* set/get hardware addresses of multiple CS outputs*/
static const char spiApiCommandKeyword33[] PROGMEM = "cs_add_pin";  	      	/* set new cs address*/
static const char spiApiCommandKeyword34[] PROGMEM = "csap";
static const char spiApiCommandKeyword35[] PROGMEM = "cs_remove_pin";       	/* remove cs address*/
static const char spiApiCommandKeyword36[] PROGMEM = "csrp";
static const char spiApiCommandKeyword37[] PROGMEM = "spi_enable";				/* get/set enable SPI*/
static const char spiApiCommandKeyword38[] PROGMEM = "data_order";         		/* get/set bit endianess*/
static const char spiApiCommandKeyword39[] PROGMEM = "master";             		/* get/set master mode*/
static const char spiApiCommandKeyword40[] PROGMEM = "clock_polarity";     		/* get/set clock polarity*/
static const char spiApiCommandKeyword41[] PROGMEM = "clock_phase";        		/* get/set clock phase*/
static const char spiApiCommandKeyword42[] PROGMEM = "speed";              		/* get/set speed */
static const char spiApiCommandKeyword43[] PROGMEM = "speed_divider";      		/* get/set speed divider*/
static const char spiApiCommandKeyword44[] PROGMEM = "double_speed";       		/* get/set double speed*/
static const char spiApiCommandKeyword45[] PROGMEM = "transmit_byte_order";		/* MSB/LSB, big endian */
static const char spiApiCommandKeyword46[] PROGMEM = "reset";              		/* reset to default*/
static const char spiApiCommandKeyword47[] PROGMEM = "transmit_report";    		/* report send bytes*/
static const char spiApiCommandKeyword48[] PROGMEM = "auto_purge_write_buffer";	/* automatic purge of write buffer after write commands */
static const char spiApiCommandKeyword49[] PROGMEM = "auto_purge_read_buffer";	/* automatic purge of read buffer before write commands */

const char* const spiApiCommandKeywords[] PROGMEM = {
        spiApiCommandKeyword00, spiApiCommandKeyword01, spiApiCommandKeyword02, spiApiCommandKeyword03, spiApiCommandKeyword04,
		spiApiCommandKeyword05, spiApiCommandKeyword06, spiApiCommandKeyword07, spiApiCommandKeyword08, spiApiCommandKeyword09,
		spiApiCommandKeyword10, spiApiCommandKeyword11, spiApiCommandKeyword12, spiApiCommandKeyword13, spiApiCommandKeyword14,
		spiApiCommandKeyword15, spiApiCommandKeyword16, spiApiCommandKeyword17, spiApiCommandKeyword18, spiApiCommandKeyword19,
		spiApiCommandKeyword20, spiApiCommandKeyword21, spiApiCommandKeyword22, spiApiCommandKeyword23, spiApiCommandKeyword24,
		spiApiCommandKeyword25, spiApiCommandKeyword26, spiApiCommandKeyword27, spiApiCommandKeyword28, spiApiCommandKeyword29,
		spiApiCommandKeyword30, spiApiCommandKeyword31, spiApiCommandKeyword32, spiApiCommandKeyword33, spiApiCommandKeyword34,
		spiApiCommandKeyword35, spiApiCommandKeyword36, spiApiCommandKeyword37, spiApiCommandKeyword38, spiApiCommandKeyword39,
		spiApiCommandKeyword40, spiApiCommandKeyword41, spiApiCommandKeyword42, spiApiCommandKeyword43, spiApiCommandKeyword44,
		spiApiCommandKeyword45, spiApiCommandKeyword46, spiApiCommandKeyword47, spiApiCommandKeyword48, spiApiCommandKeyword49 };

spiApiConfig spiApiConfiguration;
bool spiApiInitialized = false;
spiApiConfig *ptr_spiApiConfiguration = &spiApiConfiguration;

void spiApiInit(void)
{
	/* initial configuration*/
	ptr_spiApiConfiguration->transmitByteOrder    = spiApiTransmitByteOrder_MSB;
	ptr_spiApiConfiguration->reportTransmit       = false;
	ptr_spiApiConfiguration->csExternalSelectMask = 0xFF;
	ptr_spiApiConfiguration->autoPurgeReadBuffer  = true;
	ptr_spiApiConfiguration->autoPurgeWriteBuffer = false;
	ptr_spiApiConfiguration->hardwareInit         = false;
	ptr_spiApiConfiguration->spiConfiguration     = spiGetConfiguration();
}


void spiApi(struct uartStruct *ptr_uartStruct)
{
	if (!spiApiInitialized )
	{
		spiApiInit();
		spiApiInitialized = true;
	}

	switch(ptr_uartStruct->number_of_arguments)
	{
		case 0:
			spiApiSubCommands(ptr_uartStruct, spiApiCommandKeyNumber_STATUS, 1);
			break;
		default:
		{
			if (ptr_uartStruct->number_of_arguments > 0)
			{
				int subCommandIndex = -1;
				// find matching command keyword
				subCommandIndex = apiFindCommandKeywordIndex(setParameter[1], spiApiCommandKeywords, spiApiCommandKeyNumber_MAXIMUM_NUMBER);
				if ( 0 <= subCommandIndex )
				{
					spiApiSubCommands(ptr_uartStruct, subCommandIndex, 2);
				}
				else if (isNumericArgument(setParameter[1], MAX_LENGTH_PARAMETER)) /*write*/
				{
					spiApiSubCommands(ptr_uartStruct, spiApiCommandKeyNumber_WRITE, 1);
				}
				else
				{
					CommunicationError_p(ERRA, SERIAL_ERROR_no_valid_command_name, true, NULL);
				}
			}
			else
			{
				CommunicationError_p(ERRG, GENERAL_ERROR_invalid_argument, true, NULL);
			}
			break;
		}
	}
	return;
}

/*
 * void spiApiSubCommands(struct uartStruct *ptr_uartStruct, int16_t subCommandIndex)
 *
 * parses for sub command keyword and calls the corresponding functions
 */

uint8_t spiApiSubCommands(struct uartStruct *ptr_uartStruct, int16_t subCommandIndex, uint8_t parameterIndex)
{
	uint8_t result = apiCommandResult_UNDEFINED;
	// find matching command keyword

	if ( 0 > subCommandIndex )
	{
		subCommandIndex = apiFindCommandKeywordIndex(setParameter[1], spiApiCommandKeywords, spiApiCommandKeyNumber_MAXIMUM_NUMBER);
	}

    /* generate header */
	switch (parameterIndex)
	{
		case 1:
			createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
			break;
		case 0:
		default:
			/* else */
			createExtendedSubCommandReceiveResponseHeader(ptr_uartStruct, ptr_uartStruct->commandKeywordIndex, subCommandIndex, spiApiCommandKeywords);
			break;
	}

	switch (subCommandIndex)
	{
		case spiApiCommandKeyNumber_STATUS:
			/* show spi status of control bits and operation settings*/
			result = spiApiSubCommandShowStatus();
			break;
		case spiApiCommandKeyNumber_S:
			/* show spi status of control bits and operation settings*/
			result = spiApiSubCommands(ptr_uartStruct, spiApiCommandKeyNumber_STATUS, 0);
			break;
		case spiApiCommandKeyNumber_WRITE:
			/* write <args>, directly setting automatically CS */
			result = spiApiSubCommandWrite(ptr_uartStruct, parameterIndex);
			break;
		case spiApiCommandKeyNumber_W:
			/* write <args>, directly setting automatically CS */
			clearString(uart_message_string, BUFFER_SIZE); /* hack: since parameterIndex is only used here */
			result = spiApiSubCommands(ptr_uartStruct, spiApiCommandKeyNumber_WRITE, parameterIndex);
			result = apiCommandResult_SUCCESS_QUIET; /* hack: output already generated by recursive call */
			break;
		case spiApiCommandKeyNumber_ADD:
			/* add <args> to write buffer*/
			result = spiApiSubCommandAdd(ptr_uartStruct);
			break;
		case spiApiCommandKeyNumber_A:
			/* add <args> to write buffer*/
			result = spiApiSubCommands(ptr_uartStruct, spiApiCommandKeyNumber_ADD, 0);
			break;
		case spiApiCommandKeyNumber_TRANSMIT:
			/* transmit write buffer auto set of CS controlled via cs_auto_enable*/
			result = spiApiSubCommandTransmit();
			break;
		case spiApiCommandKeyNumber_T:
			/* transmit write buffer auto set of CS controlled via cs_auto_enable*/
			result = spiApiSubCommands(ptr_uartStruct, spiApiCommandKeyNumber_TRANSMIT, 0);
			break;
		case spiApiCommandKeyNumber_WRITE_BUFFER:
			result = spiApiSubCommandWriteBuffer();
			break;
		case spiApiCommandKeyNumber_WB:
			result = spiApiSubCommands(ptr_uartStruct, spiApiCommandKeyNumber_WRITE_BUFFER, 0);
			break;
		case spiApiCommandKeyNumber_READ:
			/* read read Buffer*/
			result = spiApiSubCommandRead();
			break;
		case spiApiCommandKeyNumber_R:
			/* read read Buffer*/
			result = spiApiSubCommands(ptr_uartStruct, spiApiCommandKeyNumber_READ, 0);
			break;
		case spiApiCommandKeyNumber_CS:
			/* set chip select: cs <0|1|H(IGH)|L(OW)|T(RUE)|F(ALSE)>, CS is low active */
			result = spiApiSubCommandCsStatus(ptr_uartStruct);
			break;
		case spiApiCommandKeyNumber_CS_BAR:
			/* same as cs but inverse logic*/
			result = spiApiSubCommandCsBarStatus(ptr_uartStruct);
			break;
		case spiApiCommandKeyNumber_CSB:
			result = spiApiSubCommands(ptr_uartStruct, spiApiCommandKeyNumber_CS_BAR, 0);
			break;
		case spiApiCommandKeyNumber_CS_SET:
			/* set cs, optionally only for <cs mask>*/
			result = spiApiSubCommandCsSet();
			break;
		case spiApiCommandKeyNumber_CSS:
			/* set cs, optionally only for <cs mask>*/
			result = spiApiSubCommands(ptr_uartStruct, spiApiCommandKeyNumber_CS_SET, 0);
			break;
		case spiApiCommandKeyNumber_CS_RELEASE:
			/* releases cs, optionally only for <cs mask>*/
			result = spiApiSubCommandCsRelease();
			break;
		case spiApiCommandKeyNumber_CSR:
			/* releases cs, optionally only for <cs mask>*/
			result = spiApiSubCommands(ptr_uartStruct, spiApiCommandKeyNumber_CS_RELEASE, 0);
			break;
		case spiApiCommandKeyNumber_CS_SELECT_MASK:
			/* chip select output pin byte mask */
			result = spiApiSubCommandCsSelectMask(ptr_uartStruct);
			break;
		case spiApiCommandKeyNumber_CS_PINS:
			/* set hardware addresses of multiple CS outputs*/
			result = spiApiSubCommandCsPins(ptr_uartStruct);
			break;
		case spiApiCommandKeyNumber_CS_ADD_PIN:
			/* set hardware addresses of multiple CS outputs*/
			result = spiApiSubCommandCsAddPin(ptr_uartStruct);
			break;
		case spiApiCommandKeyNumber_CSAP:
			/* set hardware addresses of multiple CS outputs*/
			result = spiApiSubCommands(ptr_uartStruct, spiApiCommandKeyNumber_CS_ADD_PIN, 0);
			break;
		case spiApiCommandKeyNumber_CS_REMOVE_PIN:
			/* remove hardware addresses of multiple CS outputs*/
			result = spiApiSubCommandCsRemovePin(ptr_uartStruct);
			break;
		case spiApiCommandKeyNumber_CSRP:
			/* remove hardware addresses of multiple CS outputs*/
			result = spiApiSubCommands(ptr_uartStruct, spiApiCommandKeyNumber_CS_REMOVE_PIN, 0);
			break;
		case spiApiCommandKeyNumber_PURGE:
			/* purge write/read buffers*/
			result = spiApiSubCommandPurge();
			break;
		case spiApiCommandKeyNumber_P:
			/* purge write/read buffers*/
			result = spiApiSubCommands(ptr_uartStruct, spiApiCommandKeyNumber_PURGE, 0);
			break;
		case spiApiCommandKeyNumber_PURGE_WRITE_BUFFER:
			/* purge write buffer*/
			result = spiApiSubCommandPurgeWriteBuffer();
			break;
		case spiApiCommandKeyNumber_PW:
			/* purge write buffer*/
			result = spiApiSubCommands(ptr_uartStruct, spiApiCommandKeyNumber_PURGE_WRITE_BUFFER, 0);
			break;
		case spiApiCommandKeyNumber_PURGE_READ_BUFFER:
			/* purge read buffer*/
			result = spiApiSubCommandPurgeReadBuffer();
			break;
		case spiApiCommandKeyNumber_PR:
			/* purge read buffer*/
			result = spiApiSubCommands(ptr_uartStruct, spiApiCommandKeyNumber_PURGE_READ_BUFFER, 0);
			break;
		case spiApiCommandKeyNumber_SHOW_WRITE_BUFFER:
			/* show content of write buffer, detailed when increasing DEBG level */
			result = spiApiSubCommandShowWriteBuffer(ptr_uartStruct);
			break;
		case spiApiCommandKeyNumber_SW:
			/* show content of write buffer, detailed when increasing DEBG level */
			result = spiApiSubCommands(ptr_uartStruct, spiApiCommandKeyNumber_SHOW_WRITE_BUFFER, 0);
			break;
		case spiApiCommandKeyNumber_SHOW_READ_BUFFER:
			/* show content of read buffer, detailed when increasing DEBG level */
			result = spiApiSubCommandShowReadBuffer(ptr_uartStruct);
			break;
		case spiApiCommandKeyNumber_SR:
			result = spiApiSubCommands(ptr_uartStruct, spiApiCommandKeyNumber_SHOW_READ_BUFFER, 0);
			break;
		case spiApiCommandKeyNumber_CONTROL_BITS:
			/* get/set SPI control bits*/
			result = spiApiSubCommandControlBits(ptr_uartStruct);
			break;
		case spiApiCommandKeyNumber_C:
			/* get/set SPI control bits*/
			result = spiApiSubCommands(ptr_uartStruct, spiApiCommandKeyNumber_CONTROL_BITS, 0);
			break;
		case spiApiCommandKeyNumber_SPI_ENABLE:
			/* get/set enable SPI*/
			result = spiApiSubCommandSpiEnable(ptr_uartStruct);
			break;
		case spiApiCommandKeyNumber_DATA_ORDER:
			/* get/set bit endianess*/
			result = spiApiSubCommandDataOrder(ptr_uartStruct);
			break;
		case spiApiCommandKeyNumber_MASTER:
			/* get/set master mode*/
			result = spiApiSubCommandMaster(ptr_uartStruct);
			break;
		case spiApiCommandKeyNumber_CLOCK_POLARITY:
			/* get/set clock polarity*/
			result = spiApiSubCommandClockPolarity(ptr_uartStruct);
			break;
		case spiApiCommandKeyNumber_CLOCK_PHASE:
			/* get/set clock phase*/
			result = spiApiSubCommandClockPhase(ptr_uartStruct);
			break;
		case spiApiCommandKeyNumber_SPEED:
			/* get/set speed */
			result = spiApiSubCommandSpeed(ptr_uartStruct);
			if ( apiCommandResult_FAILURE > result)
			{
				if (1 < ptr_uartStruct->number_of_arguments)
				{
					spiApiShowStatusSpeed();
					result = apiCommandResult_SUCCESS_QUIET;
				}
			}
			break;
		case spiApiCommandKeyNumber_DOUBLE_SPEED:
			/* get/set double speed*/
			result = spiApiSubCommandDoubleSpeed(ptr_uartStruct);
			if ( apiCommandResult_FAILURE > result)
			{
				if (1 < ptr_uartStruct->number_of_arguments)
				{
					spiApiShowStatusSpeed();
					result = apiCommandResult_SUCCESS_QUIET;
				}
			}
			break;
		case spiApiCommandKeyNumber_SPEED_DIVIDER:
			/* get/set speed divider*/
			result = spiApiSubCommandSpeedDivider(ptr_uartStruct);
			break;
		case spiApiCommandKeyNumber_TRANSMIT_BYTE_ORDER:
			/* MSB/LSB, big endian */
			result = spiApiSubCommandTransmitByteOrder(ptr_uartStruct);
			break;
		case spiApiCommandKeyNumber_RESET:
			result = spiApiSubCommandReset();
			break;
		case spiApiCommandKeyNumber_TRANSMIT_REPORT:
			result = spiApiSubCommandTransmitReport(ptr_uartStruct);
			break;
		case spiApiCommandKeyNumber_AUTO_PURGE_READ_BUFFER:
			result = spiApiSubCommandAutoPurgeReadBuffer(ptr_uartStruct);
			break;
		case spiApiCommandKeyNumber_AUTO_PURGE_WRITE_BUFFER:
			result = spiApiSubCommandAutoPurgeWriteBuffer(ptr_uartStruct);
			break;
		default:
			result = -1;
			break;
	}

	/* not a recursive call */
	if (0 < parameterIndex)
	{
		spiApiSubCommandsFooter( result );
	}
	return result;
}

void spiApiSubCommandsFooter( uint16_t result )
{
	switch (result)
	{
		case apiCommandResult_SUCCESS_WITH_OUTPUT:
			UART0_Send_Message_String_p(uart_message_string, BUFFER_SIZE - 1);
			break;
		case apiCommandResult_SUCCESS_WITH_OPTIONAL_OUTPUT__OK:
			/* verbose response to commands*/
			if (debugLevelVerboseDebug <= globalDebugLevel && ((globalDebugSystemMask >> debugSystemSPI) & 1))
			{
				strncat_P(uart_message_string, PSTR("OK"), BUFFER_SIZE - 1);
				UART0_Send_Message_String_p(uart_message_string, BUFFER_SIZE - 1);
			}
			else
			{
				clearString(uart_message_string, BUFFER_SIZE);
			}
			break;
		case apiCommandResult_FAILURE_NOT_A_SUB_COMMAND:
			CommunicationError_p(ERRA, SERIAL_ERROR_no_valid_command_name, true, PSTR("not a sub command"));
			break;
		case apiCommandResult_SUCCESS_QUIET:
		case apiCommandResult_FAILURE_QUIET:
			clearString(uart_message_string, BUFFER_SIZE);
			/* printouts elsewhere generated */
			break;
		case apiCommandResult_FAILURE:
		default:
			CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, true, PSTR("command failed"));
			break;
	}

}

/*
 * *** sub commands with minimum 1 argument
 */

uint8_t spiApiSubCommandWrite(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex)
{
	uint8_t result = apiCommandResult_UNDEFINED;

	if ( true == ptr_spiApiConfiguration->autoPurgeReadBuffer )
	{
		spiApiSubCommandPurgeReadBuffer();
	}

	if ( apiCommandResult_FAILURE > spiApiPurgeAndFillWriteArray(ptr_uartStruct, parameterIndex))
	{
		/*spiWrite*/
		result = spiWriteAndReadWithChipSelect(ptr_spiApiConfiguration->transmitByteOrder, ptr_spiApiConfiguration->csExternalSelectMask);

		if (apiCommandResult_FAILURE <= result)
		{
			result = apiCommandResult_FAILURE_QUIET;
		}
		else
		{
			/*reporting*/
			if (true == ptr_spiApiConfiguration->reportTransmit)
			{
				spiApiSubCommandShowWriteBuffer(ptr_uartStruct);
				result = apiCommandResult_SUCCESS_QUIET;
			}
			else
			{
				result = apiCommandResult_SUCCESS_WITH_OPTIONAL_OUTPUT__OK;
			}
		}
	}
	else
	{
		result = apiCommandResult_FAILURE_QUIET;
	}

	if ( true == ptr_spiApiConfiguration->autoPurgeWriteBuffer )
	{
		spiApiSubCommandPurgeWriteBuffer();
	}

	return result;
}

uint8_t spiApiSubCommandAdd(struct uartStruct *ptr_uartStruct)
{
	switch( ptr_uartStruct->number_of_arguments -1 )
	{
		case 0: /* SPI a */
			/* not allowed command,
			 * either since a is not an even digit value,
			 * or a without any further arguments makes no sense.
			 *
			 * failure will be reported with in write command */

			return spiApiSubCommandWrite(ptr_uartStruct, 1);
			break;
		case 1:
		default:
	   		if ( apiCommandResult_FAILURE > spiApiAddToWriteArray(ptr_uartStruct, 2))
			{
				return apiCommandResult_SUCCESS_WITH_OPTIONAL_OUTPUT__OK;
			}
			else
			{
				return apiCommandResult_FAILURE_QUIET;
			}
			break;
	}
}

/*
 * *** sub commands with no argument
 */

uint8_t spiApiSubCommandShowStatus(void)
{
	/* header */
	createExtendedSubCommandReceiveResponseHeader(ptr_uartStruct, ptr_uartStruct->commandKeywordIndex, spiApiCommandKeyNumber_STATUS, spiApiCommandKeywords);
	UART0_Send_Message_String_p(NULL,0);

	spiApiShowStatusChipSelect();
	spiApiShowStatusControls();
	spiApiShowStatusApiSettings();
	spiApiShowStatusBuffer();

	return apiCommandResult_SUCCESS_QUIET;
}

void spiApiShowStatus( uint8_t status[], uint8_t size )
{
	uint16_t nArguments = ptr_uartStruct->number_of_arguments;
	clearString(uart_message_string, BUFFER_SIZE);

	ptr_uartStruct->number_of_arguments = 1;
	for (size_t commandIndex = 0; commandIndex < size; ++commandIndex)
	{
		spiApiSubCommands(ptr_uartStruct, status[commandIndex], 2);
	}
	ptr_uartStruct->number_of_arguments = nArguments;

}

void spiApiShowStatusApiSettings(void)
{
	uint8_t list[] = {
			spiApiCommandKeyNumber_TRANSMIT_BYTE_ORDER,
			spiApiCommandKeyNumber_TRANSMIT_REPORT,
			spiApiCommandKeyNumber_AUTO_PURGE_READ_BUFFER,
			spiApiCommandKeyNumber_AUTO_PURGE_WRITE_BUFFER };
	spiApiShowStatus( list, sizeof(list));
}

void spiApiShowStatusBuffer(void)
{
	uint8_t list[] = {
			spiApiCommandKeyNumber_SHOW_WRITE_BUFFER,
			spiApiCommandKeyNumber_SHOW_READ_BUFFER };
	spiApiShowStatus( list, sizeof(list));
}

void spiApiShowStatusChipSelect(void)
{
	uint8_t list[] = {
			spiApiCommandKeyNumber_CS,
			spiApiCommandKeyNumber_CS_BAR,
			spiApiCommandKeyNumber_CS_PINS,
			spiApiCommandKeyNumber_CS_SELECT_MASK };
	spiApiShowStatus( list, sizeof(list));
}

void spiApiShowStatusControls()
{
	uint8_t list[] = {
			spiApiCommandKeyNumber_CONTROL_BITS,
	};
	spiApiShowStatus( list, sizeof(list));
}


void spiApiShowStatusControlBits()
{
	uint8_t list[] = {
			spiApiCommandKeyNumber_SPI_ENABLE,
			spiApiCommandKeyNumber_DATA_ORDER,
			spiApiCommandKeyNumber_MASTER,
			spiApiCommandKeyNumber_CLOCK_POLARITY,
			spiApiCommandKeyNumber_CLOCK_PHASE };

	spiApiShowStatus( list, sizeof(list));
	spiApiShowStatusSpeed();
}

void spiApiShowStatusSpeed(void)
{
	uint8_t list[] = {
			spiApiCommandKeyNumber_SPEED,
			spiApiCommandKeyNumber_DOUBLE_SPEED,
			spiApiCommandKeyNumber_SPEED_DIVIDER };
	spiApiShowStatus( list, sizeof(list));
}

uint8_t spiApiSubCommandTransmit(void)
{
	uint8_t result = spiWriteAndReadWithoutChipSelect( ptr_spiApiConfiguration->transmitByteOrder );
	if (  apiCommandResult_FAILURE <= result)
	{
		result = apiCommandResult_FAILURE_QUIET;
		return result;
	}
	if ( true == ptr_spiApiConfiguration->reportTransmit )
	{
		spiApiSubCommandShowWriteBuffer(ptr_uartStruct);
		return apiCommandResult_SUCCESS_QUIET;
	}
	return apiCommandResult_SUCCESS_WITH_OPTIONAL_OUTPUT__OK;
}

uint8_t spiApiSubCommandWriteBuffer(void)
{
	uint8_t result = apiCommandResult_UNDEFINED;
	uint8_t csSelectMask = 0xFF;

	switch (ptr_uartStruct->number_of_arguments - 1)
	{
		case 0: /*default*/
			csSelectMask = ptr_spiApiConfiguration->csExternalSelectMask;
		break;
		case 1: /*else*/
		default:
			result = apiAssignParameterToValue( 2, &csSelectMask, apiVarType_UINT8, 0, 0xFF );
			if ( apiCommandResult_FAILURE <= result )
			{
				return apiCommandResult_FAILURE_QUIET;
			}
			break;
	}

	if ( true == ptr_spiApiConfiguration->autoPurgeReadBuffer )
	{
		spiApiSubCommandPurgeReadBuffer();
	}

	result = spiWriteAndReadWithChipSelect( ptr_spiApiConfiguration->transmitByteOrder, csSelectMask );

	if ( 0 != result )
	{
		result = apiCommandResult_FAILURE_QUIET;
	}
	else
	{
		if ( true == ptr_spiApiConfiguration->reportTransmit )
		{
			spiApiSubCommandShowWriteBuffer(ptr_uartStruct);
			result = apiCommandResult_SUCCESS_QUIET;
		}
		else
		{
			result = apiCommandResult_SUCCESS_WITH_OPTIONAL_OUTPUT__OK;
		}
	}

	if ( true == ptr_spiApiConfiguration->autoPurgeWriteBuffer )
	{
		spiApiSubCommandPurgeWriteBuffer();
	}

	return result;
}

uint8_t spiApiSubCommandReset(void)
{
	spiInit();
	spiApiInit();
	spiEnable(true);
	return apiCommandResult_SUCCESS_WITH_OPTIONAL_OUTPUT__OK;
}

uint8_t spiApiSubCommandPurge(void)
{
	spiApiSubCommandPurgeWriteBuffer();
	spiApiSubCommandPurgeReadBuffer();

	return apiCommandResult_SUCCESS_WITH_OPTIONAL_OUTPUT__OK;
}

uint8_t spiApiSubCommandPurgeWriteBuffer(void)
{
	spiPurgeWriteData();
	return apiCommandResult_SUCCESS_WITH_OPTIONAL_OUTPUT__OK;
}

uint8_t spiApiSubCommandPurgeReadBuffer(void)
{
	spiPurgeReadData();
	return apiCommandResult_SUCCESS_WITH_OPTIONAL_OUTPUT__OK;
}

uint8_t spiApiSubCommandRead(void)
{
	if (SPI_MSBYTE_FIRST == ptr_spiApiConfiguration->transmitByteOrder)
	{
		return spiApiShowBufferContent( ptr_uartStruct, &spiReadData, -1, spiApiCommandKeyNumber_READ);
	}
	else
	{
		return spiApiShowBufferContent( ptr_uartStruct, &spiReadData,  1, spiApiCommandKeyNumber_READ);
	}
}

/*
 * *** commands with optional arguments
 */

uint8_t spiApiSubCommandCsStatus(struct uartStruct *ptr_uartStruct)
{
	return spiApiCsStatus(ptr_uartStruct, false);
}

uint8_t spiApiSubCommandCsBarStatus(struct uartStruct *ptr_uartStruct)
{
	return spiApiCsStatus(ptr_uartStruct, true);
}

uint8_t spiApiCsStatus(struct uartStruct *ptr_uartStruct, bool invert) /*optional external mask*/
{
	uint8_t mask = 0;
	uint16_t result = apiCommandResult_SUCCESS_QUIET;

	switch (ptr_uartStruct->number_of_arguments - 1)
	{
		case 0: /* show all */
			mask = 0xFF;
		break;
		case 1: /* show mask*/
		default:
			result = apiAssignParameterToValue(2, &mask, apiVarType_UINT8, 0,0xFF);
		break;
	}

	if (apiCommandResult_FAILURE > result)
	{
		spiApiShowChipSelectStatus(0xFF & mask, invert);
		result = apiCommandResult_SUCCESS_WITH_OUTPUT;
	}

	return result;
}

void spiApiShowChipSelectStatus(uint8_t mask, bool invert)
{
	bool status = 0;
	uint8_t statusArray = spiGetCurrentChipSelectBarStatus();
	uint8_t activeMask = spiGetChipSelectArrayStatus();
	for (int chipSelectIndex = 0; chipSelectIndex < SPI_CHIPSELECT_MAXIMUM; ++chipSelectIndex)
	{
		if (mask & (0x1 << chipSelectIndex))
		{
			if (activeMask & (0x1 << chipSelectIndex))
			{
				status = ((statusArray & (0x1 << chipSelectIndex)) != 0 );
				//status = (chipSelectIndex+1)%2 > 0; /*dummy*/
				if (!invert)
				{
					status ^= 0x1;
				}
				snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s%i:%i "), uart_message_string,
						chipSelectIndex + 1,
						status);
			}
			else
			{
				snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s%i:- "), uart_message_string, chipSelectIndex + 1);
			}
		}
	}
}


uint8_t spiApiSubCommandCsSet(void)
{
	return spiApiCsSetOrCsRelease( true );
}

uint8_t spiApiSubCommandCsRelease(void)
{
	return spiApiCsSetOrCsRelease( false );
}

uint8_t spiApiCsSetOrCsRelease( bool set )
{
	uint8_t externalChipSelectMask = 0;
	uint16_t result = apiCommandResult_SUCCESS_QUIET;

	switch (ptr_uartStruct->number_of_arguments - 1)
	{
		case 0: /* set all active within configuration's mask */
			externalChipSelectMask = ptr_spiApiConfiguration->csExternalSelectMask;
		break;
		case 1: /* show mask*/
		default:
			result = apiAssignParameterToValue(2, &externalChipSelectMask, apiVarType_UINT8, 0,0xFF);
		break;
	}

	if (apiCommandResult_FAILURE > result)
	{
		if ( set )
		{
			spiSetChosenChipSelect(externalChipSelectMask);
		}
		else
		{
			spiReleaseChosenChipSelect(externalChipSelectMask);
		}

		/* report */
		spiApiShowStatus( ({ uint8_t list[] = { spiApiCommandKeyNumber_CS }; &list[0];}), 1);
		result = apiCommandResult_SUCCESS_QUIET;

	}

	return result;

}

uint8_t spiApiSubCommandCsSelectMask(struct uartStruct *ptr_uartStruct)
{
	return apiShowOrAssignParameterToValue(ptr_uartStruct->number_of_arguments - 1, 2,  &(ptr_spiApiConfiguration->csExternalSelectMask), apiVarType_UINT8, 0, 0xFF, true, NULL);
}

uint8_t spiApiSubCommandAutoPurgeReadBuffer(struct uartStruct *ptr_uartStruct)
{
	return apiShowOrAssignParameterToValue(ptr_uartStruct->number_of_arguments - 1, 2,  &(ptr_spiApiConfiguration->autoPurgeReadBuffer), apiVarType_BOOL_TrueFalse, 0, 0xFF, true, NULL);
}

uint8_t spiApiSubCommandAutoPurgeWriteBuffer(struct uartStruct *ptr_uartStruct)
{
	return apiShowOrAssignParameterToValue(ptr_uartStruct->number_of_arguments - 1, 2,  &(ptr_spiApiConfiguration->autoPurgeWriteBuffer), apiVarType_BOOL_TrueFalse, 0, 0xFF, true, NULL);
}

uint8_t spiApiSubCommandTransmitReport(struct uartStruct *ptr_uartStruct)
{
	return apiShowOrAssignParameterToValue(ptr_uartStruct->number_of_arguments - 1, 2,  &(ptr_spiApiConfiguration->reportTransmit), apiVarType_BOOL_TrueFalse, 0, 0xFF, true, NULL);
}

uint8_t spiApiSubCommandCsPins(struct uartStruct *ptr_uartStruct)
{
	/*set/get hardware addresses of multiple CS outputs*/

	uint8_t channelIndex = 0;
    uint16_t result = 0;

	switch (ptr_uartStruct->number_of_arguments - 1)
	{
		case 0: /*read all*/
			return spiApiShowChipSelectAddress(-1);
		break;
		case 1: /*read one*/
		default:
			result = apiAssignParameterToValue(2,  &(channelIndex), apiVarType_UINT8, 1, 0x8);
			if ( result < apiCommandResult_FAILURE )
			{
				result = spiApiShowChipSelectAddress(channelIndex);
			}
			break;
	}
	return result;
}


/* uint8_t spiApiShowChipSelectAddress(int8_t chipSelectIndex)
 *
 * 	-1		: 	all
 * 	1 ... 8 :	selected
 * 	else 	:	error
 */
uint8_t spiApiShowChipSelectAddress(int8_t chipSelectIndex)
{
	if (0 == chipSelectIndex || 8 < chipSelectIndex)
	{
		CommunicationError_p(ERRA, SERIAL_ERROR_arguments_exceed_boundaries, true, NULL);
		return apiCommandResult_FAILURE_QUIET;
	}

	uint8_t activeMask = spiGetChipSelectArrayStatus();
	for (int index = 0; index < SPI_CHIPSELECT_MAXIMUM; ++index)
	{
		if ( 0 < chipSelectIndex && index != chipSelectIndex -1 )
		{
			continue;
		}

		if ( 0 < chipSelectIndex || (activeMask & (0x1 << index)))
		{
			volatile uint8_t *portPointer = spiGetPortFromChipSelect(index);
			snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s%i:"), uart_message_string, index + 1);

			switch((int) portPointer)
			{
				case (int) &PORTA:
				case (int) &PORTB:
				case (int) &PORTC:
				case (int) &PORTD:
				case (int) &PORTE:
				case (int) &PORTF:
				case (int) &PORTG:
				{
					strncat_P(uart_message_string, string_PORT, BUFFER_SIZE - 1);
					switch((int) portPointer)
					{
						case (int) &PORTA:
								strncat_P(uart_message_string, PSTR("A"), BUFFER_SIZE - 1);
						break;
						case (int) &PORTB:
								strncat_P(uart_message_string, PSTR("B"), BUFFER_SIZE - 1);
						break;
						case (int) &PORTC:
								strncat_P(uart_message_string, PSTR("C"), BUFFER_SIZE - 1);
						break;
						case (int) &PORTD:
								strncat_P(uart_message_string, PSTR("D"), BUFFER_SIZE - 1);
						break;
						case (int) &PORTE:
								strncat_P(uart_message_string, PSTR("E"), BUFFER_SIZE - 1);
						break;
						case (int) &PORTF:
								strncat_P(uart_message_string, PSTR("F"), BUFFER_SIZE - 1);
						break;
						case (int) &PORTG:
								strncat_P(uart_message_string, PSTR("G"), BUFFER_SIZE - 1);
						break;
					}
				}
				break;
				default:
					snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s%p"), uart_message_string, portPointer);
					break;
			}

			snprintf_P(uart_message_string, BUFFER_SIZE - 1, string_sKommax, uart_message_string,
					spiGetPinFromChipSelect(index));

			if ( chipSelectIndex > 0 )
			{
				strncat_P(uart_message_string, PSTR(","), BUFFER_SIZE - 1);
				bool isUsed = (spiGetCurrentChipSelectArray())[index].isUsed;
				apiShowValue(uart_message_string, &isUsed, apiVarType_BOOL_OnOff );
//				apiShowValue(NULL, &((spiGetCurrentChipSelectArray())[index].isUsed), apiVarType_BOOL_OnOff );
			}
			strncat_P(uart_message_string, PSTR(" "), BUFFER_SIZE - 1);
		}
	}
	return apiCommandResult_SUCCESS_WITH_OUTPUT;
}


uint8_t spiApiSubCommandShowWriteBuffer(struct uartStruct *ptr_uartStruct)
{
	return spiApiShowBuffer( ptr_uartStruct, &spiWriteData, spiApiCommandKeyNumber_SHOW_WRITE_BUFFER);
}

uint8_t spiApiShowBuffer(struct uartStruct *ptr_uartStruct, spiByteDataArray *buffer, int8_t subCommandKeywordIndex)
{
	bool revert = false;
    uint16_t result = 0;
    int16_t nBytes = 0;

	switch (ptr_uartStruct->number_of_arguments - 1)
	{
		case 0: /*show all*/
			nBytes = 0;
		break;
		case 1: /*read selected*/
			result = apiAssignParameterToValue(2,  &(nBytes), apiVarType_UINT16, 0, INT16_MAX);
			if ( apiCommandResult_FAILURE <= result)
			{
				return result;
			}
			break;
		case 2: /*read selected, optionally inverted*/
		default:
			result = apiAssignParameterToValue(2,  &(nBytes), apiVarType_UINT16, 0, INT16_MAX);
			if ( apiCommandResult_FAILURE <= result)
			{
				return result;
			}
			result = apiAssignParameterToValue(3,  &(revert), apiVarType_BOOL, 0, 0xFF);
			if ( apiCommandResult_FAILURE <= result)
			{
				return result;
			}
			if (revert)
			{
				nBytes = 0 - nBytes;
			}
			break;
	}

	return spiApiShowBufferContent( ptr_uartStruct, buffer, nBytes, subCommandKeywordIndex);
}

uint8_t spiApiSubCommandShowReadBuffer(struct uartStruct *ptr_uartStruct)
{
	return spiApiShowBuffer( ptr_uartStruct, &spiReadData, spiApiCommandKeyNumber_SHOW_READ_BUFFER);
}

uint8_t spiApiSubCommandControlBits(struct uartStruct *ptr_uartStruct)
{
	ptr_spiApiConfiguration->spiConfiguration = spiGetConfiguration();

	uint8_t result = apiShowOrAssignParameterToValue(ptr_uartStruct->number_of_arguments - 1, 2,
			         &(ptr_spiApiConfiguration->spiConfiguration.data), apiVarType_UINT16, 0, 0x1FF, true, NULL);

	if ( apiCommandResult_FAILURE > result )
	{
		if (0 < ptr_uartStruct->number_of_arguments -1)
		{
			spiSetConfiguration(ptr_spiApiConfiguration->spiConfiguration);
		}
		UART0_Send_Message_String_p(uart_message_string, BUFFER_SIZE);

		spiApiShowStatusControlBits();

		result = apiCommandResult_SUCCESS_QUIET;
	}
	return result;
}

uint8_t spiApiSubCommandSpiEnable(struct uartStruct *ptr_uartStruct)
{
	ptr_spiApiConfiguration->spiConfiguration = spiGetConfiguration();
	uint8_t bits = ptr_spiApiConfiguration->spiConfiguration.bits.bSpe;

	uint8_t result = apiShowOrAssignParameterToValue(ptr_uartStruct->number_of_arguments - 1, 2,  &bits, apiVarType_BOOL_TrueFalse, 0, 0x1, true, NULL);

	if ( apiCommandResult_FAILURE > result )
	{
		if (0 < ptr_uartStruct->number_of_arguments -1)
		{
			ptr_spiApiConfiguration->spiConfiguration.bits.bSpe = 0x1 & bits;
			spiSetConfiguration(ptr_spiApiConfiguration->spiConfiguration);
		}
	}
	return result;
}

uint8_t spiApiSubCommandDataOrder(struct uartStruct *ptr_uartStruct)
{
	ptr_spiApiConfiguration->spiConfiguration = spiGetConfiguration();
	uint8_t bits = ptr_spiApiConfiguration->spiConfiguration.bits.bDord;

	uint8_t result = apiShowOrAssignParameterToValue(ptr_uartStruct->number_of_arguments - 1, 2,  &bits, apiVarType_UINT8, 0, 0x1, true, NULL);

	if ( apiCommandResult_FAILURE > result )
	{
		if (0 < ptr_uartStruct->number_of_arguments -1)
		{
			ptr_spiApiConfiguration->spiConfiguration.bits.bDord = 0x1 & bits;
			spiSetConfiguration(ptr_spiApiConfiguration->spiConfiguration);
		}
	}
	return result;
}

uint8_t spiApiSubCommandMaster(struct uartStruct *ptr_uartStruct)
{
	ptr_spiApiConfiguration->spiConfiguration = spiGetConfiguration();
	uint8_t bits = ptr_spiApiConfiguration->spiConfiguration.bits.bMstr;

	/* TODO: implement client ??? */
	uint8_t result = apiShowOrAssignParameterToValue(ptr_uartStruct->number_of_arguments - 1, 2,  &bits, apiVarType_BOOL_TrueFalse, 0x1, 0x1, true, NULL);

	if ( apiCommandResult_FAILURE > result )
	{
		if (0 < ptr_uartStruct->number_of_arguments -1)
		{
			ptr_spiApiConfiguration->spiConfiguration.bits.bMstr= 0x1 & bits;
			spiSetConfiguration(ptr_spiApiConfiguration->spiConfiguration);
		}
	}
	return result;
}

uint8_t spiApiSubCommandClockPolarity(struct uartStruct *ptr_uartStruct)
{
	ptr_spiApiConfiguration->spiConfiguration = spiGetConfiguration();
	uint8_t bits = ptr_spiApiConfiguration->spiConfiguration.bits.bCpol;

	uint8_t result = apiShowOrAssignParameterToValue(ptr_uartStruct->number_of_arguments - 1, 2,  &bits, apiVarType_UINT8, 0, 0x1, true, NULL);

	if ( apiCommandResult_FAILURE > result )
	{
		if (0 < ptr_uartStruct->number_of_arguments -1)
		{
			ptr_spiApiConfiguration->spiConfiguration.bits.bCpol = 0x1 & bits;
			spiSetConfiguration(ptr_spiApiConfiguration->spiConfiguration);
		}
	}
	return result;
}

uint8_t spiApiSubCommandClockPhase(struct uartStruct *ptr_uartStruct)
{
	ptr_spiApiConfiguration->spiConfiguration = spiGetConfiguration();
	uint8_t bits = ptr_spiApiConfiguration->spiConfiguration.bits.bCpha;

	uint8_t result = apiShowOrAssignParameterToValue(ptr_uartStruct->number_of_arguments - 1, 2,  &bits, apiVarType_UINT8, 0, 0x1, true, NULL);

	if ( apiCommandResult_FAILURE > result )
	{
		if (0 < ptr_uartStruct->number_of_arguments -1)
		{
			ptr_spiApiConfiguration->spiConfiguration.bits.bCpha = 0x1 & bits;
			spiSetConfiguration(ptr_spiApiConfiguration->spiConfiguration);
		}
	}
	return result;
}

uint8_t spiApiSubCommandSpeed(struct uartStruct *ptr_uartStruct)
{
	ptr_spiApiConfiguration->spiConfiguration = spiGetConfiguration();
	uint8_t bits = ptr_spiApiConfiguration->spiConfiguration.bits.bSpr;

	uint8_t result = apiShowOrAssignParameterToValue(ptr_uartStruct->number_of_arguments - 1, 2,  &bits, apiVarType_UINT8, 0, 0x3, true, NULL);

	if ( apiCommandResult_FAILURE > result )
	{
		if (0 < ptr_uartStruct->number_of_arguments -1)
		{
			ptr_spiApiConfiguration->spiConfiguration.bits.bSpr = 0x3 & bits;
			spiSetConfiguration(ptr_spiApiConfiguration->spiConfiguration);
		}
	}
	return result;
}

uint8_t spiApiSubCommandSpeedDivider(struct uartStruct *ptr_uartStruct)
{
	ptr_spiApiConfiguration->spiConfiguration = spiGetConfiguration();
	uint8_t divider = 0;
	uint8_t shift = 0;

	switch(ptr_spiApiConfiguration->spiConfiguration.bits.bSpr & 0x3)
	{
		case 0:
			divider = 0x4;
			shift = 2;
			break;
		case 1:
			divider = 0x10;
			shift = 4;
			break;
		case 2:
			divider = 0x40;
			shift = 6;
			break;
		case 3:
			divider = 0x80;
			shift = 7;
			break;
	}
	switch(ptr_spiApiConfiguration->spiConfiguration.bits.bSpi2x & 0x1)
	{
		case 0:
			break;
		case 1:
			divider >>= 1 ;
			shift -= 1;
			break;
	}

	uint8_t result = apiShowOrAssignParameterToValue(ptr_uartStruct->number_of_arguments - 1, 2,  &divider, apiVarType_UINT8, 0, 0x80, true, NULL);

	if ( apiCommandResult_FAILURE > result )
	{
		if (0 < ptr_uartStruct->number_of_arguments -1)
		{
			switch(divider)
			{
				case 0x02:
					ptr_spiApiConfiguration->spiConfiguration.bits.bSpr   = 0x0;
					ptr_spiApiConfiguration->spiConfiguration.bits.bSpi2x = 0x1;
					shift = 1;
					break;
				case 0x04:
					ptr_spiApiConfiguration->spiConfiguration.bits.bSpr   = 0x0;
					ptr_spiApiConfiguration->spiConfiguration.bits.bSpi2x = 0x0;
					shift = 2;
					break;
				case 0x08:
					ptr_spiApiConfiguration->spiConfiguration.bits.bSpr   = 0x1;
					ptr_spiApiConfiguration->spiConfiguration.bits.bSpi2x = 0x1;
					shift = 3;
					break;
				case 0x10:
					ptr_spiApiConfiguration->spiConfiguration.bits.bSpr   = 0x1;
					ptr_spiApiConfiguration->spiConfiguration.bits.bSpi2x = 0x0;
					shift = 4;
					break;
				case 0x20:
					ptr_spiApiConfiguration->spiConfiguration.bits.bSpr   = 0x2;
					ptr_spiApiConfiguration->spiConfiguration.bits.bSpi2x = 0x1;
					shift = 5;
					break;
				case 0x40:
					ptr_spiApiConfiguration->spiConfiguration.bits.bSpr   = 0x2;
					ptr_spiApiConfiguration->spiConfiguration.bits.bSpi2x = 0x0;
					shift = 6;
					break;
				case 0x80:
					ptr_spiApiConfiguration->spiConfiguration.bits.bSpr   = 0x3;
					ptr_spiApiConfiguration->spiConfiguration.bits.bSpi2x = 0x0;
					shift = 7;
					break;
				default:
					CommunicationError_p(ERRA, SERIAL_ERROR_argument_has_invalid_type, true, NULL);
					result = apiCommandResult_FAILURE_QUIET;
					break;
			}
			if ( apiCommandResult_FAILURE > result )
			{
				spiSetConfiguration(ptr_spiApiConfiguration->spiConfiguration);
			}
		}
		snprintf_P(uart_message_string,  BUFFER_SIZE - 1, PSTR("%s (%luHz @ %luHz)"), uart_message_string, (unsigned long) (F_CPU >> shift), (unsigned long) (F_CPU) );
	}

	return result;
}

uint8_t spiApiSubCommandDoubleSpeed(struct uartStruct *ptr_uartStruct)
{
	ptr_spiApiConfiguration->spiConfiguration = spiGetConfiguration();
	uint8_t bits = ptr_spiApiConfiguration->spiConfiguration.bits.bSpi2x;

	uint8_t result = apiShowOrAssignParameterToValue(ptr_uartStruct->number_of_arguments - 1, 2,  &bits, apiVarType_BOOL_TrueFalse, 0, 0x1, true, NULL);

	if ( apiCommandResult_FAILURE > result )
	{
		if (0 < ptr_uartStruct->number_of_arguments -1)
		{
			ptr_spiApiConfiguration->spiConfiguration.bits.bSpi2x = 0x1 & bits;
			spiSetConfiguration(ptr_spiApiConfiguration->spiConfiguration);
		}
	}
	return result;
}


/*spi api settings*/
uint8_t spiApiSubCommandTransmitByteOrder(struct uartStruct *ptr_uartStruct)
{
	uint8_t result = apiShowOrAssignParameterToValue(ptr_uartStruct->number_of_arguments - 1, 2,  &(ptr_spiApiConfiguration->transmitByteOrder), apiVarType_UINT8, 0, 0x1, true, NULL);

	if ( apiCommandResult_FAILURE > result )
	{
		if (0 == ptr_uartStruct->number_of_arguments -1)
		{
			switch (ptr_spiApiConfiguration->transmitByteOrder)
			{
				case spiApiTransmitByteOrder_MSB:
					strncat_P(uart_message_string, PSTR(" (MSB/big endian)"), BUFFER_SIZE -1);
					break;
				case spiApiTransmitByteOrder_LSB:
					strncat_P(uart_message_string, PSTR(" (LSB/little endian)"), BUFFER_SIZE -1);
					break;
				default:
					strncat_P(uart_message_string, PSTR(" UNDEFINED"), BUFFER_SIZE -1);
					break;
			}
		}
	}
	return result;
}

uint8_t spiApiSubCommandCsAddPin(struct uartStruct *ptr_uartStruct)
{
	uint8_t chipSelectNumber = 0;
	spiPin cs;
    uint8_t result = apiCommandResult_UNDEFINED;
    uint8_t parameterIndex = 0;
	switch (ptr_uartStruct->number_of_arguments - 1)
	{
		case 0:
		case 1:
			CommunicationError_p(ERRA, SERIAL_ERROR_too_few_arguments, true, NULL);
			result = apiCommandResult_FAILURE_QUIET;
			break;
		case 2:
		case 3: /*write*/
		default:
			/* 1st argument, string PORTA...G */
			parameterIndex = 2;
			if ( isNumericArgument(setParameter[parameterIndex], MAX_LENGTH_PARAMETER) )
			{
				result = apiAssignParameterToValue(parameterIndex, &(cs.ptrPort), apiVarType_UINTPTR, UINT64_C(0), UINT64_C(0xFF));
			}
			else
			{
				/* match "port/PORTx/X" ?*/
				if (0 == strncasecmp_P(setParameter[parameterIndex], string_PORT, 3) && 5 == strlen(setParameter[parameterIndex]))
				{
					result = apiCommandResult_SUCCESS_QUIET;
					switch(setParameter[parameterIndex][4])
					{
						case 'A':
						case 'a':
							cs.ptrPort = &PORTA;
							break;
						case 'B':
						case 'b':
							cs.ptrPort = &PORTB;
							break;
						case 'C':
						case 'c':
							cs.ptrPort = &PORTC;
							break;
						case 'D':
						case 'd':
							cs.ptrPort = &PORTD;
							break;
						case 'E':
						case 'e':
							cs.ptrPort = &PORTE;
							break;
						case 'F':
						case 'f':
							cs.ptrPort = &PORTF;
							break;
						case 'G':
						case 'g':
							cs.ptrPort = &PORTG;
							break;
						default:
							CommunicationError_p(ERRA, SERIAL_ERROR_argument_has_invalid_type, true, NULL);
							result = apiCommandResult_FAILURE_QUIET;
							break;
					}
				}
				else
				{
					CommunicationError_p(ERRA, SERIAL_ERROR_argument_has_invalid_type, true, NULL);
					result = apiCommandResult_FAILURE_QUIET;
					break;
				}
			}
			if ( apiCommandResult_FAILURE <= result )
			{
				break;
			}

			/* 2nd argument, pin */
			parameterIndex = 3;
			result = apiAssignParameterToValue(parameterIndex, &(cs.pinNumber), apiVarType_UINT8, 0, 7);
			if ( apiCommandResult_FAILURE <= result )
			{
				break;
			}

			/* optional 3rd argument, chipSelectNumber */
			parameterIndex = 4;
			if ( ptr_uartStruct->number_of_arguments -1 > 2)
			{
				result = apiAssignParameterToValue(parameterIndex, &chipSelectNumber, apiVarType_UINT8, 1, 8);
				if ( apiCommandResult_FAILURE <= result )
				{
					break;
				}
			}
			else
			{
				/* find open slot */
				chipSelectNumber = SPI_CHIPSELECT0;
				while (chipSelectNumber < SPI_CHIPSELECT_MAXIMUM)
				{
					if (false == (spiGetCurrentChipSelectArray()[chipSelectNumber]).isUsed)
					{
						break;
					}
					chipSelectNumber++;
				}
				if ( SPI_CHIPSELECT_MAXIMUM == chipSelectNumber)
				{
					CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, true, PSTR("max #slots (%i) reached"), SPI_CHIPSELECT_MAXIMUM);
					result = apiCommandResult_FAILURE_QUIET;
					break;
				}
				/* increase to match software counting of cs [1, 8]*/
				chipSelectNumber++;
			}

			/* check doubles */
			for ( uint8_t slot  = SPI_CHIPSELECT0; slot < SPI_CHIPSELECT_MAXIMUM; slot++)
			{
				if ((spiGetCurrentChipSelectArray()[slot]).isUsed &&
					cs.pinNumber == (spiGetCurrentChipSelectArray()[slot]).pinNumber &&
					cs.ptrPort == (spiGetCurrentChipSelectArray()[slot]).ptrPort )
				{
					CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, true, PSTR("port/pin in use"));
					result = apiCommandResult_FAILURE_QUIET;
					break;
				}
			}
			if ( apiCommandResult_FAILURE <= result )
			{
				break;
			}

			/*add cs*/
			/* subtract -1 to match hardware counting of cs [0,7]*/
			result = spiAddChipSelect(cs.ptrPort, cs.pinNumber, chipSelectNumber-1);
			if ( 0 == result)
			{
				/* report */
				spiApiShowStatus( ({ uint8_t list[] = { spiApiCommandKeyNumber_CS_PINS }; &list[0];}), 1);
				result = apiCommandResult_SUCCESS_QUIET;
			}
			else
			{
				result = apiCommandResult_FAILURE_QUIET;
			}
			break;
	}
	return result;
}

uint8_t spiApiSubCommandCsRemovePin(struct uartStruct *ptr_uartStruct)
{
	uint8_t chipSelectNumber = 0;
	switch (ptr_uartStruct->number_of_arguments - 1)
	{
		case 0: /*print*/
			CommunicationError_p(ERRA, SERIAL_ERROR_too_few_arguments, true, NULL);
			return apiCommandResult_FAILURE_QUIET;
			break;
		case 1: /*write*/
		default:
			/* take second parameter, i.e. first argument to sub command and fill it into value*/
			if ( apiCommandResult_FAILURE > apiAssignParameterToValue(2, &chipSelectNumber, apiVarType_UINT8, 1, 8) )
			{
				if ( 0 == spiRemoveChipSelect(chipSelectNumber-1))
				{
					spiApiShowStatus( ({ uint8_t list[] = { spiApiCommandKeyNumber_CS_PINS }; &list[0];}), 1);
					return apiCommandResult_SUCCESS_QUIET;
				}
				else
				{
					return apiCommandResult_FAILURE_QUIET;
				}
			}
			break;
	}
	return apiCommandResult_SUCCESS_QUIET;
}


/* helpers */

uint8_t spiApiPurgeAndFillWriteArray(struct uartStruct *ptr_uartStruct, uint16_t parameterStartIndex)
{
	/* reset array*/
	spiPurgeWriteData();
	return spiApiAddToWriteArray(ptr_uartStruct, parameterStartIndex);
}

uint8_t spiApiAddToWriteArray(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex)
{
	uint8_t result = apiCommandResult_UNDEFINED;
	uint8_t nTotalArgs = ptr_uartStruct->number_of_arguments;
	uint8_t argumentCounter = parameterIndex;
    while( spiWriteData.length < sizeof(spiWriteData.data) && argumentCounter < nTotalArgs + 1)
    {
		parameterIndex = argumentCounter % MAX_PARAMETER;
		printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, filename, PSTR("%i"), argumentCounter);

		printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, filename,
				PSTR("setParameter[%i]: \"%s\""), parameterIndex, setParameter[parameterIndex]);

		/* get contents from parameter container */
		result = spiApiAddNumericParameterToByteArray(NULL, parameterIndex);
		if ( apiCommandResult_FAILURE <= result )
		{
			return result;
			break;
		}
		argumentCounter++;

		if (0 == argumentCounter % MAX_PARAMETER )
		{
    		if (strlen(decrypt_uartString_remainder))
    		{
    			/* get contents from remainder container */
    			/* reuse setParameter as long as arguments are there*/
    			clearString(resultString, BUFFER_SIZE);
    			strncpy(resultString, decrypt_uartString_remainder, BUFFER_SIZE);
    			clearString(decrypt_uartString_remainder, BUFFER_SIZE);

    			printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, filename, PSTR("'%s'"), resultString);

    			if ( 1 > uartSplitUartString( resultString )  )
    			{
    				break;
    			}
    		}
			else
			{
				break;
			}
    	}
    }

    return result;
}//END of function

uint8_t spiApiAddNumericParameterToByteArray(const char string[], uint8_t index)
{
	int8_t result;
	printDebug_p(debugLevelEventDebugVerbose, debugSystemSPI, __LINE__, filename, PSTR("para to byte[]"));

	/* get string string[] */
	if ( 0 > index || MAX_PARAMETER <= index  )
	{
		if ( NULL == string)
		{
			CommunicationError_p(ERRG, dynamicMessage_ErrorIndex, TRUE, PSTR("NULL pointer received"));
			return -1;
		}
		result = spiApiAddNumericStringToByteArray( string );
		if ( apiCommandResult_FAILURE <= result )
		{
			return result;
		}
	}
	/* get string from setParameter at the index */
	else
	{
		result = spiApiAddNumericStringToByteArray( setParameter[index] );
		if ( apiCommandResult_FAILURE <= result )
		{
			return result;
		}
	}
	return apiCommandResult_SUCCESS_WITH_OPTIONAL_OUTPUT__OK;
}

uint8_t spiApiAddNumericStringToByteArray(const char string[])
{
	printDebug_p(debugLevelEventDebugVerbose, debugSystemSPI, __LINE__, filename, PSTR("string to byte[]"));

	uint64_t value;
	size_t numberOfDigits;

	if ( NULL == string)
	{
		CommunicationError_p(ERRG, dynamicMessage_ErrorIndex, TRUE, PSTR("NULL pointer received"));
		return apiCommandResult_FAILURE_QUIET;
	}

	if ( isNumericArgument(string, MAX_LENGTH_COMMAND - MAX_LENGTH_KEYWORD - 2))
	{
		/* get number of digits*/
		numberOfDigits = getNumberOfHexDigits(string, MAX_LENGTH_COMMAND - MAX_LENGTH_KEYWORD - 2 );

		// only even number of digits are allowed, since only complete bytes are transmitted
		if (numberOfDigits%2 )
		{
			CommunicationError_p(ERRG, dynamicMessage_ErrorIndex, TRUE, PSTR("only EVEN number of digits allowed (%s)"), string);
			return apiCommandResult_FAILURE_QUIET;
		}

		/* check if new element to be added, split into bytes, MSB to LSB,
		 * would exceed the maximum size of data array */

		size_t numberOfBytes = ((numberOfDigits + numberOfDigits%2) >> 1);

		printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, filename,
				PSTR("#bytes/#digits %i/%i"), numberOfBytes, numberOfDigits);

		if ( spiWriteData.length + numberOfBytes < sizeof(spiWriteData.data) -1 )
		{
			printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, filename,
					PSTR("valueString '%s' "), string);

			if ( 16 >= numberOfDigits ) /*64 bit*/
			{
				if ( 0 == getUnsignedNumericValueFromParameterString(string, &value) )
				{
					for (int8_t byteIndex = numberOfBytes - 1; byteIndex >= 0; byteIndex--)
					{
						spiAddWriteData( 0xFF & (value >> (8 * byteIndex)) );
					}
				}
				else
				{
					CommunicationError_p(ERRA, SERIAL_ERROR_argument_has_invalid_type, TRUE, PSTR("%s"), string);
					return apiCommandResult_FAILURE_QUIET;
				}
			} /* > 64 bit*/
			else
			{
				size_t charIndex = 0;

				/* skip leading 0x/0X */
				if ( 0 == strncasecmp_P (string, PSTR("0x"), sizeof(string)))
				{
					charIndex+=2;
				}

				while (charIndex + 1 < numberOfDigits)
				{
					byte[0]=string[charIndex];
					byte[1]=string[charIndex + 1];
					charIndex+=2;

					spiAddWriteData( strtoul(byte, NULL, 16) );
				}
			}

			/* debug report */
			for (int8_t byteIndex = numberOfBytes - 1; byteIndex >= 0; byteIndex--)
			{
				printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, filename,
						PSTR("data[%i]=%x"), spiWriteData.length -1 - byteIndex, spiWriteData.data[spiWriteData.length -1 - byteIndex]);
			}
		}
		else
		{
			CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, TRUE, PSTR("too many bytes to add"));
			return apiCommandResult_FAILURE_QUIET;
		}
	}
	else
	{
		CommunicationError_p(ERRA, SERIAL_ERROR_argument_has_invalid_type, TRUE, PSTR("%s"), string);
		return apiCommandResult_FAILURE_QUIET;
	}

	return apiCommandResult_SUCCESS_WITH_OPTIONAL_OUTPUT__OK;
}

/* uint8_t spiApiShowBufferContent(spiByteDataArray *buffer, int16_t nRequestedBytes, int8_t commandKeywordIndex, int8_t subCommandKeywordIndex, PGM_P commandKeywords[])
 *
 * shows the content of the data in buffer
 * and prints out the corresponding caller's command and sub command keyword
 *
 * Arguments:
 *
 *	buffer:
 * 						pointer to the buffer struct
 * 	nRequestedBytes:
 * 						number of bytes to show
 * 						0:		 	all available (max: buffer->length)
 * 						positive:	first nBytes of buffer,
 * 										data index 0 ... nBytes
 * 						negative:	last  nBytes of buffer,
 * 										data index buffer->length - 1 - nBytes ... buffer->length - 1
 *
 * 	commandKeywordIndex
 * 	subCommandKeywordIndex
 * 	subCommandKeywordArray
 */

uint8_t spiApiShowBufferContent(struct uartStruct *ptr_uartStruct, spiByteDataArray *buffer, int16_t nRequestedBytes, int8_t subCommandKeywordIndex)
{
	uint16_t ctr = 0;
	uint16_t maxMessageSize = 0;
	size_t byteIndex = 0;
	int nBytes = nRequestedBytes;

	clearString(message, BUFFER_SIZE);

	if ( 0 < nRequestedBytes ) /* positive */
	{
		byteIndex = 0;
		nBytes = min((int)(buffer->length), nBytes);
	}
	else if ( 0 > nRequestedBytes ) /* negative */
	{
		byteIndex = max((int)(buffer->length) - abs(nBytes),0);
		nBytes = min((int)(buffer->length), abs(nBytes));
	}
	else /* 0 */
	{
		byteIndex = 0;
		nBytes = buffer->length;
	}

	/* print summary header if all elements requested */
	if ( 0 == nRequestedBytes)
	{
		/* summary header */
		createExtendedSubCommandReceiveResponseHeader(ptr_uartStruct, ptr_uartStruct->commandKeywordIndex, subCommandKeywordIndex, spiApiCommandKeywords);
		snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%selements: %#x (%#i)"), uart_message_string, buffer->length, buffer->length );
		UART0_Send_Message_String_p(NULL,0);
	}

	/* data */
	if (buffer->length)
	{
		maxMessageSize = BUFFER_SIZE - 1 - strlen_P(string_3dots) - strlen(uart_message_string);

		for (int16_t byteCtr = 0; byteIndex < buffer->length && byteCtr < nBytes; byteIndex++, byteCtr++)
		{
			if ( (byteCtr)%8 == 0)
			{
				clearString(message, BUFFER_SIZE);
				ctr++;
				if ( nBytes > 8 )
				{
					snprintf(message, maxMessageSize, "(#%i) ", ctr);
				}
			}

			snprintf(message, maxMessageSize, "%s%02X ", message, buffer->data[byteIndex]);

			if ( 0 == (byteCtr+1)%8 || byteIndex == buffer->length-1 || byteCtr+1 == nBytes )
			{
				createExtendedSubCommandReceiveResponseHeader(ptr_uartStruct, ptr_uartStruct->commandKeywordIndex, subCommandKeywordIndex, spiApiCommandKeywords);

				/* attach "..." at the end, if there are more to come*/
				if (byteCtr+1 < nBytes)
				{
					strncat_P(message, string_3dots, BUFFER_SIZE -1 );
				}

				strncat(uart_message_string, message, BUFFER_SIZE -1 );
				UART0_Send_Message_String_p(NULL,0);
			}
		}
	}
	else
	{
		if (nRequestedBytes)
		{
			/* empty buffer */
			createExtendedSubCommandReceiveResponseHeader(ptr_uartStruct, ptr_uartStruct->commandKeywordIndex, subCommandKeywordIndex, spiApiCommandKeywords);
			strncat_P(uart_message_string, PSTR("--"),BUFFER_SIZE -1  );
			return apiCommandResult_SUCCESS_WITH_OUTPUT;
		}
	}


	return apiCommandResult_SUCCESS_QUIET;
}

