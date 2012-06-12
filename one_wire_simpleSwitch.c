/*
 * VERSION 1.0 Januar 7th 2010 LATE  File: 'one_wire_simpleSwitch.c'
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

/*this function contains all the functions that are necessary
 *to read state of simple switch
 */
void read_status_simpleSwitches( struct uartStruct *ptr_uartStruct )
{
	NumDevicesFound = owiReadDevicesID(BUSES);
	if ( 0 < NumDevicesFound )
	{
		owiFindFamilyDevicesAndAccessValues(BUSES, NumDevicesFound, FAMILY_DS2405_SIMPLE_SWITCH, NULL );
	}
#warning TODO: add else
}

/*
 *this function
 */

uint16_t ReadSimpleSwitches( unsigned char bus, unsigned char * id )
{
   CommunicationError(ERRG, -1, 1, PSTR("device DS2405 not supported any more"),100);
   return owiReadWriteStatus_MAXIMUM_INDEX << 8;

   //   uint16_t value = 0;
   //	if ( 0 == OWI_DetectPresence(bus) )
   //	{
   //		return owiReadStatus_no_device_presence << 8; /* Error*/
   //	}
   //
   //	OWI_MatchRom(id, bus); /* Match id found earlier*/
   //	OWI_SendByte(0xF5, bus); //PIO Access read command //???
   //
   //	/*Read two first bytes and place them in the 16 bit channel variable*/
   //	value = OWI_ReceiveByte(bus);
   //
   //	return value | owiReadWriteStatus_OK << 8;
}//END of ReadSimpleSwitches function
