/*
 * VERSION 1.0 Januar 7th 2010 LATE  File: 'read_write_register.c'
 * Author: Linda Fouedjio
 * modified (heavily rather rebuild): Peter Zumbruch
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/iocan128.h>

#include "one_wire_adc.h"
#include "one_wire_dualSwitch.h"
#include "one_wire_temperature.h"
#include "read_write_register.h"

#include "adc.h"
#include "api.h"
#include "api_define.h"
#include "api_global.h"

#include "api_debug.h"#include "can.h"
#include "led.h"
#include "mem-check.h"

/* this function writes a value the status in the register with the of his adress
 * as input parameters the function needs a keyword, an adress and a value
 * the function has no return parameter
 */

void writeRegister( struct uartStruct *ptr_uartStruct )
{
	uint8_t write_register;

	if ( 0XFF < ptr_uartStruct->Uart_Message_ID )
   {
      general_errorCode = CommunicationError(ERRG, GENERAL_ERROR_value_has_invalid_type, 1, NULL, 0);
   }
   if ( 0XFF < ptr_uartStruct->Uart_Mask )
   {
      general_errorCode = CommunicationError(ERRG, GENERAL_ERROR_adress_has_invalid_type, 1, NULL, 0);
   }
   else
   {
      //write_register = _MMIO_BYTE(ptr_uartStruct->Uart_Message_ID & 0xFF) = ( ptr_uartStruct->Uart_Mask & 0xFF );
      _MMIO_BYTE((uint8_t) ptr_uartStruct->Uart_Message_ID & 0xFF) = (uint8_t) ( ptr_uartStruct->Uart_Mask & 0xFF );
      write_register = _MMIO_BYTE((uint8_t) ptr_uartStruct->Uart_Message_ID & 0xFF);

      /* generate header */
      clearString(uart_message_string, BUFFER_SIZE);
      createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
      snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s%x: value %x has been written"),
                 uart_message_string, (uint8_t) ( ptr_uartStruct->Uart_Mask & 0xFF ), write_register);

      if ( write_register == (uint8_t) ( ptr_uartStruct->Uart_Mask & 0xFF ))
      {
         strncat_P(uart_message_string, PSTR(" and readback matches"), BUFFER_SIZE - 1);
      }
      else
      {
         snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s and readback does not match (%x)"), uart_message_string, write_register );
      }

      UART0_Send_Message_String(NULL,0);
   }

}//END of writeRegisterfunction


/*this function reads the status of a register with the of his adress
 * as input parameters the function needs a keyword and an adress
 * the function has no return parameter
 */

void readRegister( struct uartStruct *ptr_uartStruct )
{
   if ( 0XFF < ptr_uartStruct->Uart_Message_ID )
   {
      general_errorCode = CommunicationError(ERRG, GENERAL_ERROR_value_has_invalid_type, 0, NULL, 0);
   }

   if ( 0XFF < ptr_uartStruct->Uart_Mask )
   {
      general_errorCode = CommunicationError(ERRG, GENERAL_ERROR_adress_has_invalid_type, 0, NULL, 0);
   }
   else
   {
      uint8_t read_register;

      /* generate header */
      clearString(uart_message_string,BUFFER_SIZE);
      createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);

      read_register = _MMIO_BYTE(( (uint8_t) ptr_uartStruct->Uart_Message_ID & 0xFF ));

      snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s%x %x"),
                 uart_message_string, (uint8_t) ( ptr_uartStruct->Uart_Message_ID & 0xFF ), read_register);

      UART0_Send_Message_String(NULL, 0);

   }
}//END of readRegisterfunction
