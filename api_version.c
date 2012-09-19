/*
 * api-version.c
 *
 *  Created on: Oct 10, 2011
 *      Author: P.Zumbruch@gsi.de
 */

#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdarg.h>
#include "api_define.h"
#include "api_global.h"

#include "api_debug.h"
#include "mem-check.h"
#include "api_show.h"
#include "api_version.h"

void version(void)
{
   createReceiveHeader(ptr_uartStruct,uart_message_string,BUFFER_SIZE -1 );
   snprintf_P(uart_message_string,BUFFER_SIZE -1, PSTR("%s%s %s"), uart_message_string, CODE_VERSION, __DATE__);
   UART0_Send_Message_String_p(NULL,0);

   return;
}
