#ifndef READ_WRITE_REGISTER_H
#define READ_WRITE_REGISTER_H
#include "api_define.h"
#include "api_global.h"

#include "api_debug.h"#include "api.h"

void readRegister( struct uartStruct *ptr_uartStruct ); /*this function reads the status of a register with the of his adress */
void writeRegister( struct uartStruct *ptr_uartStruct );/* this function writes a value the status in the register with the of his adress */

#endif
