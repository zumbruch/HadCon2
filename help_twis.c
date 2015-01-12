/*
   Licensed under the EUPL V.1.1, Lizenziert unter EUPL V.1.1 
*/
/*
 * twis_help.c
 *
 *  Created on: Oct 18, 2011
 *      Author: Peter Zumbruch, GSI, P.Zumbruch@gsi.de
 */

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <avr/pgmspace.h>

#include "api.h"
#include "api_help.h"
#include "api_define.h"
#include "api_global.h"
#include "api_debug.h"
#include "help_twis.h"

static const char arguments[] 		PROGMEM = "<address> <data length> <data bytes1 ... 8>";

void help_twis(char *currentReceiveHeader, char *currentCommandKeyword)
{
	if (NULL == currentReceiveHeader)  { return; }
	if (NULL == currentCommandKeyword) { return; }

	strncat(uart_message_string, message, BUFFER_SIZE - 1 );
	strncat_P(uart_message_string, PSTR(" TWI / I2C access (dummy name)"), BUFFER_SIZE - 1 );
	UART0_Send_Message_String_p(NULL,0);

	/* command */
	helpShowCommandOrResponse_p (                NULL, PSTR("           "),   PSTR("<1/0> %S"),arguments);

	/* response */
	helpShowCommandOrResponse_p (currentReceiveHeader, PSTR("(write \"0\")"), PSTR("0 %S -OK-"),arguments);
	helpShowCommandOrResponse_p (currentReceiveHeader, PSTR(" (read \"1\")"), PSTR("1 %S"), arguments);
}



