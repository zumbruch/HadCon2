/*
 * VERSION 1.0 Januar 7th 2010 LATE  File: 'one_wire_adc.c'
 * Author: Linda Fouedjio  based on Alejandro Gil
 * modified (heavily rather rebuild): Peter Zumbruch
 */

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>

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
#include "one_wire_octalSwitch.h"
#include "read_write_register.h"
#include "one_wire_temperature.h"

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

char owi_id_string[OWI_ID_LENGTH];

/*
 * this function collects all ID of devices connected to bus
 * and returns the number of found devices
 */

int8_t owiReadDevicesID( uint8_t *pins )
{
   countDEV = 0; /*reset number of devices */
   unsigned char owiSearchLastBitDeviation = 1;
   unsigned char firstSearchOnBus_flag = TRUE;
   /* TODO: the search is done per bus,
    * check if its also possible to it per bus mask
      */
   printDebug(eventDebug, debugOWI, __func__, __LINE__, PSTR(__FILE__), PSTR("TEST !!!!!  0123456789 123456789 123456789 123456789 123456789 123456789 owiBusMask 0x%x"), owiBusMask);

   for ( int8_t b = 0 ; b < PIN_BUS && countDEV < NUM_DEVICES; b++ )
   {
      /* clear last index of IDs array use as previous Address pointer from IDs array
       * see owiSearch_Rom for more details */
      for ( int8_t a = 0 ; a < 7 ; a++ )
      {
         owi_IDs[NUM_DEVICES - 1][a] = 0;
      }

      /* check bus active mask */
      if ( 0 == ((owiBusMask & pins[b]) & 0xFF) )
      {
    	  if ( eventDebug <= debug && ((debugMask >> debugOWI) & 1))
          {
              snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) fcn:%s - bus: %i passive (pin pattern 0x%x owiBusMask 0x%x)"),
            		  __LINE__, __FILE__, __func__,
            		  b, pins[b],owiBusMask);
              UART0_Send_Message_String(NULL,0);
          }
          continue;
      }
      else
      {
          if ( eventDebug <= debug && ((debugMask >> debugOWI) & 1))
          {
              snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) fcn:%s - bus: %i active  (pin pattern 0x%x owiBusMask 0x%x)"),
            		  __LINE__, __FILE__, __func__,
            		  b, pins[b],owiBusMask);
              UART0_Send_Message_String(NULL,0);
          }
      }

      if ( eventDebug <= debug && ((debugMask >> debugOWI) & 1))
      {
          if ( eventDebug <= debug && ((debugMask >> debugOWI) & 1))
          {
                  snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) fcn:%s - bus: %i active  (pin pattern 0x%x owiBusMask 0x%x)"),
                		  __LINE__, __FILE__, __func__,
                		  b, pins[b],owiBusMask);
                  UART0_Send_Message_String(NULL,0);
          }
      }


      if ( 0 != OWI_DetectPresence(pins[b]) )
      {
         /*this will search the devices IDs till the max number of devices is reached or
          *all the devices have been read out
          */

         if ( eventDebug <= debug && ((debugMask >> debugOWI) & 1))
         {
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) fcn:%s - devices detected"), __LINE__, __FILE__, __func__);
            UART0_Send_Message_String(NULL,0);
         }

         /* find all devices on that bus
          *  - count: countDEVbus
          *  - IDs: owi_IDs
          */

         // reseting bus counter, flag, and positions
         countDEVbus = 0;
         firstSearchOnBus_flag = TRUE;
         owiSearchLastBitDeviation = OWI_ROM_SEARCH_FINISHED + 1;

         while ( countDEV < NUM_DEVICES && owiSearchLastBitDeviation != OWI_ROM_SEARCH_FINISHED )
         {
            owi_IDs_pinMask[countDEV] = 0;

            OWI_DetectPresence(pins[b]);

            /* reset search last bit deviation */
            if ( TRUE == firstSearchOnBus_flag ) { owiSearchLastBitDeviation = 0; firstSearchOnBus_flag = FALSE; }

            /* OWI_SearchRom(bitPattern,...
            *         bitPattern       A pointer to an 8 byte char array where the
            *                          discovered identifier will be placed. When
            *                          searching for several slaves, a copy of the
            *                          last found identifier should be supplied in
            *                          the array, or the search will fail.
            */
            owiSearchLastBitDeviation = OWI_SearchRom(owi_IDs[NUM_DEVICES - 1], owiSearchLastBitDeviation, pins[b]);


            for ( int8_t a = 0 ; a < 8 ; a++ )
            {
               owi_IDs[countDEV][a] = owi_IDs[NUM_DEVICES - 1][a];
            }
            owi_IDs_pinMask[countDEV] = pins[b];

            if ( eventDebug <= debug && ((debugMask >> debugOWI) & 1))
            {
               /*clear strings*/
               clearString(owi_id_string, OWI_ID_LENGTH);
               /* assign current id to string */
               snprintf(owi_id_string, OWI_ID_LENGTH, "%02X%02X%02X%02X%02X%02X%02X%02X",
                        owi_IDs[countDEV][0], owi_IDs[countDEV][1], owi_IDs[countDEV][2],
                        owi_IDs[countDEV][3], owi_IDs[countDEV][4], owi_IDs[countDEV][5],
                        owi_IDs[countDEV][6], owi_IDs[countDEV][7]);

               snprintf_P(uart_message_string, BUFFER_SIZE - 1,
                          PSTR("DEBUG (%4i, %s) fcn:%s - bus: %i (0x%x), search loop total: %i 1/bus: %i ID: %s"),
                         __LINE__, __FILE__, __func__,
                         b, owi_IDs_pinMask[countDEV],
                          countDEV, countDEVbus, owi_id_string );
               UART0_Send_Message_String(NULL,0);
            }

            countDEVbus++;
            countDEV++;

         } // end of while ( countDEV < NUM_DEVICES && res != OWI_ROM_SEARCH_FINISHED )

         if ( eventDebug <= debug && ((debugMask >> debugOWI) & 1))
         {
            snprintf_P(uart_message_string, BUFFER_SIZE - 1,
                       PSTR("DEBUG (%4i, %s) fcn:%s - bus: %i found %i devices"),
                       __LINE__, __FILE__, __func__,
                       b, countDEVbus);
            UART0_Send_Message_String(NULL,0);
         }
      } //end of if((PD=OWI_DetectPresence(pins[b]))!=0)
      else
      {
         if ( eventDebug <= debug && ((debugMask >> debugOWI) & 1))
         {
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) fcn:%s - no_device_is_connected_to_the_bus"), __LINE__, __FILE__, __func__);
            UART0_Send_Message_String(NULL,0);
         }
         continue;
      }
   }// end of for ( int8_t b = 0 ; b < PIN_BUS ; b++ )
   if ( 0 == countDEV )
   {
      general_errorCode = CommunicationError(ERRG, GENERAL_ERROR_no_device_is_connected_to_the_bus, 1, NULL, 0);
   }


   if ( eventDebug <= debug && ((debugMask >> debugOWI) & 1))
   {
      snprintf_P(uart_message_string, BUFFER_SIZE - 1,
                 PSTR("DEBUG (%4i, %s) fcn:%s - all:  found %i devices"),__LINE__, __FILE__, __func__,
                 countDEV);
      UART0_Send_Message_String(NULL,0);
   }

   return countDEV;

}// END of owiReadDevicesID function

/*
 * this function lists IDs on all devices connected to one wire bus
 * the input parameter is the pointer to the uartStruct
 * returns the number of found devices
 */

int8_t owiShowDevicesID( struct uartStruct* ptr_myuartStruct)
{
#define OWI_FOUND_DEVICE_STRING_LENGTH 60
   char foundDevice[OWI_FOUND_DEVICE_STRING_LENGTH]; /*string variable to store all found devices*/
   uint8_t familyCode = 0;
   uint8_t countFoundFamilyCode = 0;
   uint8_t deviceIndex = 0;

   if ( eventDebug <= debug && ((debugMask >> debugOWI) & 1))
   {
      snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) fcn:%s"), __LINE__, __FILE__, __func__);
      UART0_Send_Message_String(NULL,0);
   }

/*
   if ( NULL == ptr_myuartStruct )
   {
	   ptr_myuartStruct = ptr_uartStruct ;
   }
*/


   countDEV = owiReadDevicesID( BUSES );

   /* once clear output string (later cleared by output procedure) */
   clearString(foundDevice, OWI_FOUND_DEVICE_STRING_LENGTH);

   /* check size of family code */
   if ( 0 < ptr_uartStruct->number_of_arguments  && 0xFF < ptr_uartStruct->Uart_Message_ID  )
   {
      if ( eventDebug <= debug && ((debugMask >> debugOWI) & 1))
      {
         snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) fcn:%s - invalid family code %i"),__LINE__, __FILE__, __func__,
        		 ptr_uartStruct->Uart_Message_ID  );
         UART0_Send_Message_String(NULL,0);
      }

      CommunicationError(ERRA, -1, 1, PSTR("invalid family code, exceeding limits [0,0xFF]"),-100);
      return 0;
   }
   else
   {
      if ( eventDebugVerbose <= debug && ((debugMask >> debugOWI) & 1))
      {
         snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) fcn:%s - valid family code %i"),__LINE__, __FILE__, __func__,
        		  ptr_uartStruct->Uart_Message_ID  );
         UART0_Send_Message_String(NULL,0);
      }

      familyCode = (uint8_t) (ptr_uartStruct->Uart_Message_ID & 0xFF);
   }
   countFoundFamilyCode = 0;

   /* once clear output string (later cleared by output procedure) */
   clearString(message, BUFFER_SIZE);
   createReceiveHeader(NULL, message, 0);
   if ( 0 != familyCode )
   {
      snprintf(message, BUFFER_SIZE - 1, "%s%02X ", message, familyCode);
   }

   /*this loop will show the IDs on foundDevice*/
   for ( deviceIndex = 0 ; deviceIndex < countDEV ; deviceIndex++ )
   {
      // output device id of found devices
      // check if requested Family Code (familyCode)
      // does match the current one or are all allowed (0), else continue
      if ( ( familyCode != 0 ) && ( owi_IDs[deviceIndex][0] != familyCode ) )
      {
         continue;
      }

      /*clear strings*/
      clearString(owi_id_string, OWI_ID_LENGTH);

      /* assign current id to string */
      snprintf(owi_id_string, OWI_ID_LENGTH, "%02X%02X%02X%02X%02X%02X%02X%02X",
               owi_IDs[deviceIndex][0], owi_IDs[deviceIndex][1], owi_IDs[deviceIndex][2], owi_IDs[deviceIndex][3],
               owi_IDs[deviceIndex][4], owi_IDs[deviceIndex][5], owi_IDs[deviceIndex][6], owi_IDs[deviceIndex][7]);

      /*send the data to see the list of all found devices*/
      snprintf_P(foundDevice, OWI_FOUND_DEVICE_STRING_LENGTH - 1, PSTR("%sbus mask: "), message, countFoundFamilyCode);
      snprintf(foundDevice, OWI_FOUND_DEVICE_STRING_LENGTH - 1, "%s0x%02X ", foundDevice, owi_IDs_pinMask[deviceIndex]);
      snprintf_P(foundDevice, OWI_FOUND_DEVICE_STRING_LENGTH - 1, PSTR("%sID: %s"), foundDevice, owi_id_string);
      /*send the data*/
      UART0_Send_Message_String(foundDevice, OWI_FOUND_DEVICE_STRING_LENGTH);

      countFoundFamilyCode++;
   } //end of for deviceIndex

   // if any device has been found, add a newline at the end
   if (0 < countFoundFamilyCode)
   {
      clearString(uart_message_string, BUFFER_SIZE);
      snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%sfound %i devices"), message, countFoundFamilyCode);
      UART0_Send_Message_String(NULL,0);
   }

   // found any matching device??
   if ( 0 == countFoundFamilyCode && 0 != familyCode )
   {
      general_errorCode = CommunicationError(ERRG, GENERAL_ERROR_family_code_not_found, TRUE, NULL, 0);
   }//end of if ( 0 == countFoundFamilyCode && 0 != familyCode )

   return countFoundFamilyCode;

}// END of owiShowDevicesID function

/*
 * owiFindFamilyDevicesAndAccessValues
 *
 * this function calls the read/write functions
 *   of the supported 1-wire devices
 * and sends the results to UART
 *
 * supported devices are:
 *
 *    - FAMILY_DS18B20_TEMP
 *    - FAMILY_DS18S20_TEMP
 *    - FAMILY_DS2405_SIMPLE_SWITCH
 *    - FAMILY_DS2413_DUAL_SWITCH
 *    - FAMILY_DS2450_ADC
 *
 * inputs:
 *    - pins:
 *           unsigned int8_t array pointer to the available pin_masks of the one-wire busses
 *    - countDev:
 *           unsigned int8_t number of devices available (found in previous lookup)
 *    - familyCode:
 *           byte to select a subset of the available devices belonging to this 1-wire family Code
 *    - selectedDeviceIndex
 *           index to select just one device by its index in the list of IDs owi_IDs[index]
 *              - < 0 : take everything
 *              - else: use as index
 *              - index >= countDev: error
 *    - ptr_value:
 *           void pointer pointing to the value to be casted by the following structures,
 *           if set switches to write access, else reading
 *              - NULL: read access
 *              - else: write access
 *
 * return values
 *    -  >= 0: number of found devices;
 *    -   < 0: errors;
 */

int8_t owiFindFamilyDevicesAndAccessValues( uint8_t *pins, uint8_t countDev, uint8_t familyCode, void *ptr_value)
{
   int8_t deviceIndex = 0;
   uint32_t readValueADC  = 0;/*variable to read value of ADC channel*/
   uint32_t readValueTemp = 0;/*variable to read temperature of sensor*/
   uint16_t readValueDS   = 0;/*variable to read value of dual switch*/
   uint16_t writeValueDS  = 0;/*variable to write value of dual switch*/
   uint16_t readValueSS   = 0;/*variable to read value of simple switch*/
   
   uint16_t readValueOS   = 0;/*variable to read value of octal switch*/
   uint16_t writeValueOS  = 0;/*variable to write value of octal switch*/
   
   uint16_t ADCValues[4];
   uint8_t foundCounter = 0;
   uint8_t write_flag = FALSE;
   int8_t selectedDeviceIndex = -1;

   /* check input */
   if (NULL == pins)
   {
      CommunicationError(ERRG, -1, 0,
                            PSTR("fcn:FindFamilyDevicesAndGetValues: 1st argument is NULL ... skipping"),
                            COMMUNICATION_ERROR_USE_GLOBAL_MESSAGE_STRING_INDEX_THRESHOLD + 1);
      return -1;
   }

   if ( 0 == countDev )
   {
      general_errorCode = CommunicationError(ERRG, GENERAL_ERROR_no_device_is_connected_to_the_bus, 0, NULL, 0);
      return 0;
   }

   if (ptr_owiStruct->idSelect_flag)
   {
      selectedDeviceIndex = owiFindIdAndGetIndex(ptr_owiStruct->id);
      if ( -1 == selectedDeviceIndex)
      {
         if ( eventDebug <= debug && ( ( debugMask >> debugOWI ) & 0x1 ) )
          {
             snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) fcn:%s - no matching ID found"),
                        __LINE__, __FILE__, __func__,
                        owi_IDs_pinMask[deviceIndex], owiBusMask);
             UART0_Send_Message_String(NULL,0);
          }
         return 0;
      }
      if ( -1 > selectedDeviceIndex )
      {
         CommunicationError(ERRG, -1, 1,
                               PSTR("fcn:owiFindFamilyDevicesAndGetValues: error while searching for id's index ... skipping"),
                               COMMUNICATION_ERROR_USE_GLOBAL_MESSAGE_STRING_INDEX_THRESHOLD + 1);
         return -1;
      }
   }

   if ( selectedDeviceIndex >=  countDev )
   {
      CommunicationError(ERRG, -1, 0,
                           PSTR("fcn:FindFamilyDevicesAndGetValues: selected device index (arg 4) is out of range ... skipping"),
                           COMMUNICATION_ERROR_USE_GLOBAL_MESSAGE_STRING_INDEX_THRESHOLD + 2);
      return -1;
   }

   /* is value pointer pointing to a real value, if not access mode is reading */
   if (NULL == ptr_value)
   {
      write_flag = FALSE;
   }
   else
   {
      write_flag = TRUE;
   }

   for ( deviceIndex = 0; deviceIndex < countDev ; deviceIndex++ )
   {
      /* is index selection used and does current device index match?*/
      if ( 0 <= selectedDeviceIndex && deviceIndex != selectedDeviceIndex) { continue; }

      /* does familyID matches current ID */
      if ( familyCode != owi_IDs[deviceIndex][0] )  { continue; }

      /* bus mask matches device's bus ? */
      if ( 0 == ( ( owiBusMask & owi_IDs_pinMask[deviceIndex] ) & 0xFF ) )
      {
          if ( eventDebug <= debug && ( ( debugMask >> debugOWI ) & 0x1 ) )
           {
              snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) fcn:%s - bus %i doesn't match owiBusMask (0x%x)"),
                         __LINE__, __FILE__, __func__,
                         owi_IDs_pinMask[deviceIndex], owiBusMask);
              UART0_Send_Message_String(NULL,0);
           }
         continue;
      }

      /*clear strings*/
      clearString(message, BUFFER_SIZE);
      clearString(owi_id_string, OWI_ID_LENGTH);

      /* assign current id to string */
      snprintf(owi_id_string, OWI_ID_LENGTH, "%02X%02X%02X%02X%02X%02X%02X%02X",
              owi_IDs[deviceIndex][0], owi_IDs[deviceIndex][1], owi_IDs[deviceIndex][2], owi_IDs[deviceIndex][3],
              owi_IDs[deviceIndex][4], owi_IDs[deviceIndex][5], owi_IDs[deviceIndex][6], owi_IDs[deviceIndex][7]);

#warning TODO: put everything in the cases in one function getXXX(bus, ID) filling the message string, since the output is device specific
#warning       and call the functions via a pre-defined jump table
      if ( FALSE == write_flag )
      {
         if ( eventDebug <= debug && ( ( debugMask >> debugOWI ) & 0x1 ) )
         {
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) fcn:%s - READ ACCESS"), __LINE__, __FILE__, __func__);
            UART0_Send_Message_String(NULL, 0);
         }
         switch ( familyCode )
         {
            case FAMILY_DS2450_ADC:
               if ( eventDebug <= debug && ( ( debugMask >> debugOWIADC ) & 0x1 ) )
               {
                  snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) fcn:%s - bus %i call: ReadADCchannels")
                             ,__LINE__, __FILE__, __func__,
                             owi_IDs_pinMask[deviceIndex]);
                  UART0_Send_Message_String(NULL,0);
               }

               readValueADC = owiReadChannelsOfSingleADCs(owi_IDs_pinMask[deviceIndex], owi_IDs[deviceIndex], ADCValues, 4);

               if ( eventDebug <= debug )
               {
                  snprintf_P(uart_message_string, BUFFER_SIZE - 1,
                             PSTR("DEBUG (%4i, %s) fcn:%s - bus %i: returned %i"),
                            __LINE__, __FILE__, __func__,
                            owi_IDs_pinMask[deviceIndex], readValueADC);
                  UART0_Send_Message_String(NULL,0);
               }

               /* read return status checking */
               if ( 0 != owiCheckReadWriteReturnStatus( readValueADC >> OWI_ADC_DS2450_MAX_RESOLUTION )) { continue; }

               clearString(message, BUFFER_SIZE);
               snprintf(message, BUFFER_SIZE - 1, "%s %.4X %.4X %.4X %.4X",
                        owi_id_string, ADCValues[0], ADCValues[1], ADCValues[2], ADCValues[3]);
               break;
            case FAMILY_DS18S20_TEMP:
            case FAMILY_DS18B20_TEMP:
               if ( eventDebug <= debug )
               {
                  snprintf_P(uart_message_string, BUFFER_SIZE - 1,
                             PSTR("DEBUG (%4i, %s) fcn:%s - bus_pattern %x : returned %i"),
                            __LINE__, __FILE__, __func__,
                            owi_IDs_pinMask[deviceIndex], readValueADC);
                  UART0_Send_Message_String(NULL,0);
               }

               readValueTemp = owiTemperatureReadSingleSensor(owi_IDs_pinMask[deviceIndex], owi_IDs[deviceIndex]);

               /* read return status checking */
               if ( 0 != owiCheckReadWriteReturnStatus( readValueTemp >> 16)) { continue; }

               /* mask off status remainder */
               readValueTemp &= 0xFFFF;

               clearString(message, BUFFER_SIZE);
               snprintf(message, BUFFER_SIZE - 1, "%s %.4lX", owi_id_string,readValueTemp);
               break;
            case FAMILY_DS2408_OCTAL_SWITCH:
            	readValueOS = ReadOctalSwitches(owi_IDs_pinMask[deviceIndex], owi_IDs[deviceIndex]);
            	if ( 0 != owiCheckReadWriteReturnStatus( readValueOS >> 8)) { continue; }

            	/* mask off status remainder */
            	readValueOS &= 0xFF;

            	clearString(message, BUFFER_SIZE);
            	snprintf(message, BUFFER_SIZE - 1, "%s %.2X", owi_id_string, readValueOS);
            	break;
            case FAMILY_DS2413_DUAL_SWITCH:
               readValueDS = ReadDualSwitches(owi_IDs_pinMask[deviceIndex], owi_IDs[deviceIndex]);
               /* read return status checking */
               if ( 0 != owiCheckReadWriteReturnStatus( readValueDS >> 8)) { continue; }

               /* mask off status remainder */
               readValueDS &= 0xFF;

               clearString(message, BUFFER_SIZE);
               snprintf(message, BUFFER_SIZE - 1, "%s %.2X", owi_id_string, readValueDS);
               break;
            case FAMILY_DS2405_SIMPLE_SWITCH:
               readValueSS = ReadSimpleSwitches(owi_IDs_pinMask[deviceIndex], owi_IDs[deviceIndex]);
               /* read return status checking */
               if ( 0 != owiCheckReadWriteReturnStatus( readValueSS >> 8)) { continue; }

               /* mask off status remainder */
               readValueSS &= 0xFF;

               clearString(message, BUFFER_SIZE);
               snprintf(message, BUFFER_SIZE - 1, "%s %.2X", owi_id_string, readValueSS);
               break;
            default:
               general_errorCode = CommunicationError(ERRG, GENERAL_ERROR_undefined_family_code, 0, NULL, 0);
               continue;
               break;
         }
      }
      else /* write access: write_flag >= TRUE */
      {
         if ( eventDebug <= debug && ( ( debugMask >> debugOWI ) & 0x1 ) )
         {
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) fcn:%s - WRITE ACCESS"), __LINE__, __FILE__, __func__);
            UART0_Send_Message_String(NULL, 0);
         }

         switch ( familyCode )
         {
            case FAMILY_DS2408_OCTAL_SWITCH:
               writeValueOS = 0xFF & *((uint8_t*) ptr_value);
               readValueOS = WriteOctalSwitches(owi_IDs_pinMask[deviceIndex], owi_IDs[deviceIndex], writeValueOS);

               /* read return status checking */
               if ( 0 != owiCheckReadWriteReturnStatus( readValueOS >> 8)) { continue; }

               /* mask off status remainder */
               readValueOS &= 0xFF;

               clearString(message, BUFFER_SIZE);
               snprintf(message, BUFFER_SIZE - 1, "%s %.2X", owi_id_string, readValueOS);
            break;

            case FAMILY_DS2413_DUAL_SWITCH:
               writeValueDS = 0xFF & *((uint8_t*) ptr_value);
               readValueDS = WriteDualSwitches(owi_IDs_pinMask[deviceIndex], owi_IDs[deviceIndex], writeValueDS);

               /* read return status checking */
               if ( 0 != owiCheckReadWriteReturnStatus( readValueDS >> 8)) { continue; }

               /* mask off status remainder */
               readValueDS &= 0xFF;

               clearString(message, BUFFER_SIZE);
               snprintf(message, BUFFER_SIZE - 1, "%s %.2X", owi_id_string, readValueDS);
               break;
//            case FAMILY_DS2405_SIMPLE_SWITCH:
//               readValueSS = ReadSimpleSwitches(owi_IDs_pinMask[deviceIndex], owi_IDs[deviceIndex]);
//               /* read return status checking */
//               if ( 0 != owiCheckReadWriteReturnStatus( readValueSS >> 8)) { continue; }
//
//               /* mask off status remainder */
//               readValueSS &= 0xFF;
//
//               clearString(message, BUFFER_SIZE);
//               snprintf(message, BUFFER_SIZE - 1, "%s %.2X", owi_id_string, readValueSS);
               break;
            default:
               general_errorCode = CommunicationError(ERRG, GENERAL_ERROR_undefined_family_code, 0, NULL, 0);
               continue;
               break;
         }
      }

      /* generate header */
      clearString(uart_message_string, BUFFER_SIZE);
      createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);

      /* combine header and message */
      strncat(uart_message_string, message, BUFFER_SIZE -1);

#if HADCON_VERSION == 1
#warning TODO: is it necessary to have a trailing \r and the woLF send
#warning output may hide information by just using '\\r'
      strncat(uart_message_string, "\r", BUFFER_SIZE -1);

      /*send the data*/
      UART0_Send_Message_String_woLF(uart_message_string, BUFFER_SIZE - 1);
#elif HADCON_VERSION == 2
      /*send the data*/
      UART0_Send_Message_String(uart_message_string, BUFFER_SIZE - 1);
#else
#error undefined HADCON_VERSION
#endif
      /*send the data*/
      UART0_Send_Message_String_woLF(uart_message_string, BUFFER_SIZE - 1);

      /*clear strings*/
      clearString(uart_message_string, BUFFER_SIZE);
      clearString(message, BUFFER_SIZE);

      foundCounter++;
   }//for (deviceIndex=0;deviceIndex<countDev;deviceIndex++)

   if ( 0 < foundCounter)
   {
      UART0_Transmit('\n');
   }

   return foundCounter;
}//END of owiFindFamilyDevicesAndAccessValues


void setOneWireBusMask(struct uartStruct *ptr_uartStruct)
{
   /* one-wire set/get active pins/bus mask*/
   /* command     : OWSP value
    * set response: ...
    * get response: RECV OWSP value */

   /*TODO: change CAN naming to more general */

   switch(ptr_uartStruct->number_of_arguments)
   {
      case 0:
         clearString(uart_message_string, BUFFER_SIZE);
         createReceiveHeader(NULL, NULL, 0);
         snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s %x"), uart_message_string, owiBusMask );
         UART0_Send_Message_String(NULL,0);
         break;
      case 1:
      default:
         if ( 0xFFFF < ptr_uartStruct->Uart_Message_ID )
         {
            CommunicationError(ERRA, SERIAL_ERROR_mask_is_too_long, 0, NULL, 0);
            return;
         }
         owiBusMask = (uint16_t) (ptr_uartStruct->Uart_Message_ID & 0xFFFF);
         break;
   }
   if ( verboseDebug <= debug && ( ( debugMask >> debugOWI ) & 0x1 ) )
   {
      if ( 0 < ptr_uartStruct->number_of_arguments)
      {
         ptr_uartStruct->number_of_arguments = 0;
         setOneWireBusMask(ptr_uartStruct);
      }
   }
}

uint32_t getOneWireBusMask(struct uartStruct *ptr_uartStruct)
{
   /*one-wire read active pins/bus mask*/
   /* command : OWRP
    * response: RECV OWRP value */

   ptr_uartStruct->number_of_arguments = 0;
   setOneWireBusMask(ptr_uartStruct);

   return owiBusMask;
}

/* this function scans the list of devices (owi_IDs)
 * for matching family ids.
 * it does an logical OR of the bus pin mask of channel with the provided mask pointer
 * and returns the number of found elements
 * 0 also in case of errors
 */
uint8_t owiScanIDS(uint16_t id, uint16_t *mask)

{
   uint8_t index = 0;
   uint8_t count = 0;
   if (NULL != mask)
   {
      for ( ; index < NumDevicesFound ; index++ )
      {
         if ( id == owi_IDs[index][0] )
         {
            count++;
            *mask |= owi_IDs_pinMask[index];
         }
      }
   }
   return count;
}

/* this function checks the status of the read function
 *
 * and prints out some information if necessary / enabled
 *
 * returns 0 if owiReadWriteStatus_OK
 * else 1
 *
 */
uint8_t owiCheckReadWriteReturnStatus( uint32_t status )
{
   switch ( status )
   {
      case owiReadWriteStatus_OK:
         return 0;
         break;
      case owiReadStatus_owi_bus_mismatch: /*bus pattern didn't match owiBusMask*/
         if ( eventDebug <= debug )
         {
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) fcn:%s - owi bus mask didn't match"),
                       __LINE__, __FILE__, __func__);
            UART0_Send_Message_String(NULL, 0);
         }
         return 1;
         break;
      case owiReadStatus_conversion_timeout: /*conversion time out*/
         return 1;
         break;
      case owiReadStatus_no_device_presence: /*presence pulse without response*/
         CommunicationError(ERRG, GENERAL_ERROR_no_device_is_connected_to_the_bus, 0, NULL, 0);
         return 1;
         break;
      default:
         snprintf_P(message, BUFFER_SIZE - 1, PSTR("Error reading %s"), owi_id_string);
         CommunicationError(ERRG, -1, 0, message, -1003);
         for ( uint8_t clearIndex = 0 ; clearIndex < BUFFER_SIZE ; clearIndex++ )
         {
            message[clearIndex] = STRING_END;
         }
         return 2;
         break;
   }
   return 2;
}

/* this function initializes the structure owiStruct pointed to by ptr_owiStruct
 *
 * input:
 *    - pointer to owiStruct structure to be initialized
 * output
 *    - 0   : if all ok
 *    - else: error
 *
 */

uint8_t owiInitOwiStruct(struct owiStruct *ptr_owiStruct)
{
   if (NULL == ptr_owiStruct)
   {
      return 1;
   }

   unsigned int i = 0;
   for (i=0; i < 8; i++) { ptr_owiStruct->id[i]=0; }
   ptr_owiStruct->idSelect_flag = FALSE;
   ptr_owiStruct->ptr_value = NULL;
   ptr_owiStruct->conv_flag = FALSE;
   ptr_owiStruct->init_flag = FALSE;
   ptr_owiStruct->value = 0;
   clearString(ptr_owiStruct->command, sizeof(ptr_owiStruct->command)/sizeof(char));

   return 0;

}

uint16_t isParameterIDThenFillOwiStructure(uint8_t parameterIndex)
{
   /* calculate length of argument and check if all of them are hex numbers */
   /* if first argument matches the above requirements of an ID
    *    - fill it into the structure
    *    - and set idSelect flag TRUE
    */
   int index = 0;
   char byte[] = "00";
   uint16_t numericLength = 0;

   if ( MAX_PARAMETER < parameterIndex)
   {
      CommunicationError(ERRG,-1,1,PSTR("invalid argument index"), 1000);
      return 0;
   }
   ptr_owiStruct->idSelect_flag = FALSE;

   numericLength = getNumericLength(setParameter[parameterIndex], MAX_LENGTH_PARAMETER);

   if ( eventDebug <= debug && ( ( debugMask >> debugOWI ) & 1 ) )
   {
      snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) fcn:%s - numeric length of argument '%s' is %i"),
                __LINE__, __FILE__, __func__,
                setParameter[parameterIndex], numericLength);
      UART0_Send_Message_String(NULL,0);
   }

   if ( 16 == numericLength)
   {
      /* fill id[] from string */
      for (index = 0; index < 8; index++)
      {
         byte[0] = setParameter[parameterIndex][index*2];
         byte[1] = setParameter[parameterIndex][index*2 +1];
         byte[2] = 0;

         ptr_owiStruct->id[index] = strtol(byte, NULL, 16);
      }

      /* set flag */
      ptr_owiStruct->idSelect_flag = TRUE;
      if ( eventDebug <= debug && ( ( debugMask >> debugOWI ) & 1 ) )
      {
         snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) fcn:%s - argument %iis an ID:"),__LINE__, __FILE__, __func__,
        		   parameterIndex);
         snprintf(uart_message_string, BUFFER_SIZE - 1, "%s %02X%02X%02X%02X%02X%02X%02X%02X", uart_message_string,
                  ptr_owiStruct->id[0], ptr_owiStruct->id[1], ptr_owiStruct->id[2], ptr_owiStruct->id[3],
                  ptr_owiStruct->id[4], ptr_owiStruct->id[5], ptr_owiStruct->id[6], ptr_owiStruct->id[7]);
         UART0_Send_Message_String(NULL,0);
      }
      return 1;
   }
   else
   {
      ptr_owiStruct->idSelect_flag = FALSE;
      if ( eventDebug <= debug && ( ( debugMask >> debugOWI ) & 1 ) )
      {
         snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) fcn:%s - argument %i is NOT an ID"),__LINE__, __FILE__, __func__,
        		 parameterIndex);
         UART0_Send_Message_String(NULL,0);
      }
      return 0;
   }
}


#warning TODO: check if this function can be used for any OW command


/*
 * uint8_t ConvertUartDataToOwiStruct(void)
 *
 * this function assigns the data of the structure uartData to the owi_struct used for many 1-wire commands
 *
 * input : none (accesses global setParameter array)
 * output: none (accesses global structure ptr_owiStruct)
 *
 * After an init of the owi structure
 * the function assumes that the arguments to be analyzed obey to the following possible configurations
 * ( ID represents an hexadecimal 64 bit string, e.g. 2A45B456C89D1278)
 *
 * 0 arguments:
 *       (nothing to do, default values)
 *
 * 1 argument :
 *   - "ID"
 *       - task: read access to a specific device
 *       - action:
 *          - store ID in structure
 *          - set idSelect flag
 *   - "value"
 *       - task: write access to all possible devices
 *       - action:
 *          - store value in structure
 *
 * 2 arguments:
 *   - "ID conversion_flag"
 *       - task: read access to a specific ID with prior conversion
 *       - action:
 *          - store ID in structure
 *          - set idSelect flag
 *          - set conversion_flag if value differs from 0
 *   - "ID value"
 *       - task: write access to a specific device
 *       - action:
 *          - store ID in structure
 *          - set idSelect flag
 *          - store value in structure
 *   - "value init_flag"
 *       - task: write access to all possible devices with prior initialization
 *       - action:
 *          - store value in structure
 *          - set init_flag if value differs from 0
 *
 * 3 arguments:
 *   - "ID conversion_flag init_flag"
 *       - task: read access to a specific ID with prior conversion and initialization
 *       - action:
 *          - store ID in structure
 *          - set idSelect flag
 *          - set conversion_flag if value differs from 0
 *   - "ID value init_flag"
 *       - task: write access to a specific device with initialization
 *       - action:
 *          - store ID in structure
 *          - store value in structure
 *          - set init_flag if value differs from 0
 *
 */
uint8_t ConvertUartDataToOwiStruct(void)
{
#warning THERE IS TOO MUCH INTELLIGENCE IN THIS FUNCTION (reduce it to filling or split it up into ID ,,,,)
   //unsigned int myid =0;
   owiInitOwiStruct(ptr_owiStruct);
   int8_t numberOfArguments = ptr_uartStruct->number_of_arguments;

   if ( eventDebug <= debug && ( ( debugMask >> debugOWI ) & 1 ) )
   {
      snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) fcn:%s - number of arguments: %i"),__LINE__, __FILE__, __func__,
    		  numberOfArguments);
      UART0_Send_Message_String(NULL,0);
   }

   if ( eventDebug <= debug && ((debugMask >> debugOWI) & 1))
   {
      snprintf_P(uart_message_string, BUFFER_SIZE - 1,
                 PSTR("DEBUG (%4i, %s) fcn:%s - %i argument(s)"),
                __LINE__, __FILE__, __func__,
                numberOfArguments);
      UART0_Send_Message_String(NULL,0);

   }

   switch(numberOfArguments)
   {
      case 0:
         /*nothing else to do*/
         break;
      case 1:
      {
         /* args:
          *       ID
          *       value
          *       command
          */

         /* ID */
         isParameterIDThenFillOwiStructure(1);

         /* value / command */
         if (FALSE == ptr_owiStruct->idSelect_flag)
         {
             if ( strlen ( setParameter[1]) == getNumericLength(setParameter[1], MAX_LENGTH_PARAMETER))
             {
                ptr_owiStruct->value = (uint16_t) strtoul(setParameter[1], &ptr_setParameter[1], 16);
                ptr_owiStruct->ptr_value = &(ptr_owiStruct->value);
             }
             else
             {
                strncpy( ptr_owiStruct->command, setParameter[1], MAX_LENGTH_PARAMETER);
             }
         }
      }
         break;
      case 2:
         /* args:
          *       ID    command
          *       ID    convert_flag
          *       ID    value
          *       value init_flag
          *       command argument
          *       command ID
          */

         /* ID */
         isParameterIDThenFillOwiStructure(1);

         /* ID command / ID value/flag */
         if (TRUE == ptr_owiStruct->idSelect_flag)
         {
            /* numeric value*/
            if ( strlen ( setParameter[2]) == getNumericLength(setParameter[2], MAX_LENGTH_PARAMETER))
            {
               /* second argument can either be numeric value (write mode) or numeric convert_flag (read_mode), indistinguishable)
                * assigning to value*/
               ptr_owiStruct->value = (uint16_t) strtoul(setParameter[2], &ptr_setParameter[2], 16);
               ptr_owiStruct->ptr_value = &(ptr_owiStruct->value);
            }
            else /* command */
            {
               strncpy( ptr_owiStruct->command, setParameter[2], MAX_LENGTH_PARAMETER);
            }
         }
         else /* value flag | command value/argument/ID*/
         {
            /* numeric value */
            if ( strlen ( setParameter[1]) == getNumericLength(setParameter[1], MAX_LENGTH_PARAMETER))
            {
               ptr_owiStruct->value = (uint16_t) strtoul(setParameter [1], &ptr_setParameter[1], 16);
               ptr_owiStruct->ptr_value = &(ptr_owiStruct->value);
#warning TODO: generalize this more, it is too specific
               ptr_owiStruct->init_flag = ( 0 != (uint16_t) strtoul(setParameter [2], &ptr_setParameter[2], 16));

               if ( eventDebug <= debug && ( ( debugMask >> debugOWI ) & 1 ) )
               {
                  snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) fcn:%s - second argument sets init_flag to: %i "),__LINE__, __FILE__, __func__,
                		  ptr_owiStruct->init_flag);
                  UART0_Send_Message_String(NULL,0);
               }
            }
            else /* command ID/value/argument */
            {
               /* command */
               strncpy( ptr_owiStruct->command, setParameter[1], MAX_LENGTH_PARAMETER);

               /* 2nd argument */
               /* is 2nd argument ID ?*/
               if ( 0 == isParameterIDThenFillOwiStructure(2) )
               {
                  /* numeric value/flag? */
                  if ( strlen ( setParameter[1]) == getNumericLength(setParameter[1], MAX_LENGTH_PARAMETER))
                  {
                     ptr_owiStruct->value = (uint16_t) strtoul(setParameter [1], &ptr_setParameter[1], 16);
                     ptr_owiStruct->ptr_value = &(ptr_owiStruct->value);
#warning TODO: generalize this more, it is too specific
                     ptr_owiStruct->init_flag = ( 0 != (uint16_t) strtoul(setParameter [2], &ptr_setParameter[2], 16));

                     if ( eventDebug <= debug && ( ( debugMask >> debugOWI ) & 1 ) )
                     {
                        snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) fcn:%s - second argument sets init_flag to: %i "),__LINE__, __FILE__, __func__,
                        		ptr_owiStruct->init_flag);
                        UART0_Send_Message_String(NULL,0);
                     }
                  }
               }
            }
         }
       break;
      case 3:
         /* args:
          *       ID    convert_flag init_flag
          *       ID    value init_flag
          *       ID    command value
          *       command arguments arguments
          *       command ID value
          */

         /* ID */
         isParameterIDThenFillOwiStructure(1);

         /* ID value/flag flag*/
         if (TRUE == ptr_owiStruct->idSelect_flag)
         {
            /* second argument can either be value (write mode) or convert_flag (read_mode), indistinguishable)
             * assigning to value*/
            ptr_owiStruct->value = (uint16_t) strtoul(setParameter [2], &ptr_setParameter[2], 16);
            ptr_owiStruct->ptr_value = &(ptr_owiStruct->value);

            /* third argument can only be a the init_flag*/
            ptr_owiStruct->init_flag = ( 0 != (uint16_t) strtoul(setParameter [3], &ptr_setParameter[3], 16));
         }
         else /* command arguments */
         {
            strncpy( ptr_owiStruct->command, setParameter[1], MAX_LENGTH_PARAMETER);
         }
         break;
      default:
         if (numberOfArguments < MAX_PARAMETER)
         {
            /* args:
             *       command arguments ... arguments
             */

            /* take first argument as command */
            strncpy( ptr_owiStruct->command, setParameter[1], MAX_LENGTH_PARAMETER);
         }
         else
         {
            CommunicationError(ERRA,-1,1,PSTR("invalid number of arguments"), 1000);
            return 1;
         }
         break;
   }
   return 0;
} // END of function ConvertUartDataToOwi(void)


/* owiFindIdAndGetIndex
 * This function searches in the global list of found/available IDs
 * whether the given 1-wire unique id is in this list and returns its index.
 *
 * Input:
 *      - id
 *          pointer to 8 byte array containing the ID to be searched for
 * Return values:
 *      - index of found item [ 0, NUM_DEVICES ]
 *      - -1 if not found
 *      - -1000 in case of error
 *
 */
int16_t owiFindIdAndGetIndex(uint8_t id[])

{
   uint16_t deviceIndex = 0;
   uint16_t positionIndex = 0;
   uint8_t match_flag = TRUE;

   /*
    * check the inputs
    */

   if ( NULL == id)
   {
      return -1000;
   }

   /* search algorithm:
    *   loop over all devices in the global list of found IDs,
    *    1. first set match_flag to true
    *    2. in a second loop compare each element of the 8 bytes array, representing the IDs with
    *       the elements of the current id.
    *    3a. If one comparison fails, the match_flag is set to FALSE, and the loop is left, so the next device can be compared
    *    3b. If after the the second loop the match_flag still is TRUE, return the current index
    *    4. After finishing all devices, i.e. no matching id has been found, return -1
    */

   /* loop over all available and found device IDs*/

   for (deviceIndex = 0; deviceIndex < NUM_DEVICES; deviceIndex++)
   {
      /* reset match flag */
      match_flag = TRUE;

      /* loop over all 8 bytes of the 64bit ID*/
      for (positionIndex = 0; positionIndex < 8; positionIndex++)
      {
         /* compare each byte
          * if different break this loop*/
         if (owi_IDs[deviceIndex][positionIndex] != id[positionIndex])
         {
            match_flag = FALSE;
            break;
         }
      }
      /* if match_flag is still TRUE return current device index*/
      if (TRUE == match_flag ) {return deviceIndex;}
   }

   return -1;
}

/*
 * checkBusAndDeviceActivityMasks(uint8_t pins, int8_t busPatternIndex, uint16_t owiBusMask, uint16_t owiDeviceMask, uint8_t verbose)
 *
 * checks pins against owiBusMask and owiDeviceMask (logical AND)
 * being verbose if verbose set to TRUE, quiet else
 *
 * returns 0
 *      if both masks have common pins
 * returns != 0
 *      if there is no common pin (either bus or device mask)
 */

uint8_t checkBusAndDeviceActivityMasks(uint8_t pins, int8_t busPatternIndex, uint16_t owiBusMask, uint16_t owiDeviceMask, uint8_t verbose)
{
   // continue if bus isn't active
   if ( 0 == ((owiBusMask & pins) & 0xFF) )
   {
      if ( TRUE == verbose && eventDebugVerbose <= debug && ((debugMask >> debugOWI) & 1))
      {
         snprintf_P(uart_message_string, BUFFER_SIZE - 1,
                    PSTR("DEBUG (%4i, %s) fcn:%s - checkBusAndDeviceActivityMasks bus: %i differs (pin pattern 0x%x owiBusMask 0x%x)"),
                   __LINE__, __FILE__, __func__,
                   busPatternIndex, pins, owiBusMask);
         UART0_Send_Message_String(NULL,0);
      }
      return 1;
   }
   else
   {
      if ( TRUE == verbose && eventDebug <= debug && ((debugMask >> debugOWI) & 1))
      {
         snprintf_P(uart_message_string, BUFFER_SIZE - 1,
                    PSTR("DEBUG (%4i, %s) fcn:%s - checkBusAndDeviceActivityMasks bus: %i active  (pin pattern 0x%x owiBusMask 0x%x)"),
                  __LINE__, __FILE__, __func__,
                  busPatternIndex, pins,owiBusMask);
         UART0_Send_Message_String(NULL,0);
      }
   }

   // continue if bus doesn't contain any device typed sensors
   if ( 0 == ((owiDeviceMask & pins) & 0xFF ) )
   {
      if ( TRUE == verbose && eventDebugVerbose <= debug && ((debugMask >> debugOWI) & 1))
      {
         snprintf_P(uart_message_string, BUFFER_SIZE - 1,
                    PSTR("DEBUG (%4i, %s) fcn:%s - checkBusAndDeviceActivityMasks bus: %i Devices: NONE (pin pattern 0x%x owiDeviceMask 0x%x)"),
                   __LINE__, __FILE__, __func__,
                   busPatternIndex, pins,owiDeviceMask);
         UART0_Send_Message_String(NULL,0);
      }

      return 1;
   }
   else
   {
      if ( TRUE == verbose && eventDebug <= debug && ((debugMask >> debugOWI) & 1))
      {
         snprintf_P(uart_message_string, BUFFER_SIZE - 1,
                    PSTR("DEBUG (%4i, %s) fcn:%s - checkBusAndDeviceActivityMasks bus: %i Devices: some (pin pattern 0x%x owiDeviceMask 0x%x)"),
                   __LINE__, __FILE__, __func__,
                   busPatternIndex, pins,owiDeviceMask);
         UART0_Send_Message_String(NULL,0);
      }
   }

   if ( TRUE == verbose && eventDebug <= debug && ((debugMask >> debugOWI) & 1))
   {
      snprintf_P(uart_message_string, BUFFER_SIZE - 1,
                 PSTR("DEBUG (%4i, %s) fcn:%s - checkBusAndDeviceActivityMasks bus: %i passed all criteria)"),
                __LINE__, __FILE__, __func__,
                busPatternIndex);
      UART0_Send_Message_String(NULL,0);
   }

   return 0;
}


/*
 * uint8_t generateCommonPinsPattern(uint8_t *pins, const uint16_t owiBusMask, const uint16_t owiDeviceMask)
 *
 * combines active pins (bus and device mask) of array pins into one common pins set returned
 *
 * input:
 *      pins: array of 8 pin mask
 *      owiBusMask: general activity bus mask
 *      owiDeviceMask: bus mask of currently available devices
 *
 * returns
 *      common pin pattern
 */

uint8_t generateCommonPinsPattern(uint8_t *pins, const uint16_t owiBusMask, const uint16_t owiDeviceMask)
{
   if ( NULL == pins)
   { return 0; }

   uint8_t commonPins = 0;

   if ( eventDebug <= debug && ((debugMask >> debugOWI) & 1))
   {
      snprintf_P(uart_message_string, BUFFER_SIZE - 1,
                 PSTR("DEBUG (%4i, %s) fcn:%s - bus: owiDeviceMask = 0x%x"),
                __LINE__, __FILE__, __func__,
                owiDeviceMask);
      UART0_Send_Message_String(NULL,0);
   }

   for ( int8_t busPatternIndex = 0 ; busPatternIndex < PIN_BUS ; busPatternIndex++ )
   {
      if ( 0 != checkBusAndDeviceActivityMasks(pins[busPatternIndex], busPatternIndex, owiBusMask, owiDeviceMask, TRUE ))
      {
         if ( eventDebugVerbose <= debug && ((debugMask >> debugOWI) & 1))
         {
            snprintf_P(uart_message_string, BUFFER_SIZE - 1,
                       PSTR("DEBUG (%4i, %s) fcn:%s - bus: checkBusAndDeviceActivityMasks failed"),
                      __LINE__, __FILE__, __func__,
                      busPatternIndex, pins[busPatternIndex],commonPins);
            UART0_Send_Message_String(NULL,0);
            continue;
         }
      }
      else
      {
         if ( eventDebug <= debug && ((debugMask >> debugOWI) & 1))
         {
            snprintf_P(uart_message_string, BUFFER_SIZE - 1,
                       PSTR("DEBUG (%4i, %s) fcn:%s - bus: %i combining 0x%x to common set of pins 0x%x)"),
                       __LINE__, __FILE__, __func__,
                       busPatternIndex, pins[busPatternIndex],commonPins);
            UART0_Send_Message_String(NULL,0);
         }
      }

      commonPins |= pins[busPatternIndex];
   }
   if ( eventDebug <= debug && ((debugMask >> debugOWI) & 1))
   {
      snprintf_P(uart_message_string, BUFFER_SIZE - 1,
                 PSTR("DEBUG (%4i, %s) fcn:%s - final common pins: 0x%x"),
                __LINE__, __FILE__, __func__,
                commonPins);
      UART0_Send_Message_String(NULL,0);

   }
   return commonPins;
}

