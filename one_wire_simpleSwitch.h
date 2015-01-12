/*
   Licensed under the EUPL V.1.1, Lizenziert unter EUPL V.1.1 
*/
/* The one_wire_simpleSwitch.h is a header file for the functions specific to simpleSwitch via one wire bus*/

#ifndef ONE_WIRE_SIMPLESWITCH__H
#define ONE_WIRE_SIMPLESWITCH__H
#include "one_wire.h"
#include "one_wire_adc.h"
#include "one_wire_dualSwitch.h"
#include "read_write_register.h"
#include "one_wire_temperature.h"

#include "adc.h"
#include "api.h"
#include "api_define.h"
#include "api_global.h"

#include "api_debug.h"#include "can.h"

void read_status_simpleSwitches( struct uartStruct *ptr_uartStruct ); /*this function contains all the functions that are necessary to read state of simple switch */

uint16_t ReadSimpleSwitches( unsigned char bus, unsigned char * id );/*this function read the state of simple switch*/

#endif
