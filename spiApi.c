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

#warning TODO: remove abbrev and replace by first match, requieres priority ordering, and option in keyword lookup

static const char spiApiCommandKeyword00[] PROGMEM = "add"; 				/* add <args> to write buffer*/
static const char spiApiCommandKeyword01[] PROGMEM = "a";
static const char spiApiCommandKeyword02[] PROGMEM = "control_bits";       /* get/set SPI control bits*/
static const char spiApiCommandKeyword03[] PROGMEM = "c";
static const char spiApiCommandKeyword04[] PROGMEM = "purge";				/* purge write/read buffer*/
static const char spiApiCommandKeyword05[] PROGMEM = "p";
static const char spiApiCommandKeyword06[] PROGMEM = "read";				/* read read Buffer*/
static const char spiApiCommandKeyword07[] PROGMEM = "r";
static const char spiApiCommandKeyword08[] PROGMEM = "status";				/* show spi status of control bits and operation settings*/
static const char spiApiCommandKeyword09[] PROGMEM = "s";
static const char spiApiCommandKeyword10[] PROGMEM = "transmit"; 			/* transmit write buffer auto set of CS controlled via cs_auto_enable*/
static const char spiApiCommandKeyword11[] PROGMEM = "t";
static const char spiApiCommandKeyword12[] PROGMEM = "write";				/* write <args>, directly setting automatically CS */
static const char spiApiCommandKeyword13[] PROGMEM = "w";
static const char spiApiCommandKeyword14[] PROGMEM = "purge_write_buffer";	/* purge write buffer*/
static const char spiApiCommandKeyword15[] PROGMEM = "pw";
static const char spiApiCommandKeyword16[] PROGMEM = "purge_read_buffer";  /* purge read buffer*/
static const char spiApiCommandKeyword17[] PROGMEM = "pr";
static const char spiApiCommandKeyword18[] PROGMEM = "show_write_buffer";  /* show content of write buffer, detailed when increasing DEBG level */
static const char spiApiCommandKeyword19[] PROGMEM = "sw";
static const char spiApiCommandKeyword20[] PROGMEM = "show_read_buffer";   /* show content of read buffer, detailed when increasing DEBG level */
static const char spiApiCommandKeyword21[] PROGMEM = "sr";
static const char spiApiCommandKeyword22[] PROGMEM = "cs";                 /* set chip select: cs <0|1|H(IGH)|L(OW)|T(RUE)|F(ALSE)>, CS is low active */
static const char spiApiCommandKeyword23[] PROGMEM = "cs_bar";             /* same as cs but inverse logic*/
static const char spiApiCommandKeyword24[] PROGMEM = "csb";
static const char spiApiCommandKeyword25[] PROGMEM = "cs_set";             /* set cs, optionally only for <cs mask>*/
static const char spiApiCommandKeyword26[] PROGMEM = "css";
static const char spiApiCommandKeyword27[] PROGMEM = "cs_release";         /* releases cs, optionally only for <cs mask>*/
static const char spiApiCommandKeyword28[] PROGMEM = "csr";
static const char spiApiCommandKeyword29[] PROGMEM = "cs_select_mask";     /* chip select output pin byte mask */
static const char spiApiCommandKeyword30[] PROGMEM = "cs_pins";            /* set hardware addresses of multiple CS outputs*/
static const char spiApiCommandKeyword31[] PROGMEM = "cs_auto_enable";     /* cs_auto_enable [<a(ll)|w(rite)|t(ransmit)> <0|1 ..>: enable/disable automatic setting for write and transmit actions*/
static const char spiApiCommandKeyword32[] PROGMEM = "spi_enable";			/* get/set enable SPI*/
static const char spiApiCommandKeyword33[] PROGMEM = "data_order";         /* get/set bit endianess*/
static const char spiApiCommandKeyword34[] PROGMEM = "master";             /* get/set master mode*/
static const char spiApiCommandKeyword35[] PROGMEM = "clock_polarity";     /* get/set clock polarity*/
static const char spiApiCommandKeyword36[] PROGMEM = "clock_phase";        /* get/set clock phase*/
static const char spiApiCommandKeyword37[] PROGMEM = "speed";              /* get/set speed */
static const char spiApiCommandKeyword38[] PROGMEM = "speed_divider";      /* get/set speed divider*/
static const char spiApiCommandKeyword39[] PROGMEM = "double_speed";       /* get/set double speed*/
static const char spiApiCommandKeyword40[] PROGMEM = "transmit_byte_order";/* MSB/LSB, big endian */
static const char spiApiCommandKeyword41[] PROGMEM = "complete_byte";      /* completing byte by zero at the end or the beginning, due to odd hex digit*/
static const char spiApiCommandKeyword42[] PROGMEM = "reset";              /* reset to default*/
static const char spiApiCommandKeyword43[] PROGMEM = "transmit_report";    /* report send bytes*/

const char* spiApiCommandKeywords[] PROGMEM = {
        spiApiCommandKeyword00, spiApiCommandKeyword01, spiApiCommandKeyword02, spiApiCommandKeyword03, spiApiCommandKeyword04,
		spiApiCommandKeyword05, spiApiCommandKeyword06, spiApiCommandKeyword07, spiApiCommandKeyword08, spiApiCommandKeyword09,
		spiApiCommandKeyword10, spiApiCommandKeyword11, spiApiCommandKeyword12, spiApiCommandKeyword13, spiApiCommandKeyword14,
		spiApiCommandKeyword15, spiApiCommandKeyword16, spiApiCommandKeyword17, spiApiCommandKeyword18, spiApiCommandKeyword19,
		spiApiCommandKeyword20, spiApiCommandKeyword21, spiApiCommandKeyword22, spiApiCommandKeyword23, spiApiCommandKeyword24,
		spiApiCommandKeyword25, spiApiCommandKeyword26, spiApiCommandKeyword27, spiApiCommandKeyword28, spiApiCommandKeyword29,
		spiApiCommandKeyword30, spiApiCommandKeyword31, spiApiCommandKeyword32, spiApiCommandKeyword33, spiApiCommandKeyword34,
		spiApiCommandKeyword35, spiApiCommandKeyword36, spiApiCommandKeyword37, spiApiCommandKeyword38, spiApiCommandKeyword39,
		spiApiCommandKeyword40, spiApiCommandKeyword41, spiApiCommandKeyword42, spiApiCommandKeyword43 };

/* initial configuration*/
spiApiConfig spiApiConfiguration = {
		.transmitByteOrder    = SPI_MSBYTE_FIRST,
		.byteCompletion       = SPI_MSBYTE_FIRST,
		.reportTransmit       = false,
		.csWriteAutoEnable    = true,
		.csExternalSelectMask = 0xFF,
		.hardwareInit         = false,
		.spiConfiguraton.data = 0x0
};

spiApiConfig *ptr_spiApiConfiguration = &spiApiConfiguration;

void spiApi(struct uartStruct *ptr_uartStruct)
{

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
				CommunicationError_p(ERRG, dynamicMessage_ErrorIndex, true, PSTR("parsing failed"));
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

void spiApiSubCommands(struct uartStruct *ptr_uartStruct, int16_t subCommandIndex, uint8_t parameterIndex)
{
	uint8_t result = 0;
	// find matching command keyword

	if ( 0 > subCommandIndex )
	{
		subCommandIndex = apiFindCommandKeywordIndex(setParameter[1], spiApiCommandKeywords, spiApiCommandKeyNumber_MAXIMUM_NUMBER);
	}

    /* generate header */
	switch (parameterIndex)
	{
		case 0:
			/*recursiveCall*/
			break;
		case 1:
			createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
			break;
		default:
			/* else */
			createExtendedSubCommandReceiveResponseHeader(ptr_uartStruct, ptr_uartStruct->commandKeywordIndex, subCommandIndex, spiApiCommandKeywords);
			break;
	}

	switch (subCommandIndex)
	{
		case spiApiCommandKeyNumber_WRITE:
		case spiApiCommandKeyNumber_W:
			/* write <args>, directly setting automatically CS */
			result = spiApiSubCommandWrite(ptr_uartStruct, parameterIndex);
			break;
		case spiApiCommandKeyNumber_STATUS:
		case spiApiCommandKeyNumber_S:
			/* show spi status of control bits and operation settings*/
			result = spiApiSubCommandShowStatus();
			break;
		case spiApiCommandKeyNumber_ADD:
		case spiApiCommandKeyNumber_A:
			/* add <args> to write buffer*/
			result = spiApiSubCommandAdd(ptr_uartStruct);
			break;
		case spiApiCommandKeyNumber_TRANSMIT:
		case spiApiCommandKeyNumber_T:
			/* transmit write buffer auto set of CS controlled via cs_auto_enable*/
			result = spiApiSubCommandTransmit();
			break;
		case spiApiCommandKeyNumber_READ:
		case spiApiCommandKeyNumber_R:
			/* read read Buffer*/
			result = spiApiSubCommandRead();
			break;
		case spiApiCommandKeyNumber_CS:
			/* set chip select: cs <0|1|H(IGH)|L(OW)|T(RUE)|F(ALSE)>, CS is low active */
			result = spiApiSubCommandCsStatus(ptr_uartStruct);
			break;
		case spiApiCommandKeyNumber_CS_BAR:
		case spiApiCommandKeyNumber_CSB:
			/* same as cs but inverse logic*/
			result = spiApiSubCommandCsBarStatus(ptr_uartStruct);
			break;
		case spiApiCommandKeyNumber_CS_SET:
		case spiApiCommandKeyNumber_CSS:
			/* releases cs, optionally only for <cs mask>*/
			result = spiApiSubCommandCsSet();
			break;
		case spiApiCommandKeyNumber_CS_RELEASE:
		case spiApiCommandKeyNumber_CSR:
			/* completing byte by zero at the end or the beginning, due to odd hex digit*/
			result = spiApiSubCommandCsRelease();
			break;
		case spiApiCommandKeyNumber_CS_SELECT_MASK:
			/* chip select output pin byte mask */
			result = spiApiSubCommandCsSelectMask(ptr_uartStruct);
			break;
		case spiApiCommandKeyNumber_CS_PINS:
			/* set hardware addresses of multiple CS outputs*/
			result = spiApiSubCommandCsPins(ptr_uartStruct);
			break;
		case spiApiCommandKeyNumber_CS_AUTO_ENABLE:
			/* cs_auto_enable [<a(ll)|w(rite)|t(ransmit)> <0|1 ..>: enable/disable automatic setting for write actions*/
			result = spiApiSubCommandCsAutoEnable(ptr_uartStruct);
			break;
		case spiApiCommandKeyNumber_PURGE:
		case spiApiCommandKeyNumber_P:
			/* purge write/read buffers*/
			result = spiApiSubCommandPurge();
			break;
		case spiApiCommandKeyNumber_PURGE_WRITE_BUFFER:
		case spiApiCommandKeyNumber_PW:
			/* purge write buffer*/
			result = spiApiSubCommandPurgeWriteBuffer();
			break;
		case spiApiCommandKeyNumber_PURGE_READ_BUFFER:
		case spiApiCommandKeyNumber_PR:
			/* purge read buffer*/
			result = spiApiSubCommandPurgeReadBuffer();
			break;
		case spiApiCommandKeyNumber_SHOW_WRITE_BUFFER:
		case spiApiCommandKeyNumber_SW:
			/* show content of write buffer, detailed when increasing DEBG level */
			result = spiApiSubCommandShowWriteBuffer(ptr_uartStruct);
			break;
		case spiApiCommandKeyNumber_SHOW_READ_BUFFER:
		case spiApiCommandKeyNumber_SR:
			/* show content of read buffer, detailed when increasing DEBG level */
			result = spiApiSubCommandShowReadBuffer(ptr_uartStruct);
			break;
		case spiApiCommandKeyNumber_CONTROL_BITS:
		case spiApiCommandKeyNumber_C:
			/* get/set SPI control bits*/
			result = spiApiSubCommandControlBits(ptr_uartStruct);
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
			break;
		case spiApiCommandKeyNumber_SPEED_DIVIDER:
			/* get/set speed divider*/
			result = spiApiSubCommandSpeedDivider(ptr_uartStruct);
			break;
		case spiApiCommandKeyNumber_DOUBLE_SPEED:
			/* get/set double speed*/
			result = spiApiSubCommandDoubleSpeed(ptr_uartStruct);
			break;
		case spiApiCommandKeyNumber_TRANSMIT_BYTE_ORDER:
			/* MSB/LSB, big endian */
			result = spiApiSubCommandTransmitByteOrder(ptr_uartStruct);
			break;
		case spiApiCommandKeyNumber_COMPLETE_BYTE:
			/* completing byte by zero at the end or the beginning, due to odd hex digit*/
			result = spiApiSubCommandCompleteByte(ptr_uartStruct);
			break;
		case spiApiCommandKeyNumber_RESET:
			result = spiApiSubCommandReset();
			break;
		case spiApiCommandKeyNumber_TRANSMIT_REPORT:
			result = spiApiSubCommandTransmitReport(ptr_uartStruct);
			break;
		default:
			result = -1;
			break;
	}


	switch (result)
	{
		case spiApiCommandResult_SUCCESS_WITH_OUTPUT:
			UART0_Send_Message_String_p(uart_message_string, BUFFER_SIZE - 1);
			break;
		case spiApiCommandResult_SUCCESS_WITH_OPTIONAL_OUTPUT:
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
		case spiApiCommandResult_FAILURE_NOT_A_SUB_COMMAND:
			CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, true, PSTR("not a sub command"));
			break;
		case spiApiCommandResult_SUCCESS_WITH_OUTPUT_ELSEWHERE:
		case spiApiCommandResult_FAILURE_QUIET:
			clearString(uart_message_string, BUFFER_SIZE);
			/* printouts elsewhere generated */
			break;
		case spiApiCommandResult_FAILURE:
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
	if ( 0 < spiApiFillWriteArray(ptr_uartStruct, parameterIndex))
	{
		/*spiWrite*/
		if ( true == spiApiConfiguration.csWriteAutoEnable )
		{
			spiWriteAndReadWithChipSelect( spiApiConfiguration.transmitByteOrder, spiApiConfiguration.csExternalSelectMask );
		}
		else
		{
			spiApiSubCommandTransmit();
		}
		/*reporting*/
		if ( true == spiApiConfiguration.reportTransmit )
		{
			spiApiSubCommandShowWriteBuffer(ptr_uartStruct);
			return spiApiCommandResult_SUCCESS_WITH_OUTPUT_ELSEWHERE;
		}
		return spiApiCommandResult_SUCCESS_WITH_OPTIONAL_OUTPUT;
	}

	return spiApiCommandResult_FAILURE;
}

uint8_t spiApiSubCommandAdd(struct uartStruct *ptr_uartStruct)
{
	if ( 1 == ptr_uartStruct->number_of_arguments)
	{
		/* SPI a - misinterpretation
		 * overlap of hex number 'a' and the 'a' add command */
		return spiApiSubCommandWrite(ptr_uartStruct, 1);
	}
	else if ( 0 < spiApiAddToWriteArray(ptr_uartStruct, 2))
	{
		return spiApiCommandResult_SUCCESS_WITH_OPTIONAL_OUTPUT;
	}
	else
	{
		CommunicationError_p( ERRA, SERIAL_ERROR_too_few_arguments, true, NULL);
		return spiApiCommandResult_FAILURE_QUIET;
	}
}

/*
 * *** sub commands with no argument
 */

uint8_t spiApiSubCommandShowStatus(void)
{
	static uint8_t status[] = {
			spiApiCommandKeyNumber_CS,
			spiApiCommandKeyNumber_CS_PINS,
			spiApiCommandKeyNumber_CS_SELECT_MASK,

			spiApiCommandKeyNumber_CONTROL_BITS,
			spiApiCommandKeyNumber_SPI_ENABLE,
			spiApiCommandKeyNumber_DATA_ORDER,
			spiApiCommandKeyNumber_MASTER,
			spiApiCommandKeyNumber_CLOCK_POLARITY,
			spiApiCommandKeyNumber_CLOCK_PHASE,
			spiApiCommandKeyNumber_SPEED,
			spiApiCommandKeyNumber_SPEED_DIVIDER,
			spiApiCommandKeyNumber_DOUBLE_SPEED,

			spiApiCommandKeyNumber_CS_AUTO_ENABLE,
			spiApiCommandKeyNumber_TRANSMIT_BYTE_ORDER,
			spiApiCommandKeyNumber_COMPLETE_BYTE,
			spiApiCommandKeyNumber_TRANSMIT_REPORT,

			spiApiCommandKeyNumber_SHOW_WRITE_BUFFER,
			spiApiCommandKeyNumber_SHOW_READ_BUFFER
	};

	clearString(uart_message_string, BUFFER_SIZE);

	for (size_t commandIndex = 0; commandIndex < sizeof(status); ++commandIndex)
	{
		ptr_uartStruct->number_of_arguments = 1;
		spiApiSubCommands(ptr_uartStruct, status[commandIndex], 2);
	}

	return spiApiCommandResult_SUCCESS_WITH_OUTPUT_ELSEWHERE;
}

uint8_t spiApiSubCommandTransmit(void)
{
	spiWriteAndReadWithoutChipSelect( spiApiConfiguration.transmitByteOrder );
	if ( true == spiApiConfiguration.reportTransmit )
	{
		spiApiSubCommandShowWriteBuffer(ptr_uartStruct);
		return spiApiCommandResult_SUCCESS_WITH_OUTPUT_ELSEWHERE;
	}
	return spiApiCommandResult_SUCCESS_WITH_OPTIONAL_OUTPUT;
}

uint8_t spiApiSubCommandReset(void)
{
	spiInit();
	return spiApiCommandResult_SUCCESS_WITH_OPTIONAL_OUTPUT;
}

uint8_t spiApiSubCommandPurge(void)
{
	spiApiSubCommandPurgeWriteBuffer();
	spiApiSubCommandPurgeReadBuffer();

	return spiApiCommandResult_SUCCESS_WITH_OPTIONAL_OUTPUT;
}

uint8_t spiApiSubCommandPurgeWriteBuffer(void)
{
	spiPurgeWriteData();
	return spiApiCommandResult_SUCCESS_WITH_OPTIONAL_OUTPUT;
}

uint8_t spiApiSubCommandPurgeReadBuffer(void)
{
	spiPurgeReadData();
	return spiApiCommandResult_SUCCESS_WITH_OPTIONAL_OUTPUT;
}

uint8_t spiApiSubCommandRead(void)
{
	if (SPI_MSBYTE_FIRST == spiApiConfiguration.transmitByteOrder)
	{
		return spiApiShowBufferContent( ptr_uartStruct, &spiReadData, -1, spiApiCommandKeyNumber_READ);
	}
	else
	{
		return spiApiShowBufferContent( ptr_uartStruct, &spiReadData,  1, spiApiCommandKeyNumber_READ);
	}
}

void spiApiGetChipSelectStatus(uint8_t mask, bool invert)
{
	bool status;
	uint8_t activeMask = getChipSelectArrayStatus();
	for (int chipSelectIndex = 0; chipSelectIndex < CHIP_MAXIMUM; ++chipSelectIndex)
	{
		if (mask & (0x1 << chipSelectIndex))
		{
			if (activeMask & (0x1 << chipSelectIndex))
			{
				//status = spiGetCsStatus(chipSelectIndex) > 0;
				status = (chipSelectIndex+1)%2 > 0; /*dummy*/
				if (invert)
				{
					status ^= 0x1;
				}
				snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s%i:%i "), uart_message_string,
						chipSelectIndex + 1,
						status);
#warning TODO: getCsStatus
			}
			else
			{
				snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s%i:- "), uart_message_string, chipSelectIndex + 1);
			}
		}
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
	uint64_t value = 0;
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;

	switch (nArgumentArgs)
	{
		case 0: /* read */
		{
			spiApiGetChipSelectStatus(0xFF, invert);
			return spiApiCommandResult_SUCCESS_WITH_OUTPUT;
		}
		break;
		case 1: /*mask*/
		default:
			if ( 0 != getUnsignedNumericValueFromParameterIndex(2, &value))
			{
				return spiApiCommandResult_FAILURE_QUIET;
			}
			else if (value > 0xFF )
			{
				CommunicationError_p(ERRA, SERIAL_ERROR_arguments_exceed_boundaries, true, NULL);
				return spiApiCommandResult_FAILURE_QUIET;
			}
			else
			{
				spiApiGetChipSelectStatus(0xFF & value, invert);
				return spiApiCommandResult_SUCCESS_WITH_OUTPUT;
			}
		break;
	}

	return spiApiCommandResult_SUCCESS_WITH_OPTIONAL_OUTPUT;
}

uint8_t spiApiSubCommandCsSet(void) /*optional external mask*/
{
	return spiApiCsSetCsRelease( true );
}

uint8_t spiApiSubCommandCsRelease(void) /*optional external mask*/
{
	return spiApiCsSetCsRelease( false );
}

uint8_t spiApiCsSetCsRelease( bool set )
{
	uint64_t inputValue = 0;
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;
	uint8_t outputValue = 0;

	switch (nArgumentArgs)
	{
		case 0: /* set all active within configuration's mask */
			outputValue = 0xFF & spiApiConfiguration.csExternalSelectMask;
		break;
		case 1: /*set all active within mask*/
		default:
			if ( 0 != getUnsignedNumericValueFromParameterIndex(2, &inputValue))
			{
				return spiApiCommandResult_FAILURE_QUIET;
			}
			else if (inputValue > 0xFF )
			{
				CommunicationError_p(ERRA, SERIAL_ERROR_arguments_exceed_boundaries, true, NULL);
				return spiApiCommandResult_FAILURE_QUIET;
			}
			else
			{
				outputValue = (0xFF & (0xFF & inputValue));
			}

			break;
	}

	if ( set )
	{
		spiSetChosenChipSelect(outputValue);
	}
	else
	{
		spiReleaseChosenChipSelect(outputValue);
	}

	return spiApiCommandResult_SUCCESS_WITH_OPTIONAL_OUTPUT;

}

uint8_t spiApiSubCommandCsSelectMask(struct uartStruct *ptr_uartStruct)
{
#if 0
	spiApiReadWrite1ByteValue(ptr_uartStruct, )
}

uint8_t spiApiReadWrite1ByteValue(struct uartStruct *ptr_uartStruct, )
{
#endif

	uint64_t inputValue = 0;
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;

	switch (nArgumentArgs)
	{
		case 0: /*read*/
			snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s%X"), uart_message_string, spiApiConfiguration.csExternalSelectMask);
			return spiApiCommandResult_SUCCESS_WITH_OUTPUT;
		break;
		case 1: /*write*/
		default:
			if ( 0 != getUnsignedNumericValueFromParameterIndex(2, &inputValue))
			{
				return spiApiCommandResult_FAILURE_QUIET;
			}
			else if (inputValue > 0xFF )
			{
				CommunicationError_p(ERRA, SERIAL_ERROR_arguments_exceed_boundaries, true, NULL);
				return spiApiCommandResult_FAILURE_QUIET;
			}
			else
			{
				/* set */
				spiApiConfiguration.csExternalSelectMask = 0xFF & inputValue;
				/* report by recursive call */
				ptr_uartStruct->number_of_arguments = 1;
				spiApiSubCommandCsSelectMask(ptr_uartStruct);
				ptr_uartStruct->number_of_arguments = nArgumentArgs + 1;

				return spiApiCommandResult_SUCCESS_WITH_OUTPUT;
			}

			break;
	}

	return spiApiCommandResult_SUCCESS_WITH_OUTPUT;
}


uint8_t spiApiSubCommandCsAutoEnable(struct uartStruct *ptr_uartStruct)
{
	// printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*)  ( filename ), (const char*) (empty));
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;

	return spiApiCommandResult_SUCCESS_WITH_OUTPUT;
}

uint8_t spiApiSubCommandTransmitReport(struct uartStruct *ptr_uartStruct)
{
	// printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*)  ( filename ), (const char*) (empty));
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;

	return spiApiCommandResult_SUCCESS_WITH_OUTPUT;
}

uint8_t spiApiSubCommandCsPins(struct uartStruct *ptr_uartStruct)
{
	// printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*)  ( filename ), (const char*) (empty));
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;

	return spiApiCommandResult_SUCCESS_WITH_OUTPUT;
}


uint8_t spiApiSubCommandShowWriteBuffer(struct uartStruct *ptr_uartStruct)
{
	// printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*)  ( filename ), (const char*) (empty));
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;
	return spiApiShowBufferContent( ptr_uartStruct, &spiWriteData, 0, spiApiCommandKeyNumber_SHOW_WRITE_BUFFER);

	return spiApiCommandResult_SUCCESS_WITH_OPTIONAL_OUTPUT;
}

uint8_t spiApiSubCommandShowReadBuffer(struct uartStruct *ptr_uartStruct)
{
	// printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*)  ( filename ), (const char*) (empty));
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;
	return spiApiShowBufferContent( ptr_uartStruct, &spiReadData, 0, spiApiCommandKeyNumber_SHOW_WRITE_BUFFER);

	return spiApiCommandResult_SUCCESS_WITH_OPTIONAL_OUTPUT;
}

uint8_t spiApiSubCommandControlBits(struct uartStruct *ptr_uartStruct)
{
	// printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*)  ( filename ), (const char*) (empty));
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;

	return spiApiCommandResult_SUCCESS_WITH_OUTPUT;
}

uint8_t spiApiSubCommandSpiEnable(struct uartStruct *ptr_uartStruct)
{
	// printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*)  ( filename ), (const char*) (empty));
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;

	return spiApiCommandResult_SUCCESS_WITH_OUTPUT;
}

uint8_t spiApiSubCommandDataOrder(struct uartStruct *ptr_uartStruct)
{
	// printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*)  ( filename ), (const char*) (empty));
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;

	return spiApiCommandResult_SUCCESS_WITH_OUTPUT;
}

uint8_t spiApiSubCommandMaster(struct uartStruct *ptr_uartStruct)
{
	// printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*)  ( filename ), (const char*) (empty));
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;

	return spiApiCommandResult_SUCCESS_WITH_OUTPUT;
}

uint8_t spiApiSubCommandClockPolarity(struct uartStruct *ptr_uartStruct)
{
	// printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*)  ( filename ), (const char*) (empty));
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;

	return spiApiCommandResult_SUCCESS_WITH_OUTPUT;
}

uint8_t spiApiSubCommandClockPhase(struct uartStruct *ptr_uartStruct)
{
	// printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*)  ( filename ), (const char*) (empty));
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;

	return spiApiCommandResult_SUCCESS_WITH_OUTPUT;
}

uint8_t spiApiSubCommandSpeed(struct uartStruct *ptr_uartStruct)
{
	// printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*)  ( filename ), (const char*) (empty));
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;

	return spiApiCommandResult_SUCCESS_WITH_OUTPUT;
}

uint8_t spiApiSubCommandSpeedDivider(struct uartStruct *ptr_uartStruct)
{
	// printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*)  ( filename ), (const char*) (empty));
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;

	return spiApiCommandResult_SUCCESS_WITH_OUTPUT;
}

uint8_t spiApiSubCommandDoubleSpeed(struct uartStruct *ptr_uartStruct)
{
	// printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*)  ( filename ), (const char*) (empty));
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;

	return spiApiCommandResult_SUCCESS_WITH_OUTPUT;
}

uint8_t spiApiSubCommandTransmitByteOrder(struct uartStruct *ptr_uartStruct)
{
	// printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*)  ( filename ), (const char*) (empty));
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;

	return spiApiCommandResult_SUCCESS_WITH_OUTPUT;
}

uint8_t spiApiSubCommandCompleteByte(struct uartStruct *ptr_uartStruct)
{
	// printDebug_p(debugLevelEventDebug, debugSystemSPI, __LINE__, (const char*)  ( filename ), (const char*) (empty));
	int16_t nArgumentArgs =  0;
	nArgumentArgs = ptr_uartStruct->number_of_arguments - 1;

	return spiApiCommandResult_SUCCESS_WITH_OUTPUT;
}


/* helpers */

size_t spiApiFillWriteArray(struct uartStruct *ptr_uartStruct, uint16_t parameterStartIndex)
{
	/* reset array length*/
	spiWriteData.length = 0;

	spiApiAddToWriteArray(ptr_uartStruct, parameterStartIndex);

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
    				PSTR("%S%S%S"), dots, dots, dots);
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
						spiAddWriteData( 0xFF & (value >> (8 * byteIndex)) );

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
						spiAddWriteData( strtoul(byte, NULL, 16) );
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

/*
 * uint8_t spiApiShowBufferContent(spiByteDataArray *buffer, int16_t nBytes, int8_t commandKeywordIndex, int8_t subCommandKeywordIndex, PGM_P commandKeywords[])
 *
 * shows the content of the data in buffer
 * and prints out the corresponding caller's command and sub command keyword
 *
 * Arguments:
 *
 *	buffer:
 * 			pointer to the buffer struct
 * 	nBytes:
 * 			number of bytes to show
 * 			0:		 	all available (max: buffer->length)
 * 			positive:	first nBytes of buffer, data index 0 ... nBytes
 * 			negative:	last  nBytes of buffer, data index buffer->length - 1 - nBytes ... buffer->length - 1
 *
 * 	commandKeywordIndex
 * 	subCommandKeywordIndex
 * 	subCommandKeywordArray
 */

uint8_t spiApiShowBufferContent(struct uartStruct *ptr_uartStruct, spiByteDataArray *buffer, int16_t nBytes, int8_t subCommandKeywordIndex)
{
	uint16_t ctr = 0;
	uint16_t maxMessageSize = 0;
	size_t byteIndex = 0;
	if ( 0 < nBytes ) /* positive */
	{
		byteIndex = 0;
	}
	else if ( 0 > nBytes ) /* negative */
	{
		byteIndex = max((int)(buffer->length) - abs(nBytes),0);
	}
	else /* 0 */
	{
		nBytes = buffer->length;
	}

	if (abs(nBytes) == buffer->length)
	{
		/* header */
		createExtendedSubCommandReceiveResponseHeader(ptr_uartStruct, ptr_uartStruct->commandKeywordIndex, subCommandKeywordIndex, spiApiCommandKeywords);
		snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%selements: %#x (%#i)"), uart_message_string, buffer->length, buffer->length );
		UART0_Send_Message_String(NULL,0);
	}

	/* data */
	clearString(message, BUFFER_SIZE);
	createExtendedSubCommandReceiveResponseHeader(ptr_uartStruct, ptr_uartStruct->commandKeywordIndex, subCommandKeywordIndex, spiApiCommandKeywords);
	maxMessageSize = BUFFER_SIZE - 1 - strlen_P(dots) - strlen(uart_message_string);

	for (int16_t byteIndexCtr = 0; byteIndex < buffer->length && byteIndexCtr < abs(nBytes); byteIndex++, byteIndexCtr++)
	{
		if ( (byteIndex)%8 == 0)
		{
			clearString(message, BUFFER_SIZE);
			ctr++;
			if ( abs(nBytes) > 8 )
			{
				snprintf(message, maxMessageSize, "(#%i) ", ctr);
			}
		}

		snprintf(message, maxMessageSize, "%s%02X ", message, buffer->data[byteIndex]);

		if ( 0 == (byteIndexCtr +1 )%8 || byteIndex == buffer->length -1 || byteIndexCtr == nBytes )
		{
			createExtendedSubCommandReceiveResponseHeader(ptr_uartStruct, ptr_uartStruct->commandKeywordIndex, subCommandKeywordIndex, spiApiCommandKeywords);

			/* attach "..." at the end, if there are more to come*/
			if (byteIndexCtr < nBytes - 1)
			{
				strncat_P(message, dots, BUFFER_SIZE -1 );
			}

			strncat(uart_message_string, message, BUFFER_SIZE -1 );
			UART0_Send_Message_String(NULL,0);
		}
	}

	return 0;
}

