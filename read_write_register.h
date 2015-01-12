/*
   Licensed under the EUPL V.1.1, Lizenziert unter EUPL V.1.1 
*/
#ifndef READ_WRITE_REGISTER_H
#define READ_WRITE_REGISTER_H
#include "api_define.h"
#include "api_global.h"

#include "api_debug.h"
#include "api.h"

void readRegister( struct uartStruct *ptr_uartStruct ); /*this function reads the status of a register with the of his address */
void writeRegister( struct uartStruct *ptr_uartStruct );/* this function writes a value the status in the register with the of his address */
uint8_t writeInto8bitRegister(uint8_t address, uint8_t value);
uint8_t readFrom8bitRegister(uint8_t address);

#endif
