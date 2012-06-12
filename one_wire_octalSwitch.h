#ifndef ONE_WIRE_OCTALSWITCH__H
#define ONE_WIRE_OCTALSWITCH__H
#include "api_define.h"
#include "api_global.h"
#include "api.h"

void owiOctalSwitches( struct uartStruct *ptr_uartStruct );
void HelpOwiOctalSwitches( struct uartStruct *ptr_uartStruct );
uint16_t ReadOctalSwitches( unsigned char bus, unsigned char * id);
uint16_t WriteOctalSwitches( unsigned char bus_pattern, unsigned char * id, uint8_t value );

#endif
