#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <math.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/iocan128.h>

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

uint8_t owi_IDs[NUM_DEVICES][8]; /*Global variable to store the ID numbers of ALL devices*/
uint8_t owi_IDs_pinMask[NUM_DEVICES]; /*Global variable to store the bus position corresponding*/
uint8_t NumDevicesFound;
uint8_t BUSES[8] = { OWI_PIN_0, OWI_PIN_1, OWI_PIN_2, OWI_PIN_3, OWI_PIN_4, OWI_PIN_5, OWI_PIN_6, OWI_PIN_7 };

//char PD;/*variable to check found devices*/
int8_t countDEV = 0;/*number of devices (initialized to 0)*/
int8_t countDEVbus = 0;/*number of devices on the current bus (initialized to 0)*/

/*
 * DEBUG settings
 */
uint8_t debug = 0;
uint32_t debugMask = 0x0;

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


int32_t canDefaultBaudRate = CAN_DEFAULT_BAUD_RATE;

uint8_t usercommand;
/*variable to store values*/


char* ptr_setParameter[MAX_PARAMETER];

volatile unsigned char timer0Ready;/*variable for Timer0  Interrupt*/
volatile unsigned char timer1Ready;/*variable for Timer1  Interrupt*/
volatile unsigned char timer0AReady;/*variable for Timer0 Output Compare A  Interrupt*/
volatile unsigned char timer0ASchedulerReady;/*variable for Timer0 Output Compare A  Interrupt*/
volatile unsigned char canReady;/*variable for can interrupt*/
volatile unsigned char uartReady;/*variable for Uart interrupt*/
char schedulerString[5] = "OWTP";/*only for scheduler*/
char keepAliveString[15] = "PING";/*only for keep_alive function*/
char uartString[BUFFER_SIZE]; /* variable for storage received a complete string via UART */
char decrypt_uartString[BUFFER_SIZE];
char temp_canString[MAX_LENGTH_CAN_DATA];
char store_canData[MAX_LENGTH_CAN_DATA];
char canString[MAX_LENGTH_CAN_DATA];
char ring_buffer[MAX_INPUT][MAX_LENGTH_CAN_DATA]; /*buffer to store CAN data*/
char setParameter[MAX_PARAMETER][MAX_LENGTH_PARAMETER];
char uart_message_string[BUFFER_SIZE];
char message[BUFFER_SIZE];
unsigned char nextCharPos; /*pointer of the variable uartString  */
uint8_t can_errorCode = 0; /* error code for CAN-communication */
uint8_t twi_errorCode = 0; /* error code for I2C/TWI-communication */
uint8_t uart_errorCode = 0; /* error code for UART-communication */
uint8_t mailbox_errorCode = 0; /* error code for Message Object Block */
uint8_t general_errorCode = 0; /*  general error code */
uint8_t ptr_subscribe = 0;/* pointer of variable subscribe_ID and subscribe_mask */
uint8_t mob; /*variable  for Message Object Block in the interrupt routine*/
int8_t can_init = 0; /* variable to call CAN_Init function*/
int8_t twim_init = 0; /* variable to call TWIM_Init function*/
int8_t owi_init = 0; /* variable to call OWI_Init function*/
int8_t timer0_init = 0; /* variable to call Timer0_Init function*/
int8_t timer0A_init = 0;/* variable to call Timer0A_Init function*/
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

//struct owiIdStruct owiIDs[NUM_DEVICES];
//struct owiIdStruct *ptr_owiIDs[NUM_DEVICES];

uint16_t owiBusMask;
uint16_t adcBusMask;

unsigned short unusedMemoryStart;

#warning check if done remove pgm_read_byte from all CommunicationErrors and set the corresponding arrays back from PROGMEM, also printout pointers
int main( void )
{
   unusedMemoryStart = get_mem_unused(); /* get initial memory status */

//   debug = noDebug;
   debug = verboseDebug;
//   debug = eventDebug;
//   debug = periodicDebug;
   debugMask = (0x1L << debugSystems_MAXIMUM_INDEX) -1; /*all enabled*/

   ptr_uartStruct = &uartFrame; /* initialize pointer for CPU-structure */
   //	initUartStruct(ptr_uartStruct); /*initialize basic properties of uartStruct
   
   ptr_canStruct = &canFrame; /* initialize pointer for CAN-structure */

   ptr_owiStruct = &owiFrame; /* initialize pointer for 1-wire structure*/
   owiInitOwiStruct(ptr_owiStruct); /* initialize basic properties*/

   /* initialize flag */
   nextCharPos = 0;
   uartReady = 0;
   canReady = 0;
   timer0Ready = 0;
   timer1Ready = 0;
   timer0AReady = 0;
   timer0ASchedulerReady = 0;
   ptr_subscribe = 2; /*start value of pointer*/
   ptr_buffer_in = NULL; /*initialize pointer for write in buffer_ring*/
   ptr_buffer_out = NULL; /*initialize pointer for read in  buffer_ring*/
   res0 = 0, res1 = 0, res2 = 0, res3 = 0;
   res4 = 0, res5 = 0, res6 = 0, res7 = 0;

   relayThresholdCurrentState = relayThresholdState_IDLE;

   owiBusMask = 0xFF;
   adcBusMask = 0x0F;

   // init section

   Initialization();

   clearString(uart_message_string, BUFFER_SIZE);
   snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("SYST system (re)started"));
   UART0_Send_Message_String(NULL,0);

   if ( verboseDebug <= debug && ( ( debugMask >> debugMain ) & 0x1 ) )
   {
      snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) starting main (after init)"), __LINE__, __FILE__);
      UART0_Send_Message_String(NULL,0);
      owiTemperatureFindParasiticlyPoweredDevices(TRUE);

/*
      char c = 0;
      char s[] = "\0";
      snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) String tests: char c=0:%i '0':%i \"\\0\"==0? => %s ending==\"\\0\"? => %s"), __LINE__, __FILE__, c,
                 '0', s[0] == 0 ? "yes" : "no", s[0] == s[1] ? "yes" : "no");
      UART0_Send_Message_String(NULL,0);
*/
   }
   showMem(ptr_uartStruct, commandShowKeyNumber_UNUSED_MEM_START);
   showMem(ptr_uartStruct, commandShowKeyNumber_UNUSED_MEM_NOW);
   owiApiFlag(ptr_uartStruct, owiApiCommandKeyNumber_COMMON_ADC_CONVERSION);
   owiApiFlag(ptr_uartStruct, owiApiCommandKeyNumber_COMMON_TEMPERATURE_CONVERSION);

   // main endless loop
   int16_t indi = 0;

   while ( 1 )
   {


      indi++;
//	  if (0==indi%1000) {PING = (0x1 << 0);}

      if ( periodicDebug <= debug )
      {
         snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) ALIV --- alive --- %i"), __LINE__, __FILE__, indi);
         UART0_Send_Message_String(NULL,0);
      }

      // UART has received a string
      if ( periodicDebug <= debug && ( ( debugMask >> debugUART ) & 0x1 ) )
      {
         snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) UART%s"), __LINE__, __FILE__, ( 1 == uartReady ) ? "--yes" : "");
         UART0_Send_Message_String(NULL,0);
      }

      if ( 1 == uartReady )/* of the ISR was completely receive a string */
      {
         cli();
         // disable interrupts

         /* TODO:
          * can those to lines be moved into Process_Uart_Event ?
          */
         strncpy(decrypt_uartString, uartString, BUFFER_SIZE - 1);
         /* clear uartString, avoiding memset*/
         clearString(uartString, BUFFER_SIZE);

         if ( eventDebug < debug && ((debugMask >> debugUART) & 0x1) )
         {
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) UART string received:%s"), __LINE__, __FILE__, decrypt_uartString);
            UART0_Send_Message_String(NULL,0);
         }

         Process_Uart_Event();

         uartReady = 0; /* restore flag */

         sei();
         // (re)enable interrupts
      }

      // CANbus interface has received a message
      if ( periodicDebug <= debug && ( ( debugMask >> debugCAN ) & 0x1 ) )
      {
         snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) CAN %s"), __LINE__, __FILE__, ( 1 == canReady ) ? "--yes" : "");
         UART0_Send_Message_String(NULL,0);
      }

      if ( 1 == canReady ) /* of the ISR data has been fully received */
      {
         cli();
         // disable interrupts

         Convert_pack_canFrame_to_UartFormat(ptr_canStruct);
         canReady = 0; /* restore flag */

         sei();
         // (re)enable interrupts
      }

      // timer 0 set by ISR
      if ( periodicDebug <= debug && ( ( debugMask >> debugTIMER0 ) & 0x1 ) )
      {
         snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) timer0%s"), __LINE__, __FILE__, ( 1 == timer0Ready ) ? "--yes" : "");
         UART0_Send_Message_String(NULL,0);
      }
      if ( 1 == timer0Ready )
      {
         cli();
/*         Get_BusState(); */
         timer0Ready = 0;
         sei();
      }

      // timer 1 set by ISR
      if ( periodicDebug <= debug && ( ( debugMask >> debugTIMER1 ) & 0x1 ) )
      {
         snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) timer1%s"), __LINE__, __FILE__, ( 1 == timer1Ready ) ? "--yes" : "");
         UART0_Send_Message_String(NULL,0);
      }
      if ( 1 == timer1Ready )
      {
         if ( flag_pingActive )
         {
            cli();
            // disable interrupts
            strncpy(uart_message_string, keepAliveString, BUFFER_SIZE - 1);
            UART0_Send_Message_String(NULL,0);

            timer1Ready = 0;
            sei();
         }
      }

      // timer 0A set by ISR
      if ( periodicDebug <= debug && ( ( debugMask >> debugTIMER0A ) & 0x1 ) )
      {
         snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) timer0A%s"), __LINE__, __FILE__,
                    ( 1 == timer0AReady ) ? "--yes" : "");
         UART0_Send_Message_String(NULL,0);
      }
      if ( 1 == timer0AReady )
      {
         cli();
/*         Get_CanError(); */
         timer0AReady = 0; /* restore flag */
         sei();
      }

      // timer 0Aready set by ISR
      if ( periodicDebug <= debug && ( ( debugMask >> debugTIMER0AScheduler ) & 0x1 ) )
      {
         snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) timer0A%s"), __LINE__, __FILE__,
                    ( 1 == timer0ASchedulerReady ) ? "--yes" : "");
         UART0_Send_Message_String(NULL,0);
      }
      if(1 == timer0ASchedulerReady )
      {
         cli();
         if (0 != owiTemperatureConversionGoingOnCountDown)
         {
            owiTemperatureConversionGoingOnCountDown--;
         }
         timer0ASchedulerReady = 0; /* restore flag */
         sei();
      }

      /*relay block*/
      if ( periodicDebug <= debug && ( ( debugMask >> debugRELAY ) & 0x1 ) )
      {
         snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) %s"), __LINE__, __FILE__, "Relays");
         UART0_Send_Message_String(NULL, 0);
      }

      if (relayThresholdEnable_flag && relayThresholdAllThresholdsValid)
      {
          if ( periodicDebug <= debug && ( ( debugMask >> debugRELAY ) & 0x1 ) )
          {
             snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) %s"), __LINE__, __FILE__, "Relays enable and valid");
             UART0_Send_Message_String(NULL, 0);
          }
#warning TODO: too often cli/sei disable slow interactions, find solution maybe timer
    	 //cli(); //clear all interupts active
//         relayThresholdDetermineStateAndTriggerRelay(relayThresholdInputSource_RADC);
         relayThresholdDetermineStateAndTriggerRelay(relayThresholdInputSource_OWI_MDC_PRESSURE_1);
         relayThresholdDetermineStateAndTriggerRelay(relayThresholdInputSource_OWI_MDC_PRESSURE_2);
         //sei(); //set all interupts active
      }
      else
      {
          if ( periodicDebug <= debug && ( ( debugMask >> debugRELAY ) & 0x1 ) )
          {
             snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) %s: relay enable: %i - thr valid: %i"), __LINE__, __FILE__, __FUNCTION__, relayThresholdEnable_flag, relayThresholdAllThresholdsValid);
             UART0_Send_Message_String(NULL, 0);
          }
      }
      if ( periodicDebug <= debug )
      {
         snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) %s"), __LINE__, __FILE__, "-------------------------");
         UART0_Send_Message_String(NULL,0);
      }
   }// END of while

   return 0; //EXIT_SUCCESS;
} // END of main
