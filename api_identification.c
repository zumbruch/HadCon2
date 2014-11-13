/*
 * api-identification.c
 *
 *  Created on: May 5, 2014
 *      Author: P.Zumbruch@gsi.de
 */

#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdarg.h>
#include "api.h"
#include "api_define.h"
#include "api_identification.h"

//static const char filename[] 		PROGMEM = __FILE__;

void identification(void)
{
   if ( commandKeyNumber_MAXIMUM_NUMBER <= ptr_uartStruct->commandKeywordIndex)
   {
	   ptr_uartStruct->commandKeywordIndex = commandKeyNumber_IDN;
   }
   createReceiveHeader(NULL,NULL,0);
   snprintf_P(uart_message_string,BUFFER_SIZE -1, PSTR("%s%s"), uart_message_string, IDENTIFICATION);
   UART0_Send_Message_String_p(NULL,0);

   return;
}
