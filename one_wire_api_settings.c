/*
 * one_wire_api_settings.c
 *
 *  Created on: May 3, 2010
 *      Author: zumbruch
 */


#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "api_define.h"
#include "api_global.h"

#include "api_debug.h"#include "mem-check.h"
#include "one_wire_temperature.h"
#include "one_wire_adc.h"
#include "one_wire_api_settings.h"

/* max length defined by MAX_LENGTH_PARAMETER */
static const char owiApiCommandKeyword00[] PROGMEM = "common_temp_convert";
static const char owiApiCommandKeyword01[] PROGMEM = "common_adc_convert";

//const showCommand_t showCommands[] PROGMEM =
//{
//      { (int8_t(*)(struct uartStruct)) showFreeMemNow, owiApiCommandKeyword00 },
//      { (int8_t(*)(struct uartStruct)) showUnusedMemNow, owiApiCommandKeyword01 },
//      { (int8_t(*)(struct uartStruct)) showUnusedMemStart, owiApiCommandKeyword02 }
//};

const char* owiApiCommandKeywords[] PROGMEM = {
        owiApiCommandKeyword00,
        owiApiCommandKeyword01};


int8_t owiApi(struct uartStruct *ptr_uartStruct)
{
    uint8_t index;
    //int8_t (*func)(struct uartStruct);
    //struct showCommand_t

     printDebug_p(debugLevelEventDebug, debugSystemOWIApiSettings, __LINE__, PSTR(__FILE__), PSTR("begin"));

    switch(ptr_uartStruct->number_of_arguments)
    {
    case 0:
        for (index = 0; index < owiApiCommandKeyNumber_MAXIMUM_NUMBER; index++)
        {
             printDebug_p(debugLevelEventDebug, debugSystemOWIApiSettings, __LINE__, PSTR(__FILE__), PSTR("all begin %i"), index);
            ptr_uartStruct->number_of_arguments = 1;

            clearString(setParameter[1], MAX_LENGTH_PARAMETER);
            snprintf_P(setParameter[1],MAX_LENGTH_PARAMETER -1, (const prog_char*) (pgm_read_word( &(owiApiCommandKeywords[index]))));

             printDebug_p(debugLevelEventDebug, debugSystemOWIApiSettings, __LINE__, PSTR(__FILE__), PSTR("recursive call of %s with parameter \"%s\" (%p)"), __func__, &setParameter[1][0], &setParameter[1][0]);

            owiApi(ptr_uartStruct);

             printDebug_p(debugLevelEventDebug, debugSystemOWIApiSettings, __LINE__, PSTR(__FILE__), PSTR("all end %i"), index);

            ptr_uartStruct->number_of_arguments = 0;
        }
        break;
    default: /* index > 0 */
        index = 0;

        // find matching owiApi keyword
        while (index < owiApiCommandKeyNumber_MAXIMUM_NUMBER)
        {
           if ( 0 == strncmp_P(&setParameter[1][0], (const char*) (pgm_read_word( &(owiApiCommandKeywords[index]))), MAX_LENGTH_PARAMETER) )
           {
               printDebug_p(debugLevelEventDebug, debugSystemOWIApiSettings, __LINE__, PSTR(__FILE__), PSTR("keyword %s matches"), &setParameter[1][0]);
              break;
           }
           else
           {
               printDebug_p(debugLevelEventDebugVerbose, debugSystemOWIApiSettings, __LINE__, PSTR(__FILE__), PSTR("keyword %s doesn't match"), &setParameter[1][0]);
           }
           index++;
        }

        switch (index)
        {
           case owiApiCommandKeyNumber_COMMON_ADC_CONVERSION:
           case owiApiCommandKeyNumber_COMMON_TEMPERATURE_CONVERSION:
              owiApiFlag(ptr_uartStruct, index);
              break;
           default:
              CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, FALSE, PSTR("owiApi:invalid argument"));
              return 1;
              break;
        }
        break;
    }

     printDebug_p(debugLevelEventDebug, debugSystemOWIApiSettings, __LINE__, PSTR(__FILE__), PSTR("end"));

    return 0;
}

int8_t owiApiFlag(struct uartStruct * ptr_uartStruct, uint8_t index)
{
   uint8_t *ptr_flag;
   switch (index)
   {
      case owiApiCommandKeyNumber_COMMON_ADC_CONVERSION:
         ptr_flag = &owiUseCommonAdcConversion_flag;
         break;
      case owiApiCommandKeyNumber_COMMON_TEMPERATURE_CONVERSION:
         ptr_flag = &owiUseCommonTemperatureConversion_flag;
         break;
      default:
         CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, FALSE, PSTR("owiApiFlag:invalid argument"));
         return 1;
         break;
   }

   createExtendedSubCommandReceiveResponseHeader(ptr_uartStruct, commandKeyNumber_OWSA, index, owiApiCommandKeywords);
   if (1 < ptr_uartStruct->number_of_arguments)  /*set value*/
   {
      *ptr_flag = ( 0 != strtoul(setParameter[2], &ptr_setParameter[2], 16));
   }
   snprintf_P(uart_message_string,BUFFER_SIZE -1, PSTR("%s%i"), uart_message_string, *ptr_flag);
   strncat_P(uart_message_string,((*ptr_flag)?PSTR(" (TRUE)"):PSTR(" (FALSE)")),BUFFER_SIZE -1);
   UART0_Send_Message_String_p(NULL,0);

   return 0;
}
