/*
 * VERSION 1.0 Januar 7th 2010 LATE  File: 'can.c'
 * Author: Linda Fouedjio
 * modified (heavily rather rebuild): Peter Zumbruch
 * modified: Florian Feldbauer
 * modified: Peter Zumbruch, July 2011
 * modified: Peter Zumbruch, April/May 2013
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

static const char filename[] 		                              PROGMEM = __FILE__;
static const char string_setCanBitTimingColon[]                   PROGMEM = "setCanBitTiming:";
static const char string_bit_rate_pre_scaler[]                    PROGMEM = "bit rate pre scaler";
static const char string_ignoring_TX_calls_for_maxDot_Dot1f_s[]   PROGMEM = "ignoring TX calls for max. %.1f s";
static const char string_selecting_mob_i_oBraceSRTRcBrace[]       PROGMEM = "selecting mob %i (%SRTR)";

volatile unsigned char canUseOldRecvMessage_flag;
volatile unsigned char canUseNewRecvMessage_flag;


/*variable to subscribe and unsubscribe some messages via CAN bus */
uint32_t subscribe_ID[MAX_LENGTH_SUBSCRIBE];
uint32_t subscribe_mask[MAX_LENGTH_SUBSCRIBE];

/* pointer of array for defined can error number*/

static const char ce00[] PROGMEM = "Bus Off Mode";
static const char ce01[] PROGMEM = "Bus Off Mode, BOFFIT interrupt";
static const char ce02[] PROGMEM = "Error Passive Mode";
static const char ce03[] PROGMEM = "CAN controller disabled";
static const char ce04[] PROGMEM = "Bit Error";
static const char ce05[] PROGMEM = "Stuff Error";
static const char ce06[] PROGMEM = "CRC Error";
static const char ce07[] PROGMEM = "Form Error";
static const char ce08[] PROGMEM = "Acknowledgment Error";
static const char ce09[] PROGMEM = "CAN was not successfully initialized";
static const char ce10[] PROGMEM = "CAN communication timeout";
static const char ce11[] PROGMEM = "Stuff Error General";
static const char ce12[] PROGMEM = "CRC Error General";
static const char ce13[] PROGMEM = "Form Error General";
static const char ce14[] PROGMEM = "Acknowledgment Error General";

const char *can_error[] PROGMEM = { ce00, ce01, ce02, ce03, ce04, ce05, ce06, ce07, ce08, ce09, ce10, ce11, ce12, ce13, ce14 };

/* pointer of array for defined mailbox error */

static const char me00[] PROGMEM = "all mailboxes already in use";
static const char me01[] PROGMEM = "message ID not found";
static const char me02[] PROGMEM = "this message already exists";

const char *mob_error[] PROGMEM = { me00, me01, me02 };

static const char cb00[] PROGMEM = "error active";
static const char cb01[] PROGMEM = "error passive";
static const char cb02[] PROGMEM = "bus off";
static const char cb03[] PROGMEM = "can disabled";
static const char cb04[] PROGMEM = "undefined";

const char *canBusModes[] PROGMEM = { cb00, cb01, cb02, cb03, cb04 };

/*
 * canSubscribeMessage creates/sets a listener to an ID/mask for a free MOb
 * the function has a pointer of the serial structure as input and returns no parameter
 */

#warning TODO: CAN define operation to set timer prescaler
#warning TODO: CAN define operation to time stamping

void canSubscribeMessage( struct uartStruct *ptr_uartStruct )
{
   int8_t findMob;
   uint8_t equality = 1;
#warning TODO: CAN report success ?
   for ( uint8_t count_subscribe = 2 ; count_subscribe <= 14 ; count_subscribe++ )
   {
      if ( ( ptr_uartStruct->Uart_Message_ID ) == ( subscribe_ID[count_subscribe] ) && ( ptr_uartStruct->Uart_Mask ) == ( subscribe_mask[count_subscribe] ) )
      {
         equality = 0;
         mobErrorCode = CommunicationError_p(ERRM, MOB_ERROR_this_message_already_exists, FALSE, NULL);
      }
   }
   if ( 1 == equality )
   {
      subscribe_ID[ptr_subscribe] = ( ptr_uartStruct->Uart_Message_ID );
      subscribe_mask[ptr_subscribe] = ( ptr_uartStruct->Uart_Mask );
      ptr_subscribe++;

      findMob = canGetFreeMob();

      if ( 14 < ptr_subscribe )
      {
         ptr_subscribe = 2;
      }

      if ( ( -1 ) == findMob )
      {
         mobErrorCode = CommunicationError_p(ERRM, MOB_ERROR_all_mailboxes_already_in_use, FALSE, NULL);
      }
      else
      {
         CANPAGE = ( findMob << MOBNB0 );
         CANSTMOB = 0x00; /* cancel pending operation */
         CANCDMOB = 0x00;
         CANHPMOB = 0x00; /* enable direct canMob indexing */

         canSetMObCanIDandMask(ptr_uartStruct->Uart_Message_ID, ptr_uartStruct->Uart_Mask, TRUE, FALSE);

         CANCDMOB = ( 1 << CONMOB1 ); /* enable reception mode */

         uint16_t mask = 1 << findMob;
         CANIE2 |= mask;
         CANIE1 |= ( mask >> 8 );
      }
   }
}//END of canSubscribeMessage function

/*
 * canUnsubscribeMessage removes a listener for an ID/mask
 * the function has a pointer of the serial structure as input and returns no parameter
 */

void canUnsubscribeMessage( struct uartStruct *ptr_uartStruct )
{
#warning TODO: CAN report success ?
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

         canSetMObCanIDandMask(0x0, 0x0, FALSE, FALSE);

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
      mobErrorCode = CommunicationError_p(ERRM, MOB_ERROR_message_ID_not_found, FALSE, NULL);
   }
} //END of canUnsubscribeMessage


/*
 * this function runs a command and might expect (RTR=1) data
 * the function has a pointer of the serial structure as input and returns no parameter
 */

void canSendMessage( struct uartStruct *ptr_uartStruct )
{
	uint32_t id     = ptr_uartStruct->Uart_Message_ID;
	uint32_t mask   = ptr_uartStruct->Uart_Mask;
	uint8_t  rtr    = ptr_uartStruct->Uart_Rtr;
	uint8_t  length = ptr_uartStruct->Uart_Length;

	static uint8_t currentBusModeStatus = canChannelMode_UNDEFINED;

	currentBusModeStatus = canGetCurrentBusModeStatus(); // output: CAN_DISABLED, BUS_OFF, ERROR_PASSIVE, ERROR_ACTIVE

	printDebug_p(debugLevelEventDebug, debugSystemCAN, __LINE__, filename,
			PSTR("bus mode: %S, bus state %S, changed: %S"),
			(const char*) (pgm_read_word( &(canBusModes[currentBusModeStatus]))),
			(const char*) (pgm_read_word( &(canBusModes[canBusStoredState]))),
			(currentBusModeStatus != canBusStoredState) ? PSTR("yes") : PSTR("no"));

	/*coming from undefined and having currently error status, give it a try */
	if (canChannelMode_UNDEFINED == canBusStoredState)
	{
		switch (currentBusModeStatus)
		{
			case canChannelMode_ERROR_ACTIVE:
				break;
			case canChannelMode_CAN_DISABLED:
			case canChannelMode_BUS_OFF:
			case canChannelMode_ERROR_PASSIVE:
				currentBusModeStatus = canChannelMode_ERROR_ACTIVE;

				printDebug_p(debugLevelEventDebug, debugSystemCAN, __LINE__, filename,
						PSTR("coming from: %S, try to set bus state back to %S"),
						(const char*) (pgm_read_word( &(canBusModes[canBusStoredState   ]))),
						(const char*) (pgm_read_word( &(canBusModes[currentBusModeStatus])))
						);
				break;
			default:
				break;
		}
	}

	switch (currentBusModeStatus)
	{
		case canChannelMode_CAN_DISABLED:
		case canChannelMode_BUS_OFF:
		case canChannelMode_ERROR_PASSIVE:
		{
			switch (currentBusModeStatus) /* mind the order */
			{
			case canChannelMode_CAN_DISABLED:
				canErrorCode = CAN_ERROR_Can_Controller_disabled;
				break;
			case canChannelMode_BUS_OFF:
				canErrorCode = CAN_ERROR_Bus_Off_Mode;
				break;
			case canChannelMode_ERROR_PASSIVE:
				canErrorCode = CAN_ERROR_Error_Passive_Mode;
				break;
			}

			/* only on change or in debug*/
			if ( debugLevelVerboseDebug > globalDebugLevel )
			{
				if (currentBusModeStatus != canBusStoredState)
				{
					CommunicationError_p(ERRC, canErrorCode, TRUE,
								         PSTR("%S, no further notification"),
								         string_ignoring_TX_calls_for_maxDot_Dot1f_s,
								         canBusStateResetInterval_seconds);
					canBusStoredState = currentBusModeStatus;

				}
			}
			else
			{
					CommunicationError_p(ERRC, canErrorCode, TRUE,
							             PSTR("%S"),
								         string_ignoring_TX_calls_for_maxDot_Dot1f_s,
							             canBusStateResetInterval_seconds);
			}
		}
		break;
		case canChannelMode_ERROR_ACTIVE:
		{
			printDebug_p(debugLevelEventDebug, debugSystemCAN, __LINE__, filename,
					PSTR("preparation of send message: id %x mask %x RTR %x length %x"), id, mask, rtr, length);

			if (0 == rtr)
			{
				/* set channel number to 1 */
				CANPAGE = (0x1 << MOBNB0);
				printDebug_p(debugLevelEventDebug, debugSystemCAN, __LINE__, filename,
						string_selecting_mob_i_oBraceSRTRcBrace, 0, PSTR("no"));
			}
			else /*RTR*/
			{
				/* set channel number to 0 */
				CANPAGE = (0 << MOBNB0);
				printDebug_p(debugLevelEventDebug, debugSystemCAN, __LINE__, filename,
						string_selecting_mob_i_oBraceSRTRcBrace, 1, PSTR(""));
			}

			CANSTMOB = 0x00; /* cancel pending operation */
			CANCDMOB = 0x00; /* disable communication of current MOb */

			/* set remote transmission request bit */
			CANIDT4 = ( rtr << RTRTAG );

			/* set length of data*/
			CANCDMOB = ( length << DLC0 );

			/*set ID / MASK*/

			if ( MAX_ELEVEN_BIT >= id && MAX_ELEVEN_BIT >= mask)
			{
				/* enable CAN standard 11 bit */
				CANCDMOB &= ~( 1 << IDE );
				if (0 == rtr)
				{
					canSetMObCanIDandMask(id, mask, FALSE, FALSE);
				}
				else /*RTR*/
				{
					// set ID to send
					// set mask to receive only this message */
					canSetMObCanIDandMask(id, 0 != mask ? mask : 0x000007FF, TRUE, TRUE);
				}
			}
			else
			{
				/* enable CAN standard 29 bit */
				CANCDMOB |= ( 1 << IDE );
				if (0 == rtr )
				{
					canSetMObCanIDandMask(id, mask, FALSE, FALSE);
				}
				else /*RTR*/
				{
					// set ID to send
					// set mask to receive only this message */
					canSetMObCanIDandMask(id, 0 != mask ? mask : 0x1FFFFFFF, TRUE, TRUE);
				}
			}

			if (0 == rtr)
			{
				/*put data in mailbox*/
				for (uint8_t count_data = 0; count_data < length; count_data++)
				{
					CANMSG = ptr_uartStruct->Uart_Data[count_data];
				}
			}

			/*reset CAN MOB Status Register */
			CANSTMOB = 0x00;
			/*disable any communication mode*/
			CANCDMOB &= ~( 1 << CONMOB1 | 1 << CONMOB0 );
			/*enable transmission mode*/
			CANCDMOB |= ( 1 << CONMOB0 );

			printDebug_p(debugLevelEventDebug, debugSystemCAN, __LINE__, filename, PSTR("waiting for send message to finish"));

			/*call the function to verify that the sending (and receiving) of data is complete*/
			if ( 0 == rtr )
			{
				canWaitForCanSendMessageFinished();
			}
			else /*RTR*/
			{
				canWaitForCanSendRemoteTransmissionRequestMessageFinished();
			}
			break;
		}
		default:
			CommunicationError_p(ERRC, -1, TRUE, PSTR("undefined bus state"));
			break;
	}
}//END of canSendMessage function

/*
 *this function wait until the command is sent
 *the function has no input and output parameter
 */
void canWaitForCanSendMessageFinished( void )
{
#warning CAN: isn't TXOK an interrupt which could be waited for in ?'
	uint32_t canTimeoutCounter = CAN_TIMEOUT_US; /*Timeout for CAN-communication*/
	while ( !( CANSTMOB & ( 1 << TXOK ) ) && ( --canTimeoutCounter > 0 ) )
	{
		_delay_us(1);
	}
	/* clear transmit OK flag */
	CANSTMOB &= ~( 1 << TXOK );
	/* disable communication */
	CANCDMOB &= ~( 1 << CONMOB1 | 1 << CONMOB0);

	if ( 0 == canTimeoutCounter )
	{
		/*reset time out counter */
		canTimeoutCounter = CAN_TIMEOUT_US;

		/* timeout */
		canErrorCode = CAN_ERROR_CAN_communication_timeout;
		CommunicationError_p(ERRC, canErrorCode, TRUE, PSTR("(%i us)"), CAN_TIMEOUT_US);
		printDebug_p(debugLevelEventDebugVerbose, debugSystemCAN, __LINE__, filename, PSTR("send w/o RTR: timeout"));
	}
	else
	{
		/* give feedback for successful transmit */
		if ( debugLevelVerboseDebug <= globalDebugLevel && ( ( globalDebugSystemMask >> debugSystemCAN ) & 0x1 ) )
		{
			strncat_P(uart_message_string, (const char*) ( pgm_read_word( &(responseKeywords[responseKeyNumber_RECV])) ), BUFFER_SIZE - 1);
			strncat_P(uart_message_string, PSTR(CAN_READY), BUFFER_SIZE - 1 );
			UART0_Send_Message_String_p(NULL,0);
		}
	}

	/* all parameter initializing */
	printDebug_p(debugLevelEventDebugVerbose, debugSystemCAN, __LINE__, filename, PSTR("resetting CAN send parameters"));
	canResetParametersCANSend();

}//END of canWaitForCanSendMessageFinished

/*
 *this function wait until the command in case the request is sent
 *the function has no input and output parameter
 */

void canWaitForCanSendRemoteTransmissionRequestMessageFinished( void )
{
	uint32_t canTimeoutCounter = CAN_TIMEOUT_US; /*Timeout for CAN-communication*/
	while ( !( CANSTMOB & ( 1 << RXOK ) ) && ( --canTimeoutCounter > 0 ) )
	{
		_delay_us(1);
	}
#warning CAN TODO: compare behavior to canWaitForCanSendMessageFinished
	if ( ( 0 == canTimeoutCounter ) && ( CANSTMOB & ( 1 << TXOK ) ) )
	{
		canErrorCode = CAN_ERROR_CAN_communication_timeout;
		CommunicationError_p(ERRC, canErrorCode, TRUE, PSTR("(%i us)"), CAN_TIMEOUT_US);
		printDebug_p(debugLevelEventDebugVerbose, debugSystemCAN, __LINE__, filename, PSTR("send w/ RTR: timeout"));
		canTimeoutCounter = CAN_TIMEOUT_US;
	}
	else
	{
		CANSTMOB &= ~( 1 << TXOK ); /* reset transmission flag */
		/* disable communication */
		CANCDMOB &= ~( 1 << CONMOB1 | 1 << CONMOB0);
	}

	/* reset all send parameters */
	canResetParametersCANSend();

}//END of canWaitForCanSendRemoteTransmissionRequestMessageFinished function

/*
 *This function grabs the CAN data received in a string
 *the function has a pointer of the CAN structure as input and returns no parameter
 */

void canConvertCanFrameToUartFormat( struct canStruct *ptr_canStruct )
{

#warning TODO RECV messages twice with and without obsolete mob. Which Keyword?

	if (TRUE == canUseOldRecvMessage_flag)
	{
		clearString(message, BUFFER_SIZE);

		strncat_P(message, (const char*) ( pgm_read_word( &(responseKeywords[responseKeyNumber_RECV])) ), BUFFER_SIZE - 1);
		snprintf_P(message, BUFFER_SIZE - 1, PSTR("%s %x %x %x"),message, ptr_canStruct->mob, ptr_canStruct->id, ptr_canStruct->length );
		for ( uint8_t dataIndex = 0 ; dataIndex < ptr_canStruct->length && dataIndex < CAN_MAX_DATA_ELEMENTS ; dataIndex++ )
		{
			snprintf_P(message, BUFFER_SIZE - 1, PSTR("%s %x"),message, ptr_canStruct->data[dataIndex] );
		}

		UART0_Send_Message_String_p(message,0);

		clearString(message, BUFFER_SIZE);
	}

	if (TRUE == canUseNewRecvMessage_flag)
	{
		clearString(message, BUFFER_SIZE);

		strncat_P(message, (const char*) ( pgm_read_word( &(responseKeywords[responseKeyNumber_RECV])) ), BUFFER_SIZE - 1);
		strncat_P(message, PSTR(" "), BUFFER_SIZE - 1);
		strncat_P(message, (const char*) ( pgm_read_word( &(responseKeywords[responseKeyNumber_CANR])) ), BUFFER_SIZE - 1);
		snprintf_P(message, BUFFER_SIZE - 1, PSTR("%s %x %x"),message, ptr_canStruct->id, ptr_canStruct->length );
		for ( uint8_t dataIndex = 0 ; dataIndex < ptr_canStruct->length && dataIndex < CAN_MAX_DATA_ELEMENTS ; dataIndex++ )
		{
			snprintf_P(message, BUFFER_SIZE - 1, PSTR("%s %x"),message, ptr_canStruct->data[dataIndex] );
		}

		UART0_Send_Message_String_p(message,0);

		clearString(message, BUFFER_SIZE);
	}

   /*all parameter initialisieren*/
	canResetParametersCANReceive();
}//END of canConvertCanFrameToUartFormat function

/*
 *this function deletes some variable
 *the  function has no input and output variable
 */

void canResetParametersCANSend( void )
{
	/* rest all parameters */
	clearString(decrypt_uartString, BUFFER_SIZE);
	clearString(uart_message_string, BUFFER_SIZE);
	clearUartStruct(ptr_uartStruct);
	canClearCanStruct(ptr_canStruct);

}//END of canResetParametersCANSend


/*
 *this function deletes some variable
 *the  function has no input and output parameters
 */

void canResetParametersCANReceive( void )
{
	/* initialize all parameters of structure  */
	canClearCanStruct(ptr_canStruct);
	clearUartStruct(ptr_uartStruct);
}//END of canResetParametersCANReceive function


/*
 *this function deletes some variable
 *the  function has no input and output parameters
 */

void canClearCanStruct( struct canStruct *ptr_canStruct)
{ /*resets the canStruct structure to "0" */
   if ( NULL != ptr_canStruct )
   {
      ptr_canStruct->length = 0;
      ptr_canStruct->mask = 0;
      ptr_canStruct->id = 0;
      ptr_canStruct->mob = 0;
      for ( uint8_t clearIndex = 0 ; clearIndex < CAN_MAX_DATA_ELEMENTS ; clearIndex++ )
      {
         ptr_canStruct->data[clearIndex] = 0;
      }
   }
}//END of canClearCanStruct

/* canSetCanBitTimingTQUnits
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
uint8_t canSetCanBitTimingTQUnits(uint8_t numberOfTimeQuanta, uint16_t freq2BaudRatio, int8_t bitRatePreScaler,
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
				PSTR("%S no. of Time Quanta (%i) and %S (%i) are both (un)set"),
				string_setCanBitTimingColon,
				numberOfTimeQuanta,
				string_bit_rate_pre_scaler,
				bitRatePreScaler);
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
		printDebug_p(debugLevelEventDebug, debugSystemCAN, __LINE__, filename,
				PSTR("%S %S calculated: %i"),
				string_setCanBitTimingColon, string_bit_rate_pre_scaler,
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
		printDebug_p(debugLevelEventDebug, debugSystemCAN, __LINE__, filename,
				PSTR("%S no. of Time Quanta calculated: %i"),
				string_setCanBitTimingColon,
				numberOfTimeQuanta);
    }

	// - N_TQ [8,25]
    if (0 < numberOfTimeQuanta)
    {
    	if ( 8 > numberOfTimeQuanta || 25 < numberOfTimeQuanta)
    	{
    		printDebug_p(debugLevelEventDebug, debugSystemCAN, __LINE__, filename,
    				PSTR("%S no. of Time Quanta (%i) out of [8,25]"),
    				string_setCanBitTimingColon,
    				numberOfTimeQuanta);
    		CommunicationError_p(ERRC, dynamicMessage_ErrorIndex, FALSE,
    				PSTR("%S no. of Time Quanta (%i) out of [8,25]"),
    				string_setCanBitTimingColon,
    				numberOfTimeQuanta);
    		return 1;
    	}
	}

    // bitRatePreScaler [1,64]
	if ( CAN_BIT_TIMING_BIT_RATE_PRESCALER_MIN > bitRatePreScaler || CAN_BIT_TIMING_BIT_RATE_PRESCALER_MAX < bitRatePreScaler)
	{
		CommunicationError_p(ERRC, dynamicMessage_ErrorIndex, FALSE,
				PSTR("%S %S (%i) out of [%i,%i]"),
				string_setCanBitTimingColon, string_bit_rate_pre_scaler,
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
					PSTR("%S %S (%i) forbids sync jump width (%i) > 0"),
					string_setCanBitTimingColon, string_bit_rate_pre_scaler,
					bitRatePreScaler, syncJumpWidth);
			return 1;
		}
		// -- multipleSamplePointSampling
		if ( FALSE != multipleSamplePointSampling_flag )
		{
			CommunicationError_p(ERRC, dynamicMessage_ErrorIndex, FALSE,
					PSTR("%S %S (%i) forbids multiple sample points"),
					string_setCanBitTimingColon, string_bit_rate_pre_scaler,
					bitRatePreScaler);
			return 1;
		}

		// apply corrections PHS1 + 1 TQ and PHS2 - 1 TQ
		if ( FALSE != autoCorrectBaudRatePreScalerNull_flag)
		{
			printDebug_p(debugLevelEventDebug, debugSystemCAN, __LINE__, filename,
					PSTR("%S %S (%i) auto corrected phase 1/2: %i/%i"),
					string_setCanBitTimingColon, string_bit_rate_pre_scaler,
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

	printDebug_p(debugLevelEventDebug, debugSystemCAN, __LINE__, filename,
			PSTR("%S register CANBT1/2/3: %x / %x / %x"),
        			string_setCanBitTimingColon,
        			CANBT1, CANBT2, CANBT3);

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
					filename, PSTR("setCanBitTiming: sync jump width (%i TQ) out of [%i,%i]"),
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
				result = canSetCanBitTimingTQUnits(10, freq2BbaudRatio, -1, 5, 2, 2, 2, TRUE, TRUE);
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

/*canSetCanTimer 
*
*  - enables to set the can timer clock cycle either 
*    - by setting the prescaler [0,0xff] Tclk = 1/F_CPU x 8 x (prescaler + 1)
*    - or by
*    - setting the time in seconds
*    
*    unit: 
*    	either canTimerUnit_SECONDS or canTimerUnit_PRESCALER
*    inputs:
*    	value (seconds): 
*    		- double seconds for a full clock cycle 
*    			- if value is out of range, maximum valid value is set
*    	value (prescaler):
*    		- integer scale factor [0,255] 
*    			- if value is out of range, maximum valid value is set
*    			- if value is a double, floor(value) is set
*/

void canSetCanTimer( double value, uint8_t unit )
{
	uint8_t setValue = 0xFF;
	// T_OVR_Max = 1/F_CPU_IO x 0x8 x 0x10000 x 0x100
	static double canMaxTimerInterval_seconds = (1/F_CPU) * 0x8000000UL ;
	switch( unit )
	{
	case canTimerUnit_SECONDS:
		if ( 0. >= value && value < canMaxTimerInterval_seconds )
		{
			// T_OVR	 	= Tclk_CANTIM x 0x10000
			// Tclk_CANTIM 	= 1/F_CPU_IO x 8 x (CANTCON [7:0] + 1)

			// => T_OVR	 	= (1/F_CPU_IO x 8 x (CANTCON [7:0] + 1)) x 0x10000
			// 			 	= 1/F_CPU_IO x 0x80000 x 0x100/0x100 * (CANTCON [7:0] + 1)
			// 			 	= 1/F_CPU_IO x 0x80000 x 0x100 * (CANTCON [7:0] + 1)/0x100
			// 			 	= T_OVR_Max * (CANTCON [7:0] + 1)/0x100
			//
			// CANTCON 		=  ( T_OVR / T_OVR_Max ) * 0x100 - 1
			// 		 		~= ((int)(floor( T_OVR / T_OVR_Max )) >> 8) - 1

			setValue = ((uint8_t)( ((uint16_t) (floor(value/canMaxTimerInterval_seconds))) << 8)) -1 ;
		}
		else
		{
			setValue = 0xFF;
		}
		break;
	case canTimerUnit_PRESCALER:
		if (( 0 > value ) || ( 0xFF < value ))
		{
			setValue = value;
		}
		if ( value > floor(value))
		{
			setValue = (uint8_t) (floor(value));
		}
		else
		{
			setValue = value;
		}
		break;
	default:
		setValue = 0xFF;
		break;
	}

	CANTCON = setValue & 0xFF;

	canTimerInterval = (1./(F_CPU / 0x80000)) * (0xFF & CANTCON);

}

/*
 *this function initializes the CAN register for AT90CAN128 and enable the CAN-controller
 * the function has not input variable
 * the return value is an integer:
 * 1  -> the CAN initialization is successful
 * -1 -> the CAN initialization is unsuccessful
 */

int8_t canInit( int32_t Baudrate )
{
	uint8_t intstate2 = SREG;/*save global interrupt flag*/
	/*disable interrupt*/
	cli();

	/*resets the CAN controller*/
	CANGCON |= ( 1 << SWRES );
	CANGCON &= ~( 1 << SWRES );

	/* CAN General Interrupt Enable Register - CANGIE
	 *    enable interrupts*/
	CANGIE = ( 1 << ENIT )  | ( 1 << ENBOFF ) | ( 1 << ENRX )  | ( 1 << ENTX ) |
			 ( 1 << ENERR ) | ( 1 << ENBX )   | ( 1 << ENERG ) | ( 1 << ENOVRT );

	/* CAN Enable Interrupt of all MOBs register*/
	CANIE2 = 0xFF;
	CANIE1 = 0x7F;

	/* clear (some) CAN registers */
	canClearCanRegisters();

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

	canUseOldRecvMessage_flag = CAN_USE_OLD_RECV_MESSAGE_FLAG;
	canUseNewRecvMessage_flag = CAN_USE_NEW_RECV_MESSAGE_FLAG;

	/* Timer control:
 	 * set to maximum to reduce influence
	 */

	canSetCanTimer(0xFF, canTimerUnit_PRESCALER );

	canEnableCan();
	canBusStoredState = canGetCurrentBusModeStatus();
	SREG = intstate2; /*restore global interrupt flag*/
	return 1;

}// END of canInit function

void canEnableCan(void)
{
    /* set ENA/STB enable mode  */
    CANGCON |= ( 1 << ENASTB );

    /*enable reception mode here so that information for CAN come automatically*/

    //    CAN MOb Control and DLC Register - CANCDMOB
    //    • Bit 7:6 – CONMOB1:0: Configuration of Message Object
    //    These bits set the communication to be performed (no initial value after RESET).
    //    – 00 - disable.
    //    – 01 - enable transmission.
    //    – 10 - enable reception.
    //    – 11 - enable frame buffer reception
    //    These bits are not cleared once the communication is performed. The user must re-write the
    //    configuration to enable a new communication.
    //    • This operation is necessary to be able to reset the BXOK flag.
    //    • This operation also set the corresponding bit in the CANEN registers.

    /* reset to 00 */
    CANCDMOB &= ~(( 0x3 << CONMOB0 ));
    /* set to 0x2: reception */
    CANCDMOB |= ( 1 << CONMOB1 );
}

/*
 *this function deletes some CAN register for AT90CAN128
 * the function has no input and output parameters
 */

void canClearCanRegisters( void )
{
    for ( uint8_t canMob = 0 ; canMob < 15 ; canMob++ )
    {
       CANPAGE = ( canMob << MOBNB0 ); /* clear all  mailbox*/
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
       /* clear CAN General Interrupt Register
        * by writing a logical one to all but the CANIT bit,
        * which is read only
        *
        */
       CANGIT &= (0xFF &  ~(1 << CANIT));
    }
}//END of canClearCanRegisters function

/* this function gets the free communication channel
 * the function has no input parameter
 * and returns a integer value
 * freemob -> valid message object block
 * -1 -> no valid message object block
 */

int8_t canGetFreeMob( void )
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

}//END of canGetFreeMob function


/*
 *this function gives the various CAN error on the channel
 *the function has no input and output parameters
 */

void canShowGeneralStatusError( void )
{
	printDebug_p(debugLevelEventDebugVerbose, debugSystemCAN, __LINE__, filename, PSTR("entering canShowGeneralStatusError"));

	if (canCurrentGeneralStatus & (1 << BOFF))
	{
		canErrorCode = CAN_ERROR_Bus_Off_Mode;
		CommunicationError_p(ERRC, canErrorCode, FALSE,
				PSTR("TEC: %i REC: %i CANGST: %#x"), canCurrentTransmitErrorCounter, canCurrentReceiveErrorCounter,
				canCurrentGeneralStatus);
	}

	if (canCurrentGeneralStatus & (1 << ERRP))
	{
		canErrorCode = CAN_ERROR_Error_Passive_Mode;
		CommunicationError_p(ERRC, canErrorCode, FALSE,
				PSTR("TEC: %i REC: %i CANGST: %#x"), canCurrentTransmitErrorCounter, canCurrentReceiveErrorCounter,
				canCurrentGeneralStatus);
	}

	if (canCurrentGeneralInterruptRegister & (1 << BOFFIT))
	{
		canErrorCode = CAN_ERROR_Bus_Off_Mode_interrupt;
		CommunicationError_p(ERRC, canErrorCode, FALSE,
				PSTR("TEC: %i REC: %i CANGIT: %#x"), canCurrentTransmitErrorCounter, canCurrentReceiveErrorCounter,
				canCurrentGeneralInterruptRegister);
	}
	if (canCurrentGeneralInterruptRegister & (1 << SERG))
	{
		canErrorCode = CAN_ERROR_Stuff_Error_General;
		CommunicationError_p(ERRC, canErrorCode, FALSE, PSTR("CANGIT: %#x"),
				canCurrentGeneralInterruptRegister);
	}
	if (canCurrentGeneralInterruptRegister & (1 << CERG))
	{
		canErrorCode = CAN_ERROR_CRC_Error_General;
		CommunicationError_p(ERRC, canErrorCode, FALSE, PSTR("CANGIT: %#x"),
				canCurrentGeneralInterruptRegister);
	}
	if (canCurrentGeneralInterruptRegister & (1 << FERG))
	{
		canErrorCode = CAN_ERROR_Form_Error_General;
		CommunicationError_p(ERRC, canErrorCode, FALSE, PSTR("CANGIT: %#x"),
				canCurrentGeneralInterruptRegister);
	}
	if (canCurrentGeneralInterruptRegister & (1 << AERG))
	{
		canErrorCode = CAN_ERROR_Acknowledgment_Error_General;
		CommunicationError_p(ERRC, canErrorCode, FALSE, PSTR("CANGIT: %#x"),
				canCurrentGeneralInterruptRegister);
	}

}//END of canShowGeneralStatusError function




/*
 *this function prints the various CAN errors
 *the function has no input and output parameters
 */
void canShowMObError( void )
{
	static const uint16_t errorNumbers[5] = { CAN_ERROR_MOb_Acknowledgement_Error,
				                              CAN_ERROR_MOb_Form_Error,
				                              CAN_ERROR_MOb_CRC_Error,
			                                  CAN_ERROR_MOb_Stuff_Error,
			                                  CAN_ERROR_MOb_Bit_Error};

	for (int8_t bit = 4; 0 <= bit; bit--)
	{
		printDebug_p(debugLevelEventDebugVerbose, debugSystemCAN, __LINE__, filename, PSTR("canShowMObError: bit index: %i"), bit);
		if ( canCurrentMObStatus & ( 1 << bit ) )
		{
			//	• Bit 0 – AERR: Acknowledgment Error
			//		No detection of the dominant bit in the acknowledge slot.
			//	• Bit 1 – FERR: Form Error
			//		The form error results from one or more violations of the fixed form in the following bit fields:
			//		• CRC delimiter.
			//		• Acknowledgment delimiter.
			//		• EOF
			//	• Bit 2 – CERR: CRC Error
			//		The receiver performs a CRC check on every de-stuffed received message from the start of
			//		frame up to the data field. If this checking does not match with the de-stuffed CRC field, a CRC
			//		error is set.
			//	• Bit 3 – SERR: Stuff Error
			//		Detection of more than five consecutive bits with the same polarity. This flag can generate an
			//		interrupt.
			//	• Bit 4 – BERR: Bit Error (Only in Transmission)
			//		The bit value monitored is different from the bit value sent.
			//		Exceptions: the monitored recessive bit sent as a dominant bit during the arbitration field and the
			//		acknowledge slot detecting a dominant bit during the sending of an error frame.

			canErrorCode = errorNumbers[bit];
			CommunicationError_p(ERRC, canErrorCode, FALSE, PSTR("MOb#: %i CANSTMOB: %#x"), canMob,
					canCurrentMObStatus);
		}
	}

}//END of canShowMObError


uint8_t canErrorHandling( uint8_t error )
{
	switch (error)
	{
	case canState_MOB_ERROR: /* mob */
		printDebug_p(debugLevelEventDebug, debugSystemCAN, __LINE__, filename, PSTR("CAN error handling: mob status"), canReady);
		canShowMObError();
		break;
	case canState_GENERAL_ERROR: /* general */
		printDebug_p(debugLevelEventDebug, debugSystemCAN, __LINE__, filename, PSTR("CAN error handling: general status"), canReady);
		canShowGeneralStatusError();
		break;
	default:
		canErrorCode = CommunicationError(ERRC, -1, TRUE, PSTR("(unknown internal CAN status error %i)"), error);
		break;
	}

	return 0;
}

uint8_t canCheckInputParameterError( uartMessage *ptr_uartStruct )
{
	uint8_t error = FALSE;

	//#error missing break statement for number of arguments
	/* type check */
	for ( uint8_t parameterIndex = 0 ; parameterIndex < MAX_PARAMETER ; parameterIndex++ )
	{
		switch (parameterIndex)
		{
		case 1:
			if ( ( 0x7FFFFFF ) < ptr_uartStruct->Uart_Message_ID )
			{
				CommunicationError_p(ERRA, SERIAL_ERROR_arguments_exceed_boundaries, 1,
						           PSTR("%lx, [%#lx ... %#lx]"),
				                   ptr_uartStruct->Uart_Message_ID, 0, 0x7FFFFFF);
				error = TRUE;
				break;
			}
			break;
		case 2:
			if ( ( 0x7FFFFFF ) < ptr_uartStruct->Uart_Mask )
			{
				CommunicationError_p(ERRA, SERIAL_ERROR_arguments_exceed_boundaries, 1,
						           PSTR("%lx, [%#lx ... %#lx]"),
				                   ptr_uartStruct->Uart_Mask, 0, 0x7FFFFFF);
				error = TRUE;
				break;
			}
			break;
		case 3:
			if ( ( 1 ) < ptr_uartStruct->Uart_Rtr )
			{
				CommunicationError_p(ERRA, SERIAL_ERROR_arguments_exceed_boundaries, 1,
						           PSTR("%lx, [%#lx ... %#lx]"),
				                   ptr_uartStruct->Uart_Rtr, 0, 1);
				error = TRUE;
				break;
			}
			break;
		case 4:
			if ( ( 8 ) < ptr_uartStruct->Uart_Length )
			{
				CommunicationError_p(ERRA, SERIAL_ERROR_arguments_exceed_boundaries, 1,
						           PSTR("%lx, [%#lx ... %#lx]"),
				                   ptr_uartStruct->Uart_Length, 0, 8);
				error = TRUE;
				break;
			}
			break;
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
			for ( uint8_t i = 0 ; i < 8 ; i++ )
			{
				if ( ( 0xFF ) < ptr_uartStruct->Uart_Data[i] )
				{
					uartErrorCode = CommunicationError_p(ERRA, SERIAL_ERROR_arguments_exceed_boundaries, 1,
							        PSTR("%x, [%#x ... %#x]"),
							        ptr_uartStruct->Uart_Data[i], 0, 0xFF);
					error = TRUE;
					break;
				}
			}
			break;
		}

	}

	return error;
}

/*
 * canSetMObCanIDandMask
 * 		sets for the current MOb, set via CANPAGE,
 * 		the
 * 			ID
 * 			MASK
 * 		    flags:
 * 		    	enable RTR mask bit comparison
 * 		    	enable ID Extension mask bit comparison
 * 		Depending on the size (> 2**11) of id and mask either V2.0 part A or part B format is chosen
 */

void canSetMObCanIDandMask(uint32_t id, uint32_t mask, uint8_t enableRTRMaskBitComparison_flag, uint8_t enableIDExtensionMaskBitComparison_flag)
{
    // set identifier to send
    if ( ( MAX_ELEVEN_BIT >= id ) || ( MAX_ELEVEN_BIT >= mask ) )
    {
    	// V2.0 part A
    	CANIDT4 &= 0x5;
        CANIDT3  = 0x0;
    	CANIDT2  = 0xff & ( id << 5 );
        CANIDT1  = 0xff & ( id >> 3 );
    }
    else
    {
    	// V2.0 part B
    	CANIDT4 &= 0xff & (( id <<  3 ) | 0x7)  ;
        CANIDT3  = 0xff & (  id >>  5 );
    	CANIDT2  = 0xff & (  id >> 13 );
        CANIDT1  = 0xff & (  id >> 21 );
    }

    // mask comparison bits
    /* RTR mask bit comparison*/
    if ( enableRTRMaskBitComparison_flag )
    {
    	// compare RTR
    	CANIDM4 |= ( 1 << RTRMSK );
    }
    else
    {
    	// always true
    	CANIDM4 &= ~( 1 << RTRMSK );
    }

    // ID Extension (IDE) mask bit comparison
    if ( enableIDExtensionMaskBitComparison_flag )
    {
    	// compare IDE
    	CANIDM4 |= ( 1 << IDEMSK );
    }
    else
    {
    	// always true
    	CANIDM4 &= ~( 1 << IDEMSK );
    }

    // set identifier Mask
    if ( ( MAX_ELEVEN_BIT >= id ) || ( MAX_ELEVEN_BIT >= mask ) )
    {
    	// V2.0 part A
        CANIDM4 &= 0x5;
        CANIDM3  = 0x0;
        CANIDM2  = 0xff & (( mask & 0x7 ) << 5);
        CANIDM1  = 0xff & (  mask >> 3  );
    }
    else
    {
    	// V2.0 part B
        CANIDM4 &= 0xff & (( mask <<  3 ) | 0x5 );
        CANIDM3  = 0xff & (  mask >>  5 );
        CANIDM2  = 0xff & (  mask >> 13 );
        CANIDM1  = 0xff & (  mask >> 21 );
    }
}

/* this function checks whether all the received parameters are valid
 * the function has a pointer of the serial structure as input and returns no parameter
 * returns TRUE if all checks are passed
 * returns FALSE else
 */

int8_t canCheckParameterCanFormat( struct uartStruct *ptr_uartStruct)
{
   /*check range of parameter*/

   for (uint8_t index = 1; index < MAX_PARAMETER; index ++)
   {
      if ( 0 != *ptr_setParameter[index] ) return FALSE;
   }

   if ( 0x7FFFFFF  < ptr_uartStruct->Uart_Message_ID ) return FALSE;
   if ( 0x7FFFFFF  < ptr_uartStruct->Uart_Mask       ) return FALSE;
   if ( 1          < ptr_uartStruct->Uart_Rtr        ) return FALSE;
   if ( 8          < ptr_uartStruct->Uart_Length     ) return FALSE;
   if ( 0XFF       < ptr_uartStruct->Uart_Data[0]    ) return FALSE;
   if ( 0XFF       < ptr_uartStruct->Uart_Data[1]    ) return FALSE;
   if ( 0XFF       < ptr_uartStruct->Uart_Data[2]    ) return FALSE;
   if ( 0XFF       < ptr_uartStruct->Uart_Data[3]    ) return FALSE;
   if ( 0XFF       < ptr_uartStruct->Uart_Data[4]    ) return FALSE;
   if ( 0XFF       < ptr_uartStruct->Uart_Data[5]    ) return FALSE;
   if ( 0XFF       < ptr_uartStruct->Uart_Data[6]    ) return FALSE;
   if ( 0XFF       < ptr_uartStruct->Uart_Data[7]    ) return FALSE;
   return TRUE;

}


/* Interrupt Service Routine: CANIT_vect
 *
 * interrupt CAN communication
 * with defined interrupt vector CANIT_vect */

ISR(CANIT_vect)
{
	uint8_t save_canpage = CANPAGE;

	static uint16_t ctr = 0;
	ctr++;
	/* printDebug_p(debugLevelEventDebugVerbose, debugSystemCAN, __LINE__, filename,
			PSTR("ISR (%i): CANIT_vect occurred, canReady: %i ---  BEGIN of ISR SREG=%x"), ctr, canReady, SREG ); */

	// --- interrupt generated by a MOb
	// i.e. there is a least one MOb,
	//      if more than one choose the highest priority CANHPMOB
	if ((CANHPMOB & 0xF0) != 0xF0)
	{
		// set current canMob to the 'winner' in CANHPMOB
		canMob = (CANHPMOB & 0xf0) >> HPMOB0;
		// set CANPAGE to current canMob
		CANPAGE = ((canMob << MOBNB0) & 0xf0);

		// check all interrupt bits of CANSTMOB Bit 6:0

		// check whether MOb has an error Bit 4:0
		if (0 != canIsMObErrorAndAcknowledge())
		{
			/* printDebug_p(debugLevelEventDebugVerbose, debugSystemCAN, __LINE__, filename,
					PSTR("ISR (%i): CANIT_vect occurred, MOb (%i) error occurred, CANSTMOB before/after acknowledge: %#x/%#x,"),
					ctr, canMob, canCurrentMObStatus, CANSTMOB ); */

			/* printDebug_p(debugLevelEventDebugVerbose, debugSystemCAN, __LINE__, filename,
					PSTR("ISR (%i): CANIT_vect occurred, canReady: %i, MOb (%i) error occurred, setting canReady to %i"),
					ctr, canReady, canMob, canState_MOB_ERROR); */

			canReady = canState_MOB_ERROR;

			/* printDebug_p(debugLevelEventDebugVerbose, debugSystemCAN, __LINE__, filename,
					PSTR("ISR (%i): CANIT_vect occurred, canReady: %i"), ctr, canReady); */

			/* printDebug_p(debugLevelEventDebugVerbose, debugSystemCAN, __LINE__, filename,
					PSTR("ISR (%i): CANIT_vect occurred, MOb (%i), CANCDMOB: %#x"), ctr, canMob, CANCDMOB ); */

			// disable communication
			CANCDMOB &= ~(1 << CONMOB1 | 1 << CONMOB0);

			/* printDebug_p(debugLevelEventDebugVerbose, debugSystemCAN, __LINE__, filename,
					PSTR("ISR (%i): CANIT_vect occurred, MOb (%i), CANCDMOB: %#x"), ctr, canMob, CANCDMOB ); */
		}
		else if (CANSTMOB & (1 << TXOK)) // Bit 5
		{
			// MOb finished transmission

			// clear transmit OK flag
			CANSTMOB &= ~(1 << TXOK);
			// disable communication
			CANCDMOB &= ~(1 << CONMOB1 | 1 << CONMOB0);

			/* printDebug_p(debugLevelEventDebugVerbose, debugSystemCAN, __LINE__, filename,
					PSTR("ISR (%i): CANIT_vect occurred, canReady: %i, TXOK received"), ctr, canReady); */
		}
		else if (CANSTMOB & (1 << RXOK)) // Bit 6
		{
			// MOb received message

			// get mailbox number
			ptr_canStruct->mob = canMob;
			ptr_canStruct->length = CANCDMOB & 0xF;

			// get identifier
			ptr_canStruct->id = 0;
			ptr_canStruct->id = CANIDT2 >> 5;
			ptr_canStruct->id |= CANIDT1 << 3;

			// get data of selected MOb
			for (uint8_t i = 0; i < ptr_canStruct->length; i++)
			{
				ptr_canStruct->data[i] = CANMSG;
			}

			// acknowledge/clear interrupt
			CANSTMOB &= (0xFF & ~(1 << RXOK));

			// enable any previous communication (receive) mode
			CANCDMOB &= ( 1 << CONMOB1 | 1 << CONMOB0 | 1 << IDE );
			// mark, that we got an CAN_interrupt, to be handled by main
			canReady = canState_RXOK;

		}
		else
		{
			canCurrentMObStatus = CANSTMOB;
			canReady = canState_UNKNOWN;
		}

		CANPAGE = save_canpage; /* restore CANPAGE*/
	}
	else /*general interrupt*/
	{
		// --- general interrupt was generated
		if (0 != canIsGeneralStatusErrorAndAcknowledge())
		{
			canReady = canState_GENERAL_ERROR;
			//BOFFIT interrupt needs additional bus state settings

			if ( canCurrentGeneralInterruptRegister & (1 << BOFFIT))
			{
				canBusStoredState = canChannelMode_BUS_OFF;
			}

			// Abort Request
			CANGCON |= (1 << ABRQ);
			// disable communication
			CANCDMOB &= ~(1 << CONMOB1 | 1 << CONMOB0);
			// clear Abort Request bit
			CANGCON &= (0xFF & ~(1 << ABRQ));
		}
		else if ( CANGIT & (1 << BXOK))
		{
			canReady = canState_GENERAL_BXOK;
		}
		else if ( CANGIT & (1 << OVRTIM))
		{
			canReady = canState_GENERAL_OVRTIM;
		}
		else
		{
			canReady = canState_UNKNOWN;
		}
	}
} //END of ISR(CANIT_vect)

ISR(OVRIT_vect)
{
	canTimerOverrun = TRUE;

	/* reset every canBusStateResetInterval_seconds
	 * canBusStoredState to undefined
	 * to allow for recovery */

	printDebug_p(debugLevelPeriodicDebug, debugSystemCAN, __LINE__, filename,
			PSTR("bus state %S"), (const char*) (pgm_read_word( &(canBusModes[canBusStoredState]))));

	switch (canBusStoredState)
	{
		case canChannelMode_UNDEFINED:
		case canChannelMode_CAN_DISABLED:
		case canChannelMode_ERROR_ACTIVE:
			/* do not reset */
			break;
		default:
			canPeriodicCanTimerCanBusStateReset();
			printDebug_p(debugLevelPeriodicDebug, debugSystemCAN, __LINE__, filename, PSTR("bus state %S"),
					(const char*) (pgm_read_word( &(canBusModes[canBusStoredState]))));
			break;
	}

	#warning define probable prompt action if OVR occurs
}

