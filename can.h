/* The can.h is a header file for the functions and structure specific to the CAN interface between the micro controller and CAN devices.
 *the baud rate is 250Kbit/s
 */
/* Structure for CAN data */
#ifndef CAN__H
#define CAN__H
#include "api.h"
#include "api_define.h"
#include "api_global.h"

#include "api_debug.h"
#define CAN_DEFAULT_BAUD_RATE TWOHUNDERTFIFTY_KBPS
#define CAN_MAX_DATA_ELEMENTS 8

#define CAN_BIT_TIMING_INFORMATION_PROCESSING_TIME 2
#define CAN_BIT_TIMING_SYNCHRONIZATION_SEGMENT_TIME 2
#define CAN_BIT_TIMING_PROPAGATION_SEGMENT_TIME_MIN 1
#define CAN_BIT_TIMING_PROPAGATION_SEGMENT_TIME_MAX 8
#define CAN_BIT_TIMING_PHASE_SEGMENT_1_TIME_MIN 1
#define CAN_BIT_TIMING_PHASE_SEGMENT_1_TIME_MAX 8
#define CAN_BIT_TIMING_SYNC_JUMP_WIDTH_TIME_MIN 1
#define CAN_BIT_TIMING_SYNC_JUMP_WIDTH_TIME_MAX 4
#define CAN_BIT_TIMING_BIT_RATE_PRESCALER_MIN 1
#define CAN_BIT_TIMING_BIT_RATE_PRESCALER_MAX 64

#define CAN_USE_OLD_RECV_MESSAGE_FLAG TRUE;
#define CAN_USE_NEW_RECV_MESSAGE_FLAG TRUE;

#define CAN_BUS_STATE_RESET_INTERVAL_SECONDS 60.

struct canStruct
{
	uint32_t id;
	uint32_t mask;
	uint8_t  mob;
	uint8_t  length;
	unsigned char data[CAN_MAX_DATA_ELEMENTS];
};
extern struct canStruct canFrame;
extern struct canStruct *ptr_canStruct;
extern int32_t canDefaultBaudRate;

double canTimerInterval;

enum canState {
	canState_IDLE = 0,
	canState_RXOK = 1,
	canState_MOB_ERROR = 2,
	canState_GENERAL_ERROR = 3,
	canState_GENERAL_BXOK = 4,
	canState_GENERAL_OVRTIM = 5,
	canState_UNKNOWN,
	canState_MAXIMUM_INDEX
	};

extern const char* const can_error[] PROGMEM;
enum ce_index
{
   CAN_ERROR_Bus_Off_Mode = 0,
   CAN_ERROR_Bus_Off_Mode_interrupt,
   CAN_ERROR_Error_Passive_Mode,
   CAN_ERROR_Can_Controller_disabled,
   CAN_ERROR_MOb_Bit_Error,
   CAN_ERROR_MOb_Stuff_Error,
   CAN_ERROR_MOb_CRC_Error,
   CAN_ERROR_MOb_Form_Error,
   CAN_ERROR_MOb_Acknowledgement_Error,
   CAN_ERROR_CAN_was_not_successfully_initialized,
   CAN_ERROR_CAN_communication_timeout,
   CAN_ERROR_Stuff_Error_General,
   CAN_ERROR_CRC_Error_General,
   CAN_ERROR_Form_Error_General,
   CAN_ERROR_Acknowledgment_Error_General,
   CAN_ERROR_MAXIMUM_INDEX
};

extern const char* const mob_error[] PROGMEM;
enum me_index
{
   MOB_ERROR_all_mailboxes_already_in_use = 0,
   MOB_ERROR_message_ID_not_found,
   MOB_ERROR_this_message_already_exists,
   MOB_ERROR_MAXIMUM_INDEX
};

extern const char* const canBusModes[] PROGMEM;
enum canChannelModes
{
	canChannelMode_ERROR_ACTIVE = 0,
	canChannelMode_ERROR_PASSIVE,
	canChannelMode_BUS_OFF,
	canChannelMode_CAN_DISABLED,
	canChannelMode_UNDEFINED,
	canChannelMode_MAXIMUM_INDEX
};


enum canTimerUnits
{
	canTimerUnit_PRESCALER = 0,
	canTimerUnit_SECONDS,
	canTimerUnit_MAXIMUM_INDEX
};

/* implemented functions */

int8_t canInit( int32_t Baudrate ); /* initialize CAN communication*/

void canEnableCan( void );

void canShowGeneralStatusError( void ); /* state on the CAN channel */

void canShowMObError( void ); /* can error for the communication */

uint8_t canErrorHandling( uint8_t error );

void canSetMObCanIDandMask(uint32_t id, uint32_t mask, uint8_t enableRTRMaskBitComparison_flag, uint8_t enableIDExtensionMaskBitComparison_flag);

int8_t canGetFreeMob( void ); /* searches free Mob */

void canClearCanRegisters( void ); /* clean up of parameters */

void canResetParametersCANSend( void );/*this function*/

void canResetParametersCANReceive( void );/*this function*/

void canWaitForCanSendMessageFinished( void ); /* verify that the sending of data is complete */

void canWaitForCanSendRemoteTransmissionRequestMessageFinished( void ); /* verify that the sending of data in the "Request case" is complete */

void canClearCanStruct( struct canStruct *ptr_canStruct); /*resets the canStruct structure to "0" */

void canConvertCanFrameToUartFormat( struct canStruct *ptr_canStruct );/* this function collects the various CAN data in a string */

void canSendRemoteTransmissionRequestMessage( struct uartStruct *PtrFrame ); /* function for the SEND command name and RTR set, initialization of the registers with elements of the structure uartStruct*/

void canSendMessage( struct uartStruct *PtrFrame );/*function for the SEND command name and RTR is not set, initialization of the registers with elements of the structure uartStruct*/

void canSubscribeMessage( struct uartStruct *PtrFrame ); /* function for the command name SUBS, initialization of the registers with elements of the structure uartStruct */

void canUnsubscribeMessage( struct uartStruct *PtrFrame ); /* function for the command name USUB, initialization of the registers with elements of the structure uartStruct */

int canSetCanBaudRate( const uint32_t rate, const uint32_t freq );

uint8_t canCheckInputParameterError( uartMessage *ptr);

uint8_t canSetCanBitTimingTQUnits(uint8_t numberOfTimeQuanta, uint16_t freq2BaudRatio, int8_t bitRatePreScaler,
		                       uint8_t propagationTimeSegment, uint8_t phaseSegment1, uint8_t phaseSegment2, uint8_t syncJumpWidth,
				               uint8_t multipleSamplePointSampling_flag, uint8_t autoCorrectBaudRatePreScalerNull_flag);
uint8_t canBitTimingTQBasicBoundaryChecks(uint8_t propagationTimeSegment, uint8_t phaseSegment1, uint8_t phaseSegment2, uint8_t syncJumpWidth);

int8_t canCheckParameterCanFormat( struct uartStruct *ptr_uartStruct ); /* this function checks whether all the received parameters are valid*/

static inline uint8_t canGetCurrentBusModeStatus(void)
{
	if ( ! (CANGSTA & (1 << ENFG )))
	{
		return canChannelMode_CAN_DISABLED;
	}
	else if ( CANGSTA & (1 << BOFF))
	{
		return canChannelMode_BUS_OFF;
	}
	else if ( CANGSTA & (1 << ERRP ))
	{
		return canChannelMode_ERROR_PASSIVE;
	}
	else
	{
		return canChannelMode_ERROR_ACTIVE;
	}
}


static inline uint8_t canIsGeneralStatusErrorAndAcknowledge( void )
{
	/*
		To acknowledge a general interrupt, the corresponding bits of CANGIT register (BXOK, BOFFIT,...)
		must be cleared by the software application. This operation is made writing a logical one
		in these interrupt flags (writing a logical zero doesn’t change the interrupt flag value).

		• Bit 7 – CANIT: General Interrupt Flag
		This is a read only bit.
		– 0 - no interrupt.
		– 1 - CAN interrupt: image of all the CAN controller interrupts except for OVRTIM
		interrupt. This bit can be used for polling method.

		• Bit 6 – BOFFIT: Bus Off Interrupt Flag
		Writing a logical one resets this interrupt flag. BOFFIT flag is only set when the CAN enters in
		bus off mode (coming from error passive mode).
		– 0 - no interrupt.
		– 1 - bus off interrupt when the CAN enters in bus off mode.
		• Bit 5 – OVRTIM: Overrun CAN Timer
		Writing a logical one resets this interrupt flag. Entering in CAN timer overrun interrupt handler
		also reset this interrupt flag
		– 0 - no interrupt.
		– 1 - CAN timer overrun interrupt: set when the CAN timer switches from 0xFFFF to 0.
		• Bit 4 – BXOK: Frame Buffer Receive Interrupt
		Writing a logical one resets this interrupt flag. BXOK flag can be cleared only if all CONMOB
		fields of the MOb’s of the buffer have been re-written before.
		– 0 - no interrupt.
		– 1 - burst receive interrupt: set when the frame buffer receive is completed.
		• Bit 3 – SERG: Stuff Error General
		Writing a logical one resets this interrupt flag.
		– 0 - no interrupt.
		– 1 - stuff error interrupt: detection of more than 5 consecutive bits with the same
		polarity.
		• Bit 2 – CERG: CRC Error General
		Writing a logical one resets this interrupt flag.
		– 0 - no interrupt.
		– 1 - CRC error interrupt: the CRC check on destuffed message does not fit with the
		CRC field.
		• Bit 1 – FERG: Form Error General
		Writing a logical one resets this interrupt flag.
		– 0 - no interrupt.
		– 1 - form error interrupt: one or more violations of the fixed form in the CRC delimiter,
		acknowledgment delimiter or EOF.
		• Bit 0 – AERG: Acknowledgment Error General
		Writing a logical one resets this interrupt flag.
		– 0 - no interrupt.
		– 1 - acknowledgment error interrupt: no detection of the dominant bit in acknowledge
		slot.
	 */

	canCurrentGeneralStatus = CANGSTA;
	canCurrentGeneralInterruptRegister = CANGIT;
	canCurrentReceiveErrorCounter = CANREC;
	canCurrentTransmitErrorCounter = CANTEC;

	// reset all error Bits at once, leaving out read-only CANIT as well as  BKOK and OVRTIM
	CANGIT = (CANGIT & ( 0xFF & ~(1 << CANIT | 1 << BXOK | 1 << OVRTIM )));

	return ((canCurrentGeneralInterruptRegister & ( 1 << BOFFIT | 1 << SERG | 1 << CERG | 1 << FERG | 1 << AERG )) ||
			(canCurrentGeneralStatus & ( 1 << BOFF | 1 << ERRP )));

}//END of canIsGeneralStatusErrorAndAcknowledge

static inline uint8_t canIsMObErrorAndAcknowledge( void )
{
	/* testing (and resetting) the following
	 * errors of CANSTMOB:
	 * BERR, SERR, CERR, FERR, AERR*/

    static uint8_t errorBits = 0xFF & ( 1 << BERR | 1 << SERR | 1 << CERR | 1 << FERR | 1 << AERR );
    // store current status
	canCurrentMObStatus = CANSTMOB;

	/*
	 To acknowledge a mob interrupt, the corresponding bits of CANSTMOB register (RXOK,
	 TXOK,...) must be cleared by the software application. This operation needs a read-modify-write
	 software routine.

	 from manual: chapter 19.8.2, p250

		3.8.1 Interrupt Behavior
		When an interrupt occurs, the Global Interrupt Enable I-bit is cleared and all interrupts are disabled.
		The user software can write logic one to the I-bit to enable nested interrupts. All enabled
		interrupts can then interrupt the current interrupt routine. The I-bit is automatically set when a
		Return from Interrupt instruction – RETI – is executed.
		There are basically two types of interrupts. The first type is triggered by an event that sets the
		interrupt flag. For these interrupts, the Program Counter is vectored to the actual Interrupt Vector
		in order to execute the interrupt handling routine, and hardware clears the corresponding interrupt
		flag. Interrupt flags can also be cleared by writing a logic one to the flag bit position(s) to be
		cleared. If an interrupt condition occurs while the corresponding interrupt enable bit is cleared,
		the interrupt flag will be set and remembered until the interrupt is enabled, or the flag is cleared
		by software. Similarly, if one or more interrupt conditions occur while the Global Interrupt Enable
		bit is cleared, the corresponding interrupt flag(s) will be set and remembered until the Global
		Interrupt Enable bit is set, and will then be executed by order of priority.

		The second type of interrupts will trigger as long as the interrupt condition is present. These
		interrupts do not necessarily have interrupt flags. If the interrupt condition disappears before the
		interrupt is enabled, the interrupt will not be triggered.
		When the AVR exits from an interrupt, it will always return to the main program and execute one
		more instruction before any pending interrupt is served.
		Note that the Status Register is not automatically stored when entering an interrupt routine, nor
		restored when returning from an interrupt routine. This must be handled by software.
	 */

	// reset all error Bits at once, HTH
	CANSTMOB &= (0xFF & ~(errorBits));

	/*
		• Bit 4 – BERR: Bit Error (Only in Transmission)
		This flag can generate an interrupt. It must be cleared using a read-modify-write software routine
		on the whole CANSTMOB register.
		The bit value monitored is different from the bit value sent.
		Exceptions: the monitored recessive bit sent as a dominant bit during the arbitration field and the
		acknowledge slot detecting a dominant bit during the sending of an error frame.

		The rising of this flag does not disable the MOb (the corresponding ENMOB-bit of CANEN registers
		is not cleared). The next matching frame will update the BERR flag.
	 */

	return canCurrentMObStatus & errorBits;

}//END of canIsMObErrorAndAcknowledge

void canSetCanTimer( double value, uint8_t unit );

static inline void canPeriodicCanTimerCanBusStateReset(void)
{
	/* reset every canBusStateResetInterval_seconds
	 * canBusStoredState to undefined
	 * to allow for recovery
	 */

	static double cumulatedIntervalTime = 0.;

	if (canBusStateResetInterval_seconds < cumulatedIntervalTime)
	{
		canBusStoredState = canChannelMode_UNDEFINED;
		cumulatedIntervalTime = 0.;
	}
	else
	{
		cumulatedIntervalTime += canTimerInterval;
	}
}

#endif
