#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
/*
 * VERSION 1.0 Januar 7th 2010 LATE  File: 'one_wire_dualSwitch.c'
 * Author: Linda Fouedjio  based on Alejandro Gil
 * modified (heavily rather rebuild): Peter Zumbruch
 */
#include <math.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <ctype.h>

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

#include "adc.h"
#include "api.h"
#include "api_define.h"
#include "api_global.h"

#include "api_debug.h"#include "can.h"
#include "mem-check.h"

#include "OWIDeviceSpecific.h"
#include "OWIHighLevelFunctions.h"
#include "OWIBitFunctions.h"

#define DS2413_PIO_ACCESS_READ  0xF5
#define DS2413_PIO_ACCESS_WRITE 0x5A
#define DS2413_WRITE_CONFIRMATION_PATTERN 0xAA

#define DS2413_MAX_WRITE_VALUE 0x03

#warning TODO: make this max value switchable
#define OWI_DUAL_SWITCHES_MAXIMUM_WRITE_ACCESS_COUNTS 100

uint16_t owiDualSwitchMask = 0;
uint16_t* p_owiDualSwitchMask = &owiDualSwitchMask;


void owiDualSwitches( struct uartStruct *ptr_uartStruct )
{
   //int index = -1;
#warning TODO? owiReadDevicesID, depending on some settings, might be replaced by a conditional search function
   NumDevicesFound = owiReadDevicesID(BUSES);
   if ( 0 < NumDevicesFound )
   {
      owiDualSwitchMask = 0x0;

      if ( 0 < ( owiScanIDS(OWI_FAMILY_DS2413_DUAL_SWITCH, p_owiDualSwitchMask))) /* Dual Switch connected */
      {
         /* additional arguments ?
          *  0:
          *     read all;
          *  1:
          *       read: id,
          *       write: value,
          *  2:
          *       write: id value
          */
         /* 0xff> id >> 56 > 0x01 */
         /* needs own parsing */

         switch(ptr_uartStruct->number_of_arguments)
         {
            case 0: /*read all*/
                printDebug_p(debugLevelEventDebug, debugSystemOWIDualSwitches, __LINE__, PSTR(__FILE__), PSTR("OWDS read all"));

               owiFindFamilyDevicesAndAccessValues(BUSES, NumDevicesFound, OWI_FAMILY_DS2413_DUAL_SWITCH, NULL );
               break;
            case 1: /*read single ID / write all / invalid */
               if (TRUE == ptr_owiStruct->idSelect_flag) /* read ID */
               {
                   printDebug_p(debugLevelEventDebug, debugSystemOWIDualSwitches, __LINE__, PSTR(__FILE__), PSTR("OWDS read ID"));

                  owiFindFamilyDevicesAndAccessValues(BUSES, NumDevicesFound, OWI_FAMILY_DS2413_DUAL_SWITCH, NULL );
               }
               else /* write value / invalid */
               {
                   printDebug_p(debugLevelEventDebug, debugSystemOWIDualSwitches, __LINE__, PSTR(__FILE__), PSTR("OWDS write all: 0x%x, pointer (%p)" ), ptr_owiStruct->value, ptr_owiStruct->ptr_value);

                  if ( DS2413_MAX_WRITE_VALUE >= ptr_owiStruct->value)
                  {
                     owiFindFamilyDevicesAndAccessValues(BUSES, NumDevicesFound, OWI_FAMILY_DS2413_DUAL_SWITCH, ptr_owiStruct->ptr_value);
                  }
                  else
                  {
                     CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, TRUE, PSTR("write argument: 0x%x is out of range [0,0x%x] ... skipping"), ptr_owiStruct->value, DS2413_MAX_WRITE_VALUE);
                     return;
                  }
               }
               break;
            case 2: /* write ID value / invalid */
               if (TRUE == ptr_owiStruct->idSelect_flag) /* write ID value*/
               {
                   printDebug_p(debugLevelEventDebug, debugSystemOWIDualSwitches, __LINE__, PSTR(__FILE__), PSTR("OWDS write ID: 0x%x pointer (%p)"), ptr_owiStruct->value, ptr_owiStruct->ptr_value);

                  if ( DS2413_MAX_WRITE_VALUE >= ptr_owiStruct->value)
                  {
                     owiFindFamilyDevicesAndAccessValues(BUSES, NumDevicesFound, OWI_FAMILY_DS2413_DUAL_SWITCH, ptr_owiStruct->ptr_value );
                  }
                  else
                  {
                     CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, TRUE, PSTR("write argument: 0x%x is out of range [0,0x%x] ... skipping"), ptr_owiStruct->value, DS2413_MAX_WRITE_VALUE);
                     return;
                  }
               }
               else
               {
                  CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, TRUE, PSTR("write argument: invalid arguments"));
                  return;
               }
               break;
            default: /* invalid */
               CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, TRUE, PSTR("write argument: too many arguments"));
               break;
         }
      }
   }
}

/*
 *this function reads the state of dual switch
 */
uint16_t ReadDualSwitches( unsigned char bus_pattern, unsigned char * id )
{
   uint16_t value = 0x0;

   // continue if bus isn't active
   if ( 0 == ( ( owiBusMask & bus_pattern ) & 0xFF ) )
   {
      return owiReadStatus_owi_bus_mismatch << 8;
   }

   // continue if bus doesn't contain any Dual Switches
   if ( 0 == ( ( owiDualSwitchMask & bus_pattern ) & 0xFF ) )
   {
      return owiReadStatus_owi_bus_mismatch << 8;
   }

   /* Reset, presence.*/
   if ( OWI_DetectPresence(bus_pattern) == 0 )
   {
      return owiReadStatus_no_device_presence << 8;
   }

   OWI_MatchRom(id, bus_pattern); /* Match id found earlier*/
   OWI_SendByte(DS2413_PIO_ACCESS_READ, bus_pattern); //PIO Access read command

   /*Read first byte and place them in the 16 bit channel variable*/
   value = OWI_ReceiveByte(bus_pattern);
   value &= 0xFF;

   OWI_DetectPresence(bus_pattern); /* generate RESET to stop slave sending its status and presence pulse*/

   return value | owiReadWriteStatus_OK << 8;
}//END of ReadDualSwitches function

/*
 *this function writes the state of dual switch
 */

uint16_t WriteDualSwitches( unsigned char bus_pattern, unsigned char * id, uint8_t value )
{
   static unsigned char timeout_flag;
   static uint32_t count;
   static uint8_t status;
   static uint32_t maxcount = OWI_DUAL_SWITCHES_MAXIMUM_WRITE_ACCESS_COUNTS;

   // continue if bus isn't active
   if ( 0 == ( ( owiBusMask & bus_pattern ) & 0xFF ) )
   {
      return owiReadStatus_owi_bus_mismatch << 8;
   }

   // continue if bus doesn't contain any Dual Switches
   if ( 0 == ( ( owiDualSwitchMask & bus_pattern ) & 0xFF ) )
   {
      return owiReadStatus_owi_bus_mismatch << 8;
   }

   /* Reset, presence.*/
   if ( OWI_DetectPresence(bus_pattern) == 0 )
   {
      return owiReadStatus_no_device_presence << 8;
   }

   count = maxcount;
   timeout_flag = FALSE;
   status = 0;

   /* Match id found earlier*/
   OWI_MatchRom(id, bus_pattern);
   /* PIO Access write command */
   OWI_SendByte(DS2413_PIO_ACCESS_WRITE, bus_pattern);

   /* loop writing value and its complement, and waiting for confirmation */
   while (DS2413_WRITE_CONFIRMATION_PATTERN != status )
   {
      status = 0;
      /* timeout check */
      if ( 0 == --count)
      {
         timeout_flag = TRUE;
         break;
      }

      OWI_SendByte( (value |= 0xFC ), bus_pattern); // write value 0:on 1:off
      OWI_SendByte(~(value |= 0xFC ), bus_pattern); // to confirm, write inverted

      /*Read status */
      status = OWI_ReceiveByte(bus_pattern);
      status &= 0xFF;
   }

   if ( FALSE == timeout_flag )
   {
      /*Read first byte*/
      value = OWI_ReceiveByte(bus_pattern);
      value &= 0xFF;
      OWI_DetectPresence(bus_pattern); /* generate RESET to stop slave sending its status and presence pulse*/

      return value | owiReadWriteStatus_OK << 8;
   }
   else
   {
      OWI_DetectPresence(bus_pattern); /* generate RESET to stop slave sending its status and presence pulse*/

      CommunicationError_p(ERRG, dynamicMessage_ErrorIndex, TRUE, PSTR("OWI Dual Switch write value timeout"));

      return value | owiWriteStatus_Timeout << 8;
   }


   return  value | owiReadWriteStatus_OK << 8;
}//END of ReadDualSwitches function
