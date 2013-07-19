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
#include "spiApi.h"

//static const char filename[] 			PROGMEM = __FILE__;
static const char string_command[] 		PROGMEM = "command";
static const char string_response[]		PROGMEM = "response";
static const char string_CANMessageID[]	PROGMEM = "CAN-Message-ID";
static const char string_IDRange[]	    PROGMEM = "ID-Range";
static const char string_register[]	    PROGMEM = "register";
static const char string_value[]	    PROGMEM = "value";

static const char helpCommandKeyword00[] PROGMEM = "implemented";
static const char helpCommandKeyword01[] PROGMEM = "all";
static const char helpCommandKeyword02[] PROGMEM = "todo";

const char* helpCommandKeywords[] PROGMEM = {
	helpCommandKeyword00,
	helpCommandKeyword01,
	helpCommandKeyword02
};

void (*helpShowCommandOrResponse_p)(char[], PGM_P,  PGM_P) = helpShowCommandOrResponse;

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
						/* command */
						helpShowCommandOrResponse_p(NULL, PSTR("CAN-Message-ID ID-Range [RTR <Number of data bytes> Data0 ... Data7] "), PSTR("       "));
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s case RTR 0"), message );
						UART0_Send_Message_String_p(NULL,0);
						/* command */
						helpShowCommandOrResponse_p(NULL, PSTR("CAN-Message-ID ID-Range 0 <Number of data bytes> Data0 ... Data7]"), PSTR("       "));
						/* response */
						helpShowCommandOrResponse_p(currentReceiveHeader, NULL, PSTR("(TODO)"));
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s    [DEBG >=%i]   : %s CAN-Message-ID \"%s\""), message, debugLevelVerboseDebug, currentReceiveHeader, CAN_READY );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s case RTR 1"), message );
						UART0_Send_Message_String_p(NULL,0);
						/* command */
						helpShowCommandOrResponse_p(NULL, PSTR("CAN-Message-ID ID-Range 1 <Number of requested data bytes>]"), PSTR("       "));
						/* response */
						helpShowCommandOrResponse_p(currentReceiveHeader, PSTR("CAN_Mob CAN-Message-ID CAN-Length [Data0 ... Data7] "), PSTR("(now) "));
						/* response */
						helpShowCommandOrResponse_p(currentReceiveHeader, PSTR("CAN-Message-ID CAN-Length [Data0 ... Data7] "), PSTR("(TODO)"));
						break;
					case commandKeyNumber_SUBS:
					case commandKeyNumber_CANS:
						/* command */
						helpShowCommandOrResponse_p(NULL, PSTR("CAN-Message-ID ID-Range"), NULL);
						/* response */
						helpShowCommandOrResponse_p(currentReceiveHeader, NULL, NULL);
						break;
					case commandKeyNumber_USUB:
					case commandKeyNumber_CANU:
						/* command */
						helpShowCommandOrResponse_p(NULL, PSTR("CAN-Message-ID ID-Range"), NULL);
						/* response */
						helpShowCommandOrResponse_p(currentReceiveHeader, NULL, NULL);
						break;
					case commandKeyNumber_STAT:
						/* command */
						helpShowCommandOrResponse_p(NULL, PSTR("[ID]"), NULL);
						/* response */
						helpShowCommandOrResponse_p(currentReceiveHeader, PSTR("ID1 description1 status1"), NULL);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s           ..."), message );
						UART0_Send_Message_String_p(NULL,0);
						/* response */
						helpShowCommandOrResponse_p(currentReceiveHeader, PSTR("IDN descriptionN statusN"), NULL);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s originally ment to read the pressure in the bus"), message );
						UART0_Send_Message_String_p(NULL,0);
						break;
					case commandKeyNumber_OWTP:
						/* command */
						helpShowCommandOrResponse_p(NULL, PSTR("[ID [flag_conv] | <command_keyword> [arguments] ] "), PSTR("          "));
						/* response */
						helpShowCommandOrResponse_p(currentReceiveHeader, PSTR("ID1 value1"), PSTR("[ID]     "));
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s ..."), message );
						UART0_Send_Message_String_p(NULL,0);
						/* response */
						helpShowCommandOrResponse_p(currentReceiveHeader, PSTR("IDx valueX "), PSTR("[ID]     "));
						/* response */
						helpShowCommandOrResponse_p(currentReceiveHeader, PSTR("command_key [<corresponding answer/acknowledge>]"), PSTR("<keyword>"));
						/* available sub commands*/
						helpShowAvailableSubCommands(owiTemperatureCommandKeyNumber_MAXIMUM_NUMBER, owiTemperatureCommandKeywords);
						break;
					case commandKeyNumber_RGWR:
						/* command */
						helpShowCommandOrResponse_p(NULL, PSTR("Register [Value] "), PSTR("      "));
						/* response */
						helpShowCommandOrResponse_p(currentReceiveHeader, PSTR("the value %x has been written in Register "), PSTR("(now) "));
						/* response */
						helpShowCommandOrResponse_p(currentReceiveHeader, PSTR("Register Value (OldValue) "), PSTR("(TODO)"));
						//         writeRegister(ptr_uartStruct); /* call function with name writeRegister  */
						break;
					case commandKeyNumber_RGRE:
						/* command */
						helpShowCommandOrResponse_p(NULL, PSTR("Register"), PSTR("      "));
						/* response */
						helpShowCommandOrResponse_p(currentReceiveHeader, PSTR("the value %x has been written in Register "), PSTR("(now )"));
						/* response */
						helpShowCommandOrResponse_p(currentReceiveHeader, PSTR("Register Value "), PSTR("(TODO)"));
						break;
					case commandKeyNumber_RADC: /* read AVR's ADCs */
						/* command */
						helpShowCommandOrResponse_p(NULL, PSTR("[<ADC Channel>] "), PSTR("      "));
						break;
					case commandKeyNumber_OWAD: /* one-wire adc */
						/* command */
						helpShowCommandOrResponse_p(NULL, PSTR("[ID [flag_conv [flag_init]]]"), NULL);
						/* response */
						helpShowCommandOrResponse_p(currentReceiveHeader, PSTR("ID1 value1.1 ... value1.n"), NULL);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s           ..."), message );
						UART0_Send_Message_String_p(NULL,0);
						/* response */
						helpShowCommandOrResponse_p(currentReceiveHeader, PSTR("IDx valueX.1 ... valueX.n"), NULL);
						break;
					case commandKeyNumber_OWDS:
						/* command */
						helpShowCommandOrResponse_p(NULL, PSTR("[ID]"), PSTR("(read)   "));
						/* command */
						helpShowCommandOrResponse_p(NULL, PSTR("[ID] value"), PSTR("(write)  "));
						/* command */
						helpShowCommandOrResponse_p(NULL, PSTR("[ID] on/off/toggle"), PSTR("(keyword)"));
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
						/* command */
						helpShowCommandOrResponse_p(NULL, PSTR("[ID]"), NULL);
						break;
					case commandKeyNumber_OWLS:
						/* command */
						helpShowCommandOrResponse_p(NULL, PSTR("[<Family Code>]"), NULL);
						/* response */
						helpShowCommandOrResponse_p(currentReceiveHeader, PSTR("bus mask: 0x<bus mask> ID: <owi ID>"), NULL);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s           ..."), message );
						UART0_Send_Message_String_p(NULL,0);
						/* response */
						helpShowCommandOrResponse_p(currentReceiveHeader, PSTR("bus mask: 0x<bus mask> ID :<owi ID>"), NULL);
						break;
					case commandKeyNumber_PING:
						/* command */
						helpShowCommandOrResponse_p(NULL, PSTR(""), NULL);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: ALIV "), message );
						UART0_Send_Message_String_p(NULL,0);
						break;
					case commandKeyNumber_OWSP: /*one-wire set active pins/bus mask*/
						/* command */
						helpShowCommandOrResponse_p(NULL, PSTR("<bus mask>"), NULL);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: ... "), message );
						UART0_Send_Message_String_p(NULL,0);
						break;
					case commandKeyNumber_OWRP: /*one-wire read active pins/bus mask*/
						/* command */
						helpShowCommandOrResponse_p(NULL, NULL, NULL);
						/* response */
						helpShowCommandOrResponse_p(currentReceiveHeader, PSTR("value "), NULL);
						break;
					case commandKeyNumber_CANP:
						/* command */
						helpShowCommandOrResponse_p(NULL, PSTR("[keyword [value[s]]]"), NULL);
						/* response */
						helpShowCommandOrResponse_p(currentReceiveHeader, PSTR("keyword value[s] "), NULL);
						break;
					case commandKeyNumber_CAN:
						/* command */
						helpShowCommandOrResponse_p(NULL, PSTR("[keyword [value[s]]]"), NULL);
						/* response */
						helpShowCommandOrResponse_p(currentReceiveHeader, PSTR("keyword value[s] "), NULL);
						break;

					case commandKeyNumber_DEBG: /*set/get debug level*/
					{
						/* command */
						helpShowCommandOrResponse_p(NULL, PSTR("[level [mask]]"), NULL);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response (set): ..."), message );
						UART0_Send_Message_String_p(NULL,0);
						/* response */
						helpShowCommandOrResponse_p(currentReceiveHeader, PSTR("level mask"), PSTR("(get)"));
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s    "), message );
						UART0_Send_Message_String_p(NULL,0);

						/* available debug levels*/
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

						/* available debug masks*/
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
						/* command */
						helpShowCommandOrResponse_p(NULL, PSTR("[level]"), NULL);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response (set): ..."), message );
						UART0_Send_Message_String_p(NULL,0);
						/* response */
						helpShowCommandOrResponse_p(currentReceiveHeader, PSTR("level "), PSTR("(get)"));
						break;
					case commandKeyNumber_DBGM: /*set/get only debug system mask*/
						/* command */
						helpShowCommandOrResponse_p(NULL, PSTR("[mask]"), NULL);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response (set): ..."), message );
						UART0_Send_Message_String_p(NULL,0);
						/* response */
						helpShowCommandOrResponse_p(currentReceiveHeader, PSTR("mask"), PSTR("(get)"));
						break;
					case commandKeyNumber_PARA:
						/* command */
						helpShowCommandOrResponse_p(NULL, PSTR("[???]]"), NULL);
						/* response */
						helpShowCommandOrResponse_p(currentReceiveHeader, PSTR("pins 0xXX have/has parasitic devices"), NULL);
						/* response */
						helpShowCommandOrResponse_p(currentReceiveHeader, PSTR("pins 0xXX have been pulled HIGH within XX ms"), NULL);
						break;
					case commandKeyNumber_JTAG: /*toggle/set JTAG availability*/
						break;
					case commandKeyNumber_OWTR: /*trigger one-wire device(s) for action, if possible*/
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s one wire ???"), message );
						UART0_Send_Message_String_p(NULL,0);
						break;
					case commandKeyNumber_HELP: /*output some help*/
						/* command */
						helpShowCommandOrResponse_p(NULL, PSTR("[CMD]"), NULL);
						/* response */
						helpShowCommandOrResponse_p(currentReceiveHeader, PSTR("--- ... "), PSTR("(all)"));
						/* response */
						helpShowCommandOrResponse_p(currentReceiveHeader, PSTR("*** ... "), PSTR("(cmd)"));
						/* available sub commands*/
						helpShowAvailableSubCommands(helpCommandKeyNumber_MAXIMUM_NUMBER, helpCommandKeywords);
						break;
					case commandKeyNumber_SHOW: /*output some help*/
						/* command */
						helpShowCommandOrResponse_p(NULL, PSTR("[command_key]"), NULL);
						/* response */
						helpShowCommandOrResponse_p(currentReceiveHeader, PSTR("command_key1 ... "), PSTR("(all)"));
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s               ...               ... "), message );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s               %s command_keyN ... "), message, currentReceiveHeader );
						UART0_Send_Message_String_p(NULL,0);
						/* response */
						helpShowCommandOrResponse_p(currentReceiveHeader, PSTR("command_key"), PSTR("(cmd)"));
						/* available sub commands*/
						helpShowAvailableSubCommands(commandShowKeyNumber_MAXIMUM_NUMBER, showCommandKeywords);
						break;
					case commandKeyNumber_OWMR: /*one wire basics: match rom*/
						/* command */
						helpShowCommandOrResponse_p(NULL, PSTR("ID <pin_mask>"), NULL);
						/* response */
						helpShowCommandOrResponse_p(currentReceiveHeader, PSTR("<acknowledge> "), NULL);
						break;
					case commandKeyNumber_OWPC: /*one wire basics: presence check*/
						/* command */
						helpShowCommandOrResponse_p(NULL, PSTR("[<pin_mask>]"), NULL);
						/* response */
						helpShowCommandOrResponse_p(currentReceiveHeader, PSTR("<bit mask of used channels> "), NULL);
						break;
					case commandKeyNumber_OWRb: /*one wire basics: receive bit, wait for it*/
						/* command */
						helpShowCommandOrResponse_p(NULL, PSTR("<pin_mask> <delay> <timeout: N (times delay)> "), NULL);
						/* response */
						helpShowCommandOrResponse_p(currentReceiveHeader, PSTR("<count of delays, timeout: c<=0> "), NULL);
						break;
					case commandKeyNumber_OWRB: /*one wire basics: receive byte*/
						/* command */
						helpShowCommandOrResponse_p(NULL, PSTR("[<pin_mask>]"), NULL);
						/* response */
						helpShowCommandOrResponse_p(currentReceiveHeader, PSTR("<value> "), NULL);
						break;
					case commandKeyNumber_OWSC: /*one wire basics: send command*/
						/* command */
						helpShowCommandOrResponse_p(NULL, PSTR("<command_key_word> [<pin_mask> [arguments ...]] "), NULL);
						/* response */
						helpShowCommandOrResponse_p(currentReceiveHeader, PSTR("command_key [<corresponding answer/acknowledge>]"), NULL);
						//helpShowAvailableSubCommands(owiSendCommandKeyNumber_MAXIMUM_NUMBER, owiSendCommandKeywords)
						break;
					case commandKeyNumber_OWSB: /*one wire basics: send byte*/
						/* command */
						helpShowCommandOrResponse_p(NULL, PSTR("<byte> [<pin_mask>]"), NULL);
						/* response */
						helpShowCommandOrResponse_p(currentReceiveHeader, PSTR("<byte> <acknowledge> "), NULL);
						break;
					case commandKeyNumber_OWSA: /*one wire API settings: set/get 1-wire specific API settings*/
						/* command */
						helpShowCommandOrResponse_p(NULL, PSTR("<command_key_word> [arguments] "), NULL);
						/* response */
						helpShowCommandOrResponse_p(currentReceiveHeader, PSTR("command_key [<corresponding answer/acknowledge>]"), NULL);
						/* available sub commands*/
						helpShowAvailableSubCommands(owiApiCommandKeyNumber_MAXIMUM_NUMBER, owiApiCommandKeywords);
						break;
					case commandKeyNumber_WDOG: /*set/get watch dog status*/
						/* command */
						helpShowCommandOrResponse_p(NULL, PSTR("[???]"), NULL);
						/* response */
						helpShowCommandOrResponse_p(currentReceiveHeader, PSTR("[???]"), NULL);
						break;
					case commandKeyNumber_RLTH: /* relay threshold */
						/* command */
						helpShowCommandOrResponse_p(NULL, PSTR("[command_key_word]<"), NULL);
						/* response */
						helpShowCommandOrResponse_p(currentReceiveHeader, PSTR("[command_key_word] <value> "), NULL);
						helpShowCommandOrResponse_p(currentReceiveHeader, PSTR("command_key_word1 <value1> "), NULL);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s              [...]              ... "), message );
						UART0_Send_Message_String_p(NULL,0);
						helpShowCommandOrResponse_p(currentReceiveHeader, PSTR("command_key_word <valueN> "), NULL);
						/* available sub commands*/
						helpShowAvailableSubCommands(relayThresholdCommandKeyNumber_MAXIMUM_NUMBER, relayThresholdCommandKeywords);
						break;
					case commandKeyNumber_VERS: /* print code version */
						/* command */
						helpShowCommandOrResponse_p(NULL, NULL, NULL);
						/* response */
						helpShowCommandOrResponse_p(currentReceiveHeader, PSTR("<Version>"), NULL);
						break;
					case commandKeyNumber_I2C: /* I2C / TWI */
					case commandKeyNumber_TWIS: /* I2C / TWI */
						help_twis(currentReceiveHeader, currentCommandKeyword);
						break;
					case commandKeyNumber_CMD1: /* command (dummy name) */
					case commandKeyNumber_CMD2: /* command (dummy name) */
					case commandKeyNumber_CMD3: /* command (dummy name) */
					//case commandKeyNumber_CMD4: /* command (dummy name) */
					case commandKeyNumber_CMD5: /* command (dummy name) */
					case commandKeyNumber_CMD6: /* command (dummy name) */
					case commandKeyNumber_CMD7: /* command (dummy name) */
					case commandKeyNumber_CMD8: /* command (dummy name) */
						break;

					case commandKeyNumber_SPI:
						/* command */
						helpShowCommandOrResponse_p(NULL, PSTR("[???]"), NULL);
						/* response */
						helpShowCommandOrResponse_p(currentReceiveHeader, PSTR("[???] "), NULL);
						/* available sub commands*/
						helpShowAvailableSubCommands(spiApiCommandKeyNumber_MAXIMUM_NUMBER, spiApiCommandKeywords);
						break;

						case commandKeyNumber_GNWR:
						/* command */
						helpShowCommandOrResponse_p(NULL, PSTR("[???]"), NULL);
						/* response */
						helpShowCommandOrResponse_p(currentReceiveHeader, PSTR("[???] "), NULL);
						break;
					case commandKeyNumber_GNRE:
						/* command */
						helpShowCommandOrResponse_p(NULL, PSTR("[???]"), NULL);
						/* response */
						helpShowCommandOrResponse_p(currentReceiveHeader, PSTR("[???] "), NULL);
						break;
					case commandKeyNumber_OW8S:
						/* command */
						helpShowCommandOrResponse_p(NULL, PSTR("- reads all switches and shows the IDs"), NULL);
						/* command */
						helpShowCommandOrResponse_p(NULL, PSTR("<ID> - reads single switch"), NULL);
						/* command */
						helpShowCommandOrResponse_p(NULL, PSTR("<value> - writes <value> to all switches"), NULL);
						/* command */
						helpShowCommandOrResponse_p(NULL, PSTR("<ID> <value> - writes <value> to single switch"), NULL);
						/* response */
						helpShowCommandOrResponse_p(currentReceiveHeader, PSTR("<current state of switch>"), NULL);
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

void helpShowAvailableSubCommands(int maximumIndex, const char* commandKeywords[])
{
	snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s %s available commands"), message, currentCommandKeyword );
	UART0_Send_Message_String_p(NULL,0);

	for (int i=0; i < maximumIndex; i++)
	{
		snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("    %S"), (const char*) (pgm_read_word( &(commandKeywords[i]))), MAX_LENGTH_PARAMETER) ;
		UART0_Send_Message_String_p(NULL,0);
	}
}

/* combines the strings together with the globals to a command or response help message
 *
 * command  [<modifier>]: <current Command Keyword> <string:arguments>
 * response [<modifier>]: <currentReceiveHeader> <string:values>
 *
 * if currentReceiveHeader == NULL,
 * 		command is chosen, else response
 * if modifier, or string are NULL, they are ignored
 */
#warning TODO: extend to use vargs
void helpShowCommandOrResponse(char currentReceiveHeader[], PGM_P string, PGM_P modifier)
{
	strncat(uart_message_string, message, BUFFER_SIZE - 1);
	UART0_Send_Message_String_p(NULL, 0);

	if (NULL == currentReceiveHeader)
	{
		strncat_P(uart_message_string, PSTR(" command "), BUFFER_SIZE - 1);
	}
	else
	{
		strncat_P(uart_message_string, PSTR(" response"), BUFFER_SIZE - 1);
	}

	if (modifier)
	{
		strncat_P(uart_message_string, modifier, BUFFER_SIZE - 1);
	}

	strncat_P(uart_message_string, PSTR(": "), BUFFER_SIZE - 1);

	if (NULL == currentReceiveHeader)
	{
		strncat(uart_message_string, currentCommandKeyword, BUFFER_SIZE - 1);
	}
	else
	{
		strncat_P(uart_message_string, currentReceiveHeader, BUFFER_SIZE - 1);
	}
	if (NULL != string)
	{
		strncat(uart_message_string, string, BUFFER_SIZE - 1);
	}

	UART0_Send_Message_String_p(NULL, 0);
}
