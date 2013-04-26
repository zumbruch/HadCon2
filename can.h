/* The can.h is a header file for the functions and structure specific to the CAN interface between the microcontroller and CAN devices.
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

enum canState {
	canState_IDLE = 0,
	canState_RXOK = 1,
	canState_MOB_ERROR = 2,
	canState_GENERAL_ERROR = 3,
	canState_MAXIMUM_INDEX
	};

extern const char *can_error[] PROGMEM;
enum ce_index
{
   CAN_ERROR_Can_Bus_is_off = 0,
   CAN_ERROR_Can_Bus_is_passive,
   CAN_ERROR_Can_Bus_is_on,
   CAN_ERROR_MOb_Bit_Error,
   CAN_ERROR_MOb_Stuff_Error,
   CAN_ERROR_MOb_CRC_Error,
   CAN_ERROR_MOb_Form_Error,
   CAN_ERROR_MOb_Acknowledgement_Error,
   CAN_ERROR_CAN_was_not_successfully_initialized,
   CAN_ERROR_CAN_communication_timeout,
   CAN_ERROR_MAXIMUM_INDEX
};

extern const char *mob_error[] PROGMEM;
enum me_index
{
   MOB_ERROR_all_mailboxes_already_in_use = 0,
   MOB_ERROR_message_ID_not_found,
   MOB_ERROR_this_message_already_exists,
   MOB_ERROR_MAXIMUM_INDEX
};

/* implemented functions */

int8_t canInit( int32_t Baudrate ); /* initialize CAN communication*/

void canEnableCan( void );

void canGetGeneralStatusError( void ); /* state on the CAN channel */

uint8_t canIsGeneralStatusError( void );

void canGetMObError( void ); /* can error for the communication */

uint8_t canIsMObErrorAndAcknowledge( void );

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

#endif
