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
/*
 * VERSION 1.0 Januar 7th 2010 LATE  File: 'adc.c'
 * Author: Linda Fouedjio  based  Giacomo Ortona
 * modified (heavily rather rebuild): Peter Zumbruch
 */
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/iocan128.h>

#include "api.h"
#include "api_define.h"
#include "api_global.h"

#include "api_debug.h"#include "can.h"
#include "led.h"
#include "mem-check.h"
#include "adc.h"
#include "jtag.h"

#include "one_wire_adc.h"
#include "one_wire_dualSwitch.h"
#include "one_wire_temperature.h"
#include "read_write_register.h"

#include "OWIDeviceSpecific.h"
#include "OWIHighLevelFunctions.h"
#include "OWIBitFunctions.h"

static const char filename[] 		PROGMEM = __FILE__;

#define ADC_CYCLES 3 /*the number of ADC conversions done to get an average voltage value*/
#define ADC_TIMEOUT 1000 /*wait ADC_TIMEOUT clock cycles for a single ADC conversion*/

void atmelReadADCs( struct uartStruct *ptr_uartStruct)
{
   switch(ptr_uartStruct->number_of_arguments)
   {
      case 0:
      {
         /* read all channels*/
         int8_t channelIndex = 0;
         int8_t channelIndexMax = 0;
         channelIndexMax = (disableJTAG_flag)? 8 : 4;
         for (channelIndex = 0; channelIndex < channelIndexMax; channelIndex++)
         {
            atmelCollectSingleADCChannel(channelIndex, FALSE);
         }
      }
      break;
      case 1:
      {
     	  printDebug_p(debugLevelEventDebug, debugSystemADC, __LINE__, filename, PSTR(""));


         /* read single channel */
         int8_t channelIndex = ptr_uartStruct->Uart_Message_ID;
         atmelCollectSingleADCChannel(channelIndex, FALSE);
      }
      break;
      default:
         generalErrorCode = CommunicationError_p(ERRG, dynamicMessage_ErrorIndex, TRUE, PSTR("invalid number of arguments"));
         break;
   }
   return;

}

/*this function takes the value of the voltage read from the ADC-channel
 * and convert it in the form of the API
 * as input parameters the function needs a keyword and the number of channel
 * the function has no return parameter
 * quiet: disables printout
 * channelIndex: index of adc channel [0..7]
 */

uint8_t atmelCollectSingleADCChannel( int8_t channelIndex, uint8_t quiet )
{
 	printDebug_p(debugLevelEventDebug, debugSystemADC, __LINE__, filename, PSTR("channel index %i"), channelIndex);

   for ( uint8_t i = 0 ; i <= 1 ; i++ )
   {
      _delay_ms(10);
   }

   int8_t status = eNoError;

   /*check channel index*/
   switch (channelIndex)
   {
      case 0:
      case 1:
      case 2:
      case 3:
         break;
      case 4:
      case 5:
      case 6:
      case 7:
         if ( FALSE == disableJTAG_flag)
         {
            generalErrorCode = CommunicationError_p(ERRG, GENERAL_ERROR_channel_undefined, TRUE, NULL);
            return eADCwrongAddress;
         }
         break;
      default:
         generalErrorCode = CommunicationError_p(ERRG, GENERAL_ERROR_channel_undefined, TRUE, NULL);
         return eADCwrongAddress;
         break;
   }

   /* get ADC value for channel: channelIndex*/
   status = atmelReadSingleADCChannelVoltage(channelIndex);

   /* set global status*/
   switch (channelIndex)
   {
      case 0:
         res0 = status;
         break;
      case 1:
         res1 = status;
         break;
      case 2:
         res2 = status;
         break;
      case 3:
         res3 = status;
         break;
      case 4:
         res4 = status;
         break;
      case 5:
         res5 = status;
         break;
      case 6:
         res6 = status;
         break;
      case 7:
         res7 = status;
         break;
   }

   /* check status return*/
   if ( status != eNoError )
   {
      CommunicationError_p(ERRG, dynamicMessage_ErrorIndex, TRUE, PSTR("error accessing ADC channel %i"), ptr_uartStruct->Uart_Message_ID);
      return status;
   }
    printDebug_p(debugLevelEventDebug, debugSystemADC, __LINE__, filename, PSTR("value %i"), atmelAdcValues[channelIndex]) ;

   if ( ! quiet )
   {
	   createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);

	   /*generate message*/
	   snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s%x %x"),
			   uart_message_string, channelIndex, atmelAdcValues[channelIndex]);

	   UART0_Send_Message_String_p(uart_message_string, BUFFER_SIZE - 1);
   }
   return status;
}//END of atmelCollectSingleADCChannel function

/* this function reads and converts the voltage of the ADC channels
 * as input parameters the function needs number of channel to read a voltage
 * the value returned is the voltage read from the channel
 */

int8_t atmelReadSingleADCChannelVoltage( unsigned int channel_nr )
{
	   /* generate header */

   uint16_t Timeout;
   uint16_t Value;
   uint16_t iStep;
   int16_t TotalValue;

   if (FALSE == disableJTAG_flag)
   {
      if ( channel_nr > 3 )
      {
         atmelAdcValues[channel_nr] = 0;
         return eADCwrongAddress;
      }
   }
   else /* FALSE != disableJTAG_flag */
   {
      if ( channel_nr > sizeof(atmelAdcValues)/sizeof(uint16_t))
      {
         return eADCwrongAddress;
      }
   }

   /*use 3.3V supply voltage as reference voltage
    * and select the voltage to be measured*/
   ADMUX = 0x40 | (unsigned char) channel_nr;
   /*ADMUX = 0x40 | 0x1B;*/

   TotalValue = 0;
   for ( iStep = 0; iStep < ADC_CYCLES ; iStep++ )
   {
      /* start a new single conversion*/
      ADCSRA |= BIT (ADSC);

      /* wait until conversion is complete*/
      Timeout = ADC_TIMEOUT;
      while ( !( ADCSRA & BIT (ADIF) ) && ( --Timeout > 0 ) )
         ;

      /* clear interrupt flag from former conversion*/
      ADCSRA |= BIT (ADIF);

      if ( Timeout == 0 )
      {
     	  printDebug_p(debugLevelEventDebug, debugSystemADC, __LINE__, filename, PSTR("Timeout"));
         return eADCTimeout;
      }

      /*one must read ADCL before ADCH!*/
      Value = ADCL;
      Value |= (unsigned int) ( ADCH & 0x03 ) << 8;
      TotalValue += Value;
   }

   TotalValue = TotalValue / ADC_CYCLES;

   /*save results*/
   atmelAdcValues[channel_nr] = (uint16_t) TotalValue;
    printDebug_p(debugLevelEventDebug, debugSystemADC, __LINE__, filename, PSTR("value = %i"), TotalValue) ;

   return eNoError;
}// END of atmelReadSingleADCChannelVoltage function
