/*
 * relay.c
 *
 *  Created on: Jul 16, 2010
 *      Author: zumbruch
 *  Last change: Mar 21, 2012
 */

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/iocanxx.h>

#include "api.h"
#include "api_define.h"
#include "api_global.h"
#include "adc.h"
#include "one_wire_temperature.h"
#include "jtag.h"

#include "relay.h"

static const char filename[] 		PROGMEM = __FILE__;

static const char relayThresholdCommandKeyword00[] PROGMEM = "current_state";
static const char relayThresholdCommandKeyword01[] PROGMEM = "current_values";
static const char relayThresholdCommandKeyword02[] PROGMEM = "thr_high";
static const char relayThresholdCommandKeyword03[] PROGMEM = "thr_low";
static const char relayThresholdCommandKeyword04[] PROGMEM = "enable";
static const char relayThresholdCommandKeyword05[] PROGMEM = "input_channel_mask";
static const char relayThresholdCommandKeyword06[] PROGMEM = "output_pin";
static const char relayThresholdCommandKeyword07[] PROGMEM = "led_pin";
static const char relayThresholdCommandKeyword08[] PROGMEM = "report_state_enable";
static const char relayThresholdCommandKeyword09[] PROGMEM = "relay";
static const char relayThresholdCommandKeyword10[] PROGMEM = "thresholds";
static const char relayThresholdCommandKeyword11[] PROGMEM = "status";
static const char relayThresholdCommandKeyword12[] PROGMEM = "outOfBounds_transit_lock";
static const char relayThresholdCommandKeyword13[] PROGMEM = "in_bounds_polarity";
static const char relayThresholdCommandKeyword14[] PROGMEM = "out_of_bounds_polarity";
static const char relayThresholdCommandKeyword15[] PROGMEM = "invert_polarity_logic";
static const char relayThresholdCommandKeyword16[] PROGMEM = "extern_inBounds_polarity";
static const char relayThresholdCommandKeyword17[] PROGMEM = "extern_inBounds_pin_pos";
static const char relayThresholdCommandKeyword18[] PROGMEM = "extern_inBounds_port";
static const char relayThresholdCommandKeyword19[] PROGMEM = "polarity";
static const char relayThresholdCommandKeyword20[] PROGMEM = "polarity_reinit";
static const char relayThresholdCommandKeyword21[] PROGMEM = "reinit";
static const char relayThresholdCommandKeyword22[] PROGMEM = "validate_thresholds";
static const char relayThresholdCommandKeyword23[] PROGMEM = "input_channel_add";
static const char relayThresholdCommandKeyword24[] PROGMEM = "input_channel_del";
static const char relayThresholdCommandKeyword25[] PROGMEM = "extern_thr_high_ch";
static const char relayThresholdCommandKeyword26[] PROGMEM = "extern_thr_low_ch";
static const char relayThresholdCommandKeyword27[] PROGMEM = "use_individual_thr";
static const char relayThresholdCommandKeyword28[] PROGMEM = "extern_thr_ch_mask";


const char* relayThresholdCommandKeywords[] PROGMEM = {
         relayThresholdCommandKeyword00,
         relayThresholdCommandKeyword01,
         relayThresholdCommandKeyword02,
         relayThresholdCommandKeyword03,
         relayThresholdCommandKeyword04,
         relayThresholdCommandKeyword05,
         relayThresholdCommandKeyword06,
         relayThresholdCommandKeyword07,
         relayThresholdCommandKeyword08,
         relayThresholdCommandKeyword09,
         relayThresholdCommandKeyword10,
         relayThresholdCommandKeyword11,
         relayThresholdCommandKeyword12,
         relayThresholdCommandKeyword13,
         relayThresholdCommandKeyword14,
         relayThresholdCommandKeyword15,
         relayThresholdCommandKeyword16,
         relayThresholdCommandKeyword17,
         relayThresholdCommandKeyword18,
         relayThresholdCommandKeyword19,
         relayThresholdCommandKeyword20,
         relayThresholdCommandKeyword21,
         relayThresholdCommandKeyword22,
         relayThresholdCommandKeyword23,
         relayThresholdCommandKeyword24,
         relayThresholdCommandKeyword25,
         relayThresholdCommandKeyword26,
         relayThresholdCommandKeyword27,
         relayThresholdCommandKeyword28
};

static const char relayThresholdStateString00[] PROGMEM = "INIT";
static const char relayThresholdStateString01[] PROGMEM = "IDLE";
static const char relayThresholdStateString02[] PROGMEM = "INBETWEEN";
static const char relayThresholdStateString03[] PROGMEM = "OFF_BOUNDS";
static const char relayThresholdStateString04[] PROGMEM = "UNDEFINED";

const char* relayThresholdStateStrings[] PROGMEM = {
         relayThresholdStateString00,
         relayThresholdStateString01,
         relayThresholdStateString02,
         relayThresholdStateString03,
         relayThresholdStateString04
};

uint16_t currentValues[8];

void relayThreshold(struct uartStruct *ptr_uartStruct)
{
   uint8_t index = 0;

   switch(ptr_uartStruct->number_of_arguments/* arguments of argument */)
   {
      case 0:
         for (index = 0; index < relayThresholdCommandKeyNumber_MAXIMUM_NUMBER; index ++)
         {
        	switch (index)
        	{
        	case relayThresholdCommandKeyNumber_POLARITY_REINIT:
        	case relayThresholdCommandKeyNumber_REINIT:
        	case relayThresholdCommandKeyNumber_VALIDATE_THRESHOLDS:
        	case relayThresholdCommandKeyNumber_INPUT_CHANNEL_MASK_ADD:
        	case relayThresholdCommandKeyNumber_INPUT_CHANNEL_MASK_DEL:
        	case relayThresholdCommandKeyNumber_THRESHOLDS:
        	case relayThresholdCommandKeyNumber_STATUS:
        	case relayThresholdCommandKeyNumber_POLARITY:
        		/* no action wanted */
        		break;
        	default:
        		/* show all */
        		ptr_uartStruct->number_of_arguments = 1;
        		relayThresholdMiscSubCommands(ptr_uartStruct, index);
        		ptr_uartStruct->number_of_arguments = 0;
            break;
        	}
         }
         break;
      default: /* else */
         relayThresholdMiscSubCommands(ptr_uartStruct, -1);
         break;
   }
}

void relayThresholdMiscSubCommands( struct uartStruct *ptr_uartStruct, int16_t subCommandIndex )
{
   uint32_t status = 0;
   uint64_t value = 0;
   uint64_t value2 = 0;
   uint8_t channel = 0;
   uint8_t recursive = FALSE;

   if ( 0 > subCommandIndex )
   {
      subCommandIndex = apiFindCommandKeywordIndex(setParameter[1], relayThresholdCommandKeywords, relayThresholdCommandKeyNumber_MAXIMUM_NUMBER);
   }

   /* TODO: relayThresholdMiscSubCommandsChooseFunction(ptr_uartStruct, index)*/
   switch ( ptr_uartStruct->number_of_arguments - 1 /* arguments of argument */)
   {
      case 0:
         /* printout status*/

         /* generate message */
         createExtendedSubCommandReceiveResponseHeader(ptr_uartStruct, commandKeyNumber_RLTH, subCommandIndex, relayThresholdCommandKeywords);
         switch ( subCommandIndex )
         {
            case relayThresholdCommandKeyNumber_CURRENT_VALUES:
                for ( channel=0; channel<8; channel++)
                {
             	   if (! (relayThresholdInputChannelMask & (0x1 << channel)))
             	   {
             		   continue;
             	   }

             	   value = relayThresholdsGetCurrentValue(channel);
             	   if ( UINT16_MAX < value )
             	   {
             		   CommunicationError_p(ERRG, dynamicMessage_ErrorIndex, TRUE, PSTR("channel %i failed"), channel);
             		   return;
             	   }
             	   snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%spin%i:%#x "), uart_message_string, channel, value);
                }
               break;
            case relayThresholdCommandKeyNumber_CURRENT_STATE:
               value = relayThresholdsGetCurrentState();
               snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s%i %s"), uart_message_string, value);
               strncat_P(uart_message_string, (const char*) ( pgm_read_word( &(relayThresholdStateStrings[value])) ), BUFFER_SIZE - 1);
               break;
            case relayThresholdCommandKeyNumber_THR_HIGH:
               for ( channel=0; channel<8; channel++)
               {
            	   if (0 < channel && ! relayThresholdUseIndividualThresholds)
            	   {
            		   break;
            	   }
            	   value = relayThresholdsGetHighThreshold(channel);
             	   if ( UINT16_MAX < value )
             	   {
             		   CommunicationError_p(ERRG, dynamicMessage_ErrorIndex, TRUE, PSTR("channel %i failed"), channel);
             		   return;
             	   }
            	   snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s%i:%#x "), uart_message_string, channel, value);
            	   strncat_P(uart_message_string, ( relayThresholds_valid[channel * relayThreshold_MAXIMUM_INDEX+relayThreshold_HIGH] ) ? PSTR("+ ") : PSTR("- "), BUFFER_SIZE - 1);
               }
               break;
            case relayThresholdCommandKeyNumber_THR_LOW:
                for ( channel=0; channel<8; channel++)
                {
             	   if (0 < channel && ! relayThresholdUseIndividualThresholds)
             	   {
             		   break;
             	   }
             	   value = relayThresholdsGetLowThreshold(channel);
             	   if ( UINT16_MAX < value )
             	   {
             		   CommunicationError_p(ERRG, dynamicMessage_ErrorIndex, TRUE, PSTR("channel %i failed"), channel);
             		   return;
             	   }
             	   snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s%i:%#x "), uart_message_string, channel, value);
             	   strncat_P(uart_message_string, ( relayThresholds_valid[channel * relayThreshold_MAXIMUM_INDEX+ relayThreshold_LOW] ) ? PSTR("+ ") : PSTR("- "), BUFFER_SIZE - 1);
                }
               break;
            case relayThresholdCommandKeyNumber_ENABLE:
               value = relayThresholdsGetEnableFlag();
               strncat_P(uart_message_string, ( FALSE != value ) ? PSTR("TRUE") : PSTR("FALSE"), BUFFER_SIZE - 1);
               break;
            case relayThresholdCommandKeyNumber_REPORT_STATE_ENABLE:
               value = relayThresholdsGetReportStateEnableFlag();
               strncat_P(uart_message_string, ( FALSE != value ) ? PSTR("TRUE") : PSTR("FALSE"), BUFFER_SIZE - 1);
               break;
            case relayThresholdCommandKeyNumber_INPUT_CHANNEL_MASK:
               value = relayThresholdsGetInputChannelMask();
               snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s%x"), uart_message_string, value);
               break;
            case relayThresholdCommandKeyNumber_OUTPUT_PIN:
               value = relayThresholdsGetOutputPin();
               snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s%x"), uart_message_string, value);
               break;
            case relayThresholdCommandKeyNumber_LED_PIN:
               value = relayThresholdsGetLedPin();
               snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s%x"), uart_message_string, value);
               break;
            case relayThresholdCommandKeyNumber_RELAY_VALUE:
               value = relayThresholdsGetRelayValue();
               snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s%x"), uart_message_string, value);
               break;
            case relayThresholdCommandKeyNumber_OUT_OF_BOUNDS_LOCK:
               value = relayThresholdsGetOutOfBoundsLockFlag();
               strncat_P(uart_message_string, ( FALSE != value ) ? PSTR("TRUE") : PSTR("FALSE"), BUFFER_SIZE - 1);
               break;
            case relayThresholdCommandKeyNumber_IN_BOUNDS_POLARITY:
               value = relayThresholdsGetInBoundsPolarity();
               strncat_P(uart_message_string, ( FALSE != value ) ? PSTR("TRUE") : PSTR("FALSE"), BUFFER_SIZE - 1);
               break;
            case relayThresholdCommandKeyNumber_OUT_OF_BOUNDS_POLARITY:
               value = relayThresholdsGetOutOfBoundsPolarity();
               strncat_P(uart_message_string, ( FALSE != value ) ? PSTR("TRUE") : PSTR("FALSE"), BUFFER_SIZE - 1);
               break;
            case relayThresholdCommandKeyNumber_INVERT_POLARITY_LOGIC:
               value = relayThresholdsGetInvertPolarityFlag();
               strncat_P(uart_message_string, ( FALSE != value ) ? PSTR("TRUE") : PSTR("FALSE"), BUFFER_SIZE - 1);
               break;
            case relayThresholdCommandKeyNumber_EXTERNAL_IN_BOUNDS_POLARITY:
               value = relayThresholdsGetExternalPolarity();
               strncat_P(uart_message_string, ( FALSE != value ) ? PSTR("TRUE") : PSTR("FALSE"), BUFFER_SIZE - 1);
               break;
            case relayThresholdCommandKeyNumber_EXTERNAL_IN_BOUNDS_PIN_POS:
               value = relayThresholdsGetExternalPolarityPinPos();
               snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s%x"), uart_message_string, value);
               break;
            case relayThresholdCommandKeyNumber_EXTERNAL_IN_BOUNDS_PORT:
               value = relayThresholdsGetExternalPolarityPort();
               snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s%x"), uart_message_string, value);
               break;
            case relayThresholdCommandKeyNumber_STATUS:
            	recursive = TRUE;

            	relayThresholdMiscSubCommands(ptr_uartStruct, relayThresholdCommandKeyNumber_ENABLE);
            	relayThresholdMiscSubCommands(ptr_uartStruct, relayThresholdCommandKeyNumber_CURRENT_STATE);
            	relayThresholdMiscSubCommands(ptr_uartStruct, relayThresholdCommandKeyNumber_CURRENT_VALUES);
            	relayThresholdMiscSubCommands(ptr_uartStruct, relayThresholdCommandKeyNumber_THRESHOLDS);
            	relayThresholdMiscSubCommands(ptr_uartStruct, relayThresholdCommandKeyNumber_RELAY_VALUE);
            	relayThresholdMiscSubCommands(ptr_uartStruct, relayThresholdCommandKeyNumber_INPUT_CHANNEL_MASK);
            	relayThresholdMiscSubCommands(ptr_uartStruct, relayThresholdCommandKeyNumber_OUTPUT_PIN);
            	relayThresholdMiscSubCommands(ptr_uartStruct, relayThresholdCommandKeyNumber_LED_PIN);
            	relayThresholdMiscSubCommands(ptr_uartStruct, relayThresholdCommandKeyNumber_OUT_OF_BOUNDS_LOCK);

            	break;
            case relayThresholdCommandKeyNumber_POLARITY:
            	recursive = TRUE;

            	relayThresholdMiscSubCommands(ptr_uartStruct, relayThresholdCommandKeyNumber_IN_BOUNDS_POLARITY);
            	relayThresholdMiscSubCommands(ptr_uartStruct, relayThresholdCommandKeyNumber_OUT_OF_BOUNDS_POLARITY);
            	relayThresholdMiscSubCommands(ptr_uartStruct, relayThresholdCommandKeyNumber_INVERT_POLARITY_LOGIC);
            	relayThresholdMiscSubCommands(ptr_uartStruct, relayThresholdCommandKeyNumber_EXTERNAL_IN_BOUNDS_POLARITY);
            	relayThresholdMiscSubCommands(ptr_uartStruct, relayThresholdCommandKeyNumber_EXTERNAL_IN_BOUNDS_PIN_POS);
            	relayThresholdMiscSubCommands(ptr_uartStruct, relayThresholdCommandKeyNumber_EXTERNAL_IN_BOUNDS_PORT);

            	break;
            case relayThresholdCommandKeyNumber_POLARITY_REINIT:
               relayThresholdInBoundPolarityInit();
               break;
            case relayThresholdCommandKeyNumber_REINIT:
               relayThresholdInit();
               break;
            case relayThresholdCommandKeyNumber_VALIDATE_THRESHOLDS:
               relayThresholdsSetAllRelevantThresholdsValid(TRUE);
               break;
            case relayThresholdCommandKeyNumber_THRESHOLDS:
            	recursive = TRUE;

            	relayThresholdMiscSubCommands(ptr_uartStruct, relayThresholdCommandKeyNumber_THR_LOW);
            	relayThresholdMiscSubCommands(ptr_uartStruct, relayThresholdCommandKeyNumber_THR_HIGH);

            	break;
            case relayThresholdCommandKeyNumber_EXTERN_THR_HIGH_CHANNEL:
                value = relayThresholdGetExternHighChannel();
                snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s%#x"), uart_message_string, value);
                break;
            case relayThresholdCommandKeyNumber_EXTERN_THR_LOW_CHANNEL:
                value = relayThresholdGetExternLowChannel();
                snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s%#x"), uart_message_string, value);
                break;
            case relayThresholdCommandKeyNumber_USE_INDIVIDUAL_THRESHOLDS:
                value = relayThresholdGetUseIndividualThresholds();
                strncat_P(uart_message_string, ( FALSE != value ) ? PSTR("TRUE") : PSTR("FALSE"), BUFFER_SIZE - 1);
                break;
            case relayThresholdCommandKeyNumber_EXTERN_THRESHOLD_CHANNEL_MASK:
                value = relayThresholdsGetExtThresholdsMask();
                snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s%#x"), uart_message_string, value);
                break;
            default:
               clearString(uart_message_string, BUFFER_SIZE);
               CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, TRUE, PSTR("invalid command argument"));
               return;
               break;
         }

         /*send the data*/
         if ( ! recursive ) { UART0_Send_Message_String(uart_message_string, BUFFER_SIZE - 1);}
         /*clear strings*/
         clearString(uart_message_string, BUFFER_SIZE);
         break;
         case 1:

         /* set status*/
    	 getUnsignedNumericValueFromParameterIndex(2, &value);
         switch ( subCommandIndex )
         {
            case relayThresholdCommandKeyNumber_THR_HIGH:
               status = relayThresholdsSetHighThreshold(value & 0xFFFF, -1);
               break;
            case relayThresholdCommandKeyNumber_THR_LOW:
               status = relayThresholdsSetLowThreshold(value & 0xFFFF, -1);
               break;
            case relayThresholdCommandKeyNumber_ENABLE:
               status = relayThresholdsSetEnableFlag(FALSE != value);
               break;
            case relayThresholdCommandKeyNumber_OUT_OF_BOUNDS_LOCK:
               status = relayThresholdsSetOutOfBoundsLockFlag(FALSE != value);
               break;
            case relayThresholdCommandKeyNumber_INPUT_CHANNEL_MASK:
               status = relayThresholdsSetInputChannelMask(value & 0xFF);
               break;
            case relayThresholdCommandKeyNumber_INPUT_CHANNEL_MASK_ADD:
               status = relayThresholdsAddChannelToInputChannelMask(value & 0x7);
               break;
            case relayThresholdCommandKeyNumber_INPUT_CHANNEL_MASK_DEL:
               status = relayThresholdsDeleteChannelFromInputChannelMask(value & 0x7);
               break;
            case relayThresholdCommandKeyNumber_OUTPUT_PIN:
               status = relayThresholdsSetOutputPin(value & 0x7);
               break;
            case relayThresholdCommandKeyNumber_LED_PIN:
               status = relayThresholdsSetLedPin(value & 0x7);
               break;
            case relayThresholdCommandKeyNumber_REPORT_STATE_ENABLE:
               status = relayThresholdsSetReportCurrentStatusEnableFlag(value & 0xF);
               break;
            case relayThresholdCommandKeyNumber_RELAY_VALUE:
               status = relayThresholdsSetRelayValue(value);
               break;
            case relayThresholdCommandKeyNumber_INVERT_POLARITY_LOGIC:
               status = relayThresholdsSetInvertPolarityFlag(FALSE != value);
               break;
            case relayThresholdCommandKeyNumber_EXTERNAL_IN_BOUNDS_PIN_POS:
               status = relayThresholdsSetExternalPolarityPinPos(value);
               break;
            case relayThresholdCommandKeyNumber_EXTERNAL_IN_BOUNDS_PORT:
               status = relayThresholdsSetExternalPolarityPinPos(value);
               break;
            case relayThresholdCommandKeyNumber_VALIDATE_THRESHOLDS:
               status = relayThresholdsSetAllRelevantThresholdsValid(FALSE != value);
               break;
            case relayThresholdCommandKeyNumber_EXTERN_THR_HIGH_CHANNEL:
                status = relayThresholdSetExternHighChannel(value);
                break;
            case relayThresholdCommandKeyNumber_EXTERN_THR_LOW_CHANNEL:
                status = relayThresholdSetExternLowChannel(value);
                break;
            case relayThresholdCommandKeyNumber_USE_INDIVIDUAL_THRESHOLDS:
                status = relayThresholdSetUseIndividualThresholds(FALSE != value);
                break;
            default:
               CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, TRUE, PSTR("invalid command argument"));
               return;
               break;
         }

         /*recursive call to show change*/
         if ( 0 == status )
         {
        	switch ( subCommandIndex )
        	{
            case relayThresholdCommandKeyNumber_VALIDATE_THRESHOLDS:
        		ptr_uartStruct->number_of_arguments = 1;
        		relayThresholdMiscSubCommands(ptr_uartStruct, relayThresholdCommandKeyNumber_THRESHOLDS);
        		ptr_uartStruct->number_of_arguments = 2;
               break;
        	default:
        		ptr_uartStruct->number_of_arguments = 1;
        		relayThresholdMiscSubCommands(ptr_uartStruct, subCommandIndex);
        		ptr_uartStruct->number_of_arguments = 2;
        	}

            switch ( subCommandIndex )
            {
               case relayThresholdCommandKeyNumber_ENABLE:
                  if ( value )
                  {
                     status = relayThresholdInit();
                  }
                  break;
               default:
                  break;
            }
         }
         break;
         case 2:
            /* set status*/

        	if (-1 == getUnsignedNumericValueFromParameterIndex(2, &value )) {return;}
        	if (-1 == getUnsignedNumericValueFromParameterIndex(3, &value2)) {return;}

            switch ( subCommandIndex )
            {
               case relayThresholdCommandKeyNumber_THR_HIGH:
                  status = relayThresholdsSetHighThreshold(value & 0xFFFF, value2);
                  break;
               case relayThresholdCommandKeyNumber_THR_LOW:
                  status = relayThresholdsSetLowThreshold(value & 0xFFFF, value2);
                  break;
               default:
                  CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, TRUE, PSTR("invalid command argument"));
                  return;
                  break;
            }

            /*recursive call to show change*/
            if ( 0 == status )
            {
            	switch ( subCommandIndex )
            	{
            	default:
            		ptr_uartStruct->number_of_arguments = 1;
            		relayThresholdMiscSubCommands(ptr_uartStruct, subCommandIndex);
            		ptr_uartStruct->number_of_arguments = 2;
            	}
            }
            break;
         default:
         generalErrorCode = CommunicationError_p(ERRG, dynamicMessage_ErrorIndex, TRUE, PSTR("invalid number of arguments"));
         break;
   }

   return;
}

void relayThresholdsReportCurrentStatus(void)
{
   int nargs = ptr_uartStruct->number_of_arguments;
   ptr_uartStruct->number_of_arguments = 1;
   relayThresholdMiscSubCommands(ptr_uartStruct, relayThresholdCommandKeyNumber_STATUS);
   ptr_uartStruct->number_of_arguments = nargs;
}

uint8_t relayThresholdInBoundPolarityInit(void)
{
   /* PORT F, pin "relayThresholdsExternalPolarityPinPos" determines whether relay is TRUE/FALSE when in bounds*/
   /* which is per default set to HIGH/TRUE/1 */
   switch(relayThresholdsExternalPolarityPort)
   {
      case 0xA:
         relayThresholdsExternalPolarity = (PORTA >> (relayThresholdsExternalPolarityPinPos -1) ) & 0x1;
         break;
      case 0xB:
         relayThresholdsExternalPolarity = (PORTB >> (relayThresholdsExternalPolarityPinPos -1) ) & 0x1;
         break;
      case 0xC:
         relayThresholdsExternalPolarity = (PORTC >> (relayThresholdsExternalPolarityPinPos -1) ) & 0x1;
         break;
      case 0xD:
         relayThresholdsExternalPolarity = (PORTD >> (relayThresholdsExternalPolarityPinPos -1) ) & 0x1;
         break;
      case 0xE:
         relayThresholdsExternalPolarity = (PORTE >> (relayThresholdsExternalPolarityPinPos -1) ) & 0x1;
         break;
      case 0xF:
         relayThresholdsExternalPolarity = (PORTF >> (relayThresholdsExternalPolarityPinPos -1) ) & 0x1;
         break;
      default:
         generalErrorCode = CommunicationError_p(ERRG, dynamicMessage_ErrorIndex, TRUE, PSTR("'RLTH invalid input port for external polarity"));
         break;
         return 0xFF;
   }
   relayThresholdsInBoundsPolarity = ( 0x1 == relayThresholdsExternalPolarity );
   relayThresholdsOutOfBoundsPolarity = ( 0x1 != relayThresholdsExternalPolarity );
   if (FALSE != relayThresholdInvertPolarity_flag)
   {
      relayThresholdsInBoundsPolarity    = 0x1 & (~relayThresholdsInBoundsPolarity);
      relayThresholdsOutOfBoundsPolarity = 0x1 & (~relayThresholdsOutOfBoundsPolarity);
   }
   return relayThresholdsExternalPolarity;
}

uint8_t relayThresholdInit(void)
{
	uint16_t status = FALSE;

    printDebug_p(debugLevelEventDebug, debugSystemRELAY, __LINE__, filename, PSTR("Init"));

   /* check prerequisites */
   status = relayThresholdsCheckAllRelevantThresholdsValid();
   if ( FALSE == status )
   {
      return FALSE;
   }

   /* init polarity*/
   status = relayThresholdInBoundPolarityInit(); /* return value is the polarity of the output pin when in bounds HIGH (0x1) or LOW (0x0)  or 0xFF (error)*/
   if ( 0x1 < status )
   {
	   return FALSE;
   }

   /* checks passed */
   relayThresholdCurrentState = relayThresholdState_INIT;
   relayThresholdDetermineStateAndTriggerRelay(relayThresholdInputSource_MAXIMUM_INDEX);

   return TRUE;
}

void relayThresholdDetermineStateAndTriggerRelay(uint8_t inputSource)
{
   uint16_t high = UINT16_MAX;
   uint16_t low =  0x0;
   uint8_t setValue =  0xFF;
   uint8_t newStates[8];
   uint8_t status = 0;
   uint8_t channelIndex;
   uint8_t channelIndexMax = 0;

   if ( relayThresholdInputSource_MAXIMUM_INDEX < inputSource )
   {
	   CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, TRUE, PSTR("fcn:%s: wrong input source:%i exceeds max. of %i\n"), __func__, relayThresholdInputSource_MAXIMUM_INDEX );
	   return;
   }

   if (relayThresholdInputSource_MAXIMUM_INDEX == inputSource)
   {
	   for (uint8_t source=0; source < relayThresholdInputSource_MAXIMUM_INDEX; source++)
	   {
		   /*recursive call for all available sources*/
		   relayThresholdDetermineStateAndTriggerRelay(source);
	   }

   }
   channelIndexMax = (disableJTAG_flag)? 8 : 4;
   // read in all available and needed channels

   for (channelIndex=0; channelIndex < 8; channelIndex++)
   {
	   // clear all channels
	   if ( channelIndex >= channelIndexMax ) { continue; }
	   if (0 == ((relayThresholdInputChannelMask | relayThresholdExtThresholdsMask) & (0x1 << channelIndex))) { continue; }

	   // updates value global array atmelAdcValues
	   switch(inputSource)
	   {
	   case relayThresholdInputSource_RADC:
           status = atmelCollectSingleADCChannel(channelIndex, TRUE);
           if ( eNoError != status )
           {
        	   for (channelIndex=0; channelIndex < 8; channelIndex++)
        		   currentValues[channelIndex] = atmelAdcValues[channelIndex];
        	   return;
           }

		   break;
	   }

   }
   /* for loop over mask*/

   /* determine new state */
   relayThresholdNewState = relayThresholdCurrentState;

   /* --- channel wise new states */
   for (channelIndex=0; channelIndex < 8; channelIndex++)
   {
	   newStates[channelIndex] = relayThresholdState_UNDEFINED;
	   if ( channelIndex >= channelIndexMax ) { continue; }
	   if (0 == (relayThresholdInputChannelMask & (0x1 << channelIndex))) { continue; }

	   /* get current thresholds */
	   relayThresholdGetCurrentChannelThreshold(channelIndex, &low, &high);

	   /* check order */
	   if ( high <= low )
	   {
		   newStates[channelIndex] = relayThresholdState_UNDEFINED;

		   CommunicationError_p(ERRG, dynamicMessage_ErrorIndex, FALSE, PSTR("%s (%4i, %s): %s: thresholds high [%#x] <= low [%#x] ... disabling"), __func__, __LINE__, __FILE__);
		   relayThresholdsSetEnableFlag(FALSE);
		   return;
	   }
	   else
	   {
		   newStates[channelIndex] = relayThresholdDetermineState( relayThresholdCurrentState, currentValues[channelIndex], high, low );
	   }
	   // if there is at least one undefined, quit at set state to "undefined"
	   if ( relayThresholdState_UNDEFINED == newStates[channelIndex])
	   {
		   relayThresholdNewState = relayThresholdState_UNDEFINED;
		   /* disable relay */
		   relayThresholdsSetEnableFlag(FALSE);
		   CommunicationError_p(ERRG, dynamicMessage_ErrorIndex, FALSE, PSTR("%s (%4i, %s): %s: undefined state ... disabling"), __func__, __LINE__, __FILE__);
		   return;
	   }
   }

   /* --- or/and of all input channel new states */
   for (channelIndex=0; channelIndex < 8; channelIndex++)
   {
	   if ( channelIndex >= channelIndexMax ) { continue; }
	   if (0 == (relayThresholdInputChannelMask & (0x1 << channelIndex))) { continue; }

       // if there is at least one out bounds, state is "out of bounds"
	   if ( relayThresholdState_OUT_OF_BOUNDS == newStates[channelIndex])
	   {
		   relayThresholdNewState = relayThresholdState_OUT_OF_BOUNDS;
		   break;
	   }

	   relayThresholdNewState = relayThresholdState_INBETWEEN_BOUNDS;
   }

   /* determine set value and set it*/
   if ( relayThresholdCurrentState != relayThresholdNewState )
   {
	   relayThresholdCurrentState = relayThresholdNewState;

	   setValue = 0xFF;
	   setValue = relayThresholdDetermineNewRelaySetValue( relayThresholdNewState );
	   if ( 0x1 >= setValue )
	   {
		   relayThresholdRelaySetValue = setValue;
	   }
	   else if ( relayThresholdsUndefinedIndifferentPolarity == setValue )
	   {
           return;
	   }
	   else
	   {
		   /*error*/
		   return;
	   }

	   /* physically set relay value */
	   if (relayThresholdEnable_flag)
	   {
		   if ( relayThresholdRelaySetValue != relayThresholdRelayValue )
		   {
			   relayThresholdsSetRelayValue(relayThresholdRelaySetValue); /*turns on/off relay*/
		   }
		   /* once out of bounds it stays there and is disabled */
		   if (relayThresholdOutOfBoundsStateLock_flag
				   && /* out of bounds */
			   (relayThresholdState_OUT_OF_BOUNDS == relayThresholdCurrentState)
		      )
		   {
			   relayThresholdsSetEnableFlag(FALSE);
		   }
	   }

	   if (relayThresholdReportCurrentStatusEnable_flag)
	   {
		   PORTA = PORTA | ( ( 0x1 << relayThresholdLedPin ) ); // turn on lamp

		   /* report current state and value */
		   relayThresholdsReportCurrentStatus();
	   }
   }
   return;
}

uint8_t relayThresholdGetCurrentChannelThreshold(uint8_t channel, uint16_t *low, uint16_t *high)
{
	uint8_t status=0;
	if ( 7 < channel)
	{
		CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, TRUE, PSTR("invalid channel argument"));
		status = 1;
	}
	if (NULL == low)
	{
		CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, TRUE, PSTR("low NULL pointer"));
		status = 1;
	}
	if (NULL == high)
	{
		CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, TRUE, PSTR("high NULL pointer"));
		status = 1;
	}
	if (0 != status)
	{
		return status;
	}
	// high
	if (0 > relayThresholdExternHighChannel)
	{
		/* use intern settings */
		if ( relayThresholdUseIndividualThresholds )
		{
			/* or use individual thresholds */
			*high = relayThresholdHigh[channel];
		}
		else
		{
			/* use global thresholds */
			*high = relayThresholdHigh[0];
		}
	}
	else
	{
		/* or use extern settings */
		*high = currentValues[relayThresholdExternHighChannel];
		relayThresholdHigh[0] = *high;
	}

	// low
	if (0 > relayThresholdExternLowChannel)
	{
		/* use intern settings */
		if (0 > relayThresholdExternHighChannel)
		{
			/* use intern settings */
			if ( relayThresholdUseIndividualThresholds )
			{
				*low  = relayThresholdLow[channel];
			}
			else
			{
				/* use global thresholds */
				*low  = relayThresholdLow[0];
			}
		}
	}
	else
	{
		/* or use extern settings */
		*low  = currentValues[relayThresholdExternLowChannel];
		relayThresholdLow[0] = *low;
	}
	return status;
}

uint8_t relayThresholdDetermineState( uint8_t currentState, uint32_t currentValue, uint32_t high, uint32_t low )
{
	uint8_t newState = currentState;
	switch ( currentState )
	{
		case relayThresholdState_IDLE:
			break;
		case relayThresholdState_OUT_OF_BOUNDS:
		case relayThresholdState_INIT:
		case relayThresholdState_INBETWEEN_BOUNDS:
		{
			if ( currentValue >= high )
			{
				/*turn off relays*/
				newState = relayThresholdState_OUT_OF_BOUNDS;
			}
			else if ( currentValue <= low )
			{
				/*turn off relays*/
				newState = relayThresholdState_OUT_OF_BOUNDS;
			}
			else if ( currentValue > low && currentValue < high)
			{
				/*turn on relays*/
				newState = relayThresholdState_INBETWEEN_BOUNDS;
			}
			else
			{ /*anything else is undefined*/
				newState = relayThresholdState_UNDEFINED;
			}
		}
		break;
		default:
		{
			newState = relayThresholdState_UNDEFINED;
		}
		break;
	}
	return newState;
}

uint8_t relayThresholdDetermineNewRelaySetValue( uint8_t newState )
{
	uint8_t setValue = 0xFF;
	switch ( newState )
	{
	case relayThresholdState_OUT_OF_BOUNDS:
		_delay_ms(100);
		_delay_ms(100);
		PINA = ( 0x1 << relayThresholdLedPin ); // toggle LED
		setValue = relayThresholdsOutOfBoundsPolarity;
		break;
	case relayThresholdState_INBETWEEN_BOUNDS:
		setValue = relayThresholdsInBoundsPolarity;
		break;
	case relayThresholdState_IDLE:
	case relayThresholdState_INIT:
	case relayThresholdState_UNDEFINED:
		setValue = relayThresholdsUndefinedIndifferentPolarity;
		break;
	default:
		CommunicationError_p(ERRG, dynamicMessage_ErrorIndex, FALSE, PSTR("%s (%4i, %s): unknown state"), __func__, __LINE__, __FILE__);
		break;
	}
	return setValue;
}

uint8_t relayThresholdsSetChannelThreshold(uint16_t value, int8_t channel, volatile uint16_t threshold[], uint8_t offset)
{
	uint8_t status = 0;
	uint8_t index = 0;
	if ( 0 > channel )
	{
		for (index = 0; index < 8; index++)
		{
			threshold[index] = value;
			relayThresholds_valid[index * relayThreshold_MAXIMUM_INDEX + offset] = TRUE;
		}
	}
	else if ( channel <= 7 )
	{
		threshold[channel] = value;
		relayThresholds_valid[channel * relayThreshold_MAXIMUM_INDEX + offset] = TRUE;
		relayThresholdsCheckAllRelevantThresholdsValid();
	}
	else
	{
		CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, TRUE, PSTR("invalid channel argument"));
		status = 1;
	}
	return status;
}

uint8_t relayThresholdsSetHighThreshold(uint16_t value, int8_t channel)
{
#if 1
  return relayThresholdsSetChannelThreshold(value, channel, relayThresholdHigh, relayThreshold_HIGH);
#else
	uint8_t status = 0;
	uint8_t index = 0;
	if ( 0 > channel)
	{
		for (index = 0; index < 8; index++)
		{
			relayThresholdHigh[index] = value;
			relayThresholds_valid[index * relayThreshold_MAXIMUM_INDEX
			                      + relayThreshold_HIGH] = TRUE;
		}
	}
	else if ( channel <= 7 )
	{
		relayThresholds_valid[channel * relayThreshold_MAXIMUM_INDEX
		                      + relayThreshold_HIGH] = TRUE;
		relayThresholdsCheckAllRelevantThresholdsValid();
	}
	else
	{
		CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, TRUE, PSTR("invalid command argument"));
		status = 1;
	}
	return status;
#endif
}

uint8_t relayThresholdsSetLowThreshold(uint16_t value, int8_t channel)
{
#if 1
  return relayThresholdsSetChannelThreshold(value, channel, relayThresholdLow, relayThreshold_LOW);
#else
   uint8_t status = 0;
   uint8_t index = 0;
   if ( 0 > channel)
   {
	   for (index = 0; index < 8; index++)
	   {
		   relayThresholdLow[index] = value;
		   relayThresholds_valid[index * relayThreshold_MAXIMUM_INDEX
		                         + relayThreshold_LOW] = TRUE;
	   }
   }
   else if ( channel <= 7 )
   {
	   relayThresholds_valid[index * relayThreshold_MAXIMUM_INDEX
	                         + relayThreshold_LOW] = TRUE;
	   relayThresholdsCheckAllRelevantThresholdsValid();
   }
   else
   {
	   CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, TRUE, PSTR("invalid command argument"));
	   status = 1;
   }
   return status;
#endif
}

uint8_t relayThresholdsSetEnableFlag(uint8_t flag)
{
   uint8_t status = 0;

   relayThresholdEnable_flag = ( FALSE != flag);

   if ( FALSE == relayThresholdEnable_flag)
   {
      relayThresholdCurrentState = relayThresholdState_IDLE;
      PORTA = PORTA & ( ~( 0x1 << relayThresholdLedPin ) ); // turn off lamp
   }
   else
   {
	  status = relayThresholdInit();
	  if ( FALSE == status) /* init failed */
	  {
		  relayThresholdsSetEnableFlag(FALSE);
		  status = 1;
	  }
	  else
	  {
		  PORTA = PORTA | ( ( 0x1 << relayThresholdLedPin ) ); // turn on lamp
		  status = 0;
	  }
   }

   return status;
}

uint8_t relayThresholdsSetReportCurrentStatusEnableFlag(uint8_t flag)
{
   uint8_t status = 0;

   relayThresholdReportCurrentStatusEnable_flag = ( 0 != flag);

   if ( FALSE != relayThresholdReportCurrentStatusEnable_flag)
   {
      relayThresholdsReportCurrentStatus();
   }

   return status;
}

uint8_t relayThresholdsSetOutOfBoundsLockFlag(uint8_t flag)
{
   uint8_t status = 0;

   relayThresholdOutOfBoundsLock_flag = ( 0 != flag);

   if ( FALSE != relayThresholdReportCurrentStatusEnable_flag)
   {
      relayThresholdsReportCurrentStatus();
   }

   return status;
}

uint8_t relayThresholdsSetInvertPolarityFlag(uint8_t flag)
{
   uint8_t status = 0;

   relayThresholdInvertPolarity_flag = ( 0 != flag);

   /* reinit*/
   relayThresholdInBoundPolarityInit();

   if ( FALSE != relayThresholdReportCurrentStatusEnable_flag)
   {
      relayThresholdsReportCurrentStatus();
   }

   return status;
}

uint8_t relayThresholdsSetRelayValue(uint8_t value)
{
   uint8_t status = 0;
   uint8_t setValue = 0;
   setValue = ( 0 != value);
   if (relayThresholdRelayValue != setValue)
   {
      if (TRUE == setValue)
      {
         PORTA |= ( 0x1 << relayThresholdOutputPin ) ;
      }
      else
      {
         PORTA &= ( ~( 0x1 << relayThresholdOutputPin) );
      }
      relayThresholdRelayValue = setValue;
      if (FALSE != relayThresholdReportCurrentStatusEnable_flag)
      {
         relayThresholdsReportCurrentStatus();
      }
   }

   return status;
}

uint8_t relayThresholdsSetRelaySetValue(uint8_t value)
{
   uint8_t status = 0;
   uint8_t setValue = 0;
   setValue = ( 0 != value);
   if (relayThresholdRelaySetValue != setValue)
   {
      relayThresholdRelaySetValue = setValue;
      if (FALSE != relayThresholdReportCurrentStatusEnable_flag)
      {
         relayThresholdsReportCurrentStatus();
      }
   }

   return status;
}

uint8_t relayThresholdsSetInputChannelMask(uint8_t value)
{
   uint8_t status = 0;

   relayThresholdInputChannelMask = value;

   relayThresholdsCheckAllRelevantThresholdsValid();
   return status;
}

uint8_t relayThresholdsAddChannelToInputChannelMask(uint8_t channel)
{
   uint8_t status = 0;
   if ( channel <= 7)
   {
      if ( ! disableJTAG_flag && channel > 3)
      {
          CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, TRUE, PSTR("invalid channel number, enabled JTAG allows ch 0-3"));
          status = 1;
      }
      else
      {
         relayThresholdInputChannelMask |= (0x1 << channel);
         relayThresholdsCheckAllRelevantThresholdsValid();
      }
   }
   else
   {
      CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, TRUE, PSTR("invalid channel number [0-7]"));
      status = 1;
   }
   return status;
}

uint8_t relayThresholdsDeleteChannelFromInputChannelMask(uint8_t channel)
{
   uint8_t status = 0;
   if ( channel <= 7)
   {
      relayThresholdInputChannelMask &= (~(0x1 << channel) & 0xFF);
      relayThresholdsCheckAllRelevantThresholdsValid();
   }
   else
   {
      CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, TRUE, PSTR("invalid channel number [0-7]"));
      status = 1;
   }
   return status;
}

uint8_t relayThresholdsSetOutputPin(uint8_t value)
{
   uint8_t status = 0;
   if ( value <= 7)
   {
      relayThresholdOutputPin = value;
   }
   else
   {
      CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, TRUE, PSTR("pin number [0-7]"));
      status = 1;
   }
   return status;
}

uint8_t relayThresholdsSetLedPin(uint8_t value)
{
   uint8_t status = 0;
   if ( value <= 7)
   {
      relayThresholdLedPin = value;
   }
   else
   {
      CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, TRUE, PSTR("pin number [0-7]"));
      status = 1;
   }
   return status;
}

uint8_t relayThresholdsSetExternalPolarityPinPos(uint8_t value)
{
   uint8_t status = 0;

   if ( 1 > value || 8 < value )
   {
      CommunicationError_p(ERRG, dynamicMessage_ErrorIndex, TRUE, PSTR("value out of bounds [1,8]"), value);
      return 1;
   }

   relayThresholdsExternalPolarityPinPos = value;

   /* reinit*/
   relayThresholdInBoundPolarityInit();

   if ( FALSE != relayThresholdReportCurrentStatusEnable_flag)
   {
      relayThresholdsReportCurrentStatus();
   }

   return status;
}

uint8_t relayThresholdsSetExternalPolarityPort(uint8_t value)
{
   uint8_t status = 0;

   if ( 0xA > value || 0xF < value )
   {
      CommunicationError_p(ERRG, dynamicMessage_ErrorIndex, TRUE, PSTR("value out of bounds [A,F]"), value);
      return 1;
   }

   relayThresholdsExternalPolarityPort = value;

   /* reinit*/
   relayThresholdInBoundPolarityInit();

   if ( FALSE != relayThresholdReportCurrentStatusEnable_flag)
   {
      relayThresholdsReportCurrentStatus();
   }

   return status;
}

uint8_t relayThresholdsGetInvertPolarityFlag(void)
{
   return relayThresholdInvertPolarity_flag;
}

uint32_t relayThresholdsGetCurrentValue(uint8_t channel)
{
	return relayThresholdsGetChannelValue(channel, currentValues);
}

uint32_t relayThresholdsGetLowThreshold(uint8_t channel)
{
	return relayThresholdsGetChannelValue(channel, relayThresholdLow);
}

uint32_t relayThresholdsGetHighThreshold(uint8_t channel)
{
	return relayThresholdsGetChannelValue(channel, relayThresholdHigh);
}

uint32_t relayThresholdsGetChannelValue(uint8_t channel, volatile uint16_t values[])
{
	if ( channel <= 7)
	{
		if ( ! disableJTAG_flag && channel > 3)
		{
			CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, TRUE, PSTR("invalid channel number, enabled JTAG allows ch 0-3"));
		}
		else
		{
			return (values[channel] & UINT16_MAX);
		}
	}
	else
	{
		CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, TRUE, PSTR("invalid channel number"));
	}
	return ~(0UL) & ~(UINT16_MAX);

}

uint8_t relayThresholdsGetEnableFlag(void)
{
   return relayThresholdEnable_flag;
}

uint8_t relayThresholdsGetOutOfBoundsLockFlag(void)
{
   return relayThresholdOutOfBoundsLock_flag;
}

uint8_t relayThresholdsGetReportStateEnableFlag(void)
{
   return relayThresholdReportCurrentStatusEnable_flag;
}

uint8_t relayThresholdsGetExtThresholdsMask(void)
{
   return relayThresholdExtThresholdsMask;
}

uint8_t relayThresholdsGetInBoundsPolarity(void)
{
   return (relayThresholdsInBoundsPolarity != FALSE);
}

uint8_t relayThresholdsGetOutOfBoundsPolarity(void)
{
   return (relayThresholdsOutOfBoundsPolarity != FALSE);
}

uint8_t relayThresholdsGetExternalPolarityPinPos(void)
{
   return relayThresholdsExternalPolarityPinPos;
}

uint8_t relayThresholdsGetExternalPolarityPort(void)
{
   return relayThresholdsExternalPolarityPort;
}

uint8_t relayThresholdsGetExternalPolarity(void)
{
   return (relayThresholdsExternalPolarity != FALSE);
}

uint8_t relayThresholdsGetInputChannelMask(void)
{
   return relayThresholdInputChannelMask;
}

uint8_t relayThresholdsGetOutputPin(void)
{
   return relayThresholdOutputPin;
}

uint8_t relayThresholdsGetLedPin(void)
{
   return relayThresholdLedPin;
}

uint8_t relayThresholdsGetCurrentState(void)
{
   return relayThresholdCurrentState;
}

uint8_t relayThresholdsGetRelayValue(void)
{
   return relayThresholdRelayValue;
}

uint8_t relayThresholdsGetRelaySetValue(void)
{
   return relayThresholdRelaySetValue;
}

uint8_t relayThresholdsCheckAllRelevantThresholdsValid(void)
{
	uint8_t maximum_index = 8 * relayThreshold_MAXIMUM_INDEX;
	uint8_t valid = TRUE;
	for (uint8_t index = 0; index < maximum_index; index++)
	{
		if ( relayThresholdInputChannelMask & (0x1 << index))
		{
			if (FALSE == relayThresholds_valid[index])
			{
				valid = FALSE;
				break;
			}
		}
	}

	relayThresholdsSetAllRelevantThresholdsValid(valid);

	return relayThresholdsGetAllThresholdsValid();
}

uint8_t relayThresholdsGetAllThresholdsValid(void)
{
	return relayThresholdAllThresholdsValid;
}

uint8_t relayThresholdsSetAllRelevantThresholdsValid(uint8_t value)
{
	uint8_t status = 0;

	uint8_t maximum_index = 8* relayThreshold_MAXIMUM_INDEX;
	for (uint8_t index = 0; index < maximum_index; index++)
	{
		if ( relayThresholdInputChannelMask & (0x1 << index))
		{
			relayThresholds_valid[index] = (FALSE != value) ;
		}
	}
	relayThresholdAllThresholdsValid = (FALSE != value) ;
	return status;
}

uint8_t relayThresholdGetExternHighChannel(void)
{
	return relayThresholdExternHighChannel;
}

uint8_t relayThresholdGetExternLowChannel(void)
{
	return relayThresholdExternLowChannel;
}

uint8_t relayThresholdSetExternThresholdChannel(int8_t channel, volatile int8_t *threshold )
{
	uint8_t status = 0;
	if (NULL == threshold)
	{
		CommunicationError_p(ERRG, dynamicMessage_ErrorIndex, TRUE, PSTR("invalid pointer (NULL)"));
		status = 1;
	}
	else if (channel < 0)
	{
		*threshold = -1;
	}
	else if ( channel <= 7)
	{
		if ( ! disableJTAG_flag && channel > 3)
		{
			CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, TRUE, PSTR("invalid channel number, enabled JTAG allows ch 0-3"));
			status = 1;
		}
		else
		{
			if (relayThresholdInputChannelMask & (0x1 << channel))
			{
				CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, TRUE, PSTR("overlapping with channel inputs"));
				status = 1;
			}
			else
			{
				if (-1 < *threshold )
				{
					relayThresholdExtThresholdsMask &=  ~(0x1 << *threshold);
				}
				relayThresholdExtThresholdsMask |=  (0x1 << channel);
				*threshold = channel;
			}
		}
	}
	else
	{
		CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, TRUE, PSTR("invalid command argument"));
		status = 1;
	}
	return status;

}

uint8_t relayThresholdSetExternLowChannel(int8_t channel)
{
	return relayThresholdSetExternThresholdChannel(channel, &relayThresholdExternLowChannel);
}

uint8_t relayThresholdSetExternHighChannel(int8_t channel)
{
	return relayThresholdSetExternThresholdChannel(channel, &relayThresholdExternHighChannel);
}

uint8_t relayThresholdGetUseIndividualThresholds(void)
{
	return relayThresholdUseIndividualThresholds;
}

uint8_t relayThresholdSetUseIndividualThresholds(uint8_t value)
{
    uint8_t status = 0;
    relayThresholdUseIndividualThresholds = (FALSE != value);
	return status;
}

