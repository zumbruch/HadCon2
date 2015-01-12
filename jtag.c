/*
   Licensed under the EUPL V.1.1, Lizenziert unter EUPL V.1.1 
*/
/*
 * api_jtag.c
 *
 *  Created on: Jul 7, 2011
 *      Author: Peter Zumbruch, GSI, P.Zumbruch@gsi.de
 */

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <stddef.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/iocan128.h>
#include <avr/iocanxx.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h>

#include "adc.h"
#include "api.h"
#include "api_define.h"
#include "api_global.h"

#include "api_debug.h"#include "api_show.h"
#include "api_help.h"
#include "jtag.h"
#include "can.h"
#include "mem-check.h"

static const char filename[] 		PROGMEM = __FILE__;

/*JTAG globals*/

#ifndef ALLOW_DISABLE_JTAG
   /* we need the JTAG interface, thus do not measure
    *the upper four ADC channels (used for JTAG, too)
    */
const uint8_t disableJTAG_flag = FALSE;

#else

uint8_t disableJTAG_flag = FALSE;

#endif

void disableJTAG(uint8_t disable)
{
/* disableJTAG(uint8_t disable)
 * by disabling the JTAG chain 4 more ADC channels are available
 * disable  = FALSE : disables JTAG inputs and frees ADC channels
 * disable != FALSE : enables JTAG inputs and blocks ADC channels
 */

   /* we need the JTAG interface, thus do not measure
    *the upper four ADC channels (used for JTAG, too)
    */
#ifndef ALLOW_DISABLE_JTAG
   generalErrorCode = CommunicationError_p(ERRG, dynamicMessage_ErrorIndex, TRUE, PSTR("disabling JTAG not ALLOWED"));
#else

   disableJTAG_flag = (FALSE != disable);

   /* deactivate digital input buffer for used ADC pins*/
   if ( TRUE == disableJTAG_flag)
   {
      /*
       * 6.6.6 Port Pins
       * When entering a sleep mode, all port pins should be configured to use minimum power. The
       * most important is then to ensure that no pins drive resistive loads. In sleep modes where both
       * the I/O clock (clkI/O) and the ADC clock (clkADC) are stopped, the input buffers of the device will
       * be disabled. This ensures that no power is consumed by the input logic when not needed. In
       * some cases, the input logic is needed for detecting wake-up conditions, and it will then be
       * enabled. Refer to the section “Digital Input Enable and Sleep Modes” on page 70 for details on
       * which pins are enabled. If the input buffer is enabled and the input signal is left floating or have
       * an analog signal level close to VCC/2, the input buffer will use excessive power.
       * For analog input pins, the digital input buffer should be disabled at all times. An analog signal
       * level close to VCC/2 on an input pin can cause significant current even in active mode. Digital
       * input buffers can be disabled by writing to the Digital Input Disable Registers (DIDR1 and
       * DIDR0). Refer to “Digital Input Disable Register 1 – DIDR1” on page 272 and “Digital Input Disable
       * Register 0 – DIDR0” on page 292 for details.
       */

      /*
       * 21.8.5 Digital Input Disable Register 0 – DIDR0
       *
       *  Bit 7:0 – ADC7D..ADC0D: ADC7:0 Digital Input Disable
       * When this bit is written logic one, the digital input buffer on the corresponding ADC pin is disabled.
       * The corresponding PIN Register bit will always read as zero when this bit is set. When an
       * analog signal is applied to the ADC7..0 pin and the digital input from this pin is not needed, this
       * bit should be written logic one to reduce power consumption in the digital input buffer
       */

      DIDR0 = 0xFF; // disable all digital inputs

      /*  MCU Control Register
       *      The MCU Control Register contains control bits for general MCU functions.
       *
       *  MCUCR
       *      Bits 7 JTD: JTAG Interface Disable
       *      When this bit is zero, the JTAG interface is enabled if the JTAGEN Fuse is programmed. If this
       *      bit is one, the JTAG interface is disabled. In order to avoid unintentional disabling or enabling of
       *      the JTAG interface, a timed sequence must be followed when changing this bit: The application
       *      software must write this bit to the desired value twice within four cycles to change its value. Note
       *      that this bit must not be altered when using the On-chip Debug system.
       *      If the JTAG interface is left unconnected to other JTAG circuitry, the JTD bit should be set to
       *      one. The reason for this is to avoid static current at the TDO pin in the JTAG interface.
       */

      for ( int i = 0; i < 4; i++)
      {
         MCUCR |= 0x80;   // deactivate whole JTAG interface
         MCUCR |= 0x80;   // deactivate whole JTAG interface
      }

#if HADCON_VERSION == 2
      /* for version 2 of hadcon it is possible to disable the pull-up resistors necessary for JTAG
       * by setting PG3 of atmel to LOW
       */

      printDebug_p(debugLevelEventDebug, debugSystemApiMisc, __LINE__, filename, PSTR("going to switch JTAG off - status: PING:%#x, PORTG:%#x"), PING&0xFF, PORTG&0xFF);

     PORTG &= ((0x1F) & (~(( 1 << PG3 ) | ( 1 << PG2 ))));

      printDebug_p(debugLevelEventDebug, debugSystemApiMisc, __LINE__, filename, PSTR("having switched JTAG off - status: PING:%#x, PORTG:%#x"), PING&0xFF, PORTG&0xFF);

#endif

   }
   else // enable JTAG
   {
#if HADCON_VERSION == 2
      /* for version 2 of hadcon it is necessary to reenable the pull-up resistors necessary for JTAG
       * by setting PG3 of atmel to HIGH
       */
      printDebug_p(debugLevelEventDebug, debugSystemApiMisc, __LINE__, filename, PSTR("going to switch JTAG on - status: PING:%#x, PORTG:%#x"), PING&0xFF, PORTG&0xFF);

     PORTG |=  (0x1 << PG3) | (0x1 << PG2);

      printDebug_p(debugLevelEventDebug, debugSystemApiMisc, __LINE__, filename, PSTR("having switched JTAG on - status: PING:%#x, PORTG:%#x"), PING&0xFF, PORTG&0xFF);
#endif
     for ( int i = 0; i < 4; i++)
      {
         MCUCR = MCUCR & (~0x80);   // activate whole JTAG interface
         MCUCR = MCUCR & (~0x80);   // activate whole JTAG interface
      }
      DIDR0 = 0x0F;    // reactivate JTAG enable inputs
   }
#endif
}

void modifyJTAG(struct uartStruct *ptr_uartStruct)
{
   switch(ptr_uartStruct->number_of_arguments)
   {
      case 0:
         /* printout status*/

         /* generate message */
         createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
         strncat_P(uart_message_string, PSTR("disabled: "), BUFFER_SIZE -1);
         strncat_P(uart_message_string, (disableJTAG_flag)?PSTR("TRUE"):PSTR("FALSE"),BUFFER_SIZE -1);
         UART0_Send_Message_String_p(NULL,0);
         break;
      case 1:
      {
         /* set status*/
         uint8_t disable = (0 != ptr_uartStruct->Uart_Message_ID) ? TRUE: FALSE;
         disableJTAG(disable);
         /*recursive call to show change*/
         ptr_uartStruct->number_of_arguments=0;
         modifyJTAG(ptr_uartStruct);
         ptr_uartStruct->number_of_arguments=1;
      }
      break;
      default:
         generalErrorCode = CommunicationError_p(ERRG, dynamicMessage_ErrorIndex, TRUE, PSTR("invalid number of arguments"));
         break;
   }
   return;
}
