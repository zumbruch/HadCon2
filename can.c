/*
 * VERSION 1.0 Januar 7th 2010 LATE  File: 'can.c'
 * Author: Linda Fouedjio
 * modified (heavily rather rebuild): Peter Zumbruch
 * modified: Florian Feldbauer
 * modified: Peter Zumbruch, July 2011
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

#include "one_wire.h"
#include "one_wire_adc.h"
#include "one_wire_dualSwitch.h"
#include "one_wire_simpleSwitch.h"
#include "read_write_register.h"
#include "one_wire_temperature.h"

#include "adc.h"
#include "api.h"
#include "api_define.h"
#include "api_global.h"

#include "api_debug.h"
#include "can.h"
#include "mem-check.h"

/*variable to subscribe and unsubcribe some messages via CAN bus */
uint16_t subscribe_ID[MAX_LENGTH_SUBSCRIBE];
uint16_t subscribe_mask[MAX_LENGTH_SUBSCRIBE];

/*
 * this function will read certain messages on the bus
 * the function has a pointer of the serial structure as input and returns no parameter
 */

void Subscribe_Message( struct uartStruct *ptr_uartStruct )
{
   int8_t findMob;
   uint8_t equality = 1;

   for ( uint8_t count_subscribe = 2 ; count_subscribe <= 14 ; count_subscribe++ )
   {
      if ( ( ptr_uartStruct->Uart_Message_ID ) == ( subscribe_ID[count_subscribe] ) && ( ptr_uartStruct->Uart_Mask ) == ( subscribe_mask[count_subscribe] ) )
      {
         equality = 0;
         mailbox_errorCode = CommunicationError(ERRM, MOB_ERROR_this_message_already_exists, 0, NULL, 0);
      }
   }
   if ( 1 == equality )
   {
      subscribe_ID[ptr_subscribe] = ( ptr_uartStruct->Uart_Message_ID );
      subscribe_mask[ptr_subscribe] = ( ptr_uartStruct->Uart_Mask );
      ptr_subscribe++;

      findMob = Get_FreeMob();

      if ( 14 < ptr_subscribe )
      {
         ptr_subscribe = 2;
      }

      if ( ( -1 ) == findMob )
      {
         mailbox_errorCode = CommunicationError(ERRM, MOB_ERROR_all_mailboxes_already_in_use, 0, NULL, 0);
      }
      else
      {
         CANPAGE = ( findMob << MOBNB0 );
         CANSTMOB = 0x00; /* cancel pending operation */
         CANCDMOB = 0x00;
         CANHPMOB = 0x00; /* enable direct mob indexing */

         if ( ( ( MAX_ELF_BIT ) >= ( ptr_uartStruct->Uart_Message_ID ) ) || ( ( MAX_ELF_BIT ) >= ( ptr_uartStruct->Uart_Mask ) ) )
         {
            /*set identifier to send*/
            CANIDT2 = ( ( ptr_uartStruct->Uart_Message_ID ) & 0x7 ) << 5;
            CANIDT1 = ( ptr_uartStruct->Uart_Message_ID ) >> 3;
            /* enable comparison*/
            CANIDM4 = ( 1 << RTRMSK );

            /* set identifier Mask*/
            CANIDM2 = ( ( ptr_uartStruct->Uart_Mask ) & 0x7 ) << 5;
            CANIDM1 = ( ptr_uartStruct->Uart_Mask ) >> 3;
         }
         else
         {
            /*set identifier to send  */
            CANIDT4 = ( ( ptr_uartStruct->Uart_Message_ID ) & 0x7 ) << 3;
            CANIDT3 = ( ptr_uartStruct->Uart_Message_ID ) >> 5;
            CANIDT2 = ( ptr_uartStruct->Uart_Message_ID ) >> 5;
            CANIDT1 = ( ptr_uartStruct->Uart_Message_ID ) >> 3;
            /*set identifier mask  */
            CANIDM4 = ( ( ptr_uartStruct->Uart_Mask ) & 0x7 ) << 3;
            CANIDM3 = ( ptr_uartStruct->Uart_Mask ) >> 5;
            CANIDM2 = ( ptr_uartStruct->Uart_Mask ) >> 5;
            CANIDM1 = ( ptr_uartStruct->Uart_Mask ) >> 3;
         }
         CANCDMOB = ( 1 << CONMOB1 ); /* enable reception mode */
         unsigned mask = 1 << findMob;
         CANIE2 |= mask;
         CANIE1 |= ( mask >> 8 );
      }
   }
}//END of Subscribe_Message function


/*
 * this function does not read certain messages on the bus
 *the function has a pointer of the serial structure as input and returns no parameter
 */

void Unsubscribe_Message( struct uartStruct *ptr_uartStruct )
{
   uint8_t inequality = 1;

   for ( uint8_t count_mob = 2 ; count_mob <= 14 ; count_mob++ )
   {
      CANPAGE = ( count_mob << MOBNB0 );

      if ( ((uint32_t) (( CANIDT2 >> 5 ) | ( CANIDT1 << 3 ) ) == ptr_uartStruct->Uart_Message_ID ) &&
           ((uint32_t) (( CANIDM2 >> 5 ) | ( CANIDM1 << 3 ) ) == ptr_uartStruct->Uart_Mask )         )
      {
         CANSTMOB = 0x00; /* cancel pending operation */
         CANCDMOB = 0x00; /* very important,that disable MOB*/
         CANHPMOB = 0x00;
         CANIDM1 = 0X00;
         CANIDM2 = 0X00;
         CANIDT1 = 0X00;
         CANIDT2 = 0X00;
         unsigned mask = 1 << count_mob;
         CANIE2 &= ~mask;
         CANIE1 &= ~( mask >> 8 );
         inequality = 0;
         for ( int count_subscribe = 2 ; count_subscribe <= 14 ; count_subscribe++ )
         {
            if ( ( ( ptr_uartStruct->Uart_Message_ID ) == ( subscribe_ID[count_subscribe] ) ) && ( ( ( ptr_uartStruct->Uart_Mask )
                     == ( subscribe_mask[count_subscribe] ) ) ) )
            {
               subscribe_ID[count_subscribe] = 0;
               subscribe_mask[count_subscribe] = 0;
            }
         }
      }
   }
   if ( 1 == inequality )
   {
      mailbox_errorCode = CommunicationError(ERRM, MOB_ERROR_message_ID_not_found, 0, NULL, 0);
   }
} //END of Unsubscribe_Message


/*
 * this function runs a command and expects no data
 *the function has a pointer of the serial structure as input and returns no parameter
 */

void Send_Message( struct uartStruct *ptr_uartStruct )
{

	CANPAGE = ( 1 << MOBNB0 );/*set channel number  */
	CANIDT4 = ( ptr_uartStruct->Uart_Rtr << RTRTAG ); /* enable remote transmition request */
	CANCDMOB = ( ptr_uartStruct->Uart_Length << DLC0 ); /* set lenght of data*/

	if ( ( MAX_ELF_BIT ) >= ( ptr_uartStruct->Uart_Message_ID ) )
	{
		CANCDMOB |= ( 0 << IDE ); /* enable CAN standard 11 bit */
		/*set Identifier to send */
		CANIDT2 = ( ( ptr_uartStruct->Uart_Message_ID ) & 0x7 ) << 5;
		CANIDT1 = ( ptr_uartStruct->Uart_Message_ID ) >> 3;
	}
	else
	{
		CANCDMOB |= ( 1 << IDE ); /* enable CAN standard 29 bit */
		/*set Identifier to send  */
		CANIDT2 = ( ( ptr_uartStruct->Uart_Message_ID ) & 0x7 ) << 3;
		CANIDT2 = ( ptr_uartStruct->Uart_Message_ID ) >> 5;
		CANIDT2 = ( ptr_uartStruct->Uart_Message_ID ) >> 5;
		CANIDT1 = ( ptr_uartStruct->Uart_Message_ID ) >> 5;
	}

	/*put data in mailbox*/
	for ( uint8_t count_data = 0 ; count_data < 8 ; count_data++ )
	{
		CANMSG = ptr_uartStruct->Uart_Data[count_data];
	}

	CANSTMOB = 0x00;
	CANCDMOB |= ( 1 << CONMOB0 ); /*enable transmition mode*/
	/*call the function verify that the sending of data is complete*/
	Wait_for_Can_Send_Message_Finished();
}//END of Send_Message function

/*
 * this function runs a command and expects data
 * the function has a pointer of the serial structure as input and returns no parameter
 */

void Receive_Message( struct uartStruct *ptr_uartStruct )
{
	CANPAGE = ( 0 << MOBNB0 );/*set channel number */
	CANSTMOB = 0x00; /* cancel pending operation */
	CANCDMOB = 0x00;
	CANIDT4 = ( ptr_uartStruct->Uart_Rtr << RTRTAG ); /* enable remote transmission request */
	CANCDMOB = ( ptr_uartStruct->Uart_Length << DLC0 ); /* set length of data*/

	if ( ( ( MAX_ELF_BIT ) >= ( ptr_uartStruct->Uart_Message_ID ) )
			|| ( ( MAX_ELF_BIT ) >= ( ptr_uartStruct->Uart_Mask ) ) )
	{
		CANCDMOB |= ( 0 << IDE ); /* enable CAN standard 11 bit */
		/*set Identifier to send */
		CANIDT2 = ( ( ptr_uartStruct->Uart_Message_ID ) & 0x7 ) << 5;
		CANIDT1 = ( ptr_uartStruct->Uart_Message_ID ) >> 3;
		/*set Mask to receive only Message */
		CANIDM1 = 0xFF;
		CANIDM2 = 0xFF;
		CANIDM3 = 0xFF;
		CANIDM4 = 0xFF;
	}
	else
	{
		CANCDMOB |= ( 1 << IDE ); /* enable CAN standard 29 bit */
		/*set Identifier to send  */
		CANIDT2 = ( ( ptr_uartStruct->Uart_Message_ID ) & 0x7 ) << 3;
		CANIDT2 = ( ptr_uartStruct->Uart_Message_ID ) >> 5;
		CANIDT2 = ( ptr_uartStruct->Uart_Message_ID ) >> 5;
		CANIDT1 = ( ptr_uartStruct->Uart_Message_ID ) >> 5;
	}

	/* put data in mailbox*/
	for ( uint8_t count_data = 0 ; count_data < 8 ; count_data++ )
	{
		CANMSG = ptr_uartStruct->Uart_Data[count_data];
	}
	CANSTMOB = 0;

	CANCDMOB |= ( 1 << CONMOB0 ); /*enable transmition mode*/

	/*call the function verify that the sending of data is complete*/
	Wait_for_Can_Receive_message_Finished();

}//END of Receive_Message function

/*
 *this function wait until the command is sent
 *the function has no input and output parameter
 */
void Wait_for_Can_Send_Message_Finished( void )
{
	double can_timeout1 = TIMEOUT_C; /*Timeout for CAN-communication*/
	while ( !( CANSTMOB & ( 1 << TXOK ) ) && ( --can_timeout1 > 0 ) )
	{
		/* do nothing  */
	}
	if ( 1 >= can_timeout1 )
	{
		can_errorCode = CommunicationError(ERRC, CAN_ERROR_timeout_for_CAN_communication, 0, NULL, 0);
		can_timeout1 = TIMEOUT_C;
	}

	CANSTMOB &= ~( 1 << TXOK ); /* reset transmission flag */
	CANCDMOB &= ~( 1 << CONMOB0 ); /* disable transmission mode */

	/* give feedback for successful transmit */
    if ( debugLevelVerboseDebug <= globalDebugLevel && ( ( globalDebugSystemMask >> debugSystemCAN ) & 0x1 ) )
    {
       snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("RECV %s"), READY);
       UART0_Send_Message_String_p(NULL,0);
    }

	/* all parameter initializing */
	Reset_Parameter_CANSend_Message();

}//END of Wait_for_Can_Send_Message_Finished


/*
 *this function wait until the command in case the request is sent
 *the function has no input and output parameter
 */

void Wait_for_Can_Receive_message_Finished( void )
{

	double can_timeout2 = TIMEOUT_C; /*Timeout for CAN-communication*/
	while ( !( CANSTMOB & ( 1 << RXOK ) ) && ( --can_timeout2 > 0 ) )
	{
		/* do nothing  */
	}

	if ( ( 1 >= can_timeout2 ) && ( CANSTMOB & ( 1 << TXOK ) ) )
	{
		can_errorCode = CommunicationError(ERRC, CAN_ERROR_timeout_for_CAN_communication, 0, NULL, 0);
		can_timeout2 = TIMEOUT_C;
	}
	else
	{
		CANSTMOB &= ~( 1 << TXOK ); /* reset transmition flag */
		CANCDMOB &= ~( 1 << CONMOB0 ); /* disable transmition mode */
	}

	/* all parameter initialisieren */
	Reset_Parameter_CANSend_Message();
	//Reset_Parameter_CANReceive();

}//END of Wait_for_Can_Receive_message_Finished function

/*
 *This function grabs the CAN data received in a string
 *the function has a poimter of the CAN structur as input and returns no parameter
 */

void Convert_pack_canFrame_to_UartFormat( struct canStruct *ptr_canStruct )
{

	uint16_t pack_canFrame[MAX_CAN_DATA];/* variable for storage received a complete CAN-Frame via CAN-Bus*/
	uint8_t ptr_pack_canFrame = START_POINTER_TO_RECEIVE_CAN_DATEN; /*pointer to receive CAN-data */

	pack_canFrame[0] = ptr_canStruct->Can_Mob;
	pack_canFrame[1] = ptr_canStruct->Can_Message_ID;
	pack_canFrame[2] = ptr_canStruct->Can_Length;

	for ( uint8_t count_can_data = 0 ; count_can_data < MAX_BYTE_CAN_DATA ; count_can_data++ )
	{
		pack_canFrame[ptr_pack_canFrame] = ptr_canStruct->Can_Data[count_can_data];
		ptr_pack_canFrame++;
	}

	/* grabs the received data in a string */

	for ( uint8_t i = 0 ; i < MAX_CAN_DATA ; i++ )
	{
		snprintf_P(temp_canString, MAX_LENGTH_CAN_DATA - 1, PSTR("%x"), pack_canFrame[i]);
		strncat(store_canData, temp_canString, MAX_LENGTH_CAN_DATA - 1);
		strncat_P(store_canData, PSTR(" "), MAX_LENGTH_CAN_DATA - 1);
	}

	strncat_P(canString, PSTR("RECV"), MAX_LENGTH_CAN_DATA - 1);
	strncat_P(canString, PSTR(" "), MAX_LENGTH_CAN_DATA - 1);
	strncat(canString, store_canData, MAX_LENGTH_CAN_DATA - 1);
	snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s"), canString);
	UART0_Send_Message_String_p(NULL,0);
	clearString(canString, MAX_LENGTH_CAN_DATA); /*clear the variable CanString */
	clearString(temp_canString, MAX_LENGTH_CAN_DATA); /*clear the variable temp_canString*/
	clearString(store_canData, MAX_LENGTH_CAN_DATA); /*clear the variable store_canData*/
	/*all parameter initialisieren*/
	Reset_Parameter_CANReceive();
}//END of Convert_pack_canFrame_to_UartFormat function

/*
 *this function deletes some variable
 *the  function has no input and output variable
 */

void Reset_Parameter_CANSend_Message( void )
{

	/* all parameter initialisieren */
	clearString(decrypt_uartString, BUFFER_SIZE);
	clearString(uart_message_string, BUFFER_SIZE);
	clearUartStruct(ptr_uartStruct);
	clearCanStruct(ptr_canStruct);

}//END of Reset_Parameter_CANSend_Message


/*
 *this function deletes some variable
 *the  function has no input and output parameters
 */

void Reset_Parameter_CANReceive( void )
{

	/* initialize all parameters of structure  */
	clearCanStruct(ptr_canStruct);
	clearUartStruct(ptr_uartStruct);
}//END of Reset_Parameter_CANReceive function


/*
 *this function deletes some variable
 *the  function has no input and output parameters
 */

void clearCanStruct( struct canStruct *ptr_canStruct)
{ /*resets the canStruct structure to "0" */
   if ( NULL != ptr_canStruct )
   {
      ptr_canStruct->Can_Length = 0;
      ptr_canStruct->Can_Mask = 0;
      ptr_canStruct->Can_Message_ID = 0;
      ptr_canStruct->Can_Mob = 0;
      for ( uint8_t clearIndex = 0 ; clearIndex < CAN_MAX_DATA_ELEMENTS ; clearIndex++ )
      {
         ptr_canStruct->Can_Data[clearIndex] = 0;
      }
   }
}//END of clearCanStruct

/*
 * this function calculates settings for the CanBus timing registers CANBTx
 * to achieve the given baudrate
 * return value:
 * 0: if ok
 * errors else
 * input values baudrate in kbps and i/o frequency Hz
 *
 * slow function, not to be used within fast code segments, eg. IRQ !
 * if freq == 0, assume F_CPU
 */

int setCanBaudRate( const uint32_t rate, const uint32_t freq )
{
	/*max data rate @ 8000 kHz 1Mbit*/

	switch( freq )
	{
	case 10000000UL: /* 10 MHz */
	{
		switch( rate )
		{
		case ONETHOUSAND_KBPS:
			CANBT1 = ( 0 << BRP0 ); /* 0x00 */
			CANBT2 = ( 2 << PRS0 ) | ( 1 << SJW0 ); /* 0x24 */
			CANBT3 = ( 1 << PHS20 ) | ( 1 << PHS10 ); /* 0x12 */
			break;
		case FIVEHUNDERT_KBPS:
			CANBT1 = ( 3 << BRP0 ); /* 0x06 */
			CANBT2 = ( 4 << PRS0 ) | ( 0 << SJW0 ); /* 0x08 */
			CANBT3 = ( 1 << PHS20 ) | ( 1 << PHS10 ) | ( 1 << SMP ); /* 0x13 */
			break;
		case TWOHUNDERTFIFTY_KBPS:
			CANBT1 = ( 3 << BRP0 ); /* 0x06 */
			CANBT2 = ( 4 << PRS0 ) | ( 0 << SJW0 ); /* 0x08 */

#warning @FLORIAN: what is the meaning for SMP, the code which has been used in experiment, did not set it
			/*CANBT3 = ( 1 << PHS20 ) | ( 1 << PHS10 ) | ( 1 << SMP );*/ /* 0x13 */
			CANBT3 = ( 1 << PHS20 ) | ( 1 << PHS10 );
			break;
		case ONEHUNDERTTWENTYFIVE_KBPS:
			// --- use 10 TQ (Swj = 1, Prs = 5, Phs1 = 2, Phs2 = 2)
			CANBT1 = ( 1 << BRP2 ) | ( 1 << BRP1 ) | ( 1 << BRP0 );
			CANBT2 = ( 1 << PRS2 );
			CANBT3 = ( 1 << PHS20 ) | ( 1 << PHS10 );
			break;
		case ONEHUNDERT_KBPS:
			CANBT1 = ( 7 << BRP0 ); /* 0x0e*/
			CANBT2 = ( 4 << PRS0 ) | ( 0 << SJW0 ); /* 0x08 */
			CANBT3 = ( 1 << PHS20 ) | ( 1 << PHS10 ) | ( 1 << SMP ) ; /* 0x13 */
			break;
		default:
			snprintf_P(message, BUFFER_SIZE, PSTR("not supported CAN Baudrate (%i) / CPU freq. (%i) combination"), rate, freq);
			CommunicationError_p(ERRC, -1, TRUE, message, COMMUNICATION_ERROR_USE_GLOBAL_MESSAGE_STRING_INDEX_THRESHOLD -1 );
			return -1;
			break;
		}
	}
	break;
	case 16000000UL: /* 16 MHz */
	{
		switch( rate )
		{
		case ONETHOUSAND_KBPS:
			CANBT1 = ( 1 << BRP0 );                                  /*0x02*/
			CANBT2 = ( 2 << PRS0 )  | ( 0 << SJW0 );                 /*0x04*/
			CANBT3 = ( 1 << PHS20 ) | ( 1 << PHS10 ) | ( 1 << SMP ); /*0x13*/
			break;
		case FIVEHUNDERT_KBPS:
			CANBT1 = ( 1 << BRP0 );                                  /*0x02*/
			CANBT2 = ( 6 << PRS0 )  | ( 0 << SJW0 );                 /*0x0C*/
			CANBT3 = ( 3 << PHS20 ) | ( 3 << PHS10 ) | ( 1 << SMP ); /*0x37*/
			break;
		case TWOHUNDERTFIFTY_KBPS:
			CANBT1 = ( 3 << BRP0 );                                  /*0x06*/
			CANBT2 = ( 6 << PRS0 )  | ( 0 << SJW0 );                 /*0x0C*/
			CANBT3 = ( 3 << PHS20 ) | ( 3 << PHS10 ) | ( 1 << SMP ); /*0x37*/
			break;
		case TWOHUNDERT_KBPS:
			CANBT1 = ( 4 << BRP0 );                                  /*0x08*/
			CANBT2 = ( 6 << PRS0 )  | ( 0 << SJW0 );                 /*0x0C*/
			CANBT3 = ( 3 << PHS20 ) | ( 3 << PHS10 ) | ( 1 << SMP ); /*0x37*/
			break;
		case ONEHUNDERTTWENTYFIVE_KBPS:
			CANBT1 = ( 7 << BRP0 );                                  /*0x0E*/
			CANBT2 = ( 6 << PRS0 )  | ( 0 << SJW0 );                 /*0x0C*/
			CANBT3 = ( 3 << PHS20 ) | ( 3 << PHS10 ) | ( 1 << SMP ); /*0x37*/
			break;
		case ONEHUNDERT_KBPS:
			CANBT1 = ( 9 << BRP0 );                                  /*0x12*/
			CANBT2 = ( 6 << PRS0 )  | ( 0 << SJW0 );                 /*0x0C*/
			CANBT3 = ( 3 << PHS20 ) | ( 3 << PHS10 ) | ( 1 << SMP ); /*0x37*/
			break;
		default:
			snprintf_P(message, BUFFER_SIZE, PSTR("not supported CAN Baudrate (%i) / CPU freq. (%i) combination"), rate, freq);
			CommunicationError_p(ERRC, -1, TRUE, message, COMMUNICATION_ERROR_USE_GLOBAL_MESSAGE_STRING_INDEX_THRESHOLD -1 );
			return -1;
			break;
		}
	}
	break;
	default:
		snprintf_P(message, BUFFER_SIZE, PSTR("not supported CAN Baudrate (%i) / CPU freq. (%i) combination"), rate, freq);
		CommunicationError_p(ERRC, -1, TRUE, message, COMMUNICATION_ERROR_USE_GLOBAL_MESSAGE_STRING_INDEX_THRESHOLD -1 );
		return -1;
		break;
	}
	return 0;
}

/*
 *this function initializes the CAN register for AT90CAN128 and enable the CAN-controller
 * the function has not input variable
 * the return value is an integer:
 * 1  -> the CAN initialization is successful
 * -1 -> the CAN initialization is unsuccessful
 */

int8_t CAN_Init( int32_t Baudrate )
{
	uint8_t intstate2 = SREG;/*save global interrupt flag*/
	/*disable interrupt*/
	cli();

	CANGCON |= ( 1 << SWRES );
	CANGCON &= ~( 1 << SWRES );

	/*enable general interrupt interrupt*/
	CANGIE = ( 1 << ENIT ) | ( 1 << ENBOFF ) | ( 1 << ENRX ) | ( 1 << ENTX ) | ( 1 << ENERR ) | ( 1 << ENBX ) | ( 1 << ENERG ) | ( 0 << ENOVRT );

	/* enable Interrupt of MOBs register*/
	CANIE2 = 0xFF;
	CANIE1 = 0x7F;

	/* clear (some) CAN registers */
	clearCanRegisters();

	/* set CAN baudrate register */
	if ( 0 < Baudrate)
	{
		if ( 0 != setCanBaudRate(Baudrate, F_CPU) )
		{
			SREG = intstate2; /*restore global interrupt flag*/
#warning Error handling/notify ?
			return -1;
		}
	}

	enableCan();
	SREG = intstate2; /*restore global interrupt flag*/
	return 1;

}// END of CAN_Init function

void enableCan()
{
    /* set ENA/STB enable mode  */
    CANGCON |= ( 1 << ENASTB );
    /*enable reception mode here so that information for CAN come automatically*/
    CANCDMOB |= ( 0 << CONMOB0 ) | ( 0 << CONMOB1 );
}

/*
 *this function deletes some CAN register for AT90CAN128
 * the function has no input and output parameters
 */

void clearCanRegisters( void )
{
    for ( uint8_t mob = 0 ; mob < 15 ; mob++ )
    {
       CANPAGE = ( mob << MOBNB0 ); /* clear all  mailbox*/
       CANSTMOB = 0x00; /*clear  MOB status register*/
       CANCDMOB = 0x00; /*clear MOB control register*/
       CANGSTA = 0x00; /*clear CAN general status register*/
       /*clear all identifier tag register*/
       CANIDT1 = 0X00;
       CANIDT2 = 0X00;
       CANIDT3 = 0X00;
       CANIDT4 = 0X00;
       /*clear all identifier mask register*/
       CANIDM1 = 0X00;
       CANIDM2 = 0X00;
       CANIDM3 = 0X00;
       CANIDM4 = 0X00;
       for ( unsigned char j = 0 ; j < 8 ; j++ )
       {
          CANMSG = 0; /*clear all CAN message register*/
       }
    }
}//END of clearCanRegisters function

/* this function gets the free communication channel
 * the function has no input parameter
 * and returns a integer value
 * freemob -> valid message object block
 * -1 -> no valid message object block
 */

int8_t Get_FreeMob( void )
{
	uint8_t ctrlReg;

	for ( uint8_t freemob = 2 ; freemob <= 14 ; freemob++ )
	{

		CANPAGE = freemob << 4;
		ctrlReg = ( CANCDMOB & ( ( 1 << CONMOB0 ) | ( 1 << CONMOB1 ) ) );
		if ( ctrlReg == 0 )
		{

			return freemob;
		}
	}
	return -1;

}//END of Get_FreeMob function


/*
 *this function gives the various CAN error on the channel
 *the function has no input and output parameters
 */

void Get_BusState( void )
{

	if ( CANGSTA & ( 1 << BOFF ) )
	{
		can_errorCode = CommunicationError(ERRC, CAN_ERROR_Can_Bus_is_off, 0, NULL, 0);
        //    CAN_Init(0); /* CAN reinit */
    }

	if ( CANGSTA & ( 1 << ERRP ) )
	{

		can_errorCode = CommunicationError(ERRC, CAN_ERROR_Can_Bus_is_passive, 0, NULL, 0);
        //    CAN_Init(0); /* CAN reinit*/
  }
}//END of Get_BusState function

/*
 *this function gives the various CAN errors
 *the function has no input and output parameters
 */
uint8_t Get_CanError( void )
{
	uint8_t rtn_val = 0;
	if ( CANSTMOB & ( 1 << BERR ) ) {
		CANSTMOB &= ~( 1 << BERR );
		can_errorCode = CommunicationError(ERRC, CAN_ERROR_Bit_Error, 0, NULL, 0);
		rtn_val = 1;
	}

	if ( CANSTMOB & ( 1 << SERR ) )
	{
		CANSTMOB &= ~( 1 << SERR );
		can_errorCode = CommunicationError(ERRC, CAN_ERROR_Stuff_Error, 0, NULL, 0);
		rtn_val = 1;
	}

	if ( CANSTMOB & ( 1 << CERR ) )
	{
		CANSTMOB &= ~( 1 << CERR );
		can_errorCode = CommunicationError(ERRC, CAN_ERROR_CRC_Error, 0, NULL, 0);
		rtn_val = 1;
	}

	if ( CANSTMOB & ( 1 << FERR ) )
	{
		CANSTMOB &= ~( 1 << FERR );
		can_errorCode = CommunicationError(ERRC, CAN_ERROR_Form_Error, 0, NULL, 0);
		rtn_val = 1;
	}

	if ( CANSTMOB & ( 1 << AERR ) )
	{
		CANSTMOB &= ~( 1 << AERR );
		can_errorCode = CommunicationError(ERRC, CAN_ERROR_Acknowdlegment_Error, 0, NULL, 0);
		rtn_val = 1;
	}
	return rtn_val;
}//END of Get_CanError
