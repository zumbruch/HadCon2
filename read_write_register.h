#ifndef READ_WRITE_REGISTER_H
#define READ_WRITE_REGISTER_H
#include "api_define.h"
#include "api_global.h"

#include "api_debug.h"
#include "api.h"

#define REGISTER_WRITE_INTO_8BIT_REGISTER( address, value ) (_MMIO_BYTE(((address) & 0xFF)) = ((value) & 0xFF))
#define REGISTER_READ_FROM_8BIT_REGISTER( address ) (_MMIO_BYTE(((address) & 0xFF)))
#define REGISTER_WRITE_INTO_8BIT_REGISTER_AND_READBACK( address, value ) (({REGISTER_WRITE_INTO_8BIT_REGISTER(address, value); uint8_t rbk=REGISTER_READ_FROM_8BIT_REGISTER(address);rbk;}))

void registerReadRegister( struct uartStruct *ptr_uartStruct ); /*this function reads the status of a register with the of his address */
void registerWriteRegister( struct uartStruct *ptr_uartStruct );/* this function writes a value the status in the register with the of his address */
uint8_t registerWriteInto8bitRegister(uint8_t address, uint8_t value);
uint8_t registerReadFrom8bitRegister(uint8_t address);

#endif
