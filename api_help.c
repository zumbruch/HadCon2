/*
 * api_help.c
 *
 *  Created on: Apr 23, 2010
 *  Modified  : Jun 10, 2011
 *      Author: p.zumbruch@gsi.de
 */

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
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
#include "one_wire.h"
#include "one_wire_adc.h"
#include "one_wire_dualSwitch.h"
#include "one_wire_octalSwitch.h"
#include "one_wire_simpleSwitch.h"
#include "one_wire_temperature.h"
#include "one_wire_api_settings.h"
#include "read_write_register.h"
#include "relay.h"
#include "help_twis.h"

static const char helpCommandKeyword00[] PROGMEM = "implemented";
static const char helpCommandKeyword01[] PROGMEM = "all";
static const char helpCommandKeyword02[] PROGMEM = "todo";

const char* helpCommandKeywords[] PROGMEM = {
	helpCommandKeyword00,
	helpCommandKeyword01,
	helpCommandKeyword02
};

void help(struct uartStruct *ptr_uartStruct)
{
    /* list all available commands with short descriptions
     * or
     * command specific
     *
     * command: HELP [CMND]
     * response: RECV HELP --- CMD1 "bla1"
     *               ...
     *           RECV HELP --- CMDx "blaX"
     * response: RECV HELP *** CMND "bla"
     *               ...
     * response: RECV HELP *** CMND "bla"
     */

#define LENGTH 50
	int32_t index = -1;
	int32_t indexCopy = -1;
	char header[LENGTH];
	char currentReceiveHeader[LENGTH];

	clearString(header, LENGTH);
	clearString(message, BUFFER_SIZE);
	createReceiveHeader(NULL, header, LENGTH);
	switch (ptr_uartStruct->number_of_arguments)
	{
		case 0:
		{
			snprintf_P(message, BUFFER_SIZE, PSTR("%s---"), header);
			helpAll(helpMode_IMPLEMENTED, NULL);
		}
		break;
		case 1: /* command help */
			index = apiFindCommandKeywordIndex(setParameter[1], commandKeywords, commandKeyNumber_MAXIMUM_NUMBER);
			if ( isKeywordIndex(index, commandKeyNumber_MAXIMUM_NUMBER) )
			{
				/* get current Command keyword */
				clearString(currentCommandKeyword,MAX_LENGTH_KEYWORD);
				strncat_P(currentCommandKeyword, (const char*) (pgm_read_word( &(commandKeywords[index]))), MAX_LENGTH_KEYWORD);

				/* extend already prepared header by current command keyword */
				/* compose message string */
				clearString(message, BUFFER_SIZE);
				snprintf_P(message, BUFFER_SIZE, PSTR("%s%s ***"), header, currentCommandKeyword);

				snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s %s "), message, currentCommandKeyword);
				strncat_P(uart_message_string, (const char*) (pgm_read_word( &(commandSyntaxes[index]))), BUFFER_SIZE - 1 );
				UART0_Send_Message_String_p(NULL,0);

				snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s "), message);
				strncat_P(uart_message_string, (const char*) (pgm_read_word( &(commandShortDescriptions[index]))), BUFFER_SIZE - 1);
				UART0_Send_Message_String_p(NULL,0);

				/*create keyword specific receiveHeader, e.g. "RECV ABCD"*/
				indexCopy = ptr_uartStruct->commandKeywordIndex;
				ptr_uartStruct->commandKeywordIndex = index;
				clearString(currentReceiveHeader, LENGTH);
				createReceiveHeader(NULL, currentReceiveHeader, LENGTH);
				ptr_uartStruct->commandKeywordIndex = indexCopy;

				switch (index)
				{
					case commandKeyNumber_CANT:
					case commandKeyNumber_SEND:
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s send CAN messages "), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s       command      : %s CAN-Message-ID ID-Range [RTR <Number of data bytes> Data0 ... Data7] "), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s    case RTR 0"), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s       command      : %s CAN-Message-ID ID-Range 0 <Number of data bytes> Data0 ... Data7]"), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s       response     : <nothing> "), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s        [DEBG >=%i]  : %s CAN-Message-ID \"%s\""), message, debugLevelVerboseDebug, currentReceiveHeader, READY );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s    case RTR 1"), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s       command      : %s CAN-Message-ID ID-Range 1 <Number of requested data bytes>]"), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s       response  now: %s CAN_Mob CAN-Message-ID CAN-Length [Data0 ... Data7] "), message, currentResponseKeyword );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s       response TODO: %s CAN-Message-ID CAN-Length [Data0 ... Data7] "), message, currentReceiveHeader );
						UART0_Send_Message_String_p(NULL,0);
						break;
					case commandKeyNumber_SUBS:
					case commandKeyNumber_CANS:
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s subscribe to CAN message ID "), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s CAN-Message-ID ID-Range"), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: "), message );
						UART0_Send_Message_String_p(NULL,0);
						break;
					case commandKeyNumber_USUB:
					case commandKeyNumber_CANU:
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s unsubscribe from CAN message ID "), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s CAN-Message-ID ID-Range"), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: "), message );
						UART0_Send_Message_String_p(NULL,0);
						break;
					case commandKeyNumber_STAT:
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s ??? "), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command  : %s [ID]"), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response : %s ID1 description1 status1"), message, currentReceiveHeader );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s            ..."), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s            %s IDX descriptionX statusX "), message, currentReceiveHeader );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s  originally ment to read the pressure in the bus"), message );
						UART0_Send_Message_String_p(NULL,0);
						break;
					case commandKeyNumber_OWTP:
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s one wire temperature "), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s [ID [flag_conv] | <command_keyword> [arguments] ] "), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response [ID]: %s ID1 value1"), message, currentReceiveHeader );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s                ..."), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s                %s IDx valueX "), message, currentReceiveHeader );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response <keyword>: %s command_key [<corresponding answer/acknowledge>]"), message, currentReceiveHeader );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s %s available command_keys"), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						for (int i=0; i < owiTemperatureCommandKeyNumber_MAXIMUM_NUMBER; i++)
						{
							snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s    "), message );
							strncat_P(uart_message_string, (const char*) (pgm_read_word( &(owiTemperatureCommandKeywords[i]))), MAX_LENGTH_PARAMETER) ;

							UART0_Send_Message_String_p(NULL,0);
						}
						break;
					case commandKeyNumber_RGWR:
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s register write "), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command      : %s Register [Value] "), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response now : %s the value %x has been written in Register "), message, currentResponseKeyword );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response TODO: %s Register Value (OldValue) "), message, currentReceiveHeader );
						UART0_Send_Message_String_p(NULL,0);
						//         writeRegister(ptr_uartStruct); /* call function with name writeRegister  */
						break;
					case commandKeyNumber_RGRE:
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s register write "), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command      : %s Register"), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response now : %s the value %x has been written in Register "), message, currentResponseKeyword );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response TODO: %s Register Value "), message, currentReceiveHeader );
						UART0_Send_Message_String_p(NULL,0);
						break;
					case commandKeyNumber_RADC: /* read AVR's ADCs */
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s read AVR's ADCs "), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command      : %s [<ADC Channel>] "), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						break;
					case commandKeyNumber_OWAD: /* one-wire adc */
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s one wire adc "), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s [ID [flag_conv [flag_init]]]"), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s ID1 value1.1 ... value1.n"), message, currentReceiveHeader );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s           ..."), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s           %s IDx valueX.1 ... valueX.n "), message, currentReceiveHeader );
						UART0_Send_Message_String_p(NULL,0);
						break;
					case commandKeyNumber_OWDS:
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s one wire dual switches"), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command read : %s [ID]"), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command write: %s [ID] value"), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command keyword: %s [ID] on/off/toggle"), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						break;
					case commandKeyNumber_RSET:
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s reset micro controller waiting %i s (via watchdog"), message, RESET_TIME_TO_WAIT_S );
						UART0_Send_Message_String_p(NULL,0);
						break;
					case commandKeyNumber_INIT:
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s init micro controller"), message);
						UART0_Send_Message_String_p(NULL,0);
						break;
					case commandKeyNumber_OWSS:
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s one wire single switches"), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s [ID]"), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						break;
					case commandKeyNumber_OWLS:
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s one wire list devices [of type <family code>]"), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s [<Family Code>]"), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s bus mask: 0x<bus mask> ID: <owi ID>"), message, currentReceiveHeader );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s           ..."), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s bus mask: 0x<bus mask> ID :<owi ID>"), message, currentReceiveHeader );
						UART0_Send_Message_String_p(NULL,0);
						break;
					case commandKeyNumber_PING:
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s receive an ALIV(e) periodically"), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s "), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: ALIV "), message );
						UART0_Send_Message_String_p(NULL,0);
						break;
					case commandKeyNumber_OWSP: /*one-wire set active pins/bus mask*/
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s one wire set/get pin/bus mask"), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s <bus mask>"), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: ... "), message );
						UART0_Send_Message_String_p(NULL,0);
						break;
					case commandKeyNumber_OWRP: /*one-wire read active pins/bus mask*/
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s one wire get pin/bus mask"), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : OWRP"), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s value "), message, currentReceiveHeader );
						UART0_Send_Message_String_p(NULL,0);
						break;
					case commandKeyNumber_CANP:
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s CAN preferences"), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s [keyword [value[s]]]"), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s %s keyword value[s] "), message, currentReceiveHeader, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						break;
					case commandKeyNumber_CAN:
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s CAN interface"), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s [keyword [value[s]]]"), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s %s keyword value[s] "), message, currentReceiveHeader, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						break;
					case commandKeyNumber_DEBG: /*set/get debug level*/
					{
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s set/get debug level and mask"), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s [level [mask]]"), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s set response: ..."), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s get response: %s level mask"), message, currentReceiveHeader );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s    "), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s available debug levels are:"), message );
						UART0_Send_Message_String_p(NULL,0);
						size_t maxLength = getMaximumStringArrayLength_P(debugLevelNames, debugLevel_MAXIMUM_INDEX, BUFFER_SIZE);
						/*maximum maxLength*/
						for (int i=0; i < debugLevel_MAXIMUM_INDEX; i++)
						{
							snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s    "), message );

							/* add spaces before*/
							for (size_t spaces = 0; spaces < (maxLength - strlen_P((const char*) (pgm_read_word( &(debugLevelNames[i]))))); spaces++)
							{
								strncat_P(uart_message_string, PSTR(" "), BUFFER_SIZE -1) ;
							}
							strncat_P(uart_message_string, (const char*) (pgm_read_word( &(debugLevelNames[i]))), BUFFER_SIZE -1) ;
							snprintf(uart_message_string, BUFFER_SIZE -1, "%s: 0x%X", uart_message_string, i);
							UART0_Send_Message_String_p(NULL,0);
						}
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s    "), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s available masks are:"), message );
						UART0_Send_Message_String_p(NULL,0);
						maxLength = getMaximumStringArrayLength_P(debugSystemNames, debugSystem_MAXIMUM_INDEX, BUFFER_SIZE);
						for (int i=0; i < debugSystem_MAXIMUM_INDEX; i++)
						{
							snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s    "), message );
							/* add spaces before*/
							for (size_t spaces = 0; spaces < (maxLength - strlen_P((const char*) (pgm_read_word( &(debugSystemNames[i]))))); spaces++)
							{
								strncat_P(uart_message_string, PSTR(" "), BUFFER_SIZE -1) ;
							}
							strncat_P(uart_message_string, (const char*) (pgm_read_word( &(debugSystemNames[i]))), BUFFER_SIZE -1) ;
							snprintf(uart_message_string, BUFFER_SIZE -1, "%s: 0x%08lX", uart_message_string, ((int32_t) 0x1) << i);
							UART0_Send_Message_String_p(NULL,0);
						}
					}
					break;
					case commandKeyNumber_DBGL: /*set/get debug level*/
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s set/get debug level"), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s [level]"), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s set response: ..."), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s get response: %s level "), message, currentReceiveHeader );
						UART0_Send_Message_String_p(NULL,0);
						break;
					case commandKeyNumber_DBGM: /*set/get only debug system mask*/
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s set/get debug mask"), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s [mask]"), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s set response: ..."), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s get response: %s mask"), message, currentReceiveHeader );
						UART0_Send_Message_String_p(NULL,0);
						break;
					case commandKeyNumber_PARA: /*set/get debug level*/
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s check for parasiticly connected devices"), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s [???]]"), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s %s pins 0xXX have/has parasitic devices"), message, currentReceiveHeader, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s %s pins 0xXX have been pulled HIGH within XX ms"), message, currentReceiveHeader, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						break;
					case commandKeyNumber_JTAG: /*toggle/set JTAG availability*/
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s switch of %s and enable 4 more ADC channels"), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						break;
					case commandKeyNumber_OWTR: /*trigger one-wire device(s) for action, if possible*/
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s one wire ???"), message );
						UART0_Send_Message_String_p(NULL,0);
						break;
					case commandKeyNumber_HELP: /*output some help*/
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s get all commands or specific HELP"), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s [CMD]"), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s all response: %s --- ... "), message, currentReceiveHeader );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s cmd response: %s *** ... "), message, currentReceiveHeader );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s %s available command_keys"), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						for (int i=0; i < helpCommandKeyNumber_MAXIMUM_NUMBER; i++)
						{
							snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s    "), message );
							strncat_P(uart_message_string, (const char*) (pgm_read_word( &(helpCommandKeywords[i]))), MAX_LENGTH_PARAMETER) ;

							UART0_Send_Message_String_p(NULL,0);
						}
						break;
					case commandKeyNumber_SHOW: /*output some help*/
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s show (internal) settings"), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s [command_key]"), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s all response: %s command_key1 ... "), message, currentReceiveHeader );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s               ...               ... "), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s               %s command_keyN ... "), message, currentReceiveHeader );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s cmd response: %s command_key"), message, currentReceiveHeader );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s %s available command_keys"), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						for (int i=0; i < commandShowKeyNumber_MAXIMUM_NUMBER; i++)
						{
							snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s    "), message );
							strncat_P(uart_message_string, (const char*) (pgm_read_word( &(commandShowKeywords[i]))), MAX_LENGTH_PARAMETER) ;

							UART0_Send_Message_String_p(NULL,0);
						}
						break;
					case commandKeyNumber_OWMR: /*one wire basics: match rom*/
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s one wire basics: match rom"), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s ID <pin_mask>"), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s <acknowledge> "), message, currentReceiveHeader );
						UART0_Send_Message_String_p(NULL,0);
						break;
					case commandKeyNumber_OWPC: /*one wire basics: presence check*/
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s one wire basics: presence check"), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s [<pin_mask>]"), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s <bit mask of used channels> "), message, currentReceiveHeader );
						UART0_Send_Message_String_p(NULL,0);
						break;
					case commandKeyNumber_OWRb: /*one wire basics: receive bit, wait for it*/
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s one wire basics: receive bit, wait for it"), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s <pin_mask> <delay> <timeout: N (times delay)> "), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s <count of delays, timeout: c<=0> "), message, currentReceiveHeader );
						UART0_Send_Message_String_p(NULL,0);
						break;
					case commandKeyNumber_OWRB: /*one wire basics: receive byte*/
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s one wire basics: receive byte"), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s [<pin_mask>]"), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s <value> "), message, currentReceiveHeader );
						UART0_Send_Message_String_p(NULL,0);
						break;
					case commandKeyNumber_OWSC: /*one wire basics: send command*/
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s one wire basics: send command"), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s <command_key_word> [<pin_mask> [arguments ...]] "), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s command_key [<corresponding answer/acknowledge>]"), message, currentReceiveHeader );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s %s available command_keys"), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						for (int i=0; i < 0 /*owiSendCommandKeyNumber_MAXIMUM_NUMBER*/; i++)
						{
							snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s    "), message );
							//strncat_P(uart_message_string, (const char*) (pgm_read_word( &(owiSendCommandKeywords[i]))), MAX_LENGTH_PARAMETER) ;

							UART0_Send_Message_String_p(NULL,0);
						}
						break;
					case commandKeyNumber_OWSB: /*one wire basics: send byte*/
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s one wire basics: send byte"), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s <byte> [<pin_mask>]"), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s <byte> <acknowledge> "), message, currentReceiveHeader );
						UART0_Send_Message_String_p(NULL,0);
						break;
					case commandKeyNumber_OWSA: /*one wire API settings: set/get 1-wire specific API settings*/
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s set/get 1-wire specific API settings"), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s <command_key_word> [arguments] "), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s command_key [<corresponding answer/acknowledge>]"), message, currentReceiveHeader );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s %s available command_keys"), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						for (int i=0; i < owiApiCommandKeyNumber_MAXIMUM_NUMBER; i++)
						{
							snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s    "), message );
							strncat_P(uart_message_string, (const char*) (pgm_read_word( &(owiApiCommandKeywords[i]))), MAX_LENGTH_PARAMETER) ;
							UART0_Send_Message_String_p(NULL,0);
						}
						break;
					case commandKeyNumber_WDOG: /*set/get watch dog status*/
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s set/get watch dog status"), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s [???]"), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s [???] "), message, currentReceiveHeader );
						break;
					case commandKeyNumber_RLTH: /* relay threshold */
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s manage threshold relay "), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s [command_key_word]<"), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s [command_key_word] <value> "), message, currentReceiveHeader );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s              [...]              ... "), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s [command_key_word] <value> "), message, currentReceiveHeader );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s %s available command_keys"), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						for (int i=0; i < relayThresholdCommandKeyNumber_MAXIMUM_NUMBER; i++)
						{
							snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s    "), message );
							strncat_P(uart_message_string, (const char*) (pgm_read_word( &(relayThresholdCommandKeywords[i]))), MAX_LENGTH_PARAMETER) ;

							UART0_Send_Message_String_p(NULL,0);
						}
						break;
					case commandKeyNumber_VERS: /* print code version */
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s print code version "), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s"), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s <Version> "), message, currentReceiveHeader );
						UART0_Send_Message_String_p(NULL,0);
						break;
					case commandKeyNumber_I2C: /* I2C / TWI */
					case commandKeyNumber_TWIS: /* I2C / TWI */
						help_twis(currentReceiveHeader, currentCommandKeyword);
						break;
					case commandKeyNumber_CMD1: /* command (dummy name) */
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s  command (dummy name) "), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s [???]"), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s [???] "), message, currentReceiveHeader );
						UART0_Send_Message_String_p(NULL,0);
						break;
					case commandKeyNumber_CMD2: /* command (dummy name) */
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s  command (dummy name) "), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s [???]"), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s [???] "), message, currentReceiveHeader );
						UART0_Send_Message_String_p(NULL,0);
						break;
					case commandKeyNumber_CMD3: /* command (dummy name) */
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s  command (dummy name) "), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s [???]"), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s [???] "), message, currentReceiveHeader );
						UART0_Send_Message_String_p(NULL,0);
						break;
					case commandKeyNumber_SPI: /* command (dummy name) */
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s  command (dummy name) "), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s [???]"), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s [???] "), message, currentReceiveHeader );
						UART0_Send_Message_String_p(NULL,0);
						break;
					case commandKeyNumber_GNWR: /* command (dummy name) */
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s  command (dummy name) "), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s [???]"), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s [???] "), message, currentReceiveHeader );
						UART0_Send_Message_String_p(NULL,0);
						break;
					case commandKeyNumber_GNRE: /* command (dummy name) */
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s  command (dummy name) "), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s [???]"), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s [???] "), message, currentReceiveHeader );
						UART0_Send_Message_String_p(NULL,0);
						break;
					case commandKeyNumber_OW8S: /* command (dummy name) */
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s one wire octal switches "), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s - reads all switches and shows the IDs"), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s <ID> - reads single switch"), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s <value> - writes <value> to all switches"), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s <ID> <value> - writes <value> to single switch"), message, currentCommandKeyword );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s <current state of switch>"), message, currentReceiveHeader );
						UART0_Send_Message_String_p(NULL,0);
						break;
					default:
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s %s --- unknown command"), message, header );
						UART0_Send_Message_String_p(NULL,0);
						break;
				}
			}
        else /*sub command*/
        {
			index = apiFindCommandKeywordIndex(setParameter[1], helpCommandKeywords, helpCommandKeyNumber_MAXIMUM_NUMBER);
			if ( isKeywordIndex(index, helpCommandKeyNumber_MAXIMUM_NUMBER) )
			{
				createExtendedSubCommandReceiveResponseHeader(NULL, -1, index, helpCommandKeywords );
				snprintf_P(message, BUFFER_SIZE -1 , PSTR("%s---"), uart_message_string);
				clearString(uart_message_string, BUFFER_SIZE);

				/* sub selection */
				switch (index)
				{
					case helpCommandKeyNumber_ALL:
						helpAll(helpMode_ALL, NULL);
					break;
					case helpCommandKeyNumber_IMPLEMENTED:
						helpAll(helpMode_IMPLEMENTED, NULL);
					break;
					case helpCommandKeyNumber_TODO:
						helpAll(helpMode_TODO, NULL);
					break;
					default:
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s %s --- unknown sub command"), message, header );
						UART0_Send_Message_String_p(NULL,0);
					break;
				}
			}
			else
			{
				snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s %s --- unknown command"), message, header );
				UART0_Send_Message_String_p(NULL,0);
			}
        }
        break;
        	default:
        		ptr_uartStruct->number_of_arguments--;
        		help(ptr_uartStruct);
        		break;
    }
}

void helpAll(uint8_t mode, char prefix[])
{

	char spaces[20];
    int32_t index = -1;
    uint8_t implemented = FALSE;
    if (NULL == prefix)
    {
    	prefix = message;
    }

    snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s available commands are:"), prefix);
	UART0_Send_Message_String_p(NULL,0);

	for (index = 0; index < commandKeyNumber_MAXIMUM_NUMBER; index++)
	{
		implemented = (uint8_t) (pgm_read_byte((uint8_t*) pgm_read_word( &commandImplementations[index])));

		switch(mode)
		{
		case helpMode_IMPLEMENTED:
			if (!implemented)
			{
				continue;
			}
			break;
		case helpMode_TODO:
			if (implemented)
			{
				continue;
			}
			break;
		case helpMode_ALL:
			break;
		default:
        	CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, TRUE, PSTR("unknown help mode"));
			break;
		}

		size_t strLength = strlen_P((const char*) (pgm_read_word( &(commandKeywords[index]))));
		if ( strLength < MAX_LENGTH_KEYWORD)
		{
			clearString(spaces, MAX_LENGTH_KEYWORD);
			/* prepare spaces */
			while ( MAX_LENGTH_KEYWORD - strLength > strlen(spaces))
			{
				strncat_P(spaces, PSTR(" "), MAX_LENGTH_KEYWORD -1) ;
			}
		}

		/* compose message */
		// e.g.
		//	RECV HELP --- SEND : send can message
		//	RECV HELP ---        	SEND CAN-ID ID-Range [RTR <nBytes> D0 .. D7]

		// short description
		snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s "), prefix);
		strncat_P(uart_message_string, (const char*) (pgm_read_word( &(commandKeywords[index]))), BUFFER_SIZE - 1);
		if (strLength < MAX_LENGTH_KEYWORD)
		{
			strncat(uart_message_string, spaces, BUFFER_SIZE - 1);
		}
		strncat_P(uart_message_string, PSTR(" : "), BUFFER_SIZE - 1);
		strncat_P(uart_message_string, (const char*) (pgm_read_word( &(commandShortDescriptions[index]))), BUFFER_SIZE - 1);
		if (!implemented)
		{
			strncat_P(uart_message_string, PSTR(" --- not implemented "), BUFFER_SIZE - 1);
		}
		UART0_Send_Message_String_p(NULL,0);

		// syntaxes
		void* syntaxes_p[] = { &(commandSyntaxes[index]), &(commandSyntaxAlternatives[index]), NULL };
		for (int var = 0; NULL != syntaxes_p[var]; ++var)
		{
			if ( 0 == strlen_P( (const char*) (pgm_read_word( syntaxes_p[var])) ) ) { break; }
			snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s "), prefix);

			for (size_t var = 0; var < MAX_LENGTH_KEYWORD + 3; ++var)
			{
				strncat_P(uart_message_string, PSTR(" "), BUFFER_SIZE -1 ) ;
			}

			strncat_P(uart_message_string, PSTR(" \t"), BUFFER_SIZE -1 ) ;
			strncat_P(uart_message_string, (const char*) (pgm_read_word( &(commandKeywords[index]))), BUFFER_SIZE - 1);
			strncat_P(uart_message_string, PSTR(" "), BUFFER_SIZE -1 ) ;
			strncat_P(uart_message_string, (const char*) (pgm_read_word( syntaxes_p[var])), BUFFER_SIZE - 1);
			UART0_Send_Message_String_p(NULL, 0);
		}
	}
}
