/*
   Licensed under the EUPL V.1.1, Lizenziert unter EUPL V.1.1 
*/
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

//static const char filename[] 		PROGMEM = __FILE__;

void version(void)
{
   if ( commandKeyNumber_MAXIMUM_NUMBER <= ptr_uartStruct->commandKeywordIndex)
   {
	   ptr_uartStruct->commandKeywordIndex = commandKeyNumber_VERS;
   }
   createReceiveHeader(NULL,NULL,0);
   snprintf_P(uart_message_string,BUFFER_SIZE -1, PSTR("%s%s %s %s"), uart_message_string, CODE_VERSION, __DATE__, __TIME__);
   UART0_Send_Message_String_p(NULL,0);

   return;
}
