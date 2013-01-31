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
         mailbox_errorCode = CommunicationError_p(ERRM, MOB_ERROR_this_message_already_exists, FALSE, NULL);
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
         mailbox_errorCode = CommunicationError_p(ERRM, MOB_ERROR_all_mailboxes_already_in_use, FALSE, NULL);
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
      mailbox_errorCode = CommunicationError_p(ERRM, MOB_ERROR_message_ID_not_found, FALSE, NULL);
   }
} //END of Unsubscribe_Message


/*
 * this function runs a command and expects no data
 *the function has a pointer of the serial structure as input and returns no parameter
 */

void Send_Message( struct uartStruct *ptr_uartStruct )
{

	CANPAGE = ( 1 << MOBNB0 );/*set channel number  */
	CANIDT4 = ( ptr_uartStruct->Uart_Rtr << RTRTAG ); /* enable remote transmission request */
	CANCDMOB = ( ptr_uartStruct->Uart_Length << DLC0 ); /* set length of data*/

	if ( ( MAX_ELF_BIT ) >= ( ptr_uartStruct->Uart_Message_ID ) )
	{
		CANCDMOB |= ( 0 << IDE ); /* enable CAN standard 11 bit */
		/*set Identifier to send */
		CANIDT2 = ( ( ptr_uartStruct->Uart_Message_ID ) & 0x7 ) << 5;
		CANIDT1 = ( ptr_uartStruct->Uart_Message_ID ) >> 3;
	}
	else
	{
#warning Is this correct? Look into Manual
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
	CANCDMOB |= ( 1 << CONMOB0 ); /*enable transmission mode*/
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
#warning ussleep needed?
	}
	if ( 0 >= can_timeout1 )
	{
		can_errorCode = CommunicationError_p(ERRC, CAN_ERROR_timeout_for_CAN_communication, FALSE, NULL);
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
		can_errorCode = CommunicationError_p(ERRC, CAN_ERROR_timeout_for_CAN_communication, FALSE, NULL);
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

/* setCanBitTimingTQUnits
 *
 * This functions sets/alters the can bit timing in units of the timing quanta TQ aka T_sql
 *
 * Based on a given integer F_CPU/Baudrate ratio two option are possible:
 * 		since N_TQ = F_CPU/Baudrate * 1/BitRatePrescaler
 * 			either the number of TQ is given and bitRateScaler<0
 * 				-> bit rate Scaler will be calculated
 * 			or the bit rate Scaler is given and number of TQ < 0
 * 				-> number of TQ will be calculated
 *
 * 	With those settings boundary and consistency checks are performed
 *  on the segment timings:
 *  	propagation time segment		: integer number of TQ
 *  	phase 1 segment               	: ""
 *  	phase 2 segment					: ""
 *  	optional, if > 0              	: ""
 *  		synchronization jump width	: ""
 *
 *  the flags:
 *  	multiple sample point sampling 	 		: TRUE/FALSE,
 *  		enables triple sampling before the sampling point
 *  	auto correct baud rate prescaler null	: TRUE/FALSE,
 *  		if baud rate prescaler = 1, phases1/2 have to be adopted +/-,
 *  		enables automatic correction
 *
 *  return values:
 *  	0	: everything OK
 *  	>0	: else
 */
uint8_t setCanBitTimingTQUnits(uint8_t numberOfTimeQuanta, uint16_t freq2BaudRatio, int8_t bitRatePreScaler,
		                       uint8_t propagationTimeSegment, uint8_t phaseSegment1, uint8_t phaseSegment2, uint8_t syncJumpWidth,
				               uint8_t multipleSamplePointSampling_flag, uint8_t autoCorrectBaudRatePreScalerNull_flag)
{
    int8_t tBitTime = 0;
    int8_t a = 0;
	// checks
    // numberOfTimeQuanta < 0 xor bitRateScaler < 0
    if (!(( 0 < numberOfTimeQuanta ) ^ ( 0 < bitRatePreScaler)))
    {
		CommunicationError_p(ERRC, dynamicMessage_ErrorIndex, FALSE,
				PSTR("setCanBitTiming: no. of Time Quanta (%i) and bit rate scaler (%i) are both (un)set"),
				numberOfTimeQuanta, bitRatePreScaler);
		return 1;
    }

    if ( 0 != canBitTimingTQBasicBoundaryChecks(propagationTimeSegment, phaseSegment1, phaseSegment2, syncJumpWidth))
    {
		return 1;
    }

	// - resulting T_bit [8,25] in units of TQ
    tBitTime = 0;
    tBitTime += CAN_BIT_TIMING_SYNCHRONIZATION_SEGMENT_TIME;
    tBitTime += propagationTimeSegment;
    tBitTime += phaseSegment1;
    tBitTime += phaseSegment2;

    // calculate bitRateScaler or numberOfTimeQuanta
    if ( 0 > bitRatePreScaler )
	{
		// calculate integer bitRatePreScaler without using division
		// replacing bitRatePreScaler = (freq2BaudRatio/numberOfTimeQuanta);

		a = freq2BaudRatio;
		bitRatePreScaler=0;
		while ( a >= numberOfTimeQuanta )
		{
			a -= numberOfTimeQuanta;
			bitRatePreScaler++;
		}
		printDebug_p(debugLevelEventDebug, debugSystemCAN, __LINE__, PSTR(__FILE__),
				PSTR("setCanBitTiming: bit rate pre scaler calculated: %i"),
				bitRatePreScaler);
	}

    if ( 0 > numberOfTimeQuanta)
    {
//    	numberOfTimeQuanta = freq2BaudRatio / bitRatePreScaler;
		a = freq2BaudRatio;
		numberOfTimeQuanta=0;
		while ( a >= bitRatePreScaler )
		{
			a -= bitRatePreScaler;
			numberOfTimeQuanta++;
		}
		printDebug_p(debugLevelEventDebug, debugSystemCAN, __LINE__, PSTR(__FILE__),
				PSTR("setCanBitTiming: no. of Time Quanta calculated: %i"),
				numberOfTimeQuanta);
    }

	// - N_TQ [8,25]
    if (0 < numberOfTimeQuanta)
    {
    	if ( 8 > numberOfTimeQuanta || 25 < numberOfTimeQuanta)
    	{
    		printDebug_p(debugLevelEventDebug, debugSystemCAN, __LINE__, PSTR(__FILE__),
    				PSTR("setCanBitTiming: no. of Time Quanta (%i) out of [8,25]"),
    				numberOfTimeQuanta);
    		CommunicationError_p(ERRC, dynamicMessage_ErrorIndex, FALSE,
    				PSTR("setCanBitTiming: no. of Time Quanta (%i) out of [8,25]"),
    				numberOfTimeQuanta);
    		return 1;
    	}
	}

    // bitRatePreScaler [1,64]
	if ( CAN_BIT_TIMING_BIT_RATE_PRESCALER_MIN > bitRatePreScaler || CAN_BIT_TIMING_BIT_RATE_PRESCALER_MAX < bitRatePreScaler)
	{
		CommunicationError_p(ERRC, dynamicMessage_ErrorIndex, FALSE,
				PSTR("setCanBitTiming: bit rate pre scaler (%i) out of [%i,%i]"),
				CAN_BIT_TIMING_BIT_RATE_PRESCALER_MIN, CAN_BIT_TIMING_BIT_RATE_PRESCALER_MAX, bitRatePreScaler);
		return 1;
	}

    // bitRatePreScaler == 1 [BRP = 0]
	// -- swj not allowed
	// -- multipleSamplePointSampling
	// -- autoCorrectBaudRatePreScalerNull_flag TRUE boundary check on phase segments

	if ( 1 == bitRatePreScaler )
	{
	    // -- swj not allowed
		if ( 0 < syncJumpWidth )
		{
			CommunicationError_p(ERRC, dynamicMessage_ErrorIndex, FALSE,
					PSTR("setCanBitTiming: bit rate pre scaler (%i) forbids sync jump width (%i) > 0"),
					bitRatePreScaler, syncJumpWidth);
			return 1;
		}
		// -- multipleSamplePointSampling
		if ( FALSE != multipleSamplePointSampling_flag )
		{
			CommunicationError_p(ERRC, dynamicMessage_ErrorIndex, FALSE,
					PSTR("setCanBitTiming: bit rate pre scaler (%i) forbids multiple sample points"),
					bitRatePreScaler);
			return 1;
		}

		// apply corrections PHS1 + 1 TQ and PHS2 - 1 TQ
		if ( FALSE != autoCorrectBaudRatePreScalerNull_flag)
		{
			printDebug_p(debugLevelEventDebug, debugSystemCAN, __LINE__, PSTR(__FILE__),
					PSTR("setCanBitTiming: bit rate pre scaler (%i) auto corrected phase 1/2: %i/%i"),
					bitRatePreScaler, phaseSegment1, phaseSegment2);
			phaseSegment1++;
			phaseSegment2--;
		}

		if ( 0 != canBitTimingTQBasicBoundaryChecks(propagationTimeSegment, phaseSegment1, phaseSegment2, syncJumpWidth))
	    {
			return 1;
	    }

	}

	CANBT1 = ( (  (bitRatePreScaler - 1) << BRP0) & 0xFF );
	CANBT2 = ( ( ((propagationTimeSegment -1 ) << PRS0) | (( syncJumpWidth -1 ) << SJW0) ) & 0xFF );
	CANBT3 = ( ( (( phaseSegment2 -1 ) << PHS20) | (( phaseSegment1 -1 ) << PHS10) | ( ((FALSE != multipleSamplePointSampling_flag)?1:0) << SMP)) && 0xFF );

	printDebug_p(debugLevelEventDebug, debugSystemCAN, __LINE__, PSTR(__FILE__),
			PSTR("setCanBitTiming: register CANBT1/2/3: %x / %x / %x"), CANBT1, CANBT2, CANBT3);

	return 0;
}

/* performs a boundary check on the basic bit timing times in units of the time quanta TQ
 * input: in units of TQ
 * 	propagationTimeSegments
 * 	phaseSegment1
 * 	phaseSegment2
 * 	syncJumpWidth
 *
 * returns:
 * 	0 ok
 * 	1 else
 */

uint8_t canBitTimingTQBasicBoundaryChecks(uint8_t propagationTimeSegment, uint8_t phaseSegment1, uint8_t phaseSegment2, uint8_t syncJumpWidth)
{
	// - propagation segment time [1,8]
	if ( CAN_BIT_TIMING_PROPAGATION_SEGMENT_TIME_MIN > propagationTimeSegment ||
		 CAN_BIT_TIMING_PROPAGATION_SEGMENT_TIME_MAX < propagationTimeSegment)
	{
		CommunicationError_p(ERRC, dynamicMessage_ErrorIndex, FALSE,
				PSTR("setCanBitTiming: propagation time segment (%i TQ) out of [%i,%i]"),
				propagationTimeSegment, CAN_BIT_TIMING_PROPAGATION_SEGMENT_TIME_MIN, CAN_BIT_TIMING_PROPAGATION_SEGMENT_TIME_MAX);
		return 1;
	}

	// - phase segment 1 PHYS1 [1,8]
	if ( CAN_BIT_TIMING_PHASE_SEGMENT_1_TIME_MIN > phaseSegment1 || CAN_BIT_TIMING_PHASE_SEGMENT_1_TIME_MAX < phaseSegment1)
	{
		CommunicationError_p(ERRC, dynamicMessage_ErrorIndex, FALSE,
				PSTR("setCanBitTiming: phase segment 1 (%i TQ) out of [%i,%i]"),
				phaseSegment1, CAN_BIT_TIMING_PHASE_SEGMENT_1_TIME_MIN, CAN_BIT_TIMING_PHASE_SEGMENT_1_TIME_MAX);
		return 1;
	}

	// - phase segment 2 PHYS2 [INFORMATION PROCESSING TIME, PHS1]
	if ( CAN_BIT_TIMING_INFORMATION_PROCESSING_TIME > phaseSegment2 ||
			phaseSegment1 < phaseSegment2)
	{
		CommunicationError_p(ERRC, dynamicMessage_ErrorIndex, FALSE,
				PSTR("setCanBitTiming: phase segment 2 (%i TQ) out of [%i,%i]"),
				phaseSegment2, CAN_BIT_TIMING_INFORMATION_PROCESSING_TIME, phaseSegment1);
		return 1;
	}

	// - sync jump width 0 or [1,min(4, phase segment 1 ] <= PHS2
	if ( 0 < syncJumpWidth )
	{
		if ( CAN_BIT_TIMING_SYNC_JUMP_WIDTH_TIME_MIN > syncJumpWidth ||
			 min(CAN_BIT_TIMING_SYNC_JUMP_WIDTH_TIME_MAX, phaseSegment1) < syncJumpWidth ||
			 phaseSegment2 < syncJumpWidth)
		{
			CommunicationError_p(ERRC, dynamicMessage_ErrorIndex, FALSE,
					PSTR(__FILE__), PSTR("setCanBitTiming: sync jump width (%i TQ) out of [%i,%i]"),
					syncJumpWidth, min(CAN_BIT_TIMING_SYNC_JUMP_WIDTH_TIME_MIN, min(phaseSegment1,phaseSegment2)));
			return 1;
		}
	}
	return 0;
}

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

	uint16_t freq2BbaudRatio = 0;
    uint8_t result = 0;
	switch( freq )
	{
	case 10000000UL: /* 10 MHz */
	{
		switch( rate )
		{
		case ONETHOUSAND_KBPS:
			freq2BbaudRatio = 10;
			CANBT1 = ( 0 << BRP0 ); /* 0x00 */
			CANBT2 = ( 2 << PRS0 ) | ( 1 << SJW0 ); /* 0x24 */
			CANBT3 = ( 1 << PHS20 ) | ( 1 << PHS10 ); /* 0x12 */
			break;
		case FIVEHUNDERT_KBPS:
			freq2BbaudRatio = 20;
			CANBT1 = ( 3 << BRP0 ); /* 0x06 */
			CANBT2 = ( 4 << PRS0 ) | ( 0 << SJW0 ); /* 0x08 */
			CANBT3 = ( 1 << PHS20 ) | ( 1 << PHS10 ) | ( 1 << SMP ); /* 0x13 */
			break;
		case TWOHUNDERTFIFTY_KBPS:
			freq2BbaudRatio = 40;
			CANBT1 = ( 3 << BRP0 ); /* 0x06 */
			CANBT2 = ( 4 << PRS0 ) | ( 0 << SJW0 ); /* 0x08 */
			/*CANBT3 = ( 1 << PHS20 ) | ( 1 << PHS10 ) | ( 1 << SMP );*/ /* 0x13 */
			CANBT3 = ( 1 << PHS20 ) | ( 1 << PHS10 );
			break;
		case ONEHUNDERTTWENTYFIVE_KBPS:
#define ONEHUNDERTTWENTYFIVE_NUMBER_OF_TQ 10
			freq2BbaudRatio = 80;
			switch (ONEHUNDERTTWENTYFIVE_NUMBER_OF_TQ)
			{
			case 20:
				break;
			case 16:
				break;
			case 10:
				// --- use 10 TQ (Prs = 5, Phs1 = 2, Phs2 = 2, Swj = 2, sample Points)
				result = setCanBitTimingTQUnits(10, freq2BbaudRatio, -1, 5, 2, 2, 2, TRUE, TRUE);
				break;
			case 8:
				break;
			}
			break;
		case ONEHUNDERT_KBPS:
			freq2BbaudRatio = 100;
			CANBT1 = ( 7 << BRP0 ); /* 0x0e*/
			CANBT2 = ( 4 << PRS0 ) | ( 0 << SJW0 ); /* 0x08 */
			CANBT3 = ( 1 << PHS20 ) | ( 1 << PHS10 ) | ( 1 << SMP ) ; /* 0x13 */
			break;
		default:
			CommunicationError_p(ERRC, dynamicMessage_ErrorIndex, TRUE, PSTR("not supported CAN Baudrate (%i) / CPU freq. (%i) combination"), rate, freq);
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
			freq2BbaudRatio = 16;
			CANBT1 = ( 1 << BRP0 );                                  /*0x02*/
			CANBT2 = ( 2 << PRS0 )  | ( 0 << SJW0 );                 /*0x04*/
			CANBT3 = ( 1 << PHS20 ) | ( 1 << PHS10 ) | ( 1 << SMP ); /*0x13*/
			break;
		case FIVEHUNDERT_KBPS:
			freq2BbaudRatio = 32;
			CANBT1 = ( 1 << BRP0 );                                  /*0x02*/
			CANBT2 = ( 6 << PRS0 )  | ( 0 << SJW0 );                 /*0x0C*/
			CANBT3 = ( 3 << PHS20 ) | ( 3 << PHS10 ) | ( 1 << SMP ); /*0x37*/
			break;
		case TWOHUNDERTFIFTY_KBPS:
			freq2BbaudRatio = 64;
			CANBT1 = ( 3 << BRP0 );                                  /*0x06*/
			CANBT2 = ( 6 << PRS0 )  | ( 0 << SJW0 );                 /*0x0C*/
			CANBT3 = ( 3 << PHS20 ) | ( 3 << PHS10 ) | ( 1 << SMP ); /*0x37*/
			break;
		case TWOHUNDERT_KBPS:
			freq2BbaudRatio = 80;
			CANBT1 = ( 4 << BRP0 );                                  /*0x08*/
			CANBT2 = ( 6 << PRS0 )  | ( 0 << SJW0 );                 /*0x0C*/
			CANBT3 = ( 3 << PHS20 ) | ( 3 << PHS10 ) | ( 1 << SMP ); /*0x37*/
			break;
		case ONEHUNDERTTWENTYFIVE_KBPS:
			freq2BbaudRatio = 128;
			CANBT1 = ( 7 << BRP0 );                                  /*0x0E*/
			CANBT2 = ( 6 << PRS0 )  | ( 0 << SJW0 );                 /*0x0C*/
			CANBT3 = ( 3 << PHS20 ) | ( 3 << PHS10 ) | ( 1 << SMP ); /*0x37*/
			break;
		case ONEHUNDERT_KBPS:
			freq2BbaudRatio = 160;
			CANBT1 = ( 9 << BRP0 );                                  /*0x12*/
			CANBT2 = ( 6 << PRS0 )  | ( 0 << SJW0 );                 /*0x0C*/
			CANBT3 = ( 3 << PHS20 ) | ( 3 << PHS10 ) | ( 1 << SMP ); /*0x37*/
			break;
		default:
			CommunicationError_p(ERRC, dynamicMessage_ErrorIndex, TRUE, PSTR("not supported CAN Baudrate (%i) / CPU freq. (%i) combination"), rate, freq);
			return -1;
			break;
		}
	}
	break;
	default:
		CommunicationError_p(ERRC, dynamicMessage_ErrorIndex, TRUE, PSTR("not supported CAN Baudrate (%i) / CPU freq. (%i) combination"), rate, freq);
		return -1;
		break;
	}

	if (0 != result)
	{
		CommunicationError_p(ERRC, dynamicMessage_ErrorIndex, TRUE, PSTR("CAN bit timing failed"), rate, freq);
		return -1;
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
		can_errorCode = CommunicationError_p(ERRC, CAN_ERROR_Can_Bus_is_off, FALSE, NULL);
        //    CAN_Init(0); /* CAN reinit */
    }

	if ( CANGSTA & ( 1 << ERRP ) )
	{
		can_errorCode = CommunicationError_p(ERRC, CAN_ERROR_Can_Bus_is_passive, FALSE, NULL);
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
		can_errorCode = CommunicationError_p(ERRC, CAN_ERROR_Bit_Error, FALSE, NULL);
		rtn_val = 1;
	}

	if ( CANSTMOB & ( 1 << SERR ) )
	{
		CANSTMOB &= ~( 1 << SERR );
		can_errorCode = CommunicationError_p(ERRC, CAN_ERROR_Stuff_Error, FALSE, NULL);
		rtn_val = 1;
	}

	if ( CANSTMOB & ( 1 << CERR ) )
	{
		CANSTMOB &= ~( 1 << CERR );
		can_errorCode = CommunicationError_p(ERRC, CAN_ERROR_CRC_Error, FALSE, NULL);
		rtn_val = 1;
	}

	if ( CANSTMOB & ( 1 << FERR ) )
	{
		CANSTMOB &= ~( 1 << FERR );
		can_errorCode = CommunicationError_p(ERRC, CAN_ERROR_Form_Error, FALSE, NULL);
		rtn_val = 1;
	}

	if ( CANSTMOB & ( 1 << AERR ) )
	{
		CANSTMOB &= ~( 1 << AERR );
		can_errorCode = CommunicationError_p(ERRC, CAN_ERROR_Acknowledgement_Error, FALSE, NULL);
		rtn_val = 1;
	}
	return rtn_val;
}//END of Get_CanError
