/*
   Licensed under the EUPL V.1.1, Lizenziert unter EUPL V.1.1 
*/
#ifndef ONE_WIRE_DUALSWITCH__H
#define ONE_WIRE_DUALSWITCH__H
#include "api_define.h"
#include "api_global.h"

#include "api_debug.h"#include "api.h"

void owiDualSwitches( struct uartStruct *ptr_uartStruct );
uint16_t ReadDualSwitches( unsigned char bus, unsigned char * id);
uint16_t WriteDualSwitches( unsigned char bus_pattern, unsigned char * id, uint8_t value );

#endif
