#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <util/atomic.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/iocan128.h>
#include <avr/wdt.h>

#include "one_wire.h"
#include "one_wire_adc.h"
#include "one_wire_dualSwitch.h"
#include "one_wire_simpleSwitch.h"
#include "read_write_register.h"
#include "one_wire_temperature.h"
#include "one_wire_api_settings.h"

#include "adc.h"
#include "api.h"
#include "api_define.h"
#include "api_global.h"

#include "api_debug.h"
#include "can.h"
#include "mem-check.h"
#include "api_show.h"
#include "api_version.h"
#include "relay.h"

#include "OWIDeviceSpecific.h"
#include "OWIHighLevelFunctions.h"
#include "OWIBitFunctions.h"

#include "twi_master.h"

char currentCommandKeyword[MAX_LENGTH_KEYWORD];/*variable to store command keyword e.g. "SEND"*/
char currentResponseKeyword[MAX_LENGTH_KEYWORD];/*variable to store response keyword e.g. "RECV"*/

uint16_t atmelAdcValues[8]; /* current supply voltages of the board*/
uint8_t res0, res1, res2, res3;/*parameter back to reading the voltage on ADC-channel*/
uint8_t res4, res5, res6, res7;/*parameter back to reading the voltage on ADC-channel, if JTAG is disabled*/

uint8_t owi_IDs[OWI_MAX_NUM_DEVICES][8]; /*Global variable to store the ID numbers of ALL devices*/
uint8_t owi_IDs_pinMask[OWI_MAX_NUM_DEVICES]; /*Global variable to store the bus position corresponding*/
uint8_t NumDevicesFound;
uint8_t BUSES[8] = { OWI_PIN_0, OWI_PIN_1, OWI_PIN_2, OWI_PIN_3, OWI_PIN_4, OWI_PIN_5, OWI_PIN_6, OWI_PIN_7 };

//char PD;/*variable to check found devices*/
int8_t countDEV = 0;/*number of devices (initialized to 0)*/
int8_t countDEVbus = 0;/*number of devices on the current bus (initialized to 0)*/

/*
 * DEBUG settings
 */
uint8_t globalDebugLevel = 0;
uint32_t globalDebugSystemMask = UINT_FAST32_MAX;

/*
 * RELAY
 */

volatile uint8_t relayThresholdEnable_flag = FALSE;
volatile uint8_t relayThresholdCurrentState = relayThresholdState_INIT;
volatile uint8_t relayThresholdNewState = relayThresholdState_IDLE;
volatile uint8_t relayThresholds_valid[16] = {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};
volatile uint8_t relayThresholdAllThresholdsValid = FALSE;
volatile uint8_t relayThresholdInputChannelMask = 0x0;
volatile uint8_t relayThresholdExtThresholdsMask = 0x0;
volatile uint8_t relayThresholdOutputPin = 0x0;
volatile uint8_t relayThresholdLedPin = 0x1;
volatile uint8_t relayThresholdReportCurrentStatusEnable_flag = TRUE;
volatile uint8_t relayThresholdRelayValue = FALSE;
volatile uint8_t relayThresholdRelaySetValue = FALSE;
volatile uint8_t relayThresholdOutOfBoundsLock_flag = TRUE;
volatile uint8_t relayThresholdsInBoundsPolarity = (! FALSE);
volatile uint8_t relayThresholdsOutOfBoundsPolarity = FALSE;
volatile uint8_t relayThresholdsUndefinedIndifferentPolarity = 0x0F;
volatile uint8_t relayThresholdsExternalPolarityPinPos = 0x07; /* PORT C: JDINOUT2 PIN7*/
volatile uint8_t relayThresholdsExternalPolarityPort = 0xC; /* PORT C: JDINOUT2 PIN7*/
volatile uint8_t relayThresholdsExternalPolarity = 0;
volatile uint8_t relayThresholdInvertPolarity_flag = FALSE;
volatile uint8_t relayThresholdOutOfBoundsStateLock_flag = TRUE;
volatile int8_t  relayThresholdExternHighChannel = -1;
volatile int8_t  relayThresholdExternLowChannel  = -1 ;
volatile uint8_t relayThresholdUseIndividualThresholds = FALSE;
volatile uint16_t relayThresholdHigh[8] = {0x384}; /*the maximum gas pressure*/
volatile uint16_t relayThresholdLow[8]  = {0x64};  /*the minimum gas pressure*/

uint8_t canBusStoredState = canChannelMode_UNDEFINED;
double canBusStateResetInterval_seconds = CAN_BUS_STATE_RESET_INTERVAL_SECONDS;
int32_t canDefaultBaudRate = CAN_DEFAULT_BAUD_RATE;

uint8_t usercommand;
/*variable to store values*/


char* ptr_setParameter[MAX_PARAMETER];

volatile unsigned char timer0Ready;/*variable for Timer0  Interrupt*/
volatile unsigned char timer1Ready;/*variable for Timer1  Interrupt*/
volatile unsigned char timer0AReady;/*variable for Timer0 Output Compare A  Interrupt*/
volatile unsigned char timer0ASchedulerReady;/*variable for Timer0 Output Compare A  Interrupt*/

volatile unsigned char canReady; /*variable for can interrupt*/
volatile unsigned char canTimerOverrun; /*variable for can timer overrun interrupt*/
volatile unsigned char canCurrentGeneralStatus;/*variable for can interrupt*/
volatile unsigned char canCurrentGeneralInterruptRegister;/*variable for can interrupt*/
volatile unsigned char canCurrentMObStatus;/*variable for can interrupt*/
volatile unsigned char canCurrentTransmitErrorCounter;/*variable for can error handling*/
volatile unsigned char canCurrentReceiveErrorCounter;/*variable for can error handling*/

char canStoreString[MAX_LENGTH_CAN_DATA];

volatile unsigned char uartReady;/*variable for Uart interrupt*/

char keepAliveString[15] = "PING";/*only for keep_alive function*/

char uartString[BUFFER_SIZE]; /* variable for storage received a complete string via UART */
char decrypt_uartString[BUFFER_SIZE];
char decrypt_uartString_remainder[BUFFER_SIZE];


/* unused
 *
 * char ring_buffer[MAX_INPUT][MAX_LENGTH_CAN_DATA];
 *
 *//*buffer to store CAN data*/

char setParameter[MAX_PARAMETER][MAX_LENGTH_PARAMETER];

char uart_message_string[BUFFER_SIZE];
char message[BUFFER_SIZE];
char resultString[BUFFER_SIZE];

unsigned char nextCharPos; /*pointer of the variable uartString  */
uint16_t canErrorCode = 0; /* error code for CAN-communication */
uint16_t twiErrorCode = 0; /* error code for I2C/TWI-communication */
uint16_t uartErrorCode = 0; /* error code for UART-communication */
uint16_t mobErrorCode = 0; /* error code for Message Object Block */
uint16_t generalErrorCode = 0; /*  general error code */
uint8_t ptr_subscribe = 0;/* pointer of variable subscribe_ID and subscribe_mask */
uint8_t canMob; /*variable  for Message Object Block in the interrupt routine*/

uint8_t *ptr_buffer_in = NULL; /*pointer to write CAN data in buffer_ring*/
uint8_t *ptr_buffer_out = NULL; /*pointer to read CAN data in buffer_ring*/
uint8_t flag_pingActive = 0; /*automatic ping action disabled*/

uint16_t twi_data[TWI_MAX_DATA_ELEMENTS] = {0,0,0,0,0,0,0,0};
uint8_t twi_bytes_to_transceive;

/*define UART-structure */
struct uartStruct uartFrame;
struct uartStruct *ptr_uartStruct;
/*define CAN-structure */
struct canStruct canFrame;
struct canStruct *ptr_canStruct;
/*define OWI-structure */
struct owiStruct owiFrame;
struct owiStruct *ptr_owiStruct;

/*define function pointers*/
int16_t (*UART0_Send_Message_String_p)( char *, uint16_t ) = UART0_Send_Message_String;
uint8_t (*CommunicationError_p)(uint8_t, const int16_t, const uint8_t, PGM_P, ...) = CommunicationError;
void (*printDebug_p)(uint8_t, uint32_t, int16_t, PGM_P, PGM_P, ...) = printDebug;
void (*UART0_Transmit_p)( uint8_t ) = UART0_Transmit;
void (*relayThresholdDetermineStateAndTriggerRelay_p)(uint8_t) = relayThresholdDetermineStateAndTriggerRelay;
void (*Process_Uart_Event_p)( void ) = Process_Uart_Event;
uint16_t (*clearString_p)( char[], uint16_t ) = clearString;

//struct owiIdStruct owiIDs[OWI_MAX_NUM_DEVICES];
//struct owiIdStruct *ptr_owiIDs[OWI_MAX_NUM_DEVICES];

uint16_t owiBusMask;
uint16_t adcBusMask;

unsigned short unusedMemoryStart;

uint8_t mcusr;

unsigned char watchdogIncarnationsCounter __attribute__ ((section (".noinit")));

#warning check if done remove pgm_read_byte from all CommunicationErrors and set the corresponding arrays back from PROGMEM, also printout pointers
int main( void )
{
	// The MCU Status Register provides information on which reset source caused an MCU reset.
	//	    Bit 0 – PORF:  	Power-On Reset
	//	    Bit 1 – EXTRF: 	External Reset
	//	    Bit 2 – BORF:  	Brown-Out Reset
	//		Bit 3 – WDRF:  	Watchdog Reset
	//   	Bit 4 – JTRF:  	JTAG Reset Flag
	//  							This bit is set if a reset is being caused
	//                              by a logic one in the JTAG Reset Register selected by
	//                              the JTAG instruction AVR_RESET. This bit is reset by
	// 								a Power-on Reset, or by writing a logic zero to the flag.

	// store MCUSR
	mcusr = MCUSR;
	// reset MCUSR
	MCUSR = 0;

	// get unused memory at start
	unusedMemoryStart = get_mem_unused(); /* get initial memory status */

	// debug settings
	globalDebugLevel = debugLevelNoDebug;
	globalDebugLevel = debugLevelVerboseDebug;
	//globalDebugLevel = debugLevelEventDebug;
	//globalDebugLevel = debugLevelEventDebugVerbose;
	//globalDebugLevel = debugLevelPeriodicDebug;
	//globalDebugLevel = debugLevelPeriodicDebugVerbose;

	globalDebugSystemMask = (0x1L << debugSystem_MAXIMUM_INDEX) -1; /*all enabled*/

	// init section

   Initialization();

   printDebug_p(debugLevelEventDebug, debugSystemMain, __LINE__, PSTR(__FILE__), PSTR("starting main (after init)"));

   // Start

   startMessage();

	// main endless loop
   uint32_t mainLoopIndex = 0;

   while ( 1 )
   {
	   mainLoopIndex++;

 	   printDebug_p(debugLevelPeriodicDebugVerbose, debugSystemMain, __LINE__, PSTR(__FILE__), PSTR("ALIV --- alive --- %i"), mainLoopIndex);

#warning TODO make it switchable, up-to-now disabled
 	   // watchdog
 	   wdt_enable(WDTO_2S);
 	   //wdt_disable();

#warning think of using ATOMIC_BLOCK() from util/atomic.h

	   // UART has received a string
	   printDebug_p(debugLevelPeriodicDebugVerbose, debugSystemUART, __LINE__, PSTR(__FILE__), PSTR("UART %s"), ( 1 == uartReady ) ? "--yes" : "");

	   if ( 1 == uartReady )/* of the ISR was completely receive a string */
	   {
		   // disable interrupts
		   cli();
		   Process_Uart_Event_p();

		   uartReady = 0; /* restore flag */

		   // (re)enable interrupts
		   sei();
	   }

	   // CANbus interface has received a message / error
	   printDebug_p(debugLevelPeriodicDebugVerbose, debugSystemCAN, __LINE__, PSTR(__FILE__), PSTR("CAN %s"), ( canState_IDLE != canReady ) ? "--yes" : "");

	   if ( canState_IDLE != canReady)
	   {
		   // disable interrupts
		   cli();

		   switch (canReady)
		   {
		   case canState_IDLE:
			   break;
		   case canState_RXOK:
			   /* received complete message */
			   canConvertCanFrameToUartFormat(ptr_canStruct);
			   break;
		   case canState_GENERAL_BXOK:
		   case canState_GENERAL_OVRTIM:
			   printDebug_p(debugLevelEventDebug, debugSystemCAN, __LINE__, PSTR(__FILE__),
								   PSTR("CAN canReady %i received"), canReady);
			   break;
		   case canState_MOB_ERROR:
		   case canState_GENERAL_ERROR:
		   case canState_UNKNOWN:
		   default:
			   /* can error detected */

			   printDebug_p(debugLevelEventDebug, debugSystemCAN, __LINE__, PSTR(__FILE__),
					   PSTR("CAN canReady %i received -> ErrorHandling"), canReady);

			   canErrorHandling(canReady);

			   break;
		   }

		   printDebug_p(debugLevelEventDebug, debugSystemCAN, __LINE__, PSTR(__FILE__),
				   PSTR("CAN canReady %i reset to %i"), canReady, canState_IDLE);

		   canReady = canState_IDLE; /* restore flag */

		   // (re)enable interrupts
		   sei();
	   }

	   // CAN timer overrun 
	   printDebug_p(debugLevelPeriodicDebugVerbose, debugSystemCAN, __LINE__, PSTR(__FILE__), PSTR("CAN TIMER OVR %s"), ( TRUE == canTimerOverrun ) ? "--yes" : "");

	   if ( TRUE == canTimerOverrun)
	   {
		   // disable interrupts
		   cli();
#warning define probable delayed action if OVR occurs

		   canTimerOverrun = FALSE;
		   printDebug_p(debugLevelPeriodicDebug, debugSystemCAN, __LINE__, PSTR(__FILE__), PSTR("CAN Timer Overrun received"));

		   // (re)enable interrupts
		   sei();
	   }

	   // timer 0 set by ISR
	   printDebug_p(debugLevelPeriodicDebugVerbose, debugSystemTIMER0, __LINE__, PSTR(__FILE__), PSTR("timer0 %s"), ( 1 == timer0Ready ) ? "--yes" : "");

	   if ( 1 == timer0Ready )
	   {
		   cli();
		   timer0Ready = 0;
		   sei();
	   }

	   // timer 1 set by ISR
	   printDebug_p(debugLevelPeriodicDebugVerbose, debugSystemTIMER1, __LINE__, PSTR(__FILE__), PSTR("timer1 %s"), ( 1 == timer1Ready ) ? "--yes" : "");

	   if ( 1 == timer1Ready )
	   {
		   if ( flag_pingActive )
		   {
			   cli();
			   // disable interrupts
			   strncpy(uart_message_string, keepAliveString, BUFFER_SIZE - 1);
			   UART0_Send_Message_String_p(NULL,0);

			   timer1Ready = 0;
			   sei();
		   }
	   }

	   // timer 0A set by ISR
	   printDebug_p(debugLevelPeriodicDebugVerbose, debugSystemTIMER0A, __LINE__, PSTR(__FILE__), PSTR("timer0A %s"), ( 1 == timer0AReady ) ? "--yes" : "");

	   if ( 1 == timer0AReady )
	   {
		   cli();
		   timer0AReady = 0; /* restore flag */
		   sei();
	   }

	   // timer 0Aready set by ISR
	   printDebug_p(debugLevelPeriodicDebugVerbose, debugSystemTIMER0AScheduler, __LINE__, PSTR(__FILE__), PSTR("timer0AScheduler %s"), ( 1 == timer0ASchedulerReady ) ? "--yes" : "");

	   if(1 == timer0ASchedulerReady )
	   {
		   ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
		   {
			   if (0 != owiTemperatureConversionGoingOnCountDown)
			   {
				   owiTemperatureConversionGoingOnCountDown--;
			   }
			   timer0ASchedulerReady = 0; /* restore flag */
		   }
	   }

	   /*relay block*/
	   printDebug_p(debugLevelPeriodicDebugVerbose, debugSystemRELAY, __LINE__, PSTR(__FILE__), PSTR("Relays"));

	   if (relayThresholdEnable_flag && relayThresholdAllThresholdsValid)
	   {
		   printDebug_p(debugLevelPeriodicDebugVerbose, debugSystemRELAY, __LINE__, PSTR(__FILE__), PSTR("Relays enable and valid"));
#warning TODO: too often cli/sei disable slow interactions, find solution maybe timer
		   cli(); //clear all interrupts active
		   //         relayThresholdDetermineStateAndTriggerRelay(relayThresholdInputSource_RADC);
		   relayThresholdDetermineStateAndTriggerRelay(relayThresholdInputSource_OWI_MDC_PRESSURE_1);
		   sei(); //set all interrupts active

		   cli(); //clear all interrupts active
		   relayThresholdDetermineStateAndTriggerRelay(relayThresholdInputSource_OWI_MDC_PRESSURE_2);
		   sei(); //set all interrupts active
	   }
	   else
	   {
		   printDebug_p(debugLevelPeriodicDebugVerbose, debugSystemRELAY, __LINE__, PSTR(__FILE__), PSTR("relay enable: %i - thr valid: %i"),
				        relayThresholdEnable_flag, relayThresholdAllThresholdsValid);
	   }
	   printDebug_p(debugLevelPeriodicDebugVerbose, debugSystemMain, __LINE__, PSTR(__FILE__), PSTR("-------------------------"));
   }// END of while

   return 0; //EXIT_SUCCESS;
} // END of main
