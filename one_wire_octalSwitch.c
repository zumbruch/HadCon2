#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
/* modified (heavily rather rebuild): Peter Zumbruch
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
#include "one_wire_octalSwitch.h"
#include "one_wire_dualSwitch.h"
#include "one_wire_simpleSwitch.h"
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

#define DS2408_READ_PIO_REGISTERS                 0xF0
#define DS2408_CHANNEL_ACCESS_READ                0xF5
#define DS2408_CHANNEL_ACCESS_WRITE               0x5A
#define DS2408_WRITE_CONDITIONAL_SEARCH_REGISTERS 0xCC
#define DS2408_RESET_ACTIVITY_LATCHES             0xC3
#define DS2408_WRITE_CONFIRMATION_PATTERN         0xAA

#define DS2408_MAX_WRITE_VALUE 0x0F

#warning TODO: make this max value switchable
#define OWI_OCTAL_SWITCHES_MAXIMUM_WRITE_ACCESS_COUNTS 100


uint16_t owiOctalSwitchMask = 0x00;
uint16_t* p_owiOctalSwitchMask = &owiOctalSwitchMask;


void owiOctalSwitches( struct uartStruct *ptr_uartStruct )
{
   //int index = -1;
#warning TODO? owiReadDevicesID, depending on some settings, might be replaced by a conditional search function
   NumDevicesFound = owiReadDevicesID(BUSES);
   if ( 0 < NumDevicesFound )
   {
      owiOctalSwitchMask = 0x00;

      if ( 0 < ( owiScanIDS(FAMILY_DS2408_OCTAL_SWITCH, p_owiOctalSwitchMask) ) )
      {
         /* additional arguments ? no: read all; yes: read:id, write: value, write: id all*/
         /* 0xff> id >> 56 > 0x01 */
         /* needs own parsing */

         switch(ptr_uartStruct->number_of_arguments)
         {
            case 0: /*read all*/
                printDebug_p(eventDebug, debugOWIOctalSwitches, __LINE__, PSTR(__FILE__), PSTR("OW8S read all"));

               owiFindFamilyDevicesAndAccessValues(BUSES, NumDevicesFound, FAMILY_DS2408_OCTAL_SWITCH, NULL);
               break;
            case 1: /*read single ID / write all / invalid */
            	if (TRUE == ptr_owiStruct->idSelect_flag) /* read ID */
               {
                   printDebug_p(eventDebug, debugOWIOctalSwitches, __LINE__, PSTR(__FILE__), PSTR("OW8S read ID"));

                  owiFindFamilyDevicesAndAccessValues(BUSES, NumDevicesFound, FAMILY_DS2408_OCTAL_SWITCH, NULL);
               }
               else /* write value / invalid */
               {
                   printDebug_p(eventDebug, debugOWIOctalSwitches, __LINE__, PSTR(__FILE__), PSTR("OW8S write all: 0x%x, pointer (%p)" ), ptr_owiStruct->value, ptr_owiStruct->ptr_value);

                  if ( DS2408_MAX_WRITE_VALUE >= ptr_owiStruct->value)
                  {
                     owiFindFamilyDevicesAndAccessValues(BUSES, NumDevicesFound, FAMILY_DS2408_OCTAL_SWITCH, ptr_owiStruct->ptr_value);
                  }
                  else
                  {
                     snprintf_P(message, BUFFER_SIZE, PSTR("write argument: 0x%x is out of range [0,0x%x] ... skipping"), ptr_owiStruct->value, DS2408_MAX_WRITE_VALUE);
                     CommunicationError_p(ERRA, -1, TRUE, message, COMMUNICATION_ERROR_USE_GLOBAL_MESSAGE_STRING_INDEX_THRESHOLD - 1);
                     return;
                  }
               }
               break;
            case 2: /* write ID value / invalid */
            	if (TRUE == ptr_owiStruct->idSelect_flag) /* write ID value*/
               {
                   printDebug_p(eventDebug, debugOWIOctalSwitches, __LINE__, PSTR(__FILE__), PSTR("OWDS write ID: 0x%x pointer (%p)"),  __LINE__, __FILE__, ptr_owiStruct->value, ptr_owiStruct->ptr_value);
                  if ( DS2408_MAX_WRITE_VALUE >= ptr_owiStruct->value)
                  {
                     owiFindFamilyDevicesAndAccessValues(BUSES, NumDevicesFound, FAMILY_DS2408_OCTAL_SWITCH, ptr_owiStruct->ptr_value);
                  }
                  else
                  {
                     snprintf_P(message, BUFFER_SIZE, PSTR("write argument: 0x%x is out of range [0,0x%x] ... skipping"), ptr_owiStruct->value, DS2408_MAX_WRITE_VALUE);
                     CommunicationError_p(ERRA, -1, TRUE, message, COMMUNICATION_ERROR_USE_GLOBAL_MESSAGE_STRING_INDEX_THRESHOLD - 1);
                     return;
                  }
               }
               else
               {
                  snprintf_P(message, BUFFER_SIZE, PSTR("write argument: invalid arguments"));
                  CommunicationError_p(ERRA, -1, TRUE, message, COMMUNICATION_ERROR_USE_GLOBAL_MESSAGE_STRING_INDEX_THRESHOLD - 1);
                  return;
               }
               break;
            default: /* invalid */
               snprintf_P(message, BUFFER_SIZE, PSTR("write argument: too many arguments"));
               CommunicationError_p(ERRA, -1, TRUE, message, COMMUNICATION_ERROR_USE_GLOBAL_MESSAGE_STRING_INDEX_THRESHOLD - 1);
               break;
         }
      }
   }
}

/*
 *this function reads the state of octal switch
 */
uint16_t ReadOctalSwitches( unsigned char bus_pattern, unsigned char * id )
{
   uint16_t value = 0x0;
   // continue if bus isn't active
   if ( 0 == ( ( owiBusMask & bus_pattern ) & 0xFF ) )
   {
      return owiReadStatus_owi_bus_mismatch << 8;
   }

   // continue if bus doesn't contain any Octal Switches
   if ( 0 == ( ( owiOctalSwitchMask & bus_pattern ) & 0xFF ) )
   {
      return owiReadStatus_owi_bus_mismatch << 8;
   }

   /* Reset, presence.*/
   if ( OWI_DetectPresence(bus_pattern) == 0 )
   {
      return owiReadStatus_no_device_presence << 8;
   }

   OWI_MatchRom(id, bus_pattern); /* Match id found earlier*/
   OWI_SendByte(DS2408_CHANNEL_ACCESS_READ, bus_pattern); //PIO Access read command

   /*Read first byte and place them in the 16 bit channel variable*/
   value = OWI_ReceiveByte(bus_pattern);
   value &= 0xFF;
  
   OWI_DetectPresence(bus_pattern); /* generate RESET to stop slave sending its status and presence pulse*/

   return value | owiReadWriteStatus_OK << 8;
}//END of ReadOctalSwitches function

/*
 *this function writes the state of octal switch
 */

uint16_t WriteOctalSwitches( unsigned char bus_pattern, unsigned char * id, uint8_t value )
{
   static unsigned char timeout_flag;
   static uint32_t count;
   static uint8_t status;
   static uint32_t maxcount = OWI_OCTAL_SWITCHES_MAXIMUM_WRITE_ACCESS_COUNTS;

   // continue if bus isn't active
   if ( 0 == ( ( owiBusMask & bus_pattern ) & 0xFF ) )
   {
      return owiReadStatus_owi_bus_mismatch << 8;
   }

   // continue if bus doesn't contain any Octal Switches
   if ( 0 == ( ( owiOctalSwitchMask & bus_pattern ) & 0xFF ) )
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
   OWI_SendByte(DS2408_CHANNEL_ACCESS_WRITE, bus_pattern);

   /* loop writing value and its complement, and waiting for confirmation */
   while (DS2408_WRITE_CONFIRMATION_PATTERN != status )
   {
      status = 0;
      /* timeout check */
      if ( 0 == --count)
      {
         timeout_flag = TRUE;
         break;
      }

      OWI_SendByte( (value & 0xFF ), bus_pattern); // write value 0:on 1:off
      OWI_SendByte(~(value & 0xFF ), bus_pattern); // to confirm, write inverted

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

      CommunicationError_p(ERRG, -1, 1, PSTR("OWI Octal Switch write value timeout"), 300);

      return value | owiWriteStatus_Timeout << 8;
   }


   return  value | owiReadWriteStatus_OK << 8;
}//END of ReadOctalSwitches function
