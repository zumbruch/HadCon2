/* The can.h is a header file for the functions and structure specific to the CAN interface between the microcontroller and CAN devices.
 *the baudrate is 250Kbit/s
 */
/* Structur for CAN data */
#ifndef CAN__H
#define CAN__H
#include "api.h"
#include "api_define.h"
#include "api_global.h"

#include "api_debug.h"
#define CAN_DEFAULT_BAUD_RATE TWOHUNDERTFIFTY_KBPS
#define CAN_MAX_DATA_ELEMENTS 8
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

#endif
