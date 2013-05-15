/*
 * Author: Linda Fouedjio
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

#include "one_wire_adc.h"
#include "one_wire_dualSwitch.h"
#include "one_wire_temperature.h"
#include "read_write_register.h"

#include "adc.h"
#include "api.h"
#include "api_define.h"
#include "api_global.h"

#include "api_debug.h"
#include "can.h"
#include "led.h"
#include "mem-check.h"

uint8_t writeInto8bitRegister(uint8_t address, uint8_t value)
{
	_MMIO_BYTE((address & 0xFF)) = (value & 0xFF);
	return readFrom8bitRegister(address);
}

uint8_t readFrom8bitRegister(uint8_t address)
{
  return (_MMIO_BYTE((address & 0xFF)));
}

 /* this function writes a value the status in the register with the of his address
 * as input parameters the function needs a keyword, an address and a value
 * the function has no return parameter
 */

void writeRegister(struct uartStruct *ptr_uartStruct)
{
	uint8_t readback_register;

	if (0XFF < ptr_uartStruct->Uart_Message_ID)
	{
		generalErrorCode = CommunicationError_p(ERRG, GENERAL_ERROR_value_has_invalid_type, TRUE, NULL);
	}
	if (0XFF < ptr_uartStruct->Uart_Mask)
	{
		generalErrorCode = CommunicationError_p(ERRG, GENERAL_ERROR_adress_has_invalid_type, TRUE, NULL);
	}
	else
	{
        readback_register = writeInto8bitRegister((uint8_t) ptr_uartStruct->Uart_Message_ID & 0xFF ,
        			                              (uint8_t) (ptr_uartStruct->Uart_Mask & 0xFF ));

		/* generate header */
		clearString(uart_message_string, BUFFER_SIZE);
		createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
		snprintf_P(uart_message_string, BUFFER_SIZE - 1,
				PSTR("%s%x: value %x has been written"), uart_message_string,
				(uint8_t) (ptr_uartStruct->Uart_Mask & 0xFF ), readback_register);

		if (readback_register == (uint8_t) (ptr_uartStruct->Uart_Mask & 0xFF))
		{
			strncat_P(uart_message_string, PSTR(" and readback matches"),
					BUFFER_SIZE - 1);
		}
		else
		{
			snprintf_P(uart_message_string, BUFFER_SIZE - 1,
					PSTR("%s and readback does not match (%x)"),
					uart_message_string, readback_register);
		}

		UART0_Send_Message_String_p(NULL, 0);
	}

} //END of writeRegisterfunction

/*this function reads the status of a register with the of his adress
 * as input parameters the function needs a keyword and an adress
 * the function has no return parameter
 */

void readRegister(struct uartStruct *ptr_uartStruct)
{
	if (0XFF < ptr_uartStruct->Uart_Message_ID)
	{
		generalErrorCode = CommunicationError_p(ERRG, GENERAL_ERROR_value_has_invalid_type, FALSE, NULL);
	}

	if (0XFF < ptr_uartStruct->Uart_Mask)
	{
		generalErrorCode = CommunicationError_p(ERRG, GENERAL_ERROR_adress_has_invalid_type, FALSE, NULL);
	}
	else
	{
		uint8_t read_register;

		/* generate header */
		clearString(uart_message_string, BUFFER_SIZE);
		createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);

		read_register = readFrom8bitRegister((uint8_t) ptr_uartStruct->Uart_Message_ID & 0xFF );

		snprintf_P(
				uart_message_string,
				BUFFER_SIZE - 1,
				PSTR("%s%x %x"),
				uart_message_string,
				(uint8_t) (ptr_uartStruct->Uart_Message_ID & 0xFF ), read_register)	;

		UART0_Send_Message_String_p(NULL, 0);

	}
} //END of readRegisterfunction
