/*
 * VERSION 1.0 Januar 7th 2010 LATE  File: 'one_wire_temperature.c'
 * Author: Linda Fouedjio  based on Michael Traxler based and Giacomo Ortona's hadtempsens
 * modified (heavily rather rebuild): Peter Zumbruch
 */

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/iocan128.h>

#include "api.h"
#include "api_define.h"
#include "api_global.h"

#include "api_debug.h"
#include "can.h"
#include "mem-check.h"

#include "one_wire.h"
#include "one_wire_adc.h"
#include "one_wire_dualSwitch.h"
#include "read_write_register.h"
#include "one_wire_simpleSwitch.h"
#include "one_wire_temperature.h"

#include "OWIDeviceSpecific.h"
#include "OWIHighLevelFunctions.h"
#include "OWIBitFunctions.h"

#define DS1820_START_CONVERSION         0x44
#define DS1820_READ_SCRATCHPAD          0xbe
#define DS1820_WRITE_SCRATCHPAD         0x4E
#define DS1820_CONFIG_REGISTER          0x7f
#define DS1820_READ_POWER_SUPPLY        0xb4

#ifndef OWI_TEMPERATURE_MAX_CONVERSION_TIME_MILLISECONDS
#define OWI_TEMPERATURE_MAX_CONVERSION_TIME_MILLISECONDS 750
//#warning move this 20% shift to the code not directly into the default settings
//#define OWI_TEMPERATURE_MAX_CONVERSION_TIME_MILLISECONDS 900
#endif

#ifndef OWI_TEMPERATURE_CONVERSION_DELAY_MILLISECONDS
#define OWI_TEMPERATURE_CONVERSION_DELAY_MILLISECONDS 10
#endif

#warning TODO: make it changeable through API, by changing the resolution for DS18B20
#ifndef OWI_TEMPERATURE_CONVERSION_TIME_RESOLUTION_SPLIT_FACTOR
#define OWI_TEMPERATURE_CONVERSION_TIME_RESOLUTION_DEFAULT_SHIFT_FACTOR 0 /*12 bit resolution, division by 1*/
//#define OWI_TEMPERATURE_CONVERSION_TIME_RESOLUTION_DEFAULT_SHIFT_FACTOR 1 /*11 bit resolution, division by 2*/
//#define OWI_TEMPERATURE_CONVERSION_TIME_RESOLUTION_DEFAULT_SHIFT_FACTOR 2 /*10 bit resolution, division by 4*/
//#define OWI_TEMPERATURE_CONVERSION_TIME_RESOLUTION_DEFAULT_SHIFT_FACTOR 3 /*9 bit resolution, division by 8*/
#endif

uint16_t owiTemperatureMask = 0;
uint16_t owiTemperatureMask_DS18B20 = 0;
uint16_t* p_owiTemperatureMask_DS18B20 = &owiTemperatureMask_DS18B20;
uint16_t owiTemperatureMask_DS18S20 = 0;
uint16_t* p_owiTemperatureMask_DS18S20 = &owiTemperatureMask_DS18S20;
uint16_t owiTemperatureTimeoutMask = 0xFFFF; /*bit mask of non converted channels/pins 1:conversion timeout*/
uint8_t owiUseCommonTemperatureConversion_flag = TRUE;
uint8_t owiTemperatureConversionGoingOnCountDown = 0;
uint8_t owiTemperatureIgnoreConversionResponse = TRUE;
uint16_t owiTemperatureMaxConversionTime = OWI_TEMPERATURE_MAX_CONVERSION_TIME_MILLISECONDS;
uint8_t owiTemperatureForceParasiticMode = FALSE;
uint8_t owiTemperatureParasiticModeMask = 0xFF; /*TODO*/
uint8_t owiTemperatureResolution_DS18B20 = 12 - OWI_TEMPERATURE_CONVERSION_TIME_RESOLUTION_DEFAULT_SHIFT_FACTOR;
uint8_t owiTemperatureSensorBySensorParasiticConversionFlag = FALSE;

static const char owiTemperatureCommandKeyword00[] PROGMEM = "convert_only";
static const char owiTemperatureCommandKeyword01[] PROGMEM = "no_convert";
static const char owiTemperatureCommandKeyword02[] PROGMEM = "ignore_conv_response";
static const char owiTemperatureCommandKeyword03[] PROGMEM = "max_conv_time_ms";
static const char owiTemperatureCommandKeyword04[] PROGMEM = "force_parasitic_mode";
static const char owiTemperatureCommandKeyword05[] PROGMEM = "resolution_DS18B20"; /*TODO*/
static const char owiTemperatureCommandKeyword06[] PROGMEM = "single_parasitic_conv";
     
const char* owiTemperatureCommandKeywords[] PROGMEM = {
         owiTemperatureCommandKeyword00,
         owiTemperatureCommandKeyword01,
         owiTemperatureCommandKeyword02,
         owiTemperatureCommandKeyword03,
         owiTemperatureCommandKeyword04,
         owiTemperatureCommandKeyword05,
         owiTemperatureCommandKeyword06
};


/*
 * void owiTemperatureSensors( struct uartStruct *ptr_uartStruct )
 *
 * this function checks if an command has been recognized and chooses either
 * to go to
 *      temperature reading
 *      or
 *      miscellaneous commands
 *
 */
void owiTemperatureSensors( struct uartStruct *ptr_uartStruct )
{
   if ( 0 == strlen(ptr_owiStruct->command) )
   {
      owiTemperatureReadSensors(ptr_uartStruct, TRUE);
   }
   else
   {
      owiTemperatureMiscSubCommands( ptr_uartStruct );
   }

   return;
}


/*
 *this function contains all the functions that are necessary to
 *read the temperature of sensor
 * as input parameters the function needs only a keyword
 * the function has no return parameter
 */

void owiTemperatureReadSensors( struct uartStruct *ptr_uartStruct, uint8_t convert )
{
   /* check command syntax */
   if ( 0 != owiTemperatureReadSensorsCheckCommandSyntax(ptr_uartStruct))
   {
      return;
   }

   if ( 0 != owiTemperatureGetNumberOfDevicesAndSetTemperatureMask(ptr_uartStruct))
   {
      //#warning ADC in comparison has here an init - is the Initialization always needed here, or is it just needed once at the beginning?

      /* conversions */

      switch (ptr_uartStruct->number_of_arguments)
      {
         case 0:
            /* read all */
            if ( FALSE != convert ) {owiTemperatureConversions(BUSES, TRUE, FALSE);}
            break;
         case 2:
            /* read single ID w/ temperature conversion of all buses*/
            /* during the filling it couldn't decide weather the 2nd argument
             * is a flag or a value ... no we can, its a flag */
            ptr_owiStruct->conv_flag = ( 0 != ptr_owiStruct->value);
            if (TRUE == ptr_owiStruct->conv_flag) { owiTemperatureConversions(BUSES, TRUE, FALSE); }
            break;
      }

      /*
       * access values
       *
       *   - read DS18B20
       *   - read DS18S20
       */

      int8_t familyCode[] = {
               FAMILY_DS18B20_TEMP,
               FAMILY_DS18S20_TEMP
      };

      uint8_t foundDevices = 0;

      for (uint8_t index = 0; index < sizeof(familyCode)/sizeof(int8_t); index++)
      {
          printDebug_p(eventDebug, debugOWITemperatures, __LINE__, PSTR(__FILE__), PSTR("call: FindFamilyDevicesAndGetValues family code: %x"), familyCode[index]);

         foundDevices += owiFindFamilyDevicesAndAccessValues(BUSES, NumDevicesFound, familyCode[index], NULL );
      }

      if ( TRUE == ptr_owiStruct->idSelect_flag && 0 == foundDevices)
      {
         general_errorCode = CommunicationError(ERRG, -1, TRUE, PSTR("no matching ID was found"), 4000);
      }
   }
   else
   {
      general_errorCode = CommunicationError(ERRG, -1, TRUE, PSTR("no supported 1-wire temperature device present"), 4000);
   }
}

uint8_t owiTemperatureReadSensorsCheckCommandSyntax(struct uartStruct *ptr_uartStruct)
{
   /* check for syntax:
    *    allowed arguments are:
    *       - (empty)
    *       - ID
    *       - ID conversion_flag
    *       
    *       return 0 if ok
    *       else != 0
    */

   int status = 0;
   switch (ptr_uartStruct->number_of_arguments)
   {
      case 0:
         break;
      case 1:
      case 2:
         /* read single ID w/o temperature conversion
          * or
          * read single ID w/ temperature conversion of all buses*/
         if ( FALSE == ptr_owiStruct->idSelect_flag)
         {
            snprintf_P(message, BUFFER_SIZE, PSTR("invalid arguments"));
            CommunicationError_p(ERRA, -1, TRUE, message, COMMUNICATION_ERROR_USE_GLOBAL_MESSAGE_STRING_INDEX_THRESHOLD -1 );
            status = -1;
            break;
         }
         break;
      default:
         snprintf_P(message, BUFFER_SIZE, PSTR("write argument: too many arguments"));
         CommunicationError_p(ERRA, -1, TRUE, message, COMMUNICATION_ERROR_USE_GLOBAL_MESSAGE_STRING_INDEX_THRESHOLD -1 );
         status = -1;
         break;
   }
   return status;
}

/*
 * uint16_t owiTemperatureGetNumberOfDevicesAndSetTemperatureMask(struct uartStruct *ptr_uartStruct)
 *
 * this function searches the one wire buses looking for 1-wire temp sensors
 * it fills the busses where a temp device has been found to  owiTemperatureMask and
 * returns the number of found devices
 *
 */

uint16_t owiTemperatureGetNumberOfDevicesAndSetTemperatureMask(struct uartStruct *ptr_uartStruct)
{
   uint16_t tempDevices = 0;
   NumDevicesFound = owiReadDevicesID(BUSES);
   if ( 0 == NumDevicesFound )
   {
      general_errorCode = CommunicationError(ERRG, GENERAL_ERROR_no_device_is_connected_to_the_bus, 0, NULL, 0);
      tempDevices = 0;
   }
   else
   {
      /* reset mask of busses with sensors*/
      owiTemperatureMask = 0;

      tempDevices += owiScanIDS(FAMILY_DS18B20_TEMP, p_owiTemperatureMask_DS18B20);
      tempDevices += owiScanIDS(FAMILY_DS18S20_TEMP, p_owiTemperatureMask_DS18B20);
       printDebug_p(eventDebug, debugOWITemperatures, __LINE__, PSTR(__FILE__), PSTR("misc temp commands keyword %s matches"), ptr_owiStruct->command);

      if ( 0 < tempDevices )
      {
         owiTemperatureMask |= owiTemperatureMask_DS18B20;
         owiTemperatureMask |= owiTemperatureMask_DS18S20;
      }
   }
    printDebug_p(eventDebug, debugOWITemperatures, __LINE__, PSTR(__FILE__), PSTR("found %i temp devices"), tempDevices);

   return tempDevices;
}

void owiTemperatureMiscSubCommandGetSetFlag(struct uartStruct *ptr_uartStruct, uint8_t *flag, const prog_char *text, uint8_t invert)
{
   switch(ptr_uartStruct->number_of_arguments - 1 /* arguments of argument */)
   {
      case 0:
         /* printout status*/

         /* generate message */
         createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
         strncat(uart_message_string, setParameter[1],BUFFER_SIZE -1 );
         strncat_P(uart_message_string, text, BUFFER_SIZE -1);
         if (FALSE == invert)
         {
            strncat_P(uart_message_string, (  *flag)?PSTR("TRUE"):PSTR("FALSE"),BUFFER_SIZE -1);
         }
         else
         {
            strncat_P(uart_message_string, (! *flag)?PSTR("TRUE"):PSTR("FALSE"),BUFFER_SIZE -1);
         }
         /*send the data*/
         UART0_Send_Message_String_p(uart_message_string, BUFFER_SIZE - 1);
         /*clear strings*/
         clearString(uart_message_string, BUFFER_SIZE);
         clearString(message, BUFFER_SIZE);
         break;
      case 1:

         /* set status*/

         if (FALSE == invert)
         {
            *flag = (0 != ptr_owiStruct->value) ? TRUE: FALSE;
         }
         else
         {
            *flag = !(0 != ptr_owiStruct->value) ? TRUE: FALSE;
         }

         /*recursive call to show change*/
         ptr_uartStruct->number_of_arguments=1;
         owiTemperatureMiscSubCommandGetSetFlag(ptr_uartStruct, flag, text, invert);
         ptr_uartStruct->number_of_arguments=2;
         break;
      default:
         general_errorCode = CommunicationError(ERRG, -1001, TRUE, PSTR("invalid number of arguments"), 101);
         break;
   }
}

void owiTemperatureMiscSubCommandGetSetStepByStepParasiticConversion(struct uartStruct *ptr_uartStruct)
{
   uint8_t *flag = &owiTemperatureSensorBySensorParasiticConversionFlag;

   //owiTemperatureMiscSubCommandGetSetFlag(ptr_uartStruct,flag,PSTR(":"),FALSE);
   //int8_t keywordIndex = owiTemperatureCommandKeyNumber_FORCE_PARASITIC_MODE;
   switch(ptr_uartStruct->number_of_arguments - 1 /* arguments of argument */)
   {
      case 0:
         /* printout status*/

         /* generate message */
         createExtendedSubCommandReceiveResponseHeader(ptr_uartStruct, -1,
                                                       owiTemperatureCommandKeyNumber_SINGLE_SENSOR_PARSITIC_CONVERSION,
                                                       owiTemperatureCommandKeywords);
         strncat_P(uart_message_string, ( FALSE != (*flag)) ? PSTR(" TRUE"):PSTR(" FALSE"),BUFFER_SIZE -1);
         /*send the data*/
         UART0_Send_Message_String_p(uart_message_string, BUFFER_SIZE - 1);
         /*clear strings*/
         clearString(uart_message_string, BUFFER_SIZE);
         break;
      case 1:
#if 0
         /* set status*/
         (*flag) = (0 != ptr_owiStruct->value) ? TRUE: FALSE;
#else
         /* toggle status*/
	owiTemperatureSensorBySensorParasiticConversionFlag = ( FALSE != owiTemperatureSensorBySensorParasiticConversionFlag) ;
#endif
         /*recursive call to show change*/
         ptr_uartStruct->number_of_arguments=1;
         owiTemperatureMiscSubCommandGetSetStepByStepParasiticConversion(ptr_uartStruct);
         ptr_uartStruct->number_of_arguments=2;
         break;
      default:
         general_errorCode = CommunicationError(ERRG, -1001, TRUE, PSTR("invalid number of arguments"), 101);
         break;
   }
}

void owiTemperatureMiscSubCommandGetSetForceParasiticMode(struct uartStruct *ptr_uartStruct)
{
   uint8_t *flag = &owiTemperatureForceParasiticMode;

   //owiTemperatureMiscSubCommandGetSetFlag(ptr_uartStruct,flag,PSTR(":"),FALSE);
   //int8_t keywordIndex = owiTemperatureCommandKeyNumber_FORCE_PARASITIC_MODE;
   switch(ptr_uartStruct->number_of_arguments - 1 /* arguments of argument */)
   {
      case 0:
         /* printout status*/

         /* generate message */
         createExtendedSubCommandReceiveResponseHeader(ptr_uartStruct, -1,
                                                       owiTemperatureCommandKeyNumber_FORCE_PARASITIC_MODE,
                                                       owiTemperatureCommandKeywords);
         strncat_P(uart_message_string, ( FALSE != (*flag)) ? PSTR(" TRUE"):PSTR(" FALSE"),BUFFER_SIZE -1);
         /*send the data*/
         UART0_Send_Message_String_p(uart_message_string, BUFFER_SIZE - 1);
         /*clear strings*/
         clearString(uart_message_string, BUFFER_SIZE);
         break;
      case 1:
#warning TODO why use '#if' 0? 
#if 0
         /* set status*/
         (*flag) = (0 != ptr_owiStruct->value) ? TRUE: FALSE;
#else
         /* toggle status*/
	owiTemperatureForceParasiticMode = ( FALSE != owiTemperatureForceParasiticMode) ;
#endif
         /*recursive call to show change*/
         ptr_uartStruct->number_of_arguments=1;
         owiTemperatureMiscSubCommandGetSetForceParasiticMode(ptr_uartStruct);
         ptr_uartStruct->number_of_arguments=2;
         break;
      default:
         general_errorCode = CommunicationError(ERRG, -1001, TRUE, PSTR("invalid number of arguments"), 101);
         break;
   }
}

void owiTemperatureMiscSubCommandGetSetMaxConversionTime(struct uartStruct *ptr_uartStruct)
{
   switch(ptr_uartStruct->number_of_arguments - 1 /* arguments of argument */)
   {
      case 0:
         /* printout status*/

         /* generate message */
         createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
         strncat(uart_message_string, setParameter[1],BUFFER_SIZE -1 );
         snprintf_P(uart_message_string, BUFFER_SIZE -1 , PSTR("%s%i (0x%x)"),
                    owiTemperatureMaxConversionTime, owiTemperatureMaxConversionTime);
         /*send the data*/
         UART0_Send_Message_String_p(uart_message_string, BUFFER_SIZE - 1);
         break;
      case 1:
         /* set status*/
         owiTemperatureMaxConversionTime = ptr_owiStruct->value;

         /*recursive call to show change*/
         ptr_uartStruct->number_of_arguments=1;
         owiTemperatureMiscSubCommandGetSetMaxConversionTime(ptr_uartStruct);
         ptr_uartStruct->number_of_arguments=2;
         break;
      default:
         general_errorCode = CommunicationError(ERRG, -1001, TRUE, PSTR("invalid number of arguments"), 101);
         break;
   }
}

void owiTemperatureMiscSubCommandGetSetResolution_DS18B20(struct uartStruct *ptr_uartStruct)
{
   switch(ptr_uartStruct->number_of_arguments - 1 /* arguments of argument */)
   {
      case 0:
         /* printout status*/

         /* generate message */
         createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
         strncat(uart_message_string, setParameter[1],BUFFER_SIZE -1 );
         snprintf_P(uart_message_string, BUFFER_SIZE -1 , PSTR("%s%i "),
                     owiTemperatureResolution_DS18B20);
         /*send the data*/
         UART0_Send_Message_String_p(uart_message_string, BUFFER_SIZE - 1);
         break;
      case 1:
      {
         uint16_t value = ptr_owiStruct->value;

         /* set status*/
         switch(value)
         {
            case 12:
            case 11:
            case 10:
            case  9:
               owiTemperatureResolution_DS18B20 = value;
               break;
            default:
               clearString(message, BUFFER_SIZE);
               snprintf_P(message, BUFFER_SIZE-1 , PSTR("%s value '%i' out of range [9,..,12]"),value);

               general_errorCode = CommunicationError(ERRG, -1001, TRUE, NULL ,
                                                         COMMUNICATION_ERROR_USE_GLOBAL_MESSAGE_STRING_INDEX_THRESHOLD -111);
               return;
               break;
         }

         /*recursive call to show change*/
         ptr_uartStruct->number_of_arguments=1;
         owiTemperatureMiscSubCommandGetSetResolution_DS18B20(ptr_uartStruct);
         ptr_uartStruct->number_of_arguments=2;
      }
      break;
      default:
         general_errorCode = CommunicationError(ERRG, -1001, TRUE, PSTR("invalid number of arguments"), 101);
         break;
   }
}

void owiTemperatureMiscSubCommandConvertOnly(struct uartStruct *ptr_uartStruct)
{
   switch(ptr_uartStruct->number_of_arguments - 1 /* arguments of argument */)
   {
      case 0:
         if ( 0 != owiTemperatureGetNumberOfDevicesAndSetTemperatureMask(ptr_uartStruct))
         {
            if ( 0 == owiTemperatureConversions(BUSES, TRUE, FALSE))
            {
               createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
               strncat(uart_message_string, setParameter[1],BUFFER_SIZE -1 );
               strncat_P(uart_message_string, PSTR(" done"),BUFFER_SIZE -1 );
               /*send the data*/
               UART0_Send_Message_String_p(uart_message_string, BUFFER_SIZE - 1);
               /*clear strings*/
               clearString(uart_message_string, BUFFER_SIZE);
            }
         }
         else
         {
            general_errorCode = CommunicationError(ERRG, -1, TRUE, PSTR("no supported 1-wire temperature device present"), 4000);
         }
         break;
      case 1:
         /* any further argument*/
         if ( 0 != owiTemperatureGetNumberOfDevicesAndSetTemperatureMask(ptr_uartStruct))
         {
            if ( 0 == owiTemperatureConversions(BUSES, FALSE, FALSE))
            {
               createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
               strncat(uart_message_string, setParameter[1],BUFFER_SIZE -1 );
               strncat_P(uart_message_string, PSTR(" "),BUFFER_SIZE -1 );
               strncat(uart_message_string, setParameter[2],BUFFER_SIZE -1 );
               strncat_P(uart_message_string, PSTR(" done"),BUFFER_SIZE -1 );
               /*send the data*/
               UART0_Send_Message_String_p(uart_message_string, BUFFER_SIZE - 1);
               /*clear strings*/
               clearString(uart_message_string, BUFFER_SIZE);
            }
         }
         else
         {
            general_errorCode = CommunicationError(ERRG, -1, TRUE, PSTR("no supported 1-wire temperature device present"), 4000);
         }
         break;
      default:
         CommunicationError_p(ERRA, -1, 0, PSTR("invalid number of arguments"), -1);
         break;
   }
}

void owiTemperatureMiscSubCommands( struct uartStruct *ptr_uartStruct )
{
   uint16_t index = 0;
   // find matching temperature command keyword
   while (index < owiTemperatureCommandKeyNumber_MAXIMUM_NUMBER)
   {
      if ( 0 == strncmp_P(ptr_owiStruct->command, (const char*) (pgm_read_word( &(owiTemperatureCommandKeywords[index]))), MAX_LENGTH_PARAMETER) )
      {
          printDebug_p(eventDebug, debugOWITemperatures, __LINE__, PSTR(__FILE__), PSTR("misc temp commands keyword %s matches"), ptr_owiStruct->command);
         break;
      }
      else
      {
          printDebug_p(eventDebugVerbose, debugOWITemperatures, __LINE__, PSTR(__FILE__), PSTR("misc temp commands keyword %s doesn't match"), &setParameter[1][0]);
      }
      index++;
   }

   switch (index)
   {
      case owiTemperatureCommandKeyNumber_CONVERT_ONLY:
         owiTemperatureMiscSubCommandConvertOnly(ptr_uartStruct);
         break;
      case owiTemperatureCommandKeyNumber_NO_CONVERT:
         ptr_uartStruct->number_of_arguments = 0;
         clearString(setParameter[1], MAX_LENGTH_PARAMETER);
         owiTemperatureReadSensors(ptr_uartStruct, FALSE);
         break;
      case owiTemperatureCommandKeyNumber_IGNORE_CONVERSION_RESPONSE:
         owiTemperatureMiscSubCommandGetSetFlag(ptr_uartStruct, &owiTemperatureIgnoreConversionResponse, PSTR(" ignore conv. response: "), FALSE);
         break;
      case owiTemperatureCommandKeyNumber_MAX_CONVERSION_TIME_MS:
         owiTemperatureMiscSubCommandGetSetMaxConversionTime(ptr_uartStruct);
         break;
      case owiTemperatureCommandKeyNumber_FORCE_PARASITIC_MODE:
         owiTemperatureMiscSubCommandGetSetForceParasiticMode(ptr_uartStruct);
         break;
      case owiTemperatureCommandKeyNumber_RESOLUTION_DS18B20:
         owiTemperatureMiscSubCommandGetSetResolution_DS18B20(ptr_uartStruct);
         break;
      case owiTemperatureCommandKeyNumber_SINGLE_SENSOR_PARSITIC_CONVERSION:
	 owiTemperatureMiscSubCommandGetSetStepByStepParasiticConversion(ptr_uartStruct);
	break;
      default:
         CommunicationError_p(ERRA, -1, 0, PSTR("invalid argument"), -1);
         return;
         break;
   }

   return;
}

void owiTemperatureConversionParasitic( unsigned char pins )
{
   unsigned char data = DS1820_START_CONVERSION;
   unsigned char temp;
   unsigned char i;

   // Do once for each bit
   for ( i = 0; i < 8 ; i++ )
   {
      // Determine if lsb is '0' or '1' and transmit corresponding
      // waveform on the bus.
      temp = data & 0x01;
      if ( temp )
      {
         OWI_WriteBit1(pins);
      }
      else
      {
         if (7 == i)
         {
            OWI_WriteBit0ShortRelease(pins);
         }
         else
         {
            OWI_WriteBit0(pins);
         }
      }
      // Right shift the data to get next bit.
      data >>= 1;
   }

   // lowering pin on PORTA3, to trigger external strong pull-up
   // this must come within max 10us after the last writeBit operation, critical if writing a 0,
   // therefore special OWI_WriteBit0ShortRelease call
#warning TODO unused mode stems from mdc relay, clarify
#if 0
#warning name pin 0x8
   PORTA = PORTA & (~0x8);

   _delay_ms(100);
   _delay_ms(100);
   _delay_ms(100);
   _delay_ms(100);
   _delay_ms(100);
   _delay_ms(100);
   _delay_ms(100);
   _delay_ms(100);
   _delay_ms(100);

   // raising pin on PORTA3, to release external strong pull-up
   PORTA |= 0x8;

#else
//   PORTA = PORTA & (~0x8);
   
   PORTA = 0xff;
   DDRA = 0xff;

   _delay_ms(100);
   _delay_ms(100);
   _delay_ms(100);
   _delay_ms(100);
   _delay_ms(100);
   _delay_ms(100);
   _delay_ms(100);
   _delay_ms(100);
   _delay_ms(100);

   // raising pin on PORTA3, to release external strong pull-up
   DDRA =0x00; /* disable output */
   PORTA = 0;
#endif
}

void owiTemperatureSensorBySensorParasiticConversion(uint8_t currentPins, uint16_t selectedDeviceIndex)
{
   uint8_t deviceIndex = 0;
   static const int8_t familyCode[] = {
            FAMILY_DS18B20_TEMP,
            FAMILY_DS18S20_TEMP
   };
   uint8_t skipDevice = FALSE;
   uint8_t index = 0;
  
   /* 
   OWI_SendByte(OWI_ROM_SKIP, currentPins);
   owiTemperatureConversionParasitic(currentPins);
   return;
  */

   for ( deviceIndex = 0 ; deviceIndex < countDEV ; deviceIndex++ )
   {
      /* no overlap of currentPins and devicePin */
      if ( 0 == (currentPins & owi_IDs_pinMask[deviceIndex])) { continue; }

      skipDevice = FALSE;
      for (index = 0; index < sizeof(familyCode)/sizeof(int8_t); index++)
      {
         /* is index selection used and does current device index match?*/
         if ( 0 <= selectedDeviceIndex && deviceIndex != selectedDeviceIndex)
         { skipDevice = TRUE; break; }

         /* does familyID matches current ID */
         if ( familyCode[index] != owi_IDs[deviceIndex][0] )  { skipDevice = TRUE; break; }

         /* bus mask matches device's bus ? */
         if ( 0 == ( ( owiBusMask & owi_IDs_pinMask[deviceIndex] ) & 0xFF ) )  { skipDevice = TRUE; break; }
      }
      /* skip if skipDevice is set */
      if ( FALSE != skipDevice ) { continue;}

      OWI_MatchRom(owi_IDs[deviceIndex], owi_IDs_pinMask[deviceIndex]);
      owiTemperatureConversionParasitic(owi_IDs_pinMask[deviceIndex]);
   }
}

int owiTemperatureConversions( uint8_t *pins, uint8_t waitForConversion, uint8_t spawnReleaseBlock )
{
    printDebug_p(eventDebugVerbose, debugOWITemperatures, __LINE__, PSTR(__FILE__), PSTR(""));

   uint16_t currentTimeoutMask = 0;
   uint16_t maxConversionTime = owiTemperatureMaxConversionTime;
   static unsigned char timeout_flag;
   static uint32_t count;
   static uint32_t maxcount;
   uint8_t currentPins = 0x0;
   uint8_t busPatternIndexMax = PIN_BUS;
   uint8_t status = 0;
   uint16_t selectedDeviceIndex = -1;

   /* check for parasitic lines , non verbose*/
   owiTemperatureFindParasiticlyPoweredDevices(FALSE);

   /* fill ID table and ID bus table OUTSIDE this loop !
    *   if some of the pins have parasitic devices 
    *   or 
    *   parasitic mode is forced
    */
   if ( 0 != owiTemperatureParasiticModeMask || FALSE != owiTemperatureForceParasiticMode )
   {
      /* check if no devices availabe, then return */
      if ( 0 == owiTemperatureGetNumberOfDevicesAndSetTemperatureMask(ptr_uartStruct))
      {
          printDebug_p(eventDebug, debugOWITemperatures, __LINE__, PSTR(__FILE__), PSTR("no temp sensors at all"), maxConversionTime, currentPins);
         return 0;
      }
   }

   /* if a single id has been chosen,
      find its corresponding index in the IDs list
   */
   if (ptr_owiStruct->idSelect_flag)
   {
      selectedDeviceIndex = owiFindIdAndGetIndex(ptr_owiStruct->id);
   }

   /* bus pattern loop:
    * looping over all pins
    */
   for ( int8_t busPatternIndex = 0 ; busPatternIndex < busPatternIndexMax ; busPatternIndex++ )
   {
      if ( TRUE == owiUseCommonTemperatureConversion_flag)
      {
         currentPins = generateCommonPinsPattern(pins, owiBusMask, owiTemperatureMask);
         busPatternIndexMax = 1; // finish loop after one round
      }
      else
      {
         currentPins = pins[busPatternIndex];

         if ( 0 != checkBusAndDeviceActivityMasks(currentPins, busPatternIndex, owiBusMask, owiTemperatureMask, TRUE ))
         {
            continue;
         }
      }

      /*
       * determine max common conversion time (per bus)
       */
      maxConversionTime = owiTemperatureMaxConversionTime;

      /* if there are only DS18B20 devices availabe, i.e. no DS18S20, conversion time can be lowered depending on resolution */
      if ( 0 == ((owiTemperatureMask_DS18S20 & currentPins) & 0xFF ) )
      {
          printDebug_p(eventDebug, debugOWITemperatures, __LINE__, PSTR(__FILE__), PSTR("bus: only DS18B20, adopting max. t_conv to: %i ms (pins 0x%x)"), maxConversionTime, currentPins);

         maxConversionTime = maxConversionTime >> (12 - owiTemperatureResolution_DS18B20);
      }
      else
      {
          printDebug_p(eventDebug, debugOWITemperatures, __LINE__, PSTR(__FILE__), PSTR("bus: mixed sensors, max. t_conv: %f (pins 0x%x)"), maxConversionTime, currentPins);
      }

      if ( 0 == OWI_DetectPresence(currentPins) )
      {
          printDebug_p(eventDebug, debugOWITemperatures, __LINE__, PSTR(__FILE__), PSTR("no Device present (pin pattern 0x%x)"), currentPins);

         continue;
      }
      else
      {
          printDebug_p(eventDebug, debugOWITemperatures, __LINE__, PSTR(__FILE__), PSTR("some devices present (pin pattern 0x%x)"), currentPins);
          printDebug_p(eventDebug, debugOWITemperatures, __LINE__, PSTR(__FILE__), PSTR("starting conversion sequence"));
      }

       /*
        * send conversion command to all devices
        */

      /*
       * expect parasitic devices if forced to or the parasiticModeMask has common pins with the current pins
       */
      if (FALSE != owiTemperatureForceParasiticMode || 0 != ((owiTemperatureParasiticModeMask & currentPins) & 0xF ) )
      {
           printDebug_p(eventDebugVerbose, debugOWITemperatures, __LINE__, PSTR(__FILE__), PSTR("parasitic conversion "));

	  if ( FALSE != owiTemperatureSensorBySensorParasiticConversionFlag )
	  {
	    OWI_SendByte(OWI_ROM_SKIP, currentPins);
	    owiTemperatureConversionParasitic(currentPins);
	  }
	  else
	  {
	    owiTemperatureSensorBySensorParasiticConversion(currentPins, selectedDeviceIndex);
	    waitForConversion = FALSE;
	  }
           printDebug_p(eventDebugVerbose, debugOWITemperatures, __LINE__, PSTR(__FILE__), PSTR("parasitic conversion done"));
       }
       else
       {
          /*
           * non parasitic mode: wait / don't wait
           */
          if ( TRUE == waitForConversion )
          {
             maxcount = ( OWI_TEMPERATURE_CONVERSION_DELAY_MILLISECONDS > 0 ) ? maxConversionTime / OWI_TEMPERATURE_CONVERSION_DELAY_MILLISECONDS : 1;
             count = maxcount;
             timeout_flag = FALSE;

              printDebug_p(eventDebug, debugOWITemperatures, __LINE__, PSTR(__FILE__), PSTR("waiting for conversion"));

             uint8_t owiReadBit = 0;

             /*conversion commands to reach all devices of the current pin at once*/
             OWI_SendByte(OWI_ROM_SKIP, currentPins);
             OWI_SendByte(DS1820_START_CONVERSION, currentPins);

             while ( 0 == owiReadBit)
             {
                /* if conversion response isn't ignored, look for the currentPins to react*/
                if ( FALSE == owiTemperatureIgnoreConversionResponse )
                {
                   owiReadBit = OWI_ReadBit(currentPins);
                }
                _delay_ms(OWI_TEMPERATURE_CONVERSION_DELAY_MILLISECONDS);

                /* timeout check */
                if ( 0 == --count)
                {
                   /* if conversion response isn't ignored, set timeout flag*/
                   if ( FALSE == owiTemperatureIgnoreConversionResponse )
                   {
                      timeout_flag = TRUE;
                   }
                   break;
                }
             }
          }
       }

       if ( TRUE == waitForConversion )
       {
          status = owiTemperatureConversionEvaluateTimeoutFlag(timeout_flag, currentPins, maxcount, count, &currentTimeoutMask, maxConversionTime);
       }
       else
       {
          status = owiTemperatureConversionEvaluateTimeoutFlag(FALSE, currentPins, 0, 0, &currentTimeoutMask, 0);
          if ( TRUE == spawnReleaseBlock )
          {
             owiTemperatureConversionGoingOnCountDown = (1 > (maxConversionTime / 1000)) ? 1: (maxConversionTime / 1000);
          }
       }
   }//end of for ( int8_t busPatternIndex = 0 ; busPatternIndex < PIN_BUS ; busPatternIndex++ )

    printDebug_p(eventDebug, debugOWITemperatures, __LINE__, PSTR(__FILE__), PSTR("finished (Timeout: current = 0x%x, all = 0x%x)"), currentTimeoutMask, owiTemperatureTimeoutMask);

   return status;
}

/*
 * owiTemperatureConversionEvaluateTimeoutFlag(const unsigned char timeout_flag, const uint8_t currentPins, const uint32_t maxcount, const uint32_t count, uint16_t *currentTimeoutMask, const uint16_t maxConversionTime)
 *
 * this function checks or a given timeout flag
 *     - if not set
 *        - clears currentPins from
 *              global owiTemperatureTimeoutMask
 *        - it waits the remaining time:
 *              count x OWI_TEMPERATURE_CONVERSION_DELAY_MILLISECONDS
 *     - if set
 *        - currentPins are added to
 *              global owiTemperatureTimeoutMask
 *                  and
 *              currentTimeOutMask
 *        - error messages are issued
 *
 * return
 *      0: o.k.
 *      else: error
 */

uint8_t owiTemperatureConversionEvaluateTimeoutFlag(const unsigned char timeout_flag, const uint8_t currentPins, const uint32_t maxcount, uint32_t count, uint16_t *currentTimeoutMask, const uint16_t maxConversionTime)
{
    if ( FALSE == timeout_flag )
      {
         owiTemperatureTimeoutMask &= ~(currentPins);

         if ( 0 < maxcount )
         {
             printDebug_p(eventDebug, debugOWITemperatures, __LINE__, PSTR(__FILE__), PSTR("waited %i times a delay of"), maxcount - count); snprintf(uart_message_string, BUFFER_SIZE - 1, "%s %i ms", uart_message_string, OWI_TEMPERATURE_CONVERSION_DELAY_MILLISECONDS);
         }

         if ( count > 0 )
         {
            /*wait the remaining time*/
             printDebug_p(eventDebug, debugOWITemperatures, __LINE__, PSTR(__FILE__), PSTR("waiting the remaining %i times"), count); snprintf(uart_message_string, BUFFER_SIZE - 1, "%s %i ms", uart_message_string, OWI_TEMPERATURE_CONVERSION_DELAY_MILLISECONDS);

            while ( 0 < --count ) { _delay_ms(OWI_TEMPERATURE_CONVERSION_DELAY_MILLISECONDS); }
         }
          printDebug_p(eventDebug, debugOWITemperatures, __LINE__, PSTR(__FILE__), PSTR("conversion done"));

         return 0;
      }
      else
      {
         owiTemperatureTimeoutMask |= currentPins;
         *currentTimeoutMask |= currentPins;
         CommunicationError_p(ERRG, -1, 0, PSTR("OWI Temperature Conversion timeout"), 200);

         return 1;
      }
}

/*
 * owiTemperatureReadSingleSensor( unsigned char bus_pattern, unsigned char * id )
 *
 * this function will read the data contained in the scratchpad of the device
 * identified by the id ID number
 *
 * it will return the temperature value stored in the scratchpad as an 32bit unsigned int
 *   - where the temperature value is only maximum 16 lower bit
 *   - the remaining top 16 bit are used to signal errors
 *   - if an error occurs, this function will return values larger than 0xFFFF
 */


uint32_t owiTemperatureReadSingleSensor( unsigned char bus_pattern, unsigned char * id )
{
   uint32_t temperature = 0x0;

   /*checks*/
    printDebug_p(eventDebug, debugOWITemperatures, __LINE__, PSTR(__FILE__), PSTR("begin"));

   if ( 0 == ((owiBusMask & bus_pattern) & 0xFF) )
   {
       printDebug_p(eventDebug, debugOWITemperatures, __LINE__, PSTR(__FILE__), PSTR("passive (bus pattern 0x%x owiBusMask 0x%x)"), bus_pattern,owiBusMask);

      return (0x0L | owiReadStatus_owi_bus_mismatch) << 16;
   }

   if ( 0 != ((owiTemperatureTimeoutMask & bus_pattern) & 0xFF) )
   {
      /*conversion went into timeout*/
      snprintf_P(message, BUFFER_SIZE - 1, PSTR("OWI Temperature Conversion timeout (>%i ms, %s, bus_pattern 0x%x vs. mask: 0x%x)"),
                 OWI_TEMPERATURE_MAX_CONVERSION_TIME_MILLISECONDS, owi_id_string, bus_pattern, owiTemperatureTimeoutMask);
      CommunicationError_p(ERRG, -1, 0, message, -1001);

      clearString(message, BUFFER_SIZE);
      return (0x0L | owiReadStatus_conversion_timeout) << 16;
   }
#warning TODO: consider the case that bus_pattern has more than one bit active, but the conversion failed/succeeded not on all the same way

   /* Reset, presence */

   if ( 0 == OWI_DetectPresence(bus_pattern) )
   {
      return ( 0x0L | owiReadStatus_no_device_presence ) << 16;
   }

   /* Match id found earlier*/
   OWI_MatchRom(id, bus_pattern);

   /* Send READ SCRATCHPAD command.
    *
    * READ SCRATCHPAD [BEh]
    * This command allows the master to read the contents of the scratchpad. The data transfer starts with the
    * least significant bit of byte 0 and continues through the scratchpad until the 9th byte (byte 8 – CRC) is
    * read. The master may issue a reset to terminate reading at any time if only part of the scratchpad data is needed.
    *
    * (http://datasheets.maxim-ic.com/en/ds/DS18B20.pdf and http://datasheets.maxim-ic.com/en/ds/DS18S20.pdf)
    */

#warning no additional reset (~1ms) or detect presence (2ms) command for termination of reading is issued, why? maybe add global bus_mask to be reset?

   OWI_SendByte(DS1820_READ_SCRATCHPAD, bus_pattern);

   /* Read only two first bytes (temperature low, temperature high)*/
   /* and place them in the 16 bit temperature variable.*/
   temperature = OWI_ReceiveByte(bus_pattern);
   temperature |= ( OWI_ReceiveByte(bus_pattern) << 8 );
   temperature &= 0xFFFF;

   OWI_DetectPresence(bus_pattern);

    printDebug_p(eventDebug, debugOWITemperatures, __LINE__, PSTR(__FILE__), PSTR("end"));

   return temperature | ((0x0L | owiReadWriteStatus_OK) << 16);

}// END of owiTemperatureReadSingleSensor function



/*
 * find parasiticly powered devices
 */

void owiTemperatureFindParasiticlyPoweredDevices(unsigned char verbose)
{

   /* In some situations the bus master may not know whether the DS18B20s on the bus are parasite powered
    * or powered by external supplies. The master needs this information to determine if the strong bus pullup
    * should be used during temperature conversions. To get this information, the master can issue a Skip ROM
    * [CCh] command followed by a Read Power Supply [B4h] command followed by a “read time slot”.
    * During the read time slot, parasite powered DS18B20s will pull the bus low, and externally powered
    * DS18B20s will let the bus remain high. If the bus is pulled low, the master knows that it must supply the
    * strong pullup on the 1-Wire bus during temperature conversions
    */


   uint8_t *pins = BUSES;
   uint8_t busPatternIndexMax = PIN_BUS;

   for ( int8_t busPatternIndex = 0 ; busPatternIndex < busPatternIndexMax ; busPatternIndex++ )
   {
      uint8_t currentPins = pins[busPatternIndex];

      /*
       * detect presence
       */

      if ( 0 == OWI_DetectPresence(currentPins) )
      {
          printDebug_p(eventDebug, debugOWI, __LINE__, PSTR(__FILE__), PSTR("no Device present (pin pattern 0x%x)"), currentPins);
         continue;
      }
      else
      {
          printDebug_p(eventDebug, debugOWI, __LINE__, PSTR(__FILE__), PSTR("some devices present (pin pattern 0x%x)"), currentPins);
      }


      /*
       * send read power supply to all devices
       */

      OWI_SendByte(OWI_ROM_SKIP, currentPins);
      OWI_SendByte(DS1820_READ_POWER_SUPPLY, currentPins);

      /*
       * wait
       */

      uint32_t maxcount = 5; /* 5ms */
      uint32_t count = maxcount;
      uint8_t timeout_flag = FALSE;
      uint8_t owiReadBit = 0;
      static uint8_t delay = 1; /*ms*/

      while ( 0 == owiReadBit)
      {
          owiReadBit = OWI_ReadBit(currentPins);

          _delay_ms(delay);

            /* timeout check */
          if ( 0 == --count)
          {
               timeout_flag = TRUE;
               break;
          }
      }

      if ( TRUE == timeout_flag )
      {
         /* some devices are parasitic mode*/

         if ( FALSE != verbose)
         {
            snprintf_P(uart_message_string, BUFFER_SIZE - 1,
                       PSTR("RECV PARA parasitic devices SOME on pins 0x%x "),
                       currentPins);
            UART0_Send_Message_String_p(NULL,0);
         }
         /*set current pins in parasitic mode mask*/
         owiTemperatureParasiticModeMask |= (currentPins & 0xF);
          printDebug_p(eventDebug, debugOWITemperatures, __LINE__, PSTR(__FILE__), PSTR("parasitic devices SOME on pins 0x%x ") ,currentPins);
      }
      else
      {
         //owiTemperatureParasiticModeMask |= pins;
         if ( FALSE != verbose)
         {
            snprintf_P(uart_message_string, BUFFER_SIZE - 1,
                       PSTR("RECV PARA parasitic devices NONE on pins 0x%x (pulled HIGH within %i ms)"),
                       currentPins, (maxcount-count)*delay);
            UART0_Send_Message_String_p(NULL,0);
         }
         /*set current pins in parasitic mode mask*/
         owiTemperatureParasiticModeMask &= !(currentPins & 0xF);
          printDebug_p(eventDebug, debugOWITemperatures, __LINE__, PSTR(__FILE__), PSTR("parasitic devices NONE on pins 0x%x ") ,currentPins);
      }
   }
}
