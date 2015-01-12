/*
   Licensed under the EUPL V.1.1, Lizenziert unter EUPL V.1.1 
*/
#ifndef API_GLOBAL__H
#define API_GLOBAL__H

#include <avr/pgmspace.h>
#include "OWIDeviceSpecific.h"
#include "OWIHighLevelFunctions.h"
#include "OWIBitFunctions.h"
#include "api_define.h"


/* Canapi.h*/
extern char decrypt_uartString[BUFFER_SIZE];
extern char decrypt_uartString_remainder[BUFFER_SIZE];
extern char ring_buffer[MAX_INPUT][MAX_LENGTH_CAN_DATA];/*variable for storage all received CAN data*/
extern char setParameter[MAX_PARAMETER][MAX_LENGTH_PARAMETER]; /*storage of cut string */
extern char *ptr_setParameter[MAX_PARAMETER];
extern char uart_message_string[BUFFER_SIZE];
extern char message[BUFFER_SIZE];
extern char uartString[BUFFER_SIZE]; /* variable for storage received a complete string via UART */
extern char resultString[BUFFER_SIZE];

extern uint8_t resetSource; /* reason of a reset */

extern int8_t uart0_init; /* return variable of canInit function*/
extern int8_t can_init; /* return variable of canInit function*/
extern int8_t twim_init; /* return variable of TWIM_Init function*/
extern int8_t owi_init; /* return variable of TWIM_Init function*/
extern int8_t timer0_init; /* return variable of Timer0_Init function*/
extern int8_t timer0A_init;/* return variable of Timer0A_Init function*/

extern uint8_t canBusStoredState;
extern double  canBusStateResetInterval_seconds;
extern uint16_t canErrorCode; /* error code for CAN-communication */
extern uint16_t twiErrorCode; /* error code for I2C/TWI-communication */
extern uint16_t generalErrorCode; /*general error code */
extern uint16_t mobErrorCode; /* error code for Message Object Block */
extern uint16_t uartErrorCode; /* error code for UART-communication */

extern volatile unsigned char BufferFull;/*variable for UART Interrupt*/
extern volatile unsigned char canReady; /* variable for CAN ISR */
extern volatile unsigned char canTimerOverrun; /*variable for can timer overrun interrupt*/
extern volatile unsigned char canCurrentGeneralStatus;/*variable for can interrupt*/
extern volatile unsigned char canCurrentGeneralInterruptRegister; /*variable for can interrupt*/
extern volatile unsigned char canCurrentMObStatus;/*variable for can interrupt*/
extern volatile unsigned char canCurrentTransmitErrorCounter;/*variable for can error handling*/
extern volatile unsigned char canCurrentReceiveErrorCounter;/*variable for can error handling*/

extern volatile unsigned char timer0Ready;/*variable for Timer  Interrupt*/
extern volatile unsigned char timer1Ready;/*variable for Timer  Interrupt*/
extern volatile unsigned char timer0AReady;/*variable for Timer0 Output Compare A  Interrupt*/
extern volatile unsigned char timer0ASchedulerReady;/*variable for Timer0 Output Compare A  Interrupt*/
extern volatile unsigned char uartReady;/*variable for UART Interrupt*/

extern uint8_t canMob; /*variable  for Message Object Block in the interrupt routine*/
extern unsigned char nextCharPos;/* variable for the storage of data in serial string variable */

extern uint32_t subscribe_ID[MAX_LENGTH_SUBSCRIBE];
extern uint32_t subscribe_mask[MAX_LENGTH_SUBSCRIBE];

extern uint8_t *ptr_buffer_in; /*pointer for write in ring_buffer*/
extern uint8_t *ptr_buffer_out; /*pointer for read in ring_buffer*/
extern uint8_t ptr_subscribe; /* pointer of variable subscribe_ID and subscribe_mask */

extern uint8_t flag_pingActive; /* flag for PING mechanism */

extern const char* const serial_error[] PROGMEM;
extern const char* const can_error[] PROGMEM;
extern const char* const twi_error[] PROGMEM;
extern const char* const mob_error[] PROGMEM;
extern const char* const general_error[] PROGMEM;

extern const char* const canBusModes[] PROGMEM;

extern char currentCommandKeyword[MAX_LENGTH_KEYWORD];/*variable to store current command keyword e.g. "SEND"*/
extern char currentResponseKeyword[MAX_LENGTH_KEYWORD];/*variable to store current command keyword e.g. "RECV"*/

extern uint16_t atmelAdcValues[8];/* current supply voltages of the board*/

/*variable to store values*/
extern uint8_t usercommand;

extern uint8_t res0, res1, res2, res3; /*parameter back to reading the voltage on ADC-channel*/
extern uint8_t res4, res5, res6, res7; /*parameter back to reading the voltage on ADC-channel, if JTAG is disabled*/

extern uint8_t NumDevicesFound;

extern uint8_t owi_IDs[OWI_MAX_NUM_DEVICES][8]; /*Global variable to store the ID numbers of ALL devices*/
extern uint8_t owi_IDs_pinMask[OWI_MAX_NUM_DEVICES]; /*Global variable to store the bus position corresponding*/

extern uint8_t BUSES[8];
//extern uint8_t BUSES_all[8];

//extern char PD;/*variable to check found devices*/
extern int8_t countDEV;/*number of devices (initialized to 0)*/
extern int8_t countDEVbus;/*number of devices on the current bus (initialized to 0)*/

extern uint16_t owiBusMask;
extern uint16_t adcBusMask;

extern uint8_t mcusr;
extern unsigned char watchdogIncarnationsCounter __attribute__ ((section (".noinit")));

#endif

