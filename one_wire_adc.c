/*
 * VERSION 1.0 Januar 7th 2010 LATE  File: 'one_wire_adc.c'
 * Author: Linda Fouedjio  based on Alejandro Gil
 * modified (heavily rather rebuild): Peter Zumbruch
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
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
//#include "one_wire_temperature.h"

#include "adc.h"
#include "api.h"
#include "api_define.h"
#include "api_global.h"

#include "api_debug.h"
#include "can.h"
#include "mem-check.h"

#include "OWIDeviceSpecific.h"
#include "OWIHighLevelFunctions.h"
#include "OWIBitFunctions.h"

#define DS2450_CONVERT                  0x3C
#define DS2450_READ_MEMORY              0xAA
#define DS2450_WRITE_MEMORY             0x55

#define DS2450_ADDRESS_MEMORY_MAP_PAGE_0_CONVERSION_READOUT_INPUT_A_LSB_MSB     0x00
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_0_CONVERSION_READOUT_INPUT_A_LSB_LSB     0x00
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_0_CONVERSION_READOUT_INPUT_A_MSB_MSB     0x00
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_0_CONVERSION_READOUT_INPUT_A_MSB_LSB     0x01
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_0_CONVERSION_READOUT_INPUT_B_LSB_MSB     0x00
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_0_CONVERSION_READOUT_INPUT_B_LSB_LSB     0x02
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_0_CONVERSION_READOUT_INPUT_B_MSB_MSB     0x00
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_0_CONVERSION_READOUT_INPUT_B_MSB_LSB     0x03
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_0_CONVERSION_READOUT_INPUT_C_LSB_MSB     0x00
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_0_CONVERSION_READOUT_INPUT_C_LSB_LSB     0x04
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_0_CONVERSION_READOUT_INPUT_C_MSB_MSB     0x00
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_0_CONVERSION_READOUT_INPUT_C_MSB_LSB     0x05
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_0_CONVERSION_READOUT_INPUT_D_LSB_MSB     0x00
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_0_CONVERSION_READOUT_INPUT_D_LSB_LSB     0x06
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_0_CONVERSION_READOUT_INPUT_D_MSB_MSB     0x00
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_0_CONVERSION_READOUT_INPUT_D_MSB_LSB     0x07

#define DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_OC_OE_RESOLUTION_INPUT_A_MSB     0x00
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_OC_OE_RESOLUTION_INPUT_A_LSB     0x08
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_POR_ALARMS_RANGES_INPUT_A_MSB    0x00
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_POR_ALARMS_RANGES_INPUT_A_LSB    0x09
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_OC_OE_RESOLUTION_INPUT_B_MSB     0x00
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_OC_OE_RESOLUTION_INPUT_B_LSB     0x0A
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_POR_ALARMS_RANGES_INPUT_B_MSB    0x00
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_POR_ALARMS_RANGES_INPUT_B_LSB    0x0B
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_OC_OE_RESOLUTION_INPUT_C_MSB     0x00
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_OC_OE_RESOLUTION_INPUT_C_LSB     0x0C
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_POR_ALARMS_RANGES_INPUT_C_MSB    0x00
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_POR_ALARMS_RANGES_INPUT_C_LSB    0x0D
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_OC_OE_RESOLUTION_INPUT_D_MSB     0x00
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_OC_OE_RESOLUTION_INPUT_D_LSB     0x0E
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_POR_ALARMS_RANGES_INPUT_D_MSB    0x00
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_POR_ALARMS_RANGES_INPUT_D_LSB    0x0F

#define DS2450_ADDRESS_MEMORY_MAP_PAGE_2_ALARM_SETTINGS_LOW_INPUT_A_MSB      0x00
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_2_ALARM_SETTINGS_LOW_INPUT_A_LSB      0x10
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_2_ALARM_SETTINGS_HIGH_INPUT_A_MSB     0x00
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_2_ALARM_SETTINGS_HIGH_INPUT_A_LSB     0x11
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_2_ALARM_SETTINGS_LOW_INPUT_B_MSB      0x00
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_2_ALARM_SETTINGS_LOW_INPUT_B_LSB      0x12
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_2_ALARM_SETTINGS_HIGH_INPUT_B_MSB     0x00
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_2_ALARM_SETTINGS_HIGH_INPUT_B_LSB     0x13
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_2_ALARM_SETTINGS_LOW_INPUT_C_MSB      0x00
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_2_ALARM_SETTINGS_LOW_INPUT_C_LSB      0x14
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_2_ALARM_SETTINGS_HIGH_INPUT_C_MSB     0x00
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_2_ALARM_SETTINGS_HIGH_INPUT_C_LSB     0x15
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_2_ALARM_SETTINGS_LOW_INPUT_D_MSB      0x00
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_2_ALARM_SETTINGS_LOW_INPUT_D_LSB      0x16
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_2_ALARM_SETTINGS_HIGH_INPUT_D_MSB     0x00
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_2_ALARM_SETTINGS_HIGH_INPUT_D_LSB     0x17

#define DS2450_ADDRESS_MEMORY_MAP_PAGE_3_VCC_CONTROL_BYTE_MSB     0x00
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_3_VCC_CONTROL_BYTE_LSB     0x1C

#define DS2450_DATA_MEMORY_MAP_PAGE_3_VCC_CONTROL_BYTE     0x40 /*VCC powered mode*/

#define DS2450_OUTPUT_CONTROL_INPUT_A 0x0
#define DS2450_OUTPUT_CONTROL_INPUT_B 0x0
#define DS2450_OUTPUT_CONTROL_INPUT_C 0x0
#define DS2450_OUTPUT_CONTROL_INPUT_D 0x0

#if 1
#define DS2450_RESOLUTION_INPUT_A     0x01000 /* 8, bit resolution, only mask 0xFF is taken, so 0x00 -> 16 bits, ..0x01 -> 1 bit */
#define DS2450_RESOLUTION_INPUT_B     0x01000
#define DS2450_RESOLUTION_INPUT_C     0x01000
#define DS2450_RESOLUTION_INPUT_D     0x01000
#else
#define DS2450_RESOLUTION_INPUT_A     0x10000 /* 16, bit resolution, only mask 0xFF is taken, so 0x00 -> 16 bits, ..0x01 -> 1 bit */
#define DS2450_RESOLUTION_INPUT_B     0x10000
#define DS2450_RESOLUTION_INPUT_C     0x10000
#define DS2450_RESOLUTION_INPUT_D     0x10000
#endif
#define DS2450_POWER_ON_RESET_INPUT_A 0x1
#define DS2450_POWER_ON_RESET_INPUT_B 0x1
#define DS2450_POWER_ON_RESET_INPUT_C 0x1
#define DS2450_POWER_ON_RESET_INPUT_D 0x1

#define DS2450_ALARM_ENABLE_INPUT_A 0x3 /*ALARM ENABLE LOW: 0x1 and ALARM ENABLE HIGH: 0x2 */
#define DS2450_ALARM_ENABLE_INPUT_B 0x0 /*??*/
#define DS2450_ALARM_ENABLE_INPUT_C 0x0 /*??*/
#define DS2450_ALARM_ENABLE_INPUT_D 0x0 /*??*/

#define DS2450_INPUT_RANGE_INPUT_A 0x1 /* 0: 2.55 V, 1: 5.10 V */
#define DS2450_INPUT_RANGE_INPUT_B 0x1 /* 0: 2.55 V, 1: 5.10 V */
#define DS2450_INPUT_RANGE_INPUT_C 0x1 /* 0: 2.55 V, 1: 5.10 V */
#define DS2450_INPUT_RANGE_INPUT_D 0x1 /* 0: 2.55 V, 1: 5.10 V */

#warning TODO: make it changeable through API, by changing the resolution for DS2450, since max time is (4 x resolution * 80) us + 160us offset (max 5.3 ms)

#define DS2450_CONVERSION_CHANNEL_SELECT_MASK 0x0F
#define DS2450_CONVERSION_READOUT_CONTROL     0xAA

#ifndef OWI_ADC_MAX_CONVERSION_TIME_MILLISECONDS
#define OWI_ADC_MAX_CONVERSION_TIME_MILLISECONDS 8
#endif

#ifndef OWI_ADC_CONVERSION_DELAY_MILLISECONDS
#define OWI_ADC_CONVERSION_DELAY_MILLISECONDS 1
#endif

uint16_t owiAdcMask = 0;
uint16_t* p_owiAdcMask = &owiAdcMask;
uint16_t owiAdcTimeoutMask = 0xFFFF; /*bit mask of non converted channels/pins 1:conversion timeout*/
uint8_t owiUseCommonAdcConversion_flag = TRUE;

/* global address settings */
#warning TODO: move those to PROGMEM
uint8_t addressOutputAndResolution[4][2] =
   {
            {
                     DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_OC_OE_RESOLUTION_INPUT_A_LSB,
                     DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_OC_OE_RESOLUTION_INPUT_A_MSB
            },
            {
                     DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_OC_OE_RESOLUTION_INPUT_B_LSB,
                     DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_OC_OE_RESOLUTION_INPUT_B_MSB
            },
            {
                     DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_OC_OE_RESOLUTION_INPUT_C_LSB,
                     DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_OC_OE_RESOLUTION_INPUT_C_MSB
            },
            {
                     DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_OC_OE_RESOLUTION_INPUT_D_LSB,
                     DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_OC_OE_RESOLUTION_INPUT_D_MSB
            }
   };

uint8_t addressPORandAlarmsAndInputRange[4][2] =
{
         {
                  DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_POR_ALARMS_RANGES_INPUT_A_LSB,
                  DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_POR_ALARMS_RANGES_INPUT_A_MSB
         },
         {
                  DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_POR_ALARMS_RANGES_INPUT_B_LSB,
                  DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_POR_ALARMS_RANGES_INPUT_B_MSB
         },
         {
                  DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_POR_ALARMS_RANGES_INPUT_C_LSB,
                  DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_POR_ALARMS_RANGES_INPUT_C_MSB
         },
         {
                  DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_POR_ALARMS_RANGES_INPUT_D_LSB,
                  DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_POR_ALARMS_RANGES_INPUT_D_MSB
         }
};

/*
 * this function contains all the functions that are necessary to
 * convert analog value to digital via one wire bus
 */

void owiReadADCs( struct uartStruct *ptr_uartStruct )
{
   uint8_t foundDevices = 0;

   /* check for syntax:
    *    allowed arguments are:
    *       - (empty)
    *       - ID
    *       - ID conversion_flag
    */

   switch (ptr_uartStruct->number_of_arguments)
   {
      case 0:
         break;
      case 1:
      case 2:
      case 3:
         /* read single ID w/o adc conversion w/o initialization
          * or
          * read single ID w/ adc conversion of all buses but w/o initialization
          * or
          * read single ID w/ adc conversion of all buses and w/ initialization */
         if ( FALSE == ptr_owiStruct->idSelect_flag)
         {
            snprintf_P(message, BUFFER_SIZE, PSTR("invalid arguments"));
            CommunicationError(ERRA, -1, TRUE, message, COMMUNICATION_ERROR_USE_GLOBAL_MESSAGE_STRING_INDEX_THRESHOLD -1 );
            return;
            break;
         }
         break;
      default:
         snprintf_P(message, BUFFER_SIZE, PSTR("write argument: too many arguments"));
         CommunicationError(ERRA, -1, TRUE, message, COMMUNICATION_ERROR_USE_GLOBAL_MESSAGE_STRING_INDEX_THRESHOLD -1 );
         return;
         break;
   }

   /* find available devices and their IDs */
   NumDevicesFound = owiReadDevicesID(BUSES);

   if ( 0 < NumDevicesFound )
   {
      /* reset mask of busses with sensors*/
      owiAdcMask = 0;

      // scan for busses/pins connected to an ADC
      if ( 0 < owiScanIDS(FAMILY_DS2450_ADC,p_owiAdcMask))
      {

         /* Initialization */

#warning TODO: is the Initialization always needed here, or is it just needed once at the beginning? MOVE IT TO the beginning !!!

         switch (ptr_uartStruct->number_of_arguments)
         {
            case 0:
               /* read all */
               owiInitializeADCs(BUSES);
               break;
            case 3:
               /* read single ADC w/ conversion and initialization*/
               if (TRUE == ptr_owiStruct->init_flag) { owiInitializeADCs(BUSES); }
               break;
         }


         /* conversions */

         switch (ptr_uartStruct->number_of_arguments)
         {
            case 0:
               /* read all */
               owiMakeADCConversions(BUSES);
               break;
            case 2:
               /* read single ID w/ adc conversion on all busses */
               /* during the filling it couldn't decide weather the 2nd argument
                * is a flag or a value ... no we can, its a flag */
               ptr_owiStruct->conv_flag = ( 0 != ptr_owiStruct->value);
               if (TRUE == ptr_owiStruct->conv_flag) { owiMakeADCConversions(BUSES); }
               break;
            case 3:
               /* read single ADC w/ conversion and initialization*/
               if (TRUE == ptr_owiStruct->conv_flag) { owiMakeADCConversions(BUSES); }
               break;
         }

         /*
          * access values
          */

         if ( eventDebug <= debug && ( ( debugMask >> debugOWIADC ) & 0x1 ) )
         {
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) fcn:%s call: FindFamilyDevicesAndGetValues"), __LINE__, __FILE__, __FUNCTION__);
            UART0_Send_Message_String(NULL,0);
         }

         /*    - read DS2450 */
         foundDevices += owiFindFamilyDevicesAndAccessValues(BUSES, NumDevicesFound, FAMILY_DS2450_ADC, NULL );

         if ( TRUE == ptr_owiStruct->idSelect_flag && 0 == foundDevices)
         {
            general_errorCode = CommunicationError(ERRG, -1, TRUE, PSTR("no matching ID was found"), 4000);
         }

         if ( eventDebug <= debug && ( ( debugMask >> debugOWIADC ) & 0x1 ) )
         {
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) fcn:%s end"), __LINE__, __FILE__, __FUNCTION__);
            UART0_Send_Message_String(NULL,0);
         }
      }
   }
}//END of owiReadADCs function

/*
 *this function initializes the analog to digital converter
 */
int8_t owiInitializeADCs( uint8_t *pins )
{
#warning TODO: make it possible to access init settings for single and all devices and change the memory map addresses online

   uint8_t data = 0x0;
   static const uint8_t maxTrials = 3;

   uint8_t dataOutputAndResolution[4];
   uint8_t dataPORandAlarmsAndInputRange[4];

   /* presets */
   /* *  Power on Reset, Alarm enable, Input range */
   /*    -  Channel A */
   dataPORandAlarmsAndInputRange[0] = 0x0;
   dataPORandAlarmsAndInputRange[0] |= (DS2450_POWER_ON_RESET_INPUT_A & 0x1) << 7;
   dataPORandAlarmsAndInputRange[0] |= (  DS2450_ALARM_ENABLE_INPUT_A & 0x3) << 2;
   dataPORandAlarmsAndInputRange[0] |= (   DS2450_INPUT_RANGE_INPUT_A & 0x1) << 0;
   /*    -  Channel B */
   dataPORandAlarmsAndInputRange[1] = 0x0;
   dataPORandAlarmsAndInputRange[1] |= (DS2450_POWER_ON_RESET_INPUT_B & 0x1) << 7;
   dataPORandAlarmsAndInputRange[1] |= (  DS2450_ALARM_ENABLE_INPUT_B & 0x3) << 2;
   dataPORandAlarmsAndInputRange[1] |= (   DS2450_INPUT_RANGE_INPUT_B & 0x1) << 0;
   /*    -  Channel C */
   dataPORandAlarmsAndInputRange[2] = 0x0;
   dataPORandAlarmsAndInputRange[2] |= (DS2450_POWER_ON_RESET_INPUT_C & 0x1) << 7;
   dataPORandAlarmsAndInputRange[2] |= (  DS2450_ALARM_ENABLE_INPUT_C & 0x3) << 2;
   dataPORandAlarmsAndInputRange[2] |= (   DS2450_INPUT_RANGE_INPUT_C & 0x1) << 0;
   /*    -  Channel D */
   dataPORandAlarmsAndInputRange[3] = 0x0;
   dataPORandAlarmsAndInputRange[3] |= (DS2450_POWER_ON_RESET_INPUT_D & 0x1) << 7;
   dataPORandAlarmsAndInputRange[3] |= (  DS2450_ALARM_ENABLE_INPUT_D & 0x3) << 2;
   dataPORandAlarmsAndInputRange[3] |= (   DS2450_INPUT_RANGE_INPUT_D & 0x1) << 0;

   /* * Output Control and Resolution */
   /*    - Channel A */
   dataOutputAndResolution[0] = 0x0;
   dataOutputAndResolution[0] |= (DS2450_OUTPUT_CONTROL_INPUT_A & 0x3) << 6;
   dataOutputAndResolution[0] |= (    DS2450_RESOLUTION_INPUT_A & 0xF) << 0;
   /*    - Channel B */
   dataOutputAndResolution[0] = 0x0;
   dataOutputAndResolution[0] |= (DS2450_OUTPUT_CONTROL_INPUT_B & 0x3) << 6;
   dataOutputAndResolution[0] |= (    DS2450_RESOLUTION_INPUT_B & 0xF) << 0;
   /*    - Channel C */
   dataOutputAndResolution[0] = 0x0;
   dataOutputAndResolution[0] |= (DS2450_OUTPUT_CONTROL_INPUT_C & 0x3) << 6;
   dataOutputAndResolution[0] |= (    DS2450_RESOLUTION_INPUT_C & 0xF) << 0;
   /*    - Channel D */
   dataOutputAndResolution[0] = 0x0;
   dataOutputAndResolution[0] |= (DS2450_OUTPUT_CONTROL_INPUT_D & 0x3) << 6;
   dataOutputAndResolution[0] |= (    DS2450_RESOLUTION_INPUT_D & 0xF) << 0;

   /*SET UP non parasitic mode*/
   for ( int8_t b = 0 ; b < PIN_BUS ; b++ )
   {
      // continue if bus doesn't contain any ADCS
      if ( 0 == (owiAdcMask & (0x1 << b)))
      {
         continue;
      }

      if ( 0 == OWI_DetectPresence(pins[b]) )
      { /* the "DetectPresence" function already sends a Reset*/
         continue; // Error
      }

      /*
       * changing settings of VCC_CONTROL_BYTE
       */
      data = 0x0;
      data = DS2450_DATA_MEMORY_MAP_PAGE_3_VCC_CONTROL_BYTE;
      if ( 0 != owiADCMemoryWriteByte(pins[b],NULL,
                                      DS2450_ADDRESS_MEMORY_MAP_PAGE_3_VCC_CONTROL_BYTE_LSB,
                                      DS2450_ADDRESS_MEMORY_MAP_PAGE_3_VCC_CONTROL_BYTE_MSB,
                                      data, maxTrials))
      {
         CommunicationError(ERRG, -1, FALSE, PSTR("initialization of 1-wire ADCs: failed to set VCC_CONTROL_BYTE"), 1000);
      }


      /*
       * changing settings of channels
       */

      for (int channel = 0; channel < 4; channel++)
      {
         if ( 0 != owiADCMemoryWriteByte(pins[b], NULL,
                                         addressPORandAlarmsAndInputRange[channel][0],
                                         addressPORandAlarmsAndInputRange[channel][1],
                                         dataPORandAlarmsAndInputRange[channel], maxTrials))
         {
            CommunicationError(ERRG, -1, FALSE, PSTR("initialization of 1-wire ADCs: failed to set POR, ALARM ENABLE, and INPUT RANGE"), 1000);
         }

         if ( 0 != owiADCMemoryWriteByte(pins[b], NULL,
                                         addressOutputAndResolution[channel][0],
                                         addressOutputAndResolution[channel][1],
                                         dataOutputAndResolution[0], maxTrials))
         {
            CommunicationError(ERRG, -1, FALSE, PSTR("initialization of 1-wire ADCs: failed to set OUTPUT CONTROL and RESOLUTION"), 1000);
         }
      }
   }
   return 1;
}//END of owiInitializeADCs function

/*
 * owiADCMemoryWriteByte (unsigned char bus_pattern, unsigned char * id, uint8_t addressLSB, uint8_t addressMSB, uint8_t data, uint8_t maxTrials)
 *
 *
 * this function implements the writing of a data byte to an 16 byte address of an owi device
 * using the 0x55 command
 *
 * if a single id is specified also a data verification check is performed with a limited number of trials
 * CRC checks are not performed
 *
 * input:
 *    bus_pattern:
 *                 pattern of bus lines to send command to
 *    id         :
 *                 1-wire device id 8 byte char array
 *                    - if set, i.e. not NULL, only device with this id is addressed (MATCH_ROM)
 *                    - else every device is used (ROM_SKIP)
 *    addressLSB/
 *    addressMSB :
 *                 lower (LSB) and upper (MSB) byte of address to write data to
 *    data       :
 *                 data byte to be send
 *    maxTrials  :
 *                 number of trials to be performed before sending a failed, 0 will be increased to 1, give it at least a try
 *
 * return:
 *    0: ok
 *    1: failed
 *
 */
uint8_t owiADCMemoryWriteByte(unsigned char bus_pattern, unsigned char * id, uint8_t addressLSB, uint8_t addressMSB, uint8_t data, uint8_t maxTrials)
{
   uint16_t receive_CRC;
   uint8_t flag = FALSE;
   uint8_t verificationData = 0x0;
   uint8_t trialsCounter = maxTrials;

   /* 0 trials, give it a chance */
   if (0 == trialsCounter) { trialsCounter++;}

   while ( FALSE == flag && trialsCounter != 0)
   {
      if ( id == NULL)
      {
         OWI_SendByte(OWI_ROM_SKIP, bus_pattern);
      }
      else
      {
         OWI_MatchRom(id, bus_pattern); // Match id found earlier
      }

      OWI_SendByte(DS2450_WRITE_MEMORY, bus_pattern); //write memory
      OWI_SendByte(addressLSB, bus_pattern);
      OWI_SendByte(addressMSB, bus_pattern);

      OWI_SendByte(data, bus_pattern);

      receive_CRC = OWI_ReceiveByte(bus_pattern);           /* IMPORTANT AFTER EACH 'MEMORY WRITE' OPERATION to start memory writing*/
      receive_CRC |= ( OWI_ReceiveByte(bus_pattern) << 8 ); /* IMPORTANT AFTER EACH 'MEMORY WRITE' OPERATION to start memory writing*/

#warning TODO: add a complex check on the CRC if in single device mode
      /* only possible if there is not OWI_ROM_SKIP, so wait until this device specific is implemented*/
      if (id != NULL)
      {
      /*verify write process*/
         verificationData = OWI_ReceiveByte(bus_pattern);
         if ( data == verificationData ) /* check passed */
         {
            flag = TRUE;
         }
         else
         {
            trialsCounter--;
         }
      }
      else
      {
         flag = TRUE;
      }

      OWI_DetectPresence(bus_pattern); /*the "DetectPresence" function already sends a Reset*/
   }

   if (FALSE == flag)
   {
      return 1;
   }
   else
   {
      return 0;
   }
}

/*
 *this function makes ADC conversation of all found devices
 */
int8_t owiMakeADCConversions( uint8_t *pins )
{
   if ( eventDebug <= debug && ( ( debugMask >> debugOWIADC ) & 0x1 ) )
   {
      snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) fcn:MakeADCConversionOfAll"), __LINE__, __FILE__);
      UART0_Send_Message_String(NULL,0);
   }

   uint16_t currentTimeoutMask = 0;
   uint16_t maxConversionTime = OWI_ADC_MAX_CONVERSION_TIME_MILLISECONDS;
   static unsigned char timeout_flag;
   static uint32_t count;
   static uint32_t maxcount;
   uint8_t commonPins = 0x0;
   uint8_t currentPins = 0x0;
   uint8_t busPatternIndexMax = 0;

   uint16_t receive_CRC;

   /* first loop checking busPattern against masks and creating common bus mask*/

   for ( int8_t busPatternIndex = 0 ; busPatternIndex < PIN_BUS ; busPatternIndex++ )
   {
      // continue if bus isn't active
      if ( 0 == ((owiBusMask & pins[busPatternIndex]) & 0xFF) )
      {
         if ( eventDebug <= debug && ((debugMask >> debugOWIADC) & 1))
         {
            snprintf_P(uart_message_string, BUFFER_SIZE - 1,
                       PSTR("DEBUG (%4i, %s) fcn:MakeADCConversionOfAll bus: %i differs (pin pattern 0x%x owiBusMask 0x%x)"),
                       __LINE__, __FILE__,
                       busPatternIndex, pins[busPatternIndex],owiBusMask);
            UART0_Send_Message_String(NULL,0);
         }

         continue;
      }
      else
      {
         if ( eventDebug <= debug && ((debugMask >> debugOWIADC) & 1))
         {
            snprintf_P(uart_message_string, BUFFER_SIZE - 1,
                       PSTR("DEBUG (%4i, %s) fcn:MakeADCConversionOfAll bus: %i active  (pin pattern 0x%x owiBusMask 0x%x)"),
                       __LINE__, __FILE__,
                       busPatternIndex, pins[busPatternIndex],owiBusMask);
            UART0_Send_Message_String(NULL,0);
         }
      }
      // continue if bus doesn't contain any ADCs
      if ( 0 == ((owiAdcMask & pins[busPatternIndex]) & 0xFF ) )
      {
         if ( eventDebug <= debug && ((debugMask >> debugOWIADC) & 1))
         {
            snprintf_P(uart_message_string, BUFFER_SIZE - 1,
                       PSTR("DEBUG (%4i, %s) fcn:MakeADCConversionOfAll bus: %i ADCs: NONE (pin pattern 0x%x owiAdcMask 0x%x)"),
                       __LINE__, __FILE__,
                       busPatternIndex, pins[busPatternIndex],owiAdcMask);
            UART0_Send_Message_String(NULL,0);
         }

         continue;
      }
      else
      {
         if ( eventDebug <= debug && ((debugMask >> debugOWIADC) & 1))
         {
            snprintf_P(uart_message_string, BUFFER_SIZE - 1,
                       PSTR("DEBUG (%4i, %s) fcn:MakeADCConversionOfAll bus: %i ADCs: some (pin pattern 0x%x owiAdcMask 0x%x)"),
                       __LINE__, __FILE__,
                       busPatternIndex, pins[busPatternIndex],owiAdcMask);
            UART0_Send_Message_String(NULL,0);
         }
      }

      if ( TRUE == owiUseCommonAdcConversion_flag)
      {
         if ( eventDebug <= debug && ((debugMask >> debugOWIADC) & 1))
         {
            snprintf_P(uart_message_string, BUFFER_SIZE - 1,
                       PSTR("DEBUG (%4i, %s) fcn:MakeAdcConversionOfAll bus: %i combining 0x%x to common set of pins 0x%x)"),
                       __LINE__, __FILE__,
                       busPatternIndex, pins[busPatternIndex],commonPins);
            UART0_Send_Message_String(NULL,0);
         }

         commonPins |= pins[busPatternIndex];
      }
   }

   if ( TRUE == owiUseCommonAdcConversion_flag)
   {
      if ( eventDebug <= debug && ((debugMask >> debugOWIADC) & 1))
      {
         snprintf_P(uart_message_string, BUFFER_SIZE - 1,
                    PSTR("DEBUG (%4i, %s) fcn:MakeAdcConversionOfAll final common pins: 0x%x)"),
                    __LINE__, __FILE__,
                    commonPins);
         UART0_Send_Message_String(NULL,0);

      }

      busPatternIndexMax = 1; /*only once */
   }
   else
   {
      busPatternIndexMax = PIN_BUS;
   }

   for ( int8_t busPatternIndex = 0 ; busPatternIndex < busPatternIndexMax ; busPatternIndex++ )
   {
      if ( TRUE == owiUseCommonAdcConversion_flag)
      {
         currentPins = commonPins;
      }
      else
      {
         currentPins = pins[busPatternIndex];

         // continue if bus isn't active
         if ( 0 == ((owiBusMask & currentPins) & 0xFF) ){  continue; }

         // continue if bus doesn't contain any ADCs
         if ( 0 == ((owiAdcMask & currentPins) & 0xFF ) ){  continue; }

      }

      /* now first access to bus, within the function  */
      if ( 0 == OWI_DetectPresence(currentPins) )
      {
         if ( eventDebug <= debug && ((debugMask >> debugOWIADC) & 1))
         {
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) fcn:%s bus: %i   no Device present (pin pattern 0x%x)"), __LINE__, __FILE__, __FUNCTION__,
                       busPatternIndex, currentPins);
            UART0_Send_Message_String(NULL,0);
         }

         continue;
      }
      else
      {
         if ( eventDebug <= debug && ((debugMask >> debugOWIADC) & 1))
         {
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) fcn:%s bus: %i some devices present (pin pattern 0x%x)"), __LINE__, __FILE__, __FUNCTION__,
                       busPatternIndex, currentPins);
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) fcn:%s bus: %i starting conversion sequence"), __LINE__, __FILE__, __FUNCTION__, busPatternIndex );
            UART0_Send_Message_String(NULL,0);
         }
      }

      /*starting conversion sequence on all IDs */

      maxcount = ( OWI_ADC_CONVERSION_DELAY_MILLISECONDS > 0 ) ? maxConversionTime / OWI_ADC_CONVERSION_DELAY_MILLISECONDS : 1;

      count = maxcount;
      timeout_flag = FALSE;

      //    "loop waiting for the conversion of all temperature sensors"
      if ( eventDebug <= debug && ( ( debugMask >> debugOWIADC ) & 0x1 ) )
      {
         snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) waiting for conversion"), __LINE__, __FILE__);
         UART0_Send_Message_String(NULL,0);
      }

      OWI_SendByte(OWI_ROM_SKIP, currentPins);
      OWI_SendByte(DS2450_CONVERT, currentPins); /*conversion*/
      OWI_SendByte(DS2450_CONVERSION_CHANNEL_SELECT_MASK, currentPins); /* select mask*/
      OWI_SendByte(DS2450_CONVERSION_READOUT_CONTROL, currentPins); /* select read out control*/

      receive_CRC = OWI_ReceiveByte(currentPins);           /*IMPORTANT AFTER EACH 'MEMORY WRITE' OPERATION*/
      receive_CRC |= ( OWI_ReceiveByte(currentPins) << 8 ); /*IMPORTANT AFTER EACH 'MEMORY WRITE' OPERATION*/

      //loop that waits for the conversion to be done

      while ( OWI_ReadBit(currentPins) == 0 )
      {
         _delay_ms(OWI_ADC_CONVERSION_DELAY_MILLISECONDS);

         /* timeout check */
         if ( 0 == --count)
         {
            timeout_flag = TRUE;
            break;
         }
      }

      if ( FALSE == timeout_flag )
      {
         owiAdcTimeoutMask &= ~(currentPins);
         if ( eventDebug <= debug && ( ( debugMask >> debugOWIADC ) & 0x1 ) )
         {
            snprintf_P(uart_message_string, BUFFER_SIZE - 1,
                       PSTR("DEBUG (%4i, %s) fcn:MakeAdcConversionOfAllDevices waited %i times a delay of"),
                       __LINE__, __FILE__, maxcount - count);
            snprintf(uart_message_string, BUFFER_SIZE - 1, "%s %i ms", uart_message_string, OWI_ADC_CONVERSION_DELAY_MILLISECONDS);
            UART0_Send_Message_String(NULL,0);
         }

         if ( count > 0 )
         {
            /*wait the remaining time*/
            if ( eventDebug <= debug && ( ( debugMask >> debugOWIADC ) & 0x1 ) )
            {
               snprintf_P(uart_message_string, BUFFER_SIZE - 1,
                          PSTR("DEBUG (%4i, %s) fcn:%s waiting the remaining %i times"), __LINE__, __FILE__, __FUNCTION__, count);
               snprintf(uart_message_string, BUFFER_SIZE - 1, "%s %i ms", uart_message_string, OWI_ADC_CONVERSION_DELAY_MILLISECONDS);
               UART0_Send_Message_String(NULL,0);
            }

            while ( 0 < --count ) { _delay_ms(OWI_ADC_CONVERSION_DELAY_MILLISECONDS); }
         }
         if ( eventDebug <= debug && ( ( debugMask >> debugOWIADC ) & 0x1 ) )
         {
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) fcn:%s conversion done"), __LINE__, __FILE__, __FUNCTION__);
            UART0_Send_Message_String(NULL,0);
         }
      }
      else
      {
         owiAdcTimeoutMask |= currentPins;
         currentTimeoutMask |= currentPins;
         CommunicationError(ERRG, -1, 0, PSTR("OWI ADC Conversion timeout"), 200);

         if ( eventDebug <= debug && ( ( debugMask >> debugOWIADC ) & 0x1 ) )
         {
            snprintf_P(uart_message_string, BUFFER_SIZE - 1,
                       PSTR("DEBUG (%4i, %s) OWI Adc Conversion timeout (>%i ms) on bus_mask (%i), bus mask index %i"),
                       __LINE__, __FILE__,  maxConversionTime, currentPins,busPatternIndex);
            UART0_Send_Message_String(NULL,0);
         }
      }

      if ( eventDebug <= debug && ( ( debugMask >> debugOWIADC ) & 0x1 ) )
      {
         snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) fcn:%s make conversion bus %i finished"), __LINE__, __FILE__, __FUNCTION__, busPatternIndex);
         UART0_Send_Message_String(NULL,0);
      }
   }//end of for ( int8_t b = 0 ; b < PIN_BUS ; b++ )

   if ( eventDebug <= debug && ( ( debugMask >> debugOWIADC ) & 0x1 ) )
   {
      snprintf_P(uart_message_string, BUFFER_SIZE - 1,
                 PSTR("DEBUG (%4i, %s) fcn:MakeADCConversionOfAll make conversion finished (owiAdcTimeoutMask = 0x%x)"),
                 __LINE__, __FILE__, owiAdcTimeoutMask);
      UART0_Send_Message_String(NULL,0);
   }
   return 1;
}//END of owiMakeADCConversions function


/*
 *this function gets the ADC-value of all channels of one device
 */
uint32_t owiReadChannelsOfSingleADCs( unsigned char bus_pattern, unsigned char * id, uint16_t *array_chn, const int8_t size )
{
   static const uint8_t maxTrials = 3;
   uint8_t channelIndex = 0;
   uint16_t CRC;
   uint8_t flag = FALSE;
   uint8_t trialsCounter = maxTrials;

   /* 0 trials, give it a chance */
   if (0 == trialsCounter) { trialsCounter++;}

   /*checks*/

   if ( eventDebug <= debug && ( ( debugMask >> debugOWIADC ) & 0x1 ) )
   {
      snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) fcn:%s begin"), __LINE__, __FILE__, __FUNCTION__);
      UART0_Send_Message_String(NULL,0);
   }

   if ( 0 == ((owiBusMask & bus_pattern) & 0xFF) )
   {
      if ( eventDebug <= debug && ( ( debugMask >> debugOWIADC ) & 0x1 ) )
      {
         snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) passive (bus pattern 0x%x owiBusMask 0x%x)"), __LINE__, __FILE__,
                    bus_pattern,owiBusMask);
         UART0_Send_Message_String(NULL,0);
      }
      return owiReadStatus_owi_bus_mismatch << OWI_ADC_DS2450_MAX_RESOLUTION;
   }

   if ( 0 != ((owiAdcTimeoutMask & bus_pattern) & 0xFF) )
   {
      //conversion went into timeout

      snprintf_P(message, BUFFER_SIZE - 1, PSTR("OWI ADC Conversion timeout (>%i ms) on bus_pattern (%i)"),
                 OWI_ADC_MAX_CONVERSION_TIME_MILLISECONDS, bus_pattern);

      CommunicationError(ERRG, -1, 0, message, -1001);
      clearString(message, BUFFER_SIZE);

      return owiReadStatus_conversion_timeout << OWI_ADC_DS2450_MAX_RESOLUTION;
   }
#warning TODO: consider the case that bus_pattern has more than one bit active, but the conversion failed/succeeded not on all the same way

   /* Reset, presence */

   if ( 0 == OWI_DetectPresence(bus_pattern) )
   {
      return owiReadStatus_no_device_presence << OWI_ADC_DS2450_MAX_RESOLUTION; // Error
   }

   /* Send READ MEMORY command
    *
    * The Read Memory command is used to read conversion results, control/status data and alarm settings.
    * The bus master follows the command byte with a two byte address (TA1=(T7:T0), TA2=(T15:T8)) that
    * indicates a starting byte location within the memory map.
    *
    * With every subsequent read data time slot the bus master receives data from the DS2450
    * starting at the supplied address and continuing until the end of
    * an eight-byte page is reached. At that point the bus master will receive a 16-bit CRC of the command byte,
    * address bytes and data bytes. This CRC is computed by the DS2450 and read back by the bus master to check
    * if the command word, starting address and data were received correctly. If the CRC read by the bus master
    * is incorrect, a Reset Pulse must be issued and the entire sequence must be repeated.
    * (http://datasheets.maxim-ic.com/en/ds/DS2450.pdf)
    * */

   while ( FALSE == flag && trialsCounter != 0)
   {
      /* Match id found earlier*/
   OWI_MatchRom(id, bus_pattern); // Match id found earlier

#warning TODO: is the CRC check described above done here? if this sequence is repeated implement a timeout counter

   OWI_SendByte(DS2450_READ_MEMORY, bus_pattern);
   /* set starting address for memory read */
   OWI_SendByte(DS2450_ADDRESS_MEMORY_MAP_PAGE_0_CONVERSION_READOUT_INPUT_A_LSB_LSB, bus_pattern); //Send two bytes address (ie: 0x00 & 0x00,0x08 & 0x00,0x10 & 0x00,0x18 & 0x00)
   OWI_SendByte(DS2450_ADDRESS_MEMORY_MAP_PAGE_0_CONVERSION_READOUT_INPUT_A_LSB_MSB, bus_pattern);

   for (channelIndex = 0; channelIndex < 4; channelIndex++)
   {
      // Read a word place it in the 16 bit channel variable.
      array_chn[channelIndex] = OWI_ReceiveWord(bus_pattern);
   }

   /* Receive CRC */
   CRC = OWI_ReceiveWord(bus_pattern);

   /* Check CRC */
   flag = TRUE;
#if 0
   if ( checkCRC(...))
   {
      flag = TRUE;
   }
   else
   {
      trialsCounter--;
      OWI_DetectPresence(bus_pattern);
   }
#endif
}

   if (FALSE == flag)
   {
#warning TODO: check for this value
      return 1 | (owiReadWriteStatus_MAXIMUM_INDEX);
   }
   else
   {
      if ( eventDebug <= debug && ( ( debugMask >> debugOWIADC ) & 0x1 ) )
      {
         snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) fcn:%s retrieved data and end"), __LINE__, __FILE__, __FUNCTION__);
         UART0_Send_Message_String(NULL,0);
      }
      return 1 | (owiReadWriteStatus_OK << OWI_ADC_DS2450_MAX_RESOLUTION);
   }
}//END of owiReadChannelsOfSingleADCs function


