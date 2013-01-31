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

struct canStruct
{
	uint32_t Can_Message_ID;
	uint32_t Can_Mask;
	uint8_t Can_Mob;
	uint8_t Can_Length;
	unsigned char Can_Data[CAN_MAX_DATA_ELEMENTS];

};
extern struct canStruct canFrame;
extern struct canStruct *ptr_canStruct;
extern int32_t canDefaultBaudRate;

/* implemented functions */

int8_t CAN_Init( int32_t Baudrate ); /* initialize CAN communication*/

void enableCan( void );

void Convert_canFormat_to_UartFormat( struct canStruct *ptr_canStruct );/*This function grabs the CAN data received in a string*/

void Get_BusState( void ); /* state on the CAN channel */

uint8_t Get_CanError( void ); /* can error for the communication */

int8_t Get_FreeMob( void ); /* searches free Mob */

void clearCanRegisters( void ); /* clean up of parameters */

void Reset_Parameter_CANSend_Message( void );/*this function*/

void Reset_Parameter_CANReceive( void );/*this function*/

void Wait_for_Can_Send_Message_Finished( void ); /* verify that the sending of data is complete */

void Wait_for_Can_Receive_message_Finished( void ); /* verify that the sending of data in the "Request case" is complete */

void clearCanStruct( struct canStruct *ptr_canStruct); /*resets the canStruct structure to "0" */

void Convert_pack_canFrame_to_UartFormat( struct canStruct *ptr_canStruct );/* this function collects the various CAN data in a string */

int setCanBaudRate( const uint32_t rate, const uint32_t freq );

uint8_t setCanBitTimingTQUnits(uint8_t numberOfTimeQuanta, uint16_t freq2BaudRatio, int8_t bitRatePreScaler,
		                       uint8_t propagationTimeSegment, uint8_t phaseSegment1, uint8_t phaseSegment2, uint8_t syncJumpWidth,
				               uint8_t multipleSamplePointSampling_flag, uint8_t autoCorrectBaudRatePreScalerNull_flag);
uint8_t canBitTimingTQBasicBoundaryChecks(uint8_t propagationTimeSegment, uint8_t phaseSegment1, uint8_t phaseSegment2, uint8_t syncJumpWidth);


#endif
