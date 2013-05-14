/*
 * api_debug.c
 *
 *  Created on: Jul 7, 2011
 *      Author: Peter Zumbruch, GSI, P.Zumbruch@gsi.de
 */

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <stddef.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/iocan128.h>
#include <avr/iocanxx.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h>

#include "api.h"
#include "api_define.h"
#include "api_global.h"

#include "api_debug.h"
#include "api_show.h"
#include "api_help.h"

/* max length defined by MAX_LENGTH_PARAMETER */
static const char commandDebugKeyword00[] PROGMEM = "all";
static const char commandDebugKeyword01[] PROGMEM = "level";
static const char commandDebugKeyword02[] PROGMEM = "mask";
static const char commandDebugKeyword03[] PROGMEM = "help";

const char* commandDebugKeywords[] PROGMEM = {
		commandDebugKeyword00,
		commandDebugKeyword01,
		commandDebugKeyword02,
		commandDebugKeyword03
};

static const char debugLevelName00[] PROGMEM = "noDebug";
static const char debugLevelName01[] PROGMEM = "verboseDebug";
static const char debugLevelName02[] PROGMEM = "eventDebug";
static const char debugLevelName03[] PROGMEM = "eventDebugVerbose";
static const char debugLevelName04[] PROGMEM = "periodicDebug";
static const char debugLevelName05[] PROGMEM = "periodicDebugVerbose";

const char *debugLevelNames[] PROGMEM = {
         debugLevelName00, debugLevelName01, debugLevelName02, debugLevelName03, debugLevelName04, debugLevelName05
};

static const char debugSystemName00[] PROGMEM = "Main";
static const char debugSystemName01[] PROGMEM = "Api";
static const char debugSystemName02[] PROGMEM = "ApiMisc";
static const char debugSystemName03[] PROGMEM = "UART";
static const char debugSystemName04[] PROGMEM = "CAN";
static const char debugSystemName05[] PROGMEM = "GETVOLTAGE";
static const char debugSystemName06[] PROGMEM = "RELAY";
static const char debugSystemName07[] PROGMEM = "CommandKey";
static const char debugSystemName08[] PROGMEM = "OWI";
static const char debugSystemName09[] PROGMEM = "OWITemperatures";
static const char debugSystemName10[] PROGMEM = "Decrypt";
static const char debugSystemName11[] PROGMEM = "DEBUG";
static const char debugSystemName12[] PROGMEM = "OWIADC";
static const char debugSystemName13[] PROGMEM = "OWIDualSwitches";
static const char debugSystemName14[] PROGMEM = "OWISingleSwitches";
static const char debugSystemName15[] PROGMEM = "OWIOctalSwitches";
static const char debugSystemName16[] PROGMEM = "SHOW";
static const char debugSystemName17[] PROGMEM = "OWIApiSettings";
static const char debugSystemName18[] PROGMEM = "TIMER0";
static const char debugSystemName19[] PROGMEM = "TIMER1";
static const char debugSystemName20[] PROGMEM = "TIMER0A";
static const char debugSystemName21[] PROGMEM = "TIMER0AScheduler";
static const char debugSystemName22[] PROGMEM = "TWI";

const char *debugSystemNames[] PROGMEM =
{
		debugSystemName00, debugSystemName01, debugSystemName02, debugSystemName03, debugSystemName04, debugSystemName05,
		debugSystemName06, debugSystemName07, debugSystemName08, debugSystemName09, debugSystemName10,
		debugSystemName11, debugSystemName12, debugSystemName13, debugSystemName14, debugSystemName15,
		debugSystemName16, debugSystemName17, debugSystemName18, debugSystemName19, debugSystemName20,
		debugSystemName21, debugSystemName22
};

int8_t apiDebug(struct uartStruct *ptr_uartStruct)
{
    uint8_t hasSubCommand = FALSE;
    int8_t status = 0;

	switch(ptr_uartStruct->number_of_arguments)
	{
	    case 0:
	    	hasSubCommand = FALSE;
	    	break;
	    default:

	    	if (0 == getNumericLength(&setParameter[1][0], MAX_LENGTH_PARAMETER))
	    	{
	    		hasSubCommand = TRUE;
	    	}
	    	break;
	}

	switch(ptr_uartStruct->number_of_arguments)
	{
	    case 0:
	    	switch(ptr_uartStruct->commandKeywordIndex)
	    	{
				case commandKeyNumber_DEBG:
					apiDebugReadModifyDebugLevelAndMask(ptr_uartStruct);
					break;
				case commandKeyNumber_DBGL:
					apiDebugReadModifyDebugLevel(ptr_uartStruct);
					break;
				case commandKeyNumber_DBGM:
					apiDebugReadModifyDebugMask(ptr_uartStruct);
					break;
				default:
					CommunicationError_p(ERRU, dynamicMessage_ErrorIndex, FALSE, PSTR("apiDebug:invalid command"));
					break;
	    	}
	    	break;
	    default:
	    	if (FALSE == hasSubCommand) /*write*/
	    	{
		    	switch(ptr_uartStruct->commandKeywordIndex)
		    	{
					case commandKeyNumber_DEBG:
						apiDebugReadModifyDebugLevelAndMask(ptr_uartStruct);
						break;
					case commandKeyNumber_DBGL:
						apiDebugReadModifyDebugLevel(ptr_uartStruct);
						break;
					case commandKeyNumber_DBGM:
						apiDebugReadModifyDebugMask(ptr_uartStruct);
						break;
					default:
						CommunicationError_p(ERRU, dynamicMessage_ErrorIndex, FALSE, PSTR("apiDebug:invalid command"));
						break;
		    	}
	    	}
	    	else /*sub command*/
	    	{
				status = apiDebugSubCommands(ptr_uartStruct, -1);
	    		if (status != 0) { return -1; }
	    	}
	    	break;
	}
	return 0;
}

int8_t apiDebugSubCommands(struct uartStruct *ptr_uartStruct, int16_t subCommandIndex)
{
	uint32_t status = 0;
	uint32_t value = 0;

	if ( 0 > subCommandIndex )
	{
		subCommandIndex = 0;
		// find matching command keyword
		while ( subCommandIndex < commandDebugKeyNumber_MAXIMUM_NUMBER )
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
	}

	/* TODO: relayThresholdMiscSubCommandsChooseFunction(ptr_uartStruct, index)*/
	switch ( ptr_uartStruct->number_of_arguments - 1 /* arguments of argument */)
	{

		case 0:
	{
		/* printout status*/

		/* generate message */
		createExtendedSubCommandReceiveResponseHeader(ptr_uartStruct, -1 , subCommandIndex, commandDebugKeywords);
		switch ( subCommandIndex )
		{
		case commandDebugKeyNumber_LEVEL:
			value = globalDebugLevel;
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s%x"), uart_message_string, value);
			UART0_Send_Message_String_p(NULL,0);
            break;
		case commandDebugKeyNumber_MASK:
			value = globalDebugSystemMask;
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s0x%lx"), uart_message_string, value);
			UART0_Send_Message_String_p(NULL,0);
			break;
		case commandDebugKeyNumber_HELP:
#warning TODO add help_debug
			/*help_debug();*/
			break;
		case commandDebugKeyNumber_ALL:
			/* show all */
			for (subCommandIndex = 0; subCommandIndex < commandDebugKeyNumber_MAXIMUM_NUMBER; subCommandIndex ++)
			{
				if (commandDebugKeyNumber_HELP == subCommandIndex) { continue; } /*exclude HELP*/

				ptr_uartStruct->number_of_arguments = 1;
				apiDebugSubCommands(ptr_uartStruct, subCommandIndex);
				ptr_uartStruct->number_of_arguments = 0;
			}
			break;
		default:
			CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, FALSE, PSTR("apiDebugSubCommands:invalid argument"));
			return 1;
			break;
		}
	break;
	}
	case 1:
	{
        /* set values*/
		status = getNumericValueFromParameter(2, &value);
		if ( 0 != status ) { return -1 ; }
        switch ( subCommandIndex )
        {
			case commandDebugKeyNumber_LEVEL:
				status = apiDebugSetDebugLevel(value);
				if ( 0 != status ) { return -1; }
				break;
			case commandDebugKeyNumber_MASK:
	            status = apiDebugSetDebugMask(value);
				if ( 0 != status ) { return -1; }
				break;
           default:
              CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, TRUE, PSTR("invalid command argument"));
              return -1;
              break;
        }
        /*recursive call to show change*/
        if ( 0 == status )
        {
           ptr_uartStruct->number_of_arguments = 1;
           apiDebugSubCommands(ptr_uartStruct, subCommandIndex);
           ptr_uartStruct->number_of_arguments = 2;
        }
	}
    break;
    case 2:
    {
		status = getNumericValueFromParameter(3, &value);
		if ( 0 != status ) { return -1 ; }
        switch ( subCommandIndex )
        {
			case commandDebugKeyNumber_ALL:
				status = apiDebugSetDebugLevel(value);
				if ( 0 != status ) { return -1; }
				break;
			case commandDebugKeyNumber_MASK:
	            status = apiDebugSetDebugMask(value);
				if ( 0 != status ) { return -1; }
				break;
           default:
              CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, TRUE, PSTR("invalid command argument"));
              return -1;
              break;
        }
        /*recursive call to show change*/
        if ( 0 == status )
        {
           ptr_uartStruct->number_of_arguments = 2;
           apiDebugSubCommands(ptr_uartStruct, subCommandIndex);
           ptr_uartStruct->number_of_arguments = 3;
        }

    }
    break;
    default:
    {
    	(ptr_uartStruct->number_of_arguments)--;
    	apiDebug(ptr_uartStruct);
    }
    break;
    }
	return 0;
}

void apiDebugReadModifyDebugLevelAndMask(struct uartStruct *ptr_uartStruct)
{
	/* command : DEBG [level [mask]]
	 * set response: ...
	 * get response: RECV DEBG level mask*/

	uint32_t value = 0;
    int8_t status = 0;
	switch (ptr_uartStruct->number_of_arguments)
	{
	case 0: /*read*/
		createExtendedSubCommandReceiveResponseHeader(ptr_uartStruct, -1 , -1, NULL);
		snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s%i %lx"),
				   uart_message_string, globalDebugLevel, globalDebugSystemMask);
		UART0_Send_Message_String_p(NULL,0);
		break;
	case 1: /*write debug*/
		apiDebugReadModifyDebugLevel(ptr_uartStruct);
		break;
	case 2: /*write debug and mask*/
        status = getNumericValueFromParameter(2, &value);
        if ( 0 != status ) { return; }
		apiDebugReadModifyDebugLevel(ptr_uartStruct);
        status = apiDebugSetDebugMask(value);
		if ( 0 != status ) { return; }
		break;
	default:
		break;
	}

    /*recursive call to show change*/
	switch (ptr_uartStruct->number_of_arguments)
	{
	case 0:
		break;
	default:
		status = ptr_uartStruct->number_of_arguments;
		ptr_uartStruct->number_of_arguments = 0;
		apiDebugReadModifyDebugLevelAndMask(ptr_uartStruct);
		ptr_uartStruct->number_of_arguments = status;
		break;
	}
}

void apiDebugReadModifyDebugLevel(struct uartStruct *ptr_uartStruct)
{
	/* command : DBGL [level [mask]]
	 * set response: ...
	 * get response: RECV DBGL level*/

	/*TODO: change CAN naming to more general */

	uint32_t value = 0;
    int8_t status = 0;

	switch (ptr_uartStruct->number_of_arguments)
	{
		case 0: /*read*/
			createExtendedSubCommandReceiveResponseHeader(ptr_uartStruct, -1 , -1, NULL);
            value = globalDebugLevel;
			snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s%i"),
					uart_message_string, value);
			UART0_Send_Message_String_p(NULL,0);
			break;
		case 1: /*write debug*/
			status = getNumericValueFromParameter(1, &value);
			if ( 0 != status ) { return; }
            status = apiDebugSetDebugLevel(value);
			if ( 0 != status ) { return; }
			break;
		default:
			(ptr_uartStruct->number_of_arguments)--;
			apiDebugReadModifyDebugLevel(ptr_uartStruct);
			break;
	}
    /*recursive call to show change*/
	switch (ptr_uartStruct->number_of_arguments)
	{
	case 0:
		break;
	default:
		status = ptr_uartStruct->number_of_arguments;
		ptr_uartStruct->number_of_arguments = 0;
		apiDebugReadModifyDebugLevel(ptr_uartStruct);
		ptr_uartStruct->number_of_arguments = status;
		break;
	}
}

void apiDebugReadModifyDebugMask(struct uartStruct *ptr_uartStruct)
{
	/* command : DBGM [level [mask]]
	 * set response: ...
	 * get response: RECV DBGM level*/

	int8_t status;
	uint32_t value;

	switch (ptr_uartStruct->number_of_arguments)
	{
		case 0: /*read*/
			createExtendedSubCommandReceiveResponseHeader(ptr_uartStruct, -1 , -1, NULL);
            value = globalDebugSystemMask;
			snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s0x%lx"),
					uart_message_string, value);
			UART0_Send_Message_String_p(NULL,0);
			break;
		case 1: /*write debug*/
			status = getNumericValueFromParameter(1, &value);
			if ( 0 != status ) { return; }
            status = apiDebugSetDebugMask(value);
			if ( 0 != status ) { return; }
			break;
		default:
			(ptr_uartStruct->number_of_arguments)--;
			apiDebugReadModifyDebugMask(ptr_uartStruct);
			break;
	}

	/*recursive call to show change*/
	switch (ptr_uartStruct->number_of_arguments)
	{
	case 0:
		break;
	default:
		status = ptr_uartStruct->number_of_arguments;
		ptr_uartStruct->number_of_arguments = 0;
		apiDebugReadModifyDebugMask(ptr_uartStruct);
		ptr_uartStruct->number_of_arguments = status;
		break;
	}

}

int8_t apiDebugSetDebugLevel(uint32_t value)
{
	if ( debugLevel_MAXIMUM_INDEX <= value )
	{
		CommunicationError_p(ERRA, SERIAL_ERROR_arguments_exceed_boundaries, TRUE, NULL);
		return -1;
	}
	globalDebugLevel = (uint8_t) (value & 0xFF);

	return 0;
}

int8_t apiDebugSetDebugMask(uint32_t value)
{
    if ( (0x1UL << debugSystem_MAXIMUM_INDEX) -1 < value )
	{
		CommunicationError_p(ERRA, SERIAL_ERROR_arguments_exceed_boundaries, TRUE, NULL);
		return -1 ;
	}
	globalDebugSystemMask = (uint32_t) (value & 0xFFFFFFFF);

	return 0;
}

