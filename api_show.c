/*
 * api-show.c
 *
 *  Created on: Apr 12, 2010
 *      Author: P.Zumbruch@gsi.de
 */

#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdarg.h>
#include "api_define.h"
#include "api_global.h"

#include "api_debug.h"#include "mem-check.h"
#include "api_show.h"



/* max length defined by MAX_LENGTH_PARAMETER */
static const char commandShowKeyword00[] PROGMEM = "unused_mem_start";
static const char commandShowKeyword01[] PROGMEM = "unused_mem";
static const char commandShowKeyword02[] PROGMEM = "free_mem";
static const char commandShowKeyword03[] PROGMEM = "mem";
static const char commandShowKeyword04[] PROGMEM = "all_errors";

//const showCommand_t showCommands[] PROGMEM =
//{
//		{ (int8_t(*)(struct uartStruct)) showFreeMemNow, commandShowKeyword00 },
//		{ (int8_t(*)(struct uartStruct)) showUnusedMemNow, commandShowKeyword01 },
//		{ (int8_t(*)(struct uartStruct)) showUnusedMemStart, commandShowKeyword02 }
//};

const char* commandShowKeywords[] PROGMEM = {
		commandShowKeyword00,
		commandShowKeyword01,
		commandShowKeyword02,
		commandShowKeyword03,
		commandShowKeyword04
};


int8_t show(struct uartStruct *ptr_uartStruct)
{
	uint8_t index;
	//int8_t (*func)(struct uartStruct);
    //struct showCommand_t
 	printDebug_p(debugLevelEventDebug, debugSystemSHOW, __LINE__, PSTR(__FILE__), PSTR("show begin"));

	switch(ptr_uartStruct->number_of_arguments)
	{
	case 0:
		for (index = 0; index < commandShowKeyNumber_MAXIMUM_NUMBER; index++)
		{
 			printDebug_p(debugLevelEventDebug, debugSystemSHOW, __LINE__, PSTR(__FILE__), PSTR("unused mem now %i "), get_mem_unused());
 			printDebug_p(debugLevelEventDebug, debugSystemSHOW, __LINE__, PSTR(__FILE__), PSTR("show all begin %i"),index);

			ptr_uartStruct->number_of_arguments = 1;
			for (uint8_t i = 0; i < MAX_LENGTH_PARAMETER; i++) {setParameter[1][i]=STRING_END;}
			snprintf_P(setParameter[1],MAX_LENGTH_PARAMETER -1, (const prog_char*) (pgm_read_word( &(commandShowKeywords[index]))));

 			printDebug_p(debugLevelEventDebug, debugSystemSHOW, __LINE__, PSTR(__FILE__), PSTR("recursive call of show with parameter \"%s\" (%p)"), &setParameter[1][0], &setParameter[1][0]);

			show(ptr_uartStruct);

 			printDebug_p(debugLevelEventDebug, debugSystemSHOW, __LINE__, PSTR(__FILE__), PSTR("show all end %i"), index);

			ptr_uartStruct->number_of_arguments = 0;
		}
		break;
	case 1:
		index = 0;
		// find matching show keyword
		while (index < commandShowKeyNumber_MAXIMUM_NUMBER)
		{
		   if ( 0 == strncmp_P(&setParameter[1][0], (const char*) (pgm_read_word( &(commandShowKeywords[index]))), MAX_LENGTH_PARAMETER) )
		   {
 			   printDebug_p(debugLevelEventDebug, debugSystemSHOW, __LINE__, PSTR(__FILE__), PSTR("keyword %s matches"),&setParameter[1][0]);
			   break;
		   }
		   else
		   {
 			   printDebug_p(debugLevelEventDebugVerbose, debugSystemSHOW, __LINE__, PSTR(__FILE__), PSTR("keyword %s doesn't match"), &setParameter[1][0]);
		   }
		   index++;
		}
		switch (index)
		{
		   case commandShowKeyNumber_FREE_MEM_NOW:
		   case commandShowKeyNumber_UNUSED_MEM_NOW:
		   case commandShowKeyNumber_UNUSED_MEM_START:
			   if ( debugLevelEventDebug <= globalDebugLevel && ((globalDebugSystemMask >> debugSystemSHOW) & 0x1))
			   {
		         strncat_P(uart_message_string,(const char*) (pgm_read_word( &(commandShowKeywords[index]))),BUFFER_SIZE -1);
 		         printDebug_p(debugLevelEventDebug, debugSystemSHOW, __LINE__, PSTR(__FILE__), PSTR("call showMem for: %s"), uart_message_string);
			   }
			   showMem(ptr_uartStruct, index);
		      break;
		   case commandShowKeyNumber_MEM:
		      createExtendedSubCommandReceiveResponseHeader(ptr_uartStruct, commandKeyNumber_SHOW, index, commandShowKeywords);
		      UART0_Send_Message_String_p(NULL,0);

		      showMem(ptr_uartStruct, commandShowKeyNumber_FREE_MEM_NOW);
		      showMem(ptr_uartStruct, commandShowKeyNumber_UNUSED_MEM_START);
		      showMem(ptr_uartStruct, commandShowKeyNumber_UNUSED_MEM_NOW);
		      break;
		   case commandShowKeyNumber_ALL_ERRORS:
		      showErrors(ptr_uartStruct, index);
		      break;
		   default:
 		      printDebug_p(debugLevelEventDebug, debugSystemSHOW, __LINE__, PSTR(__FILE__), PSTR("call Communication_Error"));

		      CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, FALSE, PSTR("show:invalid argument"));
		      return 1;
		      break;
		}
		break;
		   default:
		      ptr_uartStruct->number_of_arguments = 1;
		      show(ptr_uartStruct);
		      break;
	}

     printDebug_p(debugLevelEventDebug, debugSystemSHOW, __LINE__, PSTR(__FILE__), PSTR("show end"));

	return 0;
}

int8_t showMem(struct uartStruct * ptr_uartStruct, uint8_t index)
{

   unsigned short memory;
   switch (index)
   {
      case commandShowKeyNumber_FREE_MEM_NOW:
         memory = get_mem_free();
         break;
      case commandShowKeyNumber_UNUSED_MEM_NOW:
         memory = get_mem_unused();
         break;
      case commandShowKeyNumber_UNUSED_MEM_START:
         memory = unusedMemoryStart;
         break;
      default:
         CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, FALSE, PSTR("show:invalid argument"));
         return 1;
         break;
   }

   createExtendedSubCommandReceiveResponseHeader(ptr_uartStruct, commandKeyNumber_SHOW, index, commandShowKeywords);

   snprintf_P(uart_message_string,BUFFER_SIZE -1, PSTR("%s%i"), uart_message_string, memory);

   UART0_Send_Message_String_p(NULL,0);

   return 0;
}

int8_t showFreeMemNow(struct uartStruct * ptr_uartStruct)
{
   return showMem(ptr_uartStruct, commandShowKeyNumber_FREE_MEM_NOW);
}

int8_t showUnusedMemNow(struct uartStruct * ptr_uartStruct)
{
   return showMem(ptr_uartStruct, commandShowKeyNumber_UNUSED_MEM_NOW);
}

int8_t showUnusedMemStart(struct uartStruct * ptr_uartStruct)
{
   return showMem(ptr_uartStruct, commandShowKeyNumber_UNUSED_MEM_START);
}


/*
void help_show(void)
{
   snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("RECV HELP *** show (internal) settings"));
   UART0_Send_Message_String_p(NULL,0);
   snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("RECV HELP ***     command : SHOW [command_key]"));
   UART0_Send_Message_String_p(NULL,0);
   snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("RECV HELP *** all response: RECV SHOW command_key1 ... "));
   UART0_Send_Message_String_p(NULL,0);
   snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("RECV HELP ***               ...               ... "));
   UART0_Send_Message_String_p(NULL,0);
   snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("RECV HELP ***               RECV SHOW command_keyN ... "));
   UART0_Send_Message_String_p(NULL,0);
   snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("RECV HELP *** cmd response: RECV SHOW command_key"));
   UART0_Send_Message_String_p(NULL,0);
   snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("RECV HELP *** SHOW available command_keys"));
   UART0_Send_Message_String_p(NULL,0);
   for (int i=0; i < commandShowKeyNumber_MAXIMUM_NUMBER; i++)
   {
      snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("RECV HELP ***    "));
      //              strncat_P(uart_message_string, (PGM_P) pgm_read_word( &(commandShowKeywords[i])), MAX_LENGTH_PARAMETER) ;
      //              strncat_P(uart_message_string, commandShowKeywords[i], MAX_LENGTH_PARAMETER) ;
      strncat_P(uart_message_string,(const char*) (pgm_read_word( &(commandShowKeywords[i]))),BUFFER_SIZE -1);
      //              strncat_P(uart_message_string, (const char*) (pgm_read_word( &(commandShowKeywords[i]))), MAX_LENGTH_PARAMETER) ;
      //              strncmp_P(&setParameter[1][0], (const char*) (pgm_read_word( &(commandShowKeywords[index]))), MAX_LENGTH_PARAMETER);

      UART0_Send_Message_String_p(NULL,0);
   }
}
*/

void showErrors(struct uartStruct * ptr_uartStruct, uint8_t index)
{
    unsigned char errmax[6];
    errmax[ERRA] = SERIAL_ERROR_MAXIMUM_INDEX;
    errmax[ERRC] = CAN_ERROR_MAXIMUM_INDEX;
    errmax[ERRG] = GENERAL_ERROR_MAXIMUM_INDEX;
    errmax[ERRM] = MOB_ERROR_MAXIMUM_INDEX;
    errmax[ERRT] = TWI_ERROR_MAXIMUM_INDEX;
    errmax[ERRU] = 0;

    createExtendedSubCommandReceiveResponseHeader(ptr_uartStruct, commandKeyNumber_SHOW, index, commandShowKeywords);
    strncat_P(uart_message_string,PSTR("following all default error messages:"),BUFFER_SIZE -1);

    for (int errType=0; errType < ERR_MAXIMUM_NUMBER; errType++)
    {
       for (int err=0; err < errmax[errType]; err++)
       {
          CommunicationError_p(errType, err ,FALSE ,PSTR("SHOW messages"));
       }
    }
}
