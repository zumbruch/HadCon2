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
#include "OWIcrc.h"

char owi_id_string[OWI_ID_LENGTH];

void owiCreateIdString(char string[OWI_ID_LENGTH], uint8_t array[])
{
	clearString(string, OWI_ID_LENGTH);
	/* assign current id to string */
	snprintf(string, OWI_ID_LENGTH, "%02X%02X%02X%02X%02X%02X%02X%02X",
			array[0], array[1], array[2], array[3], array[4], array[5], array[6], array[7]);
}

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

   for ( int8_t pinBusPatternIndex = 0 ; pinBusPatternIndex < OWI_MAX_NUM_PIN_BUS && countDEV < OWI_MAX_NUM_DEVICES; pinBusPatternIndex++ )
   {
      /* clear last index of IDs array use as previous Address pointer from IDs array
       * see owiSearch_Rom for more details */
      for ( int8_t a = 0 ; a < 7 ; a++ )
      {
         owi_IDs[OWI_MAX_NUM_DEVICES - 1][a] = 0;
      }

      /* check bus active mask */
      if ( 0 == ((owiBusMask & pins[pinBusPatternIndex]) & 0xFF) )
      {
     	  printDebug_p(debugLevelEventDebug, debugSystemOWI, __LINE__, PSTR(__FILE__), PSTR("message %s"), owiBusMask);

     	  printDebug_p(debugLevelEventDebug, debugSystemOWI, __LINE__, PSTR(__FILE__), PSTR("bus: %i passive (pin pattern 0x%x owiBusMask 0x%x)"), pinBusPatternIndex, pins[pinBusPatternIndex],owiBusMask);
          continue;
      }
      else
      {
           printDebug_p(debugLevelEventDebug, debugSystemOWI, __LINE__, PSTR(__FILE__), PSTR("bus: %i active  (pin pattern 0x%x owiBusMask 0x%x)"), pinBusPatternIndex, pins[pinBusPatternIndex],owiBusMask);
      }

      if ( debugLevelEventDebug <= globalDebugLevel && ((globalDebugSystemMask >> debugSystemOWI) & 0x1))
      {
           printDebug_p(debugLevelEventDebug, debugSystemOWI, __LINE__, PSTR(__FILE__), PSTR("bus: %i active  (pin pattern 0x%x owiBusMask 0x%x)"), pinBusPatternIndex, pins[pinBusPatternIndex],owiBusMask);
      }


      if ( 0 != OWI_DetectPresence(pins[pinBusPatternIndex]) )
      {
         /*this will search the devices IDs till the max number of devices is reached or
          *all the devices have been read out
          */

          printDebug_p(debugLevelEventDebug, debugSystemOWI, __LINE__, PSTR(__FILE__), PSTR("devices detected"));

         /* find all devices on that bus
          *  - count: countDEVbus
          *  - IDs: owi_IDs
          */

         // reseting bus counter, flag, and positions
         countDEVbus = 0;
         firstSearchOnBus_flag = TRUE;
         owiSearchLastBitDeviation = OWI_ROM_SEARCH_FINISHED + 1;

         uint16_t trialsCounter = 0;
         while ( OWI_MAX_NUM_DEVICES > countDEV && owiSearchLastBitDeviation != OWI_ROM_SEARCH_FINISHED && OWI_MAX_NUM_DEVICES > trialsCounter)
         {
        	trialsCounter++;
            owi_IDs_pinMask[countDEV] = 0;

            OWI_DetectPresence(pins[pinBusPatternIndex]);

            /* reset search last bit deviation */
            if ( TRUE == firstSearchOnBus_flag ) { owiSearchLastBitDeviation = 0; firstSearchOnBus_flag = FALSE; }

            /* OWI_SearchRom(bitPattern,...
            *         bitPattern       A pointer to an 8 byte char array where the
            *                          discovered identifier will be placed. When
            *                          searching for several slaves, a copy of the
            *                          last found identifier should be supplied in
            *                          the array, or the search will fail.
            */
            /* - misusing the last array element to store the latest result*/
            owiSearchLastBitDeviation = OWI_SearchRom(owi_IDs[OWI_MAX_NUM_DEVICES - 1], owiSearchLastBitDeviation, pins[pinBusPatternIndex]);

            unsigned char result = OWI_CheckRomCRC(owi_IDs[OWI_MAX_NUM_DEVICES - 1]);
            unsigned char skip_flag = FALSE;
            switch(result)
            {
            case OWI_CRC_OK:
            	printDebug_p(debugLevelEventDebugVerbose, debugSystemOWI, __LINE__, PSTR(__FILE__), PSTR("OWI CRC check passed"));
            	break;
            case OWI_CRC_ERROR:
                owiCreateIdString(owi_id_string, owi_IDs[OWI_MAX_NUM_DEVICES -1]);
            	CommunicationError_p(ERRG, dynamicMessage_ErrorIndex, TRUE, PSTR("bus: %i (Mask:0x%x) - OWI CRC check failure, skipping current id: %s"), pinBusPatternIndex, owiBusMask, owi_id_string);
            	skip_flag = TRUE;
            	break;
            default:
            	CommunicationError_p(ERRG, dynamicMessage_ErrorIndex, TRUE, PSTR("OWI CRC check internal failure"));
            	return countDEV;
            	break;
            }

            if (TRUE == skip_flag)
            {
            	continue;
            }

            for ( int8_t a = 0 ; a < 8 ; a++ )
            {
               owi_IDs[countDEV][a] = owi_IDs[OWI_MAX_NUM_DEVICES - 1][a];
            }
            owi_IDs_pinMask[countDEV] = pins[pinBusPatternIndex];

            if ( debugLevelEventDebug <= globalDebugLevel && ((globalDebugSystemMask >> debugSystemOWI) & 0x1))
            {
               owiCreateIdString(owi_id_string, owi_IDs[countDEV]);
                printDebug_p(debugLevelEventDebug, debugSystemOWI, __LINE__, PSTR(__FILE__), PSTR("bus: %i (0x%x), search loop total: %i 1/bus: %i ID: %s"), pinBusPatternIndex, owi_IDs_pinMask[countDEV], countDEV, countDEVbus, owi_id_string );
            }

            countDEVbus++;
            countDEV++;

         } // end of while ( countDEV < OWI_MAX_NUM_DEVICES && res != OWI_ROM_SEARCH_FINISHED )

          printDebug_p(debugLevelEventDebug, debugSystemOWI, __LINE__, PSTR(__FILE__), PSTR("bus: %i found %i valid devices, within %i trials"), pinBusPatternIndex, countDEVbus, trialsCounter);
      } //end of if((PD=OWI_DetectPresence(pins[pinBusPatternIndex]))!=0)
      else
      {
         printDebug_p(debugLevelEventDebug, debugSystemOWI, __LINE__, PSTR(__FILE__), PSTR("no_device_is_connected_to_the_bus"));
         continue;
      }
   }// end of for ( int8_t b = 0 ; b < OWI_MAX_NUM_PIN_BUS ; b++ )

   if ( 0 == countDEV )
   {
      generalErrorCode = CommunicationError_p(ERRG, GENERAL_ERROR_no_device_is_connected_to_the_bus, TRUE, NULL);
   }


   printDebug_p(debugLevelEventDebug, debugSystemOWI, __LINE__, PSTR(__FILE__), PSTR("found %i devices"), countDEV );

   return countDEV;

}// END of owiReadDevicesID function

/*
 * this function lists IDs on all devices connected to one wire bus
 * the input parameter is the pointer to the uartStruct
 * returns the number of found devices
 */

int8_t owiShowDevicesID( struct uartStruct* ptr_myuartStruct)
{
#warning TODO: remove local foundDevice string, not neccessary
#define OWI_FOUND_DEVICE_STRING_LENGTH 60
   char foundDevice[OWI_FOUND_DEVICE_STRING_LENGTH]; /*string variable to store all found devices*/
   uint8_t familyCode = 0;
   uint8_t countFoundFamilyCode = 0;
   uint8_t deviceIndex = 0;

    printDebug_p(debugLevelEventDebug, debugSystemOWI, __LINE__, PSTR(__FILE__), PSTR(""));
/*
   if ( NULL == ptr_myuartStruct )
   {
	   ptr_myuartStruct = ptr_uartStruct ;
   }
*/

   /* read in list of devices */
   countDEV = owiReadDevicesID( BUSES );

   /* once clear output string (later cleared by output procedure) */
   clearString(foundDevice, OWI_FOUND_DEVICE_STRING_LENGTH);

   /* check size of family code */
   if ( 0 < ptr_uartStruct->number_of_arguments  && 0xFF < ptr_uartStruct->Uart_Message_ID  )
   {
       printDebug_p(debugLevelEventDebug, debugSystemOWI, __LINE__, PSTR(__FILE__), PSTR("invalid family code %i"), ptr_uartStruct->Uart_Message_ID   );

      CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, TRUE, PSTR("invalid family code, exceeding limits [0,0xFF]"));
      return 0;
   }
   else
   {
       printDebug_p(debugLevelEventDebugVerbose, debugSystemOWI, __LINE__, PSTR(__FILE__), PSTR("valid family code %i"), ptr_uartStruct->Uart_Message_ID   );

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
      owiCreateIdString(owi_id_string, owi_IDs[deviceIndex]);

      /*send the data to see the list of all found devices*/
      snprintf_P(foundDevice, OWI_FOUND_DEVICE_STRING_LENGTH - 1, PSTR("%sbus mask: "), message, countFoundFamilyCode);
      snprintf(foundDevice, OWI_FOUND_DEVICE_STRING_LENGTH - 1, "%s0x%02X ", foundDevice, owi_IDs_pinMask[deviceIndex]);
      snprintf_P(foundDevice, OWI_FOUND_DEVICE_STRING_LENGTH - 1, PSTR("%sID: %s"), foundDevice, owi_id_string);
      /*send the data*/
      UART0_Send_Message_String_p(foundDevice, OWI_FOUND_DEVICE_STRING_LENGTH);

      countFoundFamilyCode++;
   } //end of for deviceIndex

   // if any device has been found, add a newline at the end
   if (0 < countFoundFamilyCode)
   {
      clearString(uart_message_string, BUFFER_SIZE);
      snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%sfound %i devices"), message, countFoundFamilyCode);
      UART0_Send_Message_String_p(NULL,0);
   }

   // found any matching device??
   if ( 0 == countFoundFamilyCode && 0 != familyCode )
   {
      generalErrorCode = CommunicationError_p(ERRG, GENERAL_ERROR_family_code_not_found, TRUE, NULL);
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
 *    - OWI_FAMILY_DS18B20_TEMP
 *    - OWI_FAMILY_DS18S20_TEMP
 *    - OWI_FAMILY_DS2405_SIMPLE_SWITCH
 *    - OWI_FAMILY_DS2413_DUAL_SWITCH
 *    - OWI_FAMILY_DS2450_ADC
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
# warning replace by a union
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
      CommunicationError_p(ERRG, dynamicMessage_ErrorIndex, FALSE, PSTR("fcn:FindFamilyDevicesAndGetValues: 1st argument is NULL ... skipping"));
      return -1;
   }

   if ( 0 == countDev )
   {
      generalErrorCode = CommunicationError_p(ERRG, GENERAL_ERROR_no_device_is_connected_to_the_bus, FALSE, NULL);
      return 0;
   }

   if (ptr_owiStruct->idSelect_flag)
   {
      selectedDeviceIndex = owiFindIdAndGetIndex(ptr_owiStruct->id);
      if ( -1 == selectedDeviceIndex)
      {
    	 printDebug_p(debugLevelEventDebug, debugSystemOWI, __LINE__, PSTR(__FILE__), PSTR("no matching ID found"));
         return 0;
      }
      if ( -1 > selectedDeviceIndex )
      {
         CommunicationError_p(ERRG, dynamicMessage_ErrorIndex, TRUE, PSTR("fcn:owiFindFamilyDevicesAndGetValues: error while searching for id's index ... skipping") );
         return -1;
      }
   }

   if ( selectedDeviceIndex >= countDev )
   {
      CommunicationError_p(ERRG, dynamicMessage_ErrorIndex, FALSE, PSTR("fcn:FindFamilyDevicesAndGetValues: selected device index (arg 4) is out of range ... skipping") );
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
           printDebug_p(debugLevelEventDebug, debugSystemOWI, __LINE__, PSTR(__FILE__), PSTR("bus %i doesn't match owiBusMask (0x%x)"), owi_IDs_pinMask[deviceIndex], owiBusMask);
          continue;
      }

      /*clear strings*/
      clearString(message, BUFFER_SIZE);
      clearString(resultString, BUFFER_SIZE);

      /* assign current id to string */
      owiCreateIdString(owi_id_string, owi_IDs[deviceIndex]);

#warning TODO: put everything in the cases in one function getXXX(bus, ID) filling the message string, since the output is device specific
#warning       and call the functions via a pre-defined jump table

      if ( FALSE == write_flag )
      {
     	  printDebug_p(debugLevelEventDebug, debugSystemOWI, __LINE__, PSTR(__FILE__), PSTR("READ ACCESS"));
         switch ( familyCode )
         {
            case OWI_FAMILY_DS2450_ADC:
             	printDebug_p(debugLevelEventDebug, debugSystemOWIADC, __LINE__, PSTR(__FILE__), PSTR("bus %i call: ReadADCchannels"), owi_IDs_pinMask[deviceIndex]);

            	readValueADC = owiReadChannelsOfSingleADCs(owi_IDs_pinMask[deviceIndex], owi_IDs[deviceIndex], ADCValues, 4);

             	printDebug_p(debugLevelEventDebug, debugSystemOWIADC, __LINE__, PSTR(__FILE__), PSTR("bus %i: returned %i"), owi_IDs_pinMask[deviceIndex], readValueADC);

               /* read return status checking */
               if ( 0 != owiCheckReadWriteReturnStatus( readValueADC >> OWI_ADC_DS2450_MAX_RESOLUTION )) { continue; }

               clearString(message, BUFFER_SIZE);
               snprintf(message, BUFFER_SIZE - 1, "%s%s", owi_id_string, resultString);
//               snprintf(message, BUFFER_SIZE - 1, "%s %.4X %.4X %.4X %.4X",
//                        owi_id_string, ADCValues[0], ADCValues[1], ADCValues[2], ADCValues[3]);
               break;
            case OWI_FAMILY_DS18S20_TEMP:
            case OWI_FAMILY_DS18B20_TEMP:
             	printDebug_p(debugLevelEventDebug, debugSystemOWITemperatures, __LINE__, PSTR(__FILE__), PSTR("bus_pattern %x : returned %i"), owi_IDs_pinMask[deviceIndex], readValueADC);

               readValueTemp = owiTemperatureReadSingleSensor(owi_IDs_pinMask[deviceIndex], owi_IDs[deviceIndex]);

               /* read return status checking */
               if ( 0 != owiCheckReadWriteReturnStatus( readValueTemp >> 16)) { continue; }

               /* mask off status remainder */
               readValueTemp &= 0xFFFF;

               clearString(message, BUFFER_SIZE);
               snprintf(message, BUFFER_SIZE - 1, "%s %.4lX", owi_id_string,readValueTemp);
               break;
            case OWI_FAMILY_DS2408_OCTAL_SWITCH:
            	readValueOS = ReadOctalSwitches(owi_IDs_pinMask[deviceIndex], owi_IDs[deviceIndex]);
            	if ( 0 != owiCheckReadWriteReturnStatus( readValueOS >> 8)) { continue; }

            	/* mask off status remainder */
            	readValueOS &= 0xFF;

            	clearString(message, BUFFER_SIZE);
            	snprintf(message, BUFFER_SIZE - 1, "%s %.2X", owi_id_string, readValueOS);
            	break;
            case OWI_FAMILY_DS2413_DUAL_SWITCH:
               readValueDS = ReadDualSwitches(owi_IDs_pinMask[deviceIndex], owi_IDs[deviceIndex]);
               /* read return status checking */
               if ( 0 != owiCheckReadWriteReturnStatus( readValueDS >> 8)) { continue; }

               /* mask off status remainder */
               readValueDS &= 0xFF;

               clearString(message, BUFFER_SIZE);
               snprintf(message, BUFFER_SIZE - 1, "%s %.2X", owi_id_string, readValueDS);
               break;
            case OWI_FAMILY_DS2405_SIMPLE_SWITCH:
               readValueSS = ReadSimpleSwitches(owi_IDs_pinMask[deviceIndex], owi_IDs[deviceIndex]);
               /* read return status checking */
               if ( 0 != owiCheckReadWriteReturnStatus( readValueSS >> 8)) { continue; }

               /* mask off status remainder */
               readValueSS &= 0xFF;

               clearString(message, BUFFER_SIZE);
               snprintf(message, BUFFER_SIZE - 1, "%s %.2X", owi_id_string, readValueSS);
               break;
            default:
               generalErrorCode = CommunicationError_p(ERRG, GENERAL_ERROR_undefined_family_code, FALSE, NULL);
               continue;
               break;
         }
      }
      else /* write access: write_flag >= TRUE */
      {
     	  printDebug_p(debugLevelEventDebug, debugSystemOWI, __LINE__, PSTR(__FILE__), PSTR("WRITE ACCESS"));

         switch ( familyCode )
         {
            case OWI_FAMILY_DS2408_OCTAL_SWITCH:
               writeValueOS = 0xFF & *((uint8_t*) ptr_value);
               readValueOS = WriteOctalSwitches(owi_IDs_pinMask[deviceIndex], owi_IDs[deviceIndex], writeValueOS);

               /* read return status checking */
               if ( 0 != owiCheckReadWriteReturnStatus( readValueOS >> 8)) { continue; }

               /* mask off status remainder */
               readValueOS &= 0xFF;

               clearString(message, BUFFER_SIZE);
               snprintf(message, BUFFER_SIZE - 1, "%s %.2X", owi_id_string, readValueOS);
            break;

            case OWI_FAMILY_DS2413_DUAL_SWITCH:
               writeValueDS = 0xFF & *((uint8_t*) ptr_value);
               readValueDS = WriteDualSwitches(owi_IDs_pinMask[deviceIndex], owi_IDs[deviceIndex], writeValueDS);

               /* read return status checking */
               if ( 0 != owiCheckReadWriteReturnStatus( readValueDS >> 8)) { continue; }

               /* mask off status remainder */
               readValueDS &= 0xFF;

               clearString(message, BUFFER_SIZE);
               snprintf(message, BUFFER_SIZE - 1, "%s %.2X", owi_id_string, readValueDS);
               break;
//            case OWI_FAMILY_DS2405_SIMPLE_SWITCH:
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
               generalErrorCode = CommunicationError_p(ERRG, GENERAL_ERROR_undefined_family_code, FALSE, NULL);
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
      UART0_Send_Message_String_p(uart_message_string, BUFFER_SIZE - 1);
#else
#error undefined HADCON_VERSION
#endif

      /*clear strings*/
      clearString(uart_message_string, BUFFER_SIZE);
      clearString(message, BUFFER_SIZE);

      foundCounter++;
   }//for (deviceIndex=0;deviceIndex<countDev;deviceIndex++)

#if HADCON_VERSION == 1
   if ( 0 < foundCounter)
   {
      UART0_Transmit_p('\n');
   }
#endif

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
         UART0_Send_Message_String_p(NULL,0);
         break;
      case 1:
      default:
         if ( 0xFFFF < ptr_uartStruct->Uart_Message_ID )
         {
            CommunicationError_p(ERRA, SERIAL_ERROR_mask_is_too_long, FALSE, NULL);
            return;
         }
         owiBusMask = (uint16_t) (ptr_uartStruct->Uart_Message_ID & 0xFFFF);
         break;
   }
   if ( debugLevelVerboseDebug <= globalDebugLevel && ( ( globalDebugSystemMask >> debugSystemOWI ) & 0x1 ) )
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
     	  printDebug_p(debugLevelEventDebug, debugSystemOWI, __LINE__, PSTR(__FILE__), PSTR("owi bus mask didn't match"));
         return 1;
         break;
      case owiReadStatus_conversion_timeout: /*conversion time out*/
         return 1;
         break;
      case owiReadStatus_no_device_presence: /*presence pulse without response*/
         CommunicationError_p(ERRG, GENERAL_ERROR_no_device_is_connected_to_the_bus, FALSE, NULL);
         return 1;
         break;
      default:
         CommunicationError_p(ERRG, dynamicMessage_ErrorIndex, TRUE, PSTR("Error reading %s"), owi_id_string);
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
      CommunicationError_p(ERRG, dynamicMessage_ErrorIndex, TRUE, PSTR("invalid argument index"));
      return 0;
   }
   ptr_owiStruct->idSelect_flag = FALSE;

   numericLength = getNumericLength(setParameter[parameterIndex], MAX_LENGTH_PARAMETER);
    printDebug_p(debugLevelEventDebug, debugSystemOWI, __LINE__, PSTR(__FILE__), PSTR("numeric length of argument '%s' is %i"), setParameter[parameterIndex], numericLength);

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
      if ( debugLevelEventDebug <= globalDebugLevel && ((globalDebugSystemMask >> debugSystemOWI) & 0x1))
      {
         owiCreateIdString(owi_id_string, ptr_owiStruct->id);
         printDebug_p(debugLevelEventDebug, debugSystemOWI, __LINE__, PSTR(__FILE__), PSTR("argument %i is an ID: %s"), parameterIndex, owi_id_string );
      }
      return 1;
   }
   else
   {
      ptr_owiStruct->idSelect_flag = FALSE;
       printDebug_p(debugLevelEventDebug, debugSystemOWI, __LINE__, PSTR(__FILE__), PSTR("argument %i is NOT an ID"), parameterIndex );
      return 0;
   }
}


#warning TODO: check if this function can be used for any OW command

/*
 * uint8_t owiConvertUartDataToOwiStruct(void)
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
uint8_t owiConvertUartDataToOwiStruct(void)
{
#warning THERE IS TOO MUCH INTELLIGENCE IN THIS FUNCTION (reduce it to filling or split it up into ID ,,,,)
   //unsigned int myid =0;
   owiInitOwiStruct(ptr_owiStruct);
   int8_t numberOfArguments = ptr_uartStruct->number_of_arguments;

    printDebug_p(debugLevelEventDebug, debugSystemOWI, __LINE__, PSTR(__FILE__), PSTR("number of arguments: %i"), numberOfArguments );

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

                printDebug_p(debugLevelEventDebug, debugSystemOWI, __LINE__, PSTR(__FILE__), PSTR("second argument sets init_flag to: %i "), ptr_owiStruct->init_flag );
               /*
               */
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

                      printDebug_p(debugLevelEventDebug, debugSystemOWI, __LINE__, PSTR(__FILE__), PSTR("second argument sets init_flag to: %i "), ptr_owiStruct->init_flag );
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
            CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, TRUE, PSTR("invalid number of arguments"));
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
 *      - index of found item [ 0, OWI_MAX_NUM_DEVICES ]
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

   for (deviceIndex = 0; deviceIndex < OWI_MAX_NUM_DEVICES; deviceIndex++)
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
	   if (TRUE == verbose)
	   {
 		   printDebug_p(debugLevelEventDebugVerbose, debugSystemOWI, __LINE__, PSTR(__FILE__), PSTR("checkBusAndDeviceActivityMasks bus: %i differs (pin pattern 0x%x owiBusMask 0x%x)"), busPatternIndex, pins, owiBusMask);
	   }
      return 1;

   }
   else
   {
	   if (TRUE == verbose)
	   {
 		   printDebug_p(debugLevelEventDebug, debugSystemOWI, __LINE__, PSTR(__FILE__), PSTR("checkBusAndDeviceActivityMasks bus: %i active  (pin pattern 0x%x owiBusMask 0x%x)"), busPatternIndex, pins,owiBusMask);
	   }
   }

   // continue if bus doesn't contain any device typed sensors
   if ( 0 == ((owiDeviceMask & pins) & 0xFF ) )
   {
	   if (TRUE == verbose)
	   {
 		   printDebug_p(debugLevelEventDebugVerbose, debugSystemOWI, __LINE__, PSTR(__FILE__), PSTR("checkBusAndDeviceActivityMasks bus: %i Devices: NONE (pin pattern 0x%x owiDeviceMask 0x%x)"), busPatternIndex, pins,owiDeviceMask);
	   }

      return 1;
   }
   else
   {
	   if (TRUE == verbose)
	   {
 		   printDebug_p(debugLevelEventDebug, debugSystemOWI, __LINE__, PSTR(__FILE__), PSTR("checkBusAndDeviceActivityMasks bus: %i Devices: some (pin pattern 0x%x owiDeviceMask 0x%x)"), busPatternIndex, pins,owiDeviceMask);
	   }
   }

   if (TRUE == verbose)
   {
 	   printDebug_p(debugLevelEventDebug, debugSystemOWI, __LINE__, PSTR(__FILE__), PSTR("checkBusAndDeviceActivityMasks bus: %i passed all criteria)"), busPatternIndex);
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

    printDebug_p(debugLevelEventDebug, debugSystemOWI, __LINE__, PSTR(__FILE__), PSTR("bus: owiDeviceMask = 0x%x"), owiDeviceMask);

   for ( int8_t busPatternIndex = 0 ; busPatternIndex < OWI_MAX_NUM_PIN_BUS ; busPatternIndex++ )
   {
      if ( 0 != checkBusAndDeviceActivityMasks(pins[busPatternIndex], busPatternIndex, owiBusMask, owiDeviceMask, TRUE ))
      {
          printDebug_p(debugLevelEventDebugVerbose, debugSystemOWI, __LINE__, PSTR(__FILE__), PSTR("bus: checkBusAndDeviceActivityMasks failed"), busPatternIndex, pins[busPatternIndex],commonPins);
         continue;
      }
      else
      {
          printDebug_p(debugLevelEventDebug, debugSystemOWI, __LINE__, PSTR(__FILE__), PSTR("bus: %i combining 0x%x to common set of pins 0x%x)"), busPatternIndex, pins[busPatternIndex],commonPins);
      }

      commonPins |= pins[busPatternIndex];
   }
    printDebug_p(debugLevelEventDebug, debugSystemOWI, __LINE__, PSTR(__FILE__), PSTR("final common pins: 0x%x"), commonPins);
   return commonPins;
}


/*
 * find parasite powered devices
 */

void owiFindParasitePoweredDevices(unsigned char verbose)
{

   /* In some situations the bus master may not know whether the DS18B20s on the bus are parasite powered
    * or powered by external supplies. The master needs this information to determine if the strong bus pullup
    * should be used during temperature conversions. To get this information, the master can issue a Skip ROM
    * [CCh] command followed by a Read Power Supply [B4h] command followed by a “read time slot”.
    * During the read time slot, parasite powered DS18B20s will pull the bus low, and externally powered
    * DS18B20s will let the bus remain high. If the bus is pulled low, the master knows that it must supply the
    * strong pullup on the 1-Wire bus during temperature conversions
    */


   uint8_t *pins = BUSES;
   uint8_t busPatternIndexMax = OWI_MAX_NUM_PIN_BUS;

   for ( int8_t busPatternIndex = 0 ; busPatternIndex < busPatternIndexMax ; busPatternIndex++ )
   {
      uint8_t currentPins = pins[busPatternIndex];

      /*
       * detect presence
       */

      if ( 0 == OWI_DetectPresence(currentPins) )
      {
          printDebug_p(debugLevelEventDebug, debugSystemOWI, __LINE__, PSTR(__FILE__), PSTR("no Device present (pin pattern 0x%x)"), currentPins);
         continue;
      }
      else
      {
          printDebug_p(debugLevelEventDebug, debugSystemOWI, __LINE__, PSTR(__FILE__), PSTR("some devices present (pin pattern 0x%x)"), currentPins);
      }


      /*
       * send read power supply to all devices
       */

      OWI_SendByte(OWI_ROM_SKIP, currentPins);
      OWI_SendByte(DS1820_READ_POWER_SUPPLY, currentPins);

      /*
       * wait
       */

      uint32_t maxcount = 5; /* 5ms */
      uint32_t count = maxcount;
      uint8_t timeout_flag = FALSE;
      uint8_t owiReadBit = 0;
      static uint8_t delay = 1; /*ms*/

      while ( 0 == owiReadBit)
      {
          owiReadBit = OWI_ReadBit(currentPins);

          _delay_ms(delay);

            /* timeout check */
          if ( 0 == --count)
          {
               timeout_flag = TRUE;
               break;
          }
      }

      /* prepare verbose output header */
		if (FALSE != verbose)
		{
			int8_t oldKeyword = ptr_uartStruct->commandKeywordIndex;
			ptr_uartStruct->commandKeywordIndex = commandKeyNumber_PARA;
			createReceiveHeader(NULL, NULL, 0);
			ptr_uartStruct->commandKeywordIndex = oldKeyword;

			snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%sparasitic devices "), uart_message_string);
		}

      if ( TRUE == timeout_flag )
      {
         /* some devices are parasitic mode*/

         if ( FALSE != verbose)
         {
        	 snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%sSOME on pins 0x%x "),
        			    uart_message_string, currentPins);
        	 UART0_Send_Message_String_p(NULL,0);
         }
         /*set current pins in parasitic mode mask*/
         owiTemperatureParasiticModeMask |= (currentPins & 0xF);
         printDebug_p(debugLevelEventDebug, debugSystemOWITemperatures, __LINE__, PSTR(__FILE__), PSTR("parasitic devices SOME on pins 0x%x ") ,currentPins);
      }
      else
      {
         //owiTemperatureParasiticModeMask |= pins;
         if ( FALSE != verbose)
         {
        	 snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%sNONE on pins 0x%x (pulled HIGH within %i ms)"),
        			    uart_message_string, currentPins, (maxcount - count) * delay);
        	 UART0_Send_Message_String_p(NULL,0);
         }
         /*set current pins in parasitic mode mask*/
         owiTemperatureParasiticModeMask &= !(currentPins & 0xF);
          printDebug_p(debugLevelEventDebug, debugSystemOWITemperatures, __LINE__, PSTR(__FILE__), PSTR("parasitic devices NONE on pins 0x%x ") ,currentPins);
      }
   }
}

/*
 * send variable (nArgs) number of bytes to bus_pattern
 * 		note:
 * 				due to the limitation of variable argument lists,
 * 			  	bytes are of type int masked down to (byte & 0xFF)
 * 			  	and converted to unsigned char
 * compute CRC16 of the whole data stream and
 * compare with received CRC16.
 *
 * In case of mismatch:
 * 		repeat generate via DetectPresence a reset pulse and repeat OWI_SEND_BYTE_MAX_TRIALS times
 *
 * return 0 on success
 * return 1 else
 */
uint8_t owiSendBytesAndCheckCRC16(unsigned char bus_pattern, uint8_t nArgs, ... )
{
	uint8_t arg = 0;
	uint16_t receiveCRC16 = 0;
	uint16_t computeCRC16 = 0; //or 0xFFFF ?
	va_list argumentPointers;

	receiveCRC16 = 0;
	computeCRC16 = 0;

	// compute expected CRC16
	printDebug_p(debugLevelEventDebug, debugSystemOWI, __LINE__, PSTR(__FILE__), PSTR("CRC16 computing inputs:"));

	va_start(argumentPointers, nArgs);
	computeCRC16 = vOwiComputeCRC16(computeCRC16, nArgs, argumentPointers);
	va_end(argumentPointers);

	// send bytes
	va_start(argumentPointers, nArgs);
	for (uint8_t argCounter = 0; argCounter < nArgs; argCounter++)
	{
		arg = va_arg(argumentPointers, int);
		OWI_SendByte((unsigned char)(arg & 0xFF), bus_pattern);
	}
	va_end(argumentPointers);

	receiveCRC16 = OWI_ReceiveWord(bus_pattern);           /*IMPORTANT AFTER EACH 'MEMORY WRITE' OPERATION*/

	if ( computeCRC16 != receiveCRC16 )
	{
		OWI_DetectPresence(bus_pattern); /*the "DetectPresence" function includes sending a Reset Pulse*/

		printDebug_p(debugLevelEventDebug, debugSystemOWI, __LINE__, PSTR(__FILE__), PSTR("CRC16 check send byte failed - computed 0x%x != received 0x%x"), computeCRC16, receiveCRC16);

		return RESULT_FAILURE;
	}
	else
	{
		return RESULT_OK;
	}
}

uint16_t owiComputeCRC16(uint16_t seed, uint8_t nArgs, ...)
{
	uint16_t computeCRC16 = 0;
	va_list argumentPointers;

	va_start (argumentPointers, nArgs);
	computeCRC16 = vOwiComputeCRC16(seed, nArgs, argumentPointers);
	va_end(argumentPointers);

	return computeCRC16;
}

uint16_t vOwiComputeCRC16(uint16_t seed, uint8_t nArgs, va_list argumentPointers)
{
	uint16_t computeCRC16 = seed;
	uint8_t arg = 0;

	computeCRC16 = 0;
	for (uint8_t argCounter = 0; argCounter < nArgs; argCounter++)
	{
		arg = va_arg(argumentPointers, int);
		computeCRC16 = OWI_ComputeCRC16((unsigned char)(arg & 0xFF), computeCRC16);
		printDebug_p(debugLevelEventDebug, debugSystemOWI, __LINE__, PSTR(__FILE__), PSTR("CRC16 computing no %i: 0x%x"), argCounter, arg);
	}

	return computeCRC16;
}
