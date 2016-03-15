/*
   Licensed under the EUPL V.1.1, Lizenziert unter EUPL V.1.1 
*/
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

//static const char filename[] 					PROGMEM = __FILE__;
static const char string_command[] 				PROGMEM = "command";
static const char string_response[]				PROGMEM = "response";
static const char string_CANMessageID[]			PROGMEM = "CAN-Message-ID";
static const char string_IDRange[]	            PROGMEM = "ID-Range";
static const char string_RTR[]	                PROGMEM = "RTR";
static const char string_register[]	            PROGMEM = "register";
static const char string_value[]	            PROGMEM = "value";
static const char string_Data0_dots_Data7[]		PROGMEM = "Data0 ... Data7";
static const char string_Number_of_data_bytes[]	PROGMEM = "<Number of data bytes>";
static const char string_CAN_Mob[]          	PROGMEM = "CAN_Mob";
static const char string_BLengthB[]	          	PROGMEM = "<Length>";
static const char string_1x_[]	          	    PROGMEM = " ";
static const char string_10x_[]	          	    PROGMEM = "          ";
static const char string_6x_[]	          	    PROGMEM = "      ";
static const char string_3questions[]	       	PROGMEM = "[???]";

static const char string_s_case_S_i[]          	PROGMEM = "%s case %S %i";
static const char string_S_S[]		          	PROGMEM = "%S %S";

static const char helpCommandKeyword00[] PROGMEM = "implemented";
static const char helpCommandKeyword01[] PROGMEM = "all";
static const char helpCommandKeyword02[] PROGMEM = "todo";

const char* const helpCommandKeywords[] PROGMEM = {
	helpCommandKeyword00,
	helpCommandKeyword01,
	helpCommandKeyword02
};

void (*helpShowCommandOrResponse_p)(char[], PGM_P,  PGM_P, ...) = helpShowCommandOrResponse;

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
			strncat(message, header, BUFFER_SIZE - 1 );
			strncat_P(message, PSTR("---"), BUFFER_SIZE - 1);
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

				strncat(uart_message_string, message, BUFFER_SIZE - 1 );
				strncat_P(uart_message_string, string_1x_, BUFFER_SIZE - 1 );
				strncat_P(uart_message_string, (const char*) (pgm_read_word( &(commandShortDescriptions[index]))), BUFFER_SIZE - 1);
				UART0_Send_Message_String_p(NULL,0);

				/*create keyword specific receiveHeader, e.g. "RECV ABCD"*/
				indexCopy = ptr_uartStruct->commandKeywordIndex;
				ptr_uartStruct->commandKeywordIndex = index;
				clearString(currentReceiveHeader, LENGTH);
				createReceiveHeader(NULL, currentReceiveHeader, LENGTH);
				ptr_uartStruct->commandKeywordIndex = indexCopy;

				//printDebug_p(debugLevelEventDebug, debugSystemApi, __LINE__, filename, PSTR("receiveHeader: '%s'"), currentReceiveHeader);

				switch (index)
				{
					case commandKeyNumber_CANT:
					case commandKeyNumber_SEND:
						/* command */
						helpShowCommandOrResponse_p (NULL, string_10x_, PSTR("%S %S [%S %S %S]"), string_CANMessageID, string_IDRange, string_RTR, string_Number_of_data_bytes, string_Data0_dots_Data7);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, string_s_case_S_i, message, string_RTR, 0 );
						UART0_Send_Message_String_p(NULL,0);
						/* command */
						helpShowCommandOrResponse_p (NULL, string_10x_, PSTR("%S %S %i %S %S"), string_CANMessageID, string_IDRange, 0, string_Number_of_data_bytes, string_Data0_dots_Data7);
						/* response */
						helpShowCommandOrResponse_p (currentReceiveHeader, PSTR("[DEBG > 0]"), PSTR("%S \"%S\""), string_CANMessageID, PSTR(CAN_READY) );
						UART0_Send_Message_String_p(NULL,0);
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, string_s_case_S_i, message, string_RTR, 1 );
						UART0_Send_Message_String_p(NULL,0);
						/* command */
						helpShowCommandOrResponse_p (NULL, string_10x_, PSTR("%S %S %i %S"), string_CANMessageID, string_IDRange, 1, string_Number_of_data_bytes);
						/* response */
						helpShowCommandOrResponse_p (currentReceiveHeader, PSTR("(now)     "), PSTR("%S %S %S [%S]"), string_CAN_Mob, string_CANMessageID, string_BLengthB, string_Data0_dots_Data7);
						/* response */
						helpShowCommandOrResponse_p (currentReceiveHeader, PSTR("(TODO)    "), PSTR("%S %S [%S]"), string_CANMessageID, string_BLengthB, string_Data0_dots_Data7);
						break;
					case commandKeyNumber_SUBS:
					case commandKeyNumber_CANS:
						/* command */
						helpShowCommandOrResponse_p (NULL, NULL, string_S_S, string_CANMessageID, string_IDRange);
						/* response */
						//helpShowCommandOrResponse_p (currentReceiveHeader, NULL, NULL);
						break;
					case commandKeyNumber_USUB:
					case commandKeyNumber_CANU:
						/* command */
						helpShowCommandOrResponse_p (NULL, NULL, string_S_S, string_CANMessageID, string_IDRange);
						/* response */
						//helpShowCommandOrResponse_p (currentReceiveHeader, NULL, NULL);
						break;
						//					case commandKeyNumber_STAT:
						//						/* command */
						//						helpShowCommandOrResponse_p (NULL, NULL, PSTR("[ID]"));
						//						/* response */
						//						helpShowCommandOrResponse_p (currentReceiveHeader, NULL, PSTR("ID1 description1 status1"));
						//						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s           ..."), message );
						//						UART0_Send_Message_String_p(NULL,0);
						//						helpShowCommandOrResponse_p (currentReceiveHeader, NULL, PSTR("IDN descriptionN statusN"));
						//						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s originally meant to read the pressure in the bus"), message );
						//						UART0_Send_Message_String_p(NULL,0);
						//						break;
					case commandKeyNumber_OWTP:
						/* command */
						helpShowCommandOrResponse_p (NULL, string_10x_, PSTR("[ID [flag_conv] | <command_keyword> [arguments] ] "));
						/* response */
						helpShowCommandOrResponse_p (currentReceiveHeader, PSTR("[ID]     "), PSTR("ID1 value1"));
						strncat(uart_message_string, message, BUFFER_SIZE - 1 );
						strncat_P(uart_message_string, PSTR(" ..."), BUFFER_SIZE - 1 );
						UART0_Send_Message_String_p(NULL,0);
						/* response */
						helpShowCommandOrResponse_p (currentReceiveHeader, PSTR("[ID]     "), PSTR("IDx valueX "));
						/* response */
						helpShowCommandOrResponse_p (currentReceiveHeader, PSTR("<keyword>"), PSTR("command_key [<corresponding answer/acknowledge>]"));
						/* available sub commands*/
						helpShowAvailableSubCommands(owiTemperatureCommandKeyNumber_MAXIMUM_NUMBER, owiTemperatureCommandKeywords);
						break;
					case commandKeyNumber_RGWR:
						/* command */
						helpShowCommandOrResponse_p (NULL, string_6x_, PSTR("Register [<Value>]"));
						/* response */
						helpShowCommandOrResponse_p (currentReceiveHeader, PSTR("(now) "), PSTR("<Value> has been written in Register and ..."));
						/* response */
						helpShowCommandOrResponse_p (currentReceiveHeader, PSTR("(TODO)"), PSTR("Register Value (OldValue) "));
						break;
					case commandKeyNumber_RGRE:
						/* command */
						helpShowCommandOrResponse_p (NULL, string_6x_, PSTR("Register"));
						/* response */
						helpShowCommandOrResponse_p (currentReceiveHeader, PSTR("(TODO)"), PSTR("Register Value "));
						break;
					case commandKeyNumber_RADC: /* read AVR's ADCs */
						/* command */
						helpShowCommandOrResponse_p (NULL, string_6x_, PSTR("[<ADC Channel>] "));
						break;
					case commandKeyNumber_OWAD: /* one-wire adc */
						/* command */
						helpShowCommandOrResponse_p (NULL, NULL, PSTR("[ID [flag_conv [flag_init]]]"));
						/* response */
						helpShowCommandOrResponse_p (currentReceiveHeader, NULL, PSTR("ID1 value1.1 ... value1.n"));
						strncat(uart_message_string, message, BUFFER_SIZE - 1 );
						strncat_P(uart_message_string, PSTR("           ..."), BUFFER_SIZE - 1 );
						UART0_Send_Message_String_p(NULL,0);
						/* response */
						helpShowCommandOrResponse_p (currentReceiveHeader, NULL, PSTR("IDx valueX.1 ... valueX.n"));
						break;
					case commandKeyNumber_OWDS:
						/* command */
						helpShowCommandOrResponse_p (NULL, PSTR("(read)   "), PSTR("[ID]"));
						/* command */
						helpShowCommandOrResponse_p (NULL, PSTR("(write)  "), PSTR("[ID] value"));
						/* command */
						helpShowCommandOrResponse_p (NULL, PSTR("(keyword)"), PSTR("[ID] on/off/toggle"));
						break;
					case commandKeyNumber_RSET:
						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s reset micro controller waiting %i s (via watchdog"), message, RESET_TIME_TO_WAIT_S );
						UART0_Send_Message_String_p(NULL,0);
						break;
					case commandKeyNumber_INIT:
						strncat(uart_message_string, message, BUFFER_SIZE - 1 );
						strncat_P(uart_message_string, PSTR(" init micro controller"), BUFFER_SIZE - 1 );
						UART0_Send_Message_String_p(NULL,0);
						break;
					case commandKeyNumber_OWSS:
						/* command */
						helpShowCommandOrResponse_p (NULL, NULL, PSTR("[ID]"));
						break;
					case commandKeyNumber_OWLS:
						/* command */
						helpShowCommandOrResponse_p (NULL, NULL, PSTR("[<Family Code>]"));
						/* response */
						helpShowCommandOrResponse_p (currentReceiveHeader, NULL, PSTR("bus mask: 0x<bus mask> ID: <owi ID>"));
						strncat(uart_message_string, message, BUFFER_SIZE - 1 );
						strncat_P(uart_message_string, PSTR("           ..."), BUFFER_SIZE - 1 );
						UART0_Send_Message_String_p(NULL,0);
						helpShowCommandOrResponse_p (currentReceiveHeader, NULL, PSTR("bus mask: 0x<bus mask> ID :<owi ID>"));
						break;
					case commandKeyNumber_PING:
						/* command */
						helpShowCommandOrResponse_p (NULL, NULL, PSTR(""));
						/* response */
						helpShowCommandOrResponse_p ("ALIV", NULL, PSTR(""));
						break;
					case commandKeyNumber_OWSP: /*one-wire set active pins/bus mask*/
						/* command */
						helpShowCommandOrResponse_p (NULL, NULL, PSTR("<bus mask>"));
						/* response */
						helpShowCommandOrResponse_p (currentReceiveHeader, NULL, PSTR("value"));
						break;
					case commandKeyNumber_OWRP: /*one-wire read active pins/bus mask*/
						/* command */
						helpShowCommandOrResponse_p (NULL, NULL, NULL);
						/* response */
						helpShowCommandOrResponse_p (currentReceiveHeader, NULL, PSTR("value"));
						break;
						//					case commandKeyNumber_CANP:
						//						/* command */
						//						helpShowCommandOrResponse_p (NULL, NULL, PSTR("[keyword [value[s]]]"));
						//						/* response */
						//						helpShowCommandOrResponse_p (currentReceiveHeader, NULL, PSTR("keyword value[s] "));
						//						break;
						//					case commandKeyNumber_CAN:
						//						/* command */
						//						helpShowCommandOrResponse_p (NULL, NULL, PSTR("[keyword [value[s]]]"));
						//						/* response */
						//						helpShowCommandOrResponse_p (currentReceiveHeader, NULL, PSTR("keyword value[s] "));
						//						break;
					case commandKeyNumber_DEBG: /*set/get debug level*/
					{
						/* command */
						helpShowCommandOrResponse_p (NULL, NULL, PSTR("[level [mask]]"));
						/* response */
						helpShowCommandOrResponse_p (currentReceiveHeader, PSTR("(set)"), PSTR("level [mask]"));
						helpShowCommandOrResponse_p (currentReceiveHeader, PSTR("(get)"), PSTR("level mask"));

						/* available debug levels*/
						strncat(uart_message_string, message, BUFFER_SIZE -1) ;
						strncat_P(uart_message_string, PSTR(" available debug levels are:"), BUFFER_SIZE - 1 );
						UART0_Send_Message_String_p(NULL,0);

						size_t maxLength = getMaximumStringArrayLength_P(debugLevelNames, debugLevel_MAXIMUM_INDEX, BUFFER_SIZE);
						/*maximum maxLength*/
						for (int i=0; i < debugLevel_MAXIMUM_INDEX; i++)
						{
							strncat(uart_message_string, message, BUFFER_SIZE -1) ;
							strncat_P(uart_message_string, PSTR("    "), BUFFER_SIZE - 1 );

							/* add spaces before*/
							for (size_t spaces = 0; spaces < (maxLength - strlen_P((const char*) (pgm_read_word( &(debugLevelNames[i]))))); spaces++)
							{
								strncat_P(uart_message_string, string_1x_, BUFFER_SIZE -1) ;
							}
							strncat_P(uart_message_string, (const char*) (pgm_read_word( &(debugLevelNames[i]))), BUFFER_SIZE -1) ;
							snprintf(uart_message_string, BUFFER_SIZE -1, "%s: 0x%X", uart_message_string, i);
							UART0_Send_Message_String_p(NULL,0);
						}

						/* available debug masks*/
						strncat(uart_message_string, message, BUFFER_SIZE - 1 );
						strncat_P(uart_message_string, PSTR(" available masks are:"), BUFFER_SIZE - 1 );
						UART0_Send_Message_String_p(NULL,0);

						maxLength = getMaximumStringArrayLength_P(debugSystemNames, debugSystem_MAXIMUM_INDEX, BUFFER_SIZE);
						for (int i=0; i < debugSystem_MAXIMUM_INDEX; i++)
						{
							strncat(uart_message_string, message, BUFFER_SIZE -1) ;
							strncat_P(uart_message_string, PSTR("    "), BUFFER_SIZE - 1 );
							/* add spaces before*/
							for (size_t spaces = 0; spaces < (maxLength - strlen_P((const char*) (pgm_read_word( &(debugSystemNames[i]))))); spaces++)
							{
								strncat_P(uart_message_string, string_1x_, BUFFER_SIZE -1) ;
							}
							strncat_P(uart_message_string, (const char*) (pgm_read_word( &(debugSystemNames[i]))), BUFFER_SIZE -1) ;
							snprintf(uart_message_string, BUFFER_SIZE -1, "%s: 0x%08lX", uart_message_string, (UINT32_C(0x1)) << i);
							UART0_Send_Message_String_p(NULL,0);
						}
					}
					break;
					case commandKeyNumber_DBGL: /*set/get debug level*/
						/* command */
						helpShowCommandOrResponse_p (NULL, NULL, PSTR("[level]"));
						/* response */
						helpShowCommandOrResponse_p (currentReceiveHeader, NULL, PSTR("level "));
						break;
					case commandKeyNumber_DBGM: /*set/get only debug system mask*/
						/* command */
						helpShowCommandOrResponse_p (NULL, NULL, PSTR("[mask]"));
						/* response */
						helpShowCommandOrResponse_p(currentReceiveHeader, NULL, PSTR("mask"));
						break;
					case commandKeyNumber_PARA:
						/* command */
						helpShowCommandOrResponse_p (NULL, NULL, string_3questions);
						/* response */
						helpShowCommandOrResponse_p (currentReceiveHeader, NULL, PSTR("pins 0xXX have/has parasitic devices"));
						/* response */
						helpShowCommandOrResponse_p (currentReceiveHeader, NULL, PSTR("pins 0xXX have been pulled HIGH within XX ms"));
						break;
					case commandKeyNumber_JTAG: /*toggle/set JTAG availability*/
						break;
						//					case commandKeyNumber_OWTR: /*trigger one-wire device(s) for action, if possible*/
						//						snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s one wire ???"), message );
						//						UART0_Send_Message_String_p(NULL,0);
						//						break;
					case commandKeyNumber_HELP: /*output some help*/
						/* command */
						helpShowCommandOrResponse_p (NULL, NULL, PSTR("[CMD]"));
						/* response */
						helpShowCommandOrResponse_p (currentReceiveHeader, PSTR("(all)"), PSTR("--- ... "));
						/* response */
						helpShowCommandOrResponse_p (currentReceiveHeader, PSTR("(cmd)"), PSTR("*** ... "));
						/* available sub commands*/
						helpShowAvailableSubCommands(helpCommandKeyNumber_MAXIMUM_NUMBER, helpCommandKeywords);
						break;
					case commandKeyNumber_SHOW: /*output some help*/
						/* command */
						helpShowCommandOrResponse_p (NULL, NULL, PSTR("[command_key]"));
						/* response */
						helpShowCommandOrResponse_p (currentReceiveHeader, PSTR("(all)"), PSTR("command_key1 ... "));
						strncat(uart_message_string, message, BUFFER_SIZE -1) ;
						strncat_P(uart_message_string, PSTR("               ...               ... "), BUFFER_SIZE - 1 );
						UART0_Send_Message_String_p(NULL,0);
						helpShowCommandOrResponse_p (currentReceiveHeader, PSTR("(all)"), PSTR("command_keyN ... "));
						helpShowCommandOrResponse_p (currentReceiveHeader, PSTR("(cmd)"), PSTR("command_key"));
						/* available sub commands*/
						helpShowAvailableSubCommands(commandShowKeyNumber_MAXIMUM_NUMBER, showCommandKeywords);
						break;
						//					case commandKeyNumber_OWMR: /*one wire basics: match rom*/
						//						/* command */
						//						helpShowCommandOrResponse_p (NULL, NULL, PSTR("ID <pin_mask>"));
						//						/* response */
						//						helpShowCommandOrResponse_p (currentReceiveHeader, NULL, PSTR("<acknowledge> "));
						//						break;
						//					case commandKeyNumber_OWPC: /*one wire basics: presence check*/
						//						/* command */
						//						helpShowCommandOrResponse_p (NULL, NULL, PSTR("[<pin_mask>]"));
						//						/* response */
						//						helpShowCommandOrResponse_p (currentReceiveHeader, NULL, PSTR("<bit mask of used channels> "));
						//						break;
						//					case commandKeyNumber_OWRb: /*one wire basics: receive bit, wait for it*/
						//						/* command */
						//						helpShowCommandOrResponse_p (NULL, NULL, PSTR("<pin_mask> <delay> <timeout: N (times delay)> "));
						//						/* response */
						//						helpShowCommandOrResponse_p (currentReceiveHeader, NULL, PSTR("<count of delays, timeout: c<=0> "));
						//						break;
						//					case commandKeyNumber_OWRB: /*one wire basics: receive byte*/
						//						/* command */
						//						helpShowCommandOrResponse_p (NULL, NULL, PSTR("[<pin_mask>]"));
						//						/* response */
						//						helpShowCommandOrResponse_p (currentReceiveHeader, NULL, PSTR("<value> "));
						//						break;
						//					case commandKeyNumber_OWSC: /*one wire basics: send command*/
						//						/* command */
						//						helpShowCommandOrResponse_p (NULL, NULL, PSTR("<command_key_word> [<pin_mask> [arguments ...]] "));
						//						/* response */
						//						helpShowCommandOrResponse_p (currentReceiveHeader, NULL, PSTR("command_key [<corresponding answer/acknowledge>]"));
						//						//helpShowAvailableSubCommands(owiSendCommandKeyNumber_MAXIMUM_NUMBER, owiSendCommandKeywords)
						//						break;
						//					case commandKeyNumber_OWSB: /*one wire basics: send byte*/
						//						/* command */
						//						helpShowCommandOrResponse_p (NULL, NULL, PSTR("<byte> [<pin_mask>]"));
						//						/* response */
						//						helpShowCommandOrResponse_p (currentReceiveHeader, NULL, PSTR("<byte> <acknowledge> "));
						//						break;
					case commandKeyNumber_OWSA: /*one wire API settings: set/get 1-wire specific API settings*/
						/* command */
						helpShowCommandOrResponse_p (NULL, NULL, PSTR("<command_key_word> [arguments] "));
						/* response */
						helpShowCommandOrResponse_p (currentReceiveHeader, NULL, PSTR("command_key [<corresponding answer/acknowledge>]"));
						/* available sub commands*/
						helpShowAvailableSubCommands(owiApiCommandKeyNumber_MAXIMUM_NUMBER, owiApiCommandKeywords);
						break;
					case commandKeyNumber_WDOG: /*set/get watch dog status*/
						/* command */
						helpShowCommandOrResponse_p (NULL, NULL, string_3questions);
						/* response */
						helpShowCommandOrResponse_p (currentReceiveHeader, NULL, string_3questions);
						break;
					case commandKeyNumber_RLTH: /* relay threshold */
						/* command */
						helpShowCommandOrResponse_p (NULL, NULL, PSTR("[command_key_word]<"));
						/* response */
						helpShowCommandOrResponse_p (currentReceiveHeader, NULL, PSTR("[command_key_word] <value> "));
						helpShowCommandOrResponse_p (currentReceiveHeader, NULL, PSTR("command_key_word1 <value1> "));
						strncat(uart_message_string, message, BUFFER_SIZE -1) ;
						strncat_P(uart_message_string, PSTR("              [...]              ... "), BUFFER_SIZE - 1 );
						UART0_Send_Message_String_p(NULL,0);
						helpShowCommandOrResponse_p (currentReceiveHeader, NULL, PSTR("command_key_word <valueN> "));
						/* available sub commands*/
						helpShowAvailableSubCommands(relayThresholdCommandKeyNumber_MAXIMUM_NUMBER, relayThresholdCommandKeywords);
						break;
					case commandKeyNumber_VERS: /* print code version */
						/* command */
						helpShowCommandOrResponse_p (NULL, NULL, NULL);
						/* response */
						helpShowCommandOrResponse_p (currentReceiveHeader, NULL, PSTR("<Version>"));
						break;
					case commandKeyNumber_IDN: /* command (dummy name) */
						/* command */
						helpShowCommandOrResponse_p (NULL, NULL, NULL);
						/* response */
						helpShowCommandOrResponse_p (currentReceiveHeader, NULL, PSTR("<Identification>"));
						break;
					case commandKeyNumber_I2C: /* I2C / TWI */
					case commandKeyNumber_TWIS: /* I2C / TWI */
						help_twis(currentReceiveHeader, currentCommandKeyword);
						break;
					case commandKeyNumber_CMD1: /* command (dummy name) */
					case commandKeyNumber_CMD2: /* command (dummy name) */
					case commandKeyNumber_CMD3: /* command (dummy name) */
						//case commandKeyNumber_CMD4: /* command (dummy name) */
					case commandKeyNumber_DAC: /* command (dummy name) */
					case commandKeyNumber_CMD7: /* command (dummy name) */
					case commandKeyNumber_CMD8: /* command (dummy name) */
						/* command */
						helpShowCommandOrResponse_p (NULL, NULL, string_3questions);
						/* response */
						helpShowCommandOrResponse_p (currentReceiveHeader, NULL, string_3questions);
						break;
					case commandKeyNumber_SPI:
						/* command */
						helpShowCommandOrResponse_p (NULL, NULL, PSTR("[command]"));
						/* response */
						helpShowCommandOrResponse_p (currentReceiveHeader, NULL, PSTR("command specific"));
						/* available sub commands*/
						helpShowAvailableSubCommands(spiApiCommandKeyNumber_MAXIMUM_NUMBER, spiApiCommandKeywords);
						break;
#if 2 == HADCON_VERSION
					case commandKeyNumber_GNWR:
						/* command */
						helpShowCommandOrResponse_p (NULL, NULL, string_3questions);
						/* response */
						helpShowCommandOrResponse_p (currentReceiveHeader, NULL, string_3questions);
						break;
					case commandKeyNumber_GNRE:
						/* command */
						helpShowCommandOrResponse_p (NULL, NULL, string_3questions);
						/* response */
						helpShowCommandOrResponse_p (currentReceiveHeader, NULL, string_3questions);
						break;
#else
					case commandKeyNumber_GNRE:
					case commandKeyNumber_GNWR:
						strncat(uart_message_string, message, BUFFER_SIZE -1);
						strncat_P(uart_message_string, PSTR(" --- only for HADCON_VERSION 2"), BUFFER_SIZE - 1 );
						UART0_Send_Message_String_p(NULL,0);
						break;
#endif
					case commandKeyNumber_OW8S:
						/* command */
						helpShowCommandOrResponse_p (NULL, NULL, PSTR("- reads all switches and shows the IDs"));
						/* command */
						helpShowCommandOrResponse_p (NULL, NULL, PSTR("<ID> - reads single switch"));
						/* command */
						helpShowCommandOrResponse_p (NULL, NULL, PSTR("<value> - writes <value> to all switches"));
						/* command */
						helpShowCommandOrResponse_p (NULL, NULL, PSTR("<ID> <value> - writes <value> to single switch"));
						/* response */
						helpShowCommandOrResponse_p (currentReceiveHeader, NULL, PSTR("<current state of switch>"));
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
				strncat(message, uart_message_string, BUFFER_SIZE -1) ;
				clearString(uart_message_string, BUFFER_SIZE);
				strncat_P(message, PSTR("---"), BUFFER_SIZE - 1 );

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

	strncat(uart_message_string, prefix, BUFFER_SIZE - 1 );
	strncat_P(uart_message_string, PSTR("  available commands are:"), BUFFER_SIZE - 1 );
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
				strncat_P(spaces, string_1x_, MAX_LENGTH_KEYWORD -1) ;
			}
		}

		/* compose message */
		// e.g.
		//	RECV HELP --- SEND : send can message
		//	RECV HELP ---        	SEND CAN-ID ID-Range [RTR <nBytes> D0 .. D7]

		// short description
		strncat(uart_message_string, prefix, BUFFER_SIZE - 1 );
		strncat_P(uart_message_string, string_1x_, BUFFER_SIZE - 1 );

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
		const void* syntaxes_p[] = { &(commandSyntaxes[index]), &(commandSyntaxAlternatives[index]), NULL };
		for (int var = 0; NULL != syntaxes_p[var]; ++var)
		{
			if ( 0 == strlen_P( (const char*) (pgm_read_word( syntaxes_p[var])) ) ) { break; }
			strncat(uart_message_string, prefix, BUFFER_SIZE - 1 );
			strncat_P(uart_message_string, string_1x_, BUFFER_SIZE - 1 );

			for (size_t var = 0; var < MAX_LENGTH_KEYWORD + 3; ++var)
			{
				strncat_P(uart_message_string, string_1x_, BUFFER_SIZE -1 ) ;
			}

			strncat_P(uart_message_string, PSTR(" \t"), BUFFER_SIZE -1 ) ;
			strncat_P(uart_message_string, (const char*) (pgm_read_word( &(commandKeywords[index]))), BUFFER_SIZE - 1);
			strncat_P(uart_message_string, string_1x_, BUFFER_SIZE -1 ) ;
			strncat_P(uart_message_string, (const char*) (pgm_read_word( syntaxes_p[var])), BUFFER_SIZE - 1);
			UART0_Send_Message_String_p(NULL, 0);
		}
	}
}

void helpShowAvailableSubCommands(int maximumIndex, PGM_P const commandKeywords[])
{
	strncat(uart_message_string, message, BUFFER_SIZE -1) ;
	strncat_P(uart_message_string, string_1x_, BUFFER_SIZE - 1 );
	strncat(uart_message_string, currentCommandKeyword, BUFFER_SIZE - 1 );
	strncat_P(uart_message_string, PSTR(" available commands"), BUFFER_SIZE - 1 );

	UART0_Send_Message_String_p(NULL,0);

	for (int i=0; i < maximumIndex; i++)
	{
		strncat(uart_message_string, message, BUFFER_SIZE -1) ;
		strncat_P(uart_message_string, PSTR("    "), BUFFER_SIZE - 1 );
		strncat_P(uart_message_string, (const char*) (pgm_read_word( &(commandKeywords[i]))), BUFFER_SIZE - 1 );
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

void helpShowCommandOrResponse(char* currentReceiveHeader, PGM_P modifier, PGM_P string, ...)
{
	/* header */
	strncat(uart_message_string, message, BUFFER_SIZE - 1);

	/* command / response */
	if (NULL == currentReceiveHeader)
	{
			strncat_P(uart_message_string, PSTR(" command "), BUFFER_SIZE - 1);
	}
	else
	{
			strncat_P(uart_message_string, PSTR(" response"), BUFFER_SIZE - 1);
	}


	/* cases/spaces */
	if (NULL != modifier)
	{
		strncat_P(uart_message_string, string_1x_, BUFFER_SIZE - 1);
		strncat_P(uart_message_string, modifier, BUFFER_SIZE - 1);
	}

	/* colon */
	strncat_P(uart_message_string, PSTR(": "), BUFFER_SIZE - 1);

	/* command / receive */
	if (NULL == currentReceiveHeader)
	{
			strncat(uart_message_string, currentCommandKeyword, BUFFER_SIZE - 1);
			strncat_P(uart_message_string, string_1x_, BUFFER_SIZE - 1);
	}
	else
	{
			strncat(uart_message_string, currentReceiveHeader, BUFFER_SIZE - 1);
	}

	/* arguments */
	if (NULL != string)
	{
		va_list argumentPointers;
		va_start (argumentPointers, string);
		vsnprintf_P(resultString, BUFFER_SIZE - 1, string, argumentPointers);
		va_end(argumentPointers);

		strncat(uart_message_string, resultString, BUFFER_SIZE -1);
	}

	UART0_Send_Message_String_p(NULL, 0);
}
