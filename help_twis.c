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
#include "api_define.h"
#include "api_global.h"
#include "api_debug.h"
#include "help_twis.h"

void help_twis(char *currentReceiveHeader, char *currentCommandKeyword)
{
	if (NULL == currentReceiveHeader)  { return; }
	if (NULL == currentCommandKeyword) { return; }

	snprintf_P(uart_message_string, BUFFER_SIZE - 1,
			PSTR("%s TWI / I2C access (dummy name) "), message );
	UART0_Send_Message_String_p(NULL,0);
	snprintf_P(uart_message_string, BUFFER_SIZE - 1,
			PSTR("%s command : %s <1/0> <address> <data length> <data bytes1 ... 8>"), message, currentCommandKeyword );
	UART0_Send_Message_String_p(NULL,0);
	snprintf_P(uart_message_string, BUFFER_SIZE - 1,
			PSTR("%s response (write \"0\"): %s 0 <address> <data length> <data bytes1 ... 8> -OK-"), message, currentReceiveHeader );
	UART0_Send_Message_String_p(NULL,0);
	snprintf_P(uart_message_string, BUFFER_SIZE - 1,
			PSTR("%s response  (read \"1\"): %s 1 <address> <data length> <data bytes1 ... 8> "), message, currentReceiveHeader );
	UART0_Send_Message_String_p(NULL,0);
}



