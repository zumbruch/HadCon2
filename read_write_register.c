/*
 * VERSION 1.0 Januar 7th 2010 LATE  File: 'read_write_register.c'
 * Author: Linda Fouedjio
 * Expanded by Michail Pligouroudis 30/05/2012
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

#include "api_debug.h"
#include "can.h"
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


void write_double_Register(struct uartStruct *ptr_uartStruct )
{
  uint8_t write_register;
  int step;
  char temp_ID, temp_MASK;
 
   temp_ID = ptr_uartStruct->Uart_Message_ID;	   
   temp_MASK = ptr_uartStruct->Uart_Mask;

  for(step=0; step<7; step++) 
    {
      switch (step) 
	{
	case step1://declare UART in order to send data with baud=1,025Mbps
	  {
	    ptr_uartStruct->Uart_Message_ID = UCSR1C_register_of_ATMEL_address;	   
	    ptr_uartStruct->Uart_Mask= UCSR1C_register_of_ATMEL_value;
	    break;
	  }
	case step2://delclare UART in order to send data with baud=1,025Mbps
	  {
	    ptr_uartStruct->Uart_Message_ID = UBRR1L_register_of_ATMEL_address;	   
	    ptr_uartStruct->Uart_Mask= UBRR1L_register_of_ATMEL_value;
	    break;
	  }
	case step3://delclare UART in order to send data with baud=1,025Mbps
	  {
	    ptr_uartStruct->Uart_Message_ID = UBRR1H_register_of_ATMEL_address;	   
	    ptr_uartStruct->Uart_Mask= UBRR1H_register_of_ATMEL_value;
	    break;
	  }
	case step4://delclare UART in order to send data with baud=1,025Mbps
	  {
	    ptr_uartStruct->Uart_Message_ID = UCSR1B_register_of_ATMEL_address;	   
	    ptr_uartStruct->Uart_Mask= UCSR1B_register_of_ATMEL_value_transmit;
	    break;
	  }
	case step5://delclare UART in order to send data with baud=1,025Mbps
	  {
	    ptr_uartStruct->Uart_Message_ID = UCSR1A_register_of_ATMEL_address;	   
	    ptr_uartStruct->Uart_Mask= UCSR1A_register_of_ATMEL_value;
	    break;
	  }
	case step6: //sending your new data
	  {
	    ptr_uartStruct->Uart_Message_ID = UDR1_register_of_ATMEL_address;	   
	    ptr_uartStruct->Uart_Mask= temp_ID;
	    break;
	  }
	case step7:// sending your new data
	  {
	    ptr_uartStruct->Uart_Message_ID = UDR1_register_of_ATMEL_address;	   
	    ptr_uartStruct->Uart_Mask= temp_MASK;
	    break;
	  }
	default:
	  {
	    clearString(uart_message_string, BUFFER_SIZE);
	    createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
	    snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s ERROR\n"),uart_message_string );
	    UART0_Send_Message_String(NULL,0);
	    break;
	  }
	}
      writeRegister(ptr_uartStruct); /* call function with name writeRegister  */
    } //end of for loop

  switch (temp_ID)
    {
    case CONTROL_REGISTER:
      {
	switch (temp_MASK)       
	  {
	  case DEBUG:           
	    {
	      clearString(uart_message_string, BUFFER_SIZE);
	      createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
              snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s DEBUG\n"),uart_message_string );
	      UART0_Send_Message_String(NULL,0);
	      break;
	    }
	  case RESET:         
	    {
	      clearString(uart_message_string, BUFFER_SIZE);
	      createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
              snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s RESET STATUS\n"),uart_message_string );
	      UART0_Send_Message_String(NULL,0);
	      break;
	    }
	  case PSEUDORANDOM:
	    {
	      clearString(uart_message_string, BUFFER_SIZE);
	      createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
	      snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s PSEUDORANDOM WAVEFORM OUTPUT \n"),uart_message_string);
	      UART0_Send_Message_String(NULL,0);
	      break;
	    }
	  case SQUARE:
	    {
	      clearString(uart_message_string, BUFFER_SIZE);
	      createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
	      snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s SQUARE WAVEFORM OUTPUT\n"),uart_message_string);
	      UART0_Send_Message_String(NULL,0);
	      break;
	    }
	  case PULSE:
	    {
	      clearString(uart_message_string, BUFFER_SIZE);
	      createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
	      snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s PULSE WAVEFORM OUTPUT\n"),uart_message_string);
	      UART0_Send_Message_String(NULL,0);
	      break;
	    }
	  case PSEUDORAND_TIME_PULSE:
	    {
	      clearString(uart_message_string, BUFFER_SIZE);
	      createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
	      snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s PSEUDORANDOM TIME PULSE(100ns) WAVEFORM OUTPUT\n"),uart_message_string);
	      UART0_Send_Message_String(NULL,0);
	      break;
	    }
	  default :
	    {
	      clearString(uart_message_string, BUFFER_SIZE);
	      createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
	      snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s Invalid value for the control register\n"),uart_message_string);
	      UART0_Send_Message_String(NULL,0);
	      break;
	    }
	  }
	break;
      }
    case DELAY1_MSB_REGISTER:
      {
	clearString(uart_message_string, BUFFER_SIZE);
	createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
	snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s Delay1 MSB register is written\n"),uart_message_string);
	UART0_Send_Message_String(NULL,0);
	break;
      }
    case DELAY1_MIDDLE_REGISTER:
      {
	clearString(uart_message_string, BUFFER_SIZE);
	createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
	snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s Delay1 MIDDLE register is written\n"),uart_message_string);
	UART0_Send_Message_String(NULL,0);
	break;
      }
    case DELAY1_LSB_REGISTER:
      {
	clearString(uart_message_string, BUFFER_SIZE);
	createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
	snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s Delay1 LSB register is written\n"),uart_message_string);
	UART0_Send_Message_String(NULL,0);
	break;
      }
    case DELAY2_MSB_REGISTER:
      {
	clearString(uart_message_string, BUFFER_SIZE);
	createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
	snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s Delay2 MSB register is written\n"),uart_message_string);
	UART0_Send_Message_String(NULL,0);
	break;
      }
    case DELAY2_MIDDLE_REGISTER:
      {
	clearString(uart_message_string, BUFFER_SIZE);
	createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
	snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s Delay2 MIDDLE register is written\n"),uart_message_string);
	UART0_Send_Message_String(NULL,0);
	break;
      }
    case DELAY2_LSB_REGISTER:
      {
	clearString(uart_message_string, BUFFER_SIZE);
	createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
	snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s Delay2 LSB register is written\n"),uart_message_string);
	UART0_Send_Message_String(NULL,0);
	break;
      }
    default:
      {
	clearString(uart_message_string, BUFFER_SIZE);
	createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
	snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s Invalid Address\n"),uart_message_string);
	UART0_Send_Message_String(NULL,0);
	break;
      }
    }

  
}//END of write_double_Registerfunction


void read_waveform_generator_Registers(struct uartStruct *ptr_uartStruct )
{
  uint8_t write_register;
  uint8_t read_register;
  int step= 0;
  uint8_t registers [7];
  int tempID;

   tempID = ptr_uartStruct->Uart_Message_ID;	   
  
  for(step=0; step<12; step++) 
    {
      switch (step) 
	{
	case step1://delclare UART in order to send data with baud=1,025Mbps
	  {
	    ptr_uartStruct->Uart_Message_ID = UCSR1C_register_of_ATMEL_address;	   
	    ptr_uartStruct->Uart_Mask= UCSR1C_register_of_ATMEL_value;
	    break;
	  }
	case step2://delclare UART in order to send data with baud=1,025Mbps
	  {
	    ptr_uartStruct->Uart_Message_ID = UBRR1L_register_of_ATMEL_address;	   
	    ptr_uartStruct->Uart_Mask= UBRR1L_register_of_ATMEL_value;
	    break;
	  }
	case step3://delclare UART in order to send data with baud=1,025Mbps
	  {
	    ptr_uartStruct->Uart_Message_ID = UBRR1H_register_of_ATMEL_address;	   
	    ptr_uartStruct->Uart_Mask= UBRR1H_register_of_ATMEL_value;
	    break;
	  }
	case step4://delclare UART in order to send data with baud=1,025Mbps
	  {
	    ptr_uartStruct->Uart_Message_ID = UCSR1B_register_of_ATMEL_address;	   
	    ptr_uartStruct->Uart_Mask= UCSR1B_register_of_ATMEL_value_transmit;
	    break;
	  }
	case step5://delclare UART in order to send data with baud=1,025Mbps
	  {
	    ptr_uartStruct->Uart_Message_ID = UCSR1A_register_of_ATMEL_address;	   
	    ptr_uartStruct->Uart_Mask= UCSR1A_register_of_ATMEL_value;
	    break;
	  }
	case step6: //control register address
	  {
	    ptr_uartStruct->Uart_Message_ID = UDR1_register_of_ATMEL_address;	   
	    ptr_uartStruct->Uart_Mask= CONTROL_REGISTER; //0x00
	    break;
	  }
	case step7: //control register in read status
	  {
	    ptr_uartStruct->Uart_Message_ID = UDR1_register_of_ATMEL_address;	   
	    ptr_uartStruct->Uart_Mask= READ; //0xA0
	    break;
	  }
	case step8://delclare UART in order to recieve data with baud=1,025Mbps
	  {
	    ptr_uartStruct->Uart_Message_ID = UCSR1C_register_of_ATMEL_address;	   
	    ptr_uartStruct->Uart_Mask= UCSR1C_register_of_ATMEL_value;
	    break;
	  }
	case step9://delclare UART in order to recieve data with baud=1,025Mbps
	  {
	    ptr_uartStruct->Uart_Message_ID = UBRR1L_register_of_ATMEL_address;	   
	    ptr_uartStruct->Uart_Mask= UBRR1L_register_of_ATMEL_value;
	    break;
	  }
	case step10://delclare UART in order to recieve data with baud=1,025Mbps
	  {
	    ptr_uartStruct->Uart_Message_ID = UBRR1H_register_of_ATMEL_address;	   
	    ptr_uartStruct->Uart_Mask= UBRR1H_register_of_ATMEL_value;
	    break;
	  }
	case step11://delclare UART in order to recieve data with baud=1,025Mbps
	  {
	    ptr_uartStruct->Uart_Message_ID = UCSR1A_register_of_ATMEL_address;	   
	    ptr_uartStruct->Uart_Mask= UCSR1A_register_of_ATMEL_value;
	    break;
	  }
	case step12://delclare UART in order to recieve data with baud=1,025Mbps
	  {
	    ptr_uartStruct->Uart_Message_ID = UCSR1B_register_of_ATMEL_address;	   
	    ptr_uartStruct->Uart_Mask= UCSR1B_register_of_ATMEL_value_recieve;
	    break;
	  }
	default:
	  {
	    clearString(uart_message_string, BUFFER_SIZE);
	    createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
	    snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s ERROR\n"),uart_message_string );
	    UART0_Send_Message_String(NULL,0);
	    break;
	  }
	}
      writeRegister(ptr_uartStruct); /* call function with name writeRegister  */
    } //end of for loop


  for (step=0; step<7; step++) // saving value of the registers from FPGA at the table "registers[]"
    {
      ptr_uartStruct->Uart_Message_ID = UDR1_register_of_ATMEL_address;	   
      clearString(uart_message_string,BUFFER_SIZE);
      createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
      registers[step] = _MMIO_BYTE(( (uint8_t) ptr_uartStruct->Uart_Message_ID & 0xFF ));
      snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s%x %x"),
                 uart_message_string, (uint8_t) ( ptr_uartStruct->Uart_Message_ID & 0xFF ), read_register);
      UART0_Send_Message_String(NULL, 0);
    }//end for loop

   
  for(step=0; step<5; step++) 
    {
      switch (step) 
	{
	case step1://delclare UART in order to send data with baud=1,025Mbps
	  {
	    ptr_uartStruct->Uart_Message_ID = UCSR1C_register_of_ATMEL_address;	   
	    ptr_uartStruct->Uart_Mask= UCSR1C_register_of_ATMEL_value;
	    break;
	  }
	case step2://delclare UART in order to send data with baud=1,025Mbps
	  {
	    ptr_uartStruct->Uart_Message_ID = UBRR1L_register_of_ATMEL_address;	   
	    ptr_uartStruct->Uart_Mask= UBRR1L_register_of_ATMEL_value;
	    break;
	  }
	case step3://delclare UART in order to send data with baud=1,025Mbps
	  {
	    ptr_uartStruct->Uart_Message_ID = UBRR1H_register_of_ATMEL_address;	   
	    ptr_uartStruct->Uart_Mask= UBRR1H_register_of_ATMEL_value;
	    break;
	  }
	case step4://delclare UART in order to send data with baud=1,025Mbps
	  {
	    ptr_uartStruct->Uart_Message_ID = UCSR1B_register_of_ATMEL_address;	   
	    ptr_uartStruct->Uart_Mask= UCSR1B_register_of_ATMEL_value_transmit;
	    break;
	  }
	case step5://delclare UART in order to send data with baud=1,025Mbps
	  {
	    ptr_uartStruct->Uart_Message_ID = UCSR1A_register_of_ATMEL_address;	   
	    ptr_uartStruct->Uart_Mask= UCSR1A_register_of_ATMEL_value;
	    break;
	  }
	}
    } // end for loop


    switch (tempID)
      {
      case CONTROL_REGISTER:
	{
	  clearString(uart_message_string, BUFFER_SIZE);
	  createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
	  snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s Control Register value= %x\n"),uart_message_string, registers[step7]);
	  UART0_Send_Message_String(NULL,0);
	  break;
	}

      case DELAY1_MSB_REGISTER:
	{
	  clearString(uart_message_string, BUFFER_SIZE);
	  createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
	  snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s Delay1 MSB value= %x \n"),uart_message_string, registers[step1] );
	  UART0_Send_Message_String(NULL,0); 
	  break;
	}

      case DELAY1_MIDDLE_REGISTER:
	{
	  clearString(uart_message_string, BUFFER_SIZE);
	  createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
	  snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s Delay1 MIDDLE value= %x \n"),uart_message_string, registers[step2] );
	  UART0_Send_Message_String(NULL,0);
	  break;
	}

      case DELAY1_LSB_REGISTER:
	{
	  clearString(uart_message_string, BUFFER_SIZE);
	  createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
	  snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s Delay1 LSB value= %x \n"),uart_message_string, registers[step3]);
	  UART0_Send_Message_String(NULL,0);
	  break;
	}

      case DELAY2_MSB_REGISTER:
	{
	  clearString(uart_message_string, BUFFER_SIZE);
	  createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
	  snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s Delay2 MSB value= %x\n"),uart_message_string, registers[step4]);
	  UART0_Send_Message_String(NULL,0);
	  break;
	}

      case DELAY2_MIDDLE_REGISTER:
	{
	  clearString(uart_message_string, BUFFER_SIZE);
	  createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
	  snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s Delay2 MIDDLE value= %x\n"),uart_message_string, registers[step5]);
	  UART0_Send_Message_String(NULL,0);
	  break;
	}

      case DELAY2_LSB_REGISTER:
	{
	  clearString(uart_message_string, BUFFER_SIZE);
	  createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
	  snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s Delay2 LSB value= %x\n"),uart_message_string, registers[step6]);
	  UART0_Send_Message_String(NULL,0);
	  break;
	}

      case SHOW_ALL:
	{	
	  for(step=0; step<7; step++) 
	    {
	      switch (step)       
		{
		case step1:           
		  {
		    clearString(uart_message_string, BUFFER_SIZE);
		    createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
		    snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s Delay1 MSB value= %x \n"),uart_message_string, registers[step] );
		    UART0_Send_Message_String(NULL,0); 
		    break;
		  }
		case step2:         
		  {
		    clearString(uart_message_string, BUFFER_SIZE);
		    createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
		    snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s Delay1 MIDDLE value= %x \n"),uart_message_string, registers[step] );
		    UART0_Send_Message_String(NULL,0);
		    break;
		  }
		case step3:
		  {
		    clearString(uart_message_string, BUFFER_SIZE);
		    createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
		    snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s Delay1 LSB value= %x \n"),uart_message_string, registers[step]);
		    UART0_Send_Message_String(NULL,0);
		    break;
		  }
		case step4:
		  {
		    clearString(uart_message_string, BUFFER_SIZE);
		    createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
		    snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s Delay2 MSB value= %x\n"),uart_message_string, registers[step]);
		    UART0_Send_Message_String(NULL,0);
		    break;
		  }
		case step5:
		  {
		    clearString(uart_message_string, BUFFER_SIZE);
		    createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
		    snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s Delay2 MIDDLE value= %x\n"),uart_message_string, registers[step]);
		    UART0_Send_Message_String(NULL,0);
		    break;
		  }
		case step6:
		  {
		    clearString(uart_message_string, BUFFER_SIZE);
		    createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
		    snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s Delay2 LSB value= %x\n"),uart_message_string, registers[step]);
		    UART0_Send_Message_String(NULL,0);
		    break;
		  }
		case step7:
		  {
		    clearString(uart_message_string, BUFFER_SIZE);
		    createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
		    snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s Control Register value= %x\n"),uart_message_string, registers[step]);
		    UART0_Send_Message_String(NULL,0);
		    break;
		  }
		default :
		  {
		    clearString(uart_message_string, BUFFER_SIZE);
		    createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
		    snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s ERROR\n"),uart_message_string);
		    UART0_Send_Message_String(NULL,0);
		    break;
		  }
		}
	    }
		break;
	}

      default:
	{
	  clearString(uart_message_string, BUFFER_SIZE);
	  createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
	  snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s ERROR\n"),uart_message_string);
	  UART0_Send_Message_String(NULL,0);
	  break;
	}
      }
	


    for(step=0; step<5; step++) // declaration for new recieving data stream
    {
      switch (step) 
	{
	case step1://delclare UART in order to send data with baud=1,025Mbps
	  {
	    ptr_uartStruct->Uart_Message_ID = UCSR1C_register_of_ATMEL_address;	   
	    ptr_uartStruct->Uart_Mask= UCSR1C_register_of_ATMEL_value;
	    break;
	  }
	case step2://delclare UART in order to send data with baud=1,025Mbps
	  {
	    ptr_uartStruct->Uart_Message_ID = UBRR1L_register_of_ATMEL_address;	   
	    ptr_uartStruct->Uart_Mask= UBRR1L_register_of_ATMEL_value;
	    break;
	  }
	case step3://delclare UART in order to send data with baud=1,025Mbps
	  {
	    ptr_uartStruct->Uart_Message_ID = UBRR1H_register_of_ATMEL_address;	   
	    ptr_uartStruct->Uart_Mask= UBRR1H_register_of_ATMEL_value;
	    break;
	  }
	case step4://delclare UART in order to send data with baud=1,025Mbps
	  {
	    ptr_uartStruct->Uart_Message_ID = UCSR1B_register_of_ATMEL_address;	   
	    ptr_uartStruct->Uart_Mask= UCSR1B_register_of_ATMEL_value_transmit;
	    break;
	  }
	case step5://delclare UART in order to send data with baud=1,025Mbps
	  {
	    ptr_uartStruct->Uart_Message_ID = UCSR1A_register_of_ATMEL_address;	   
	    ptr_uartStruct->Uart_Mask= UCSR1A_register_of_ATMEL_value;
	    break;
	  }
	}
      writeRegister(ptr_uartStruct); /* call function with name writeRegister  */
    }
   
}//end of read_waveform_generator_registers function
