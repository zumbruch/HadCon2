#ifndef READ_WRITE_REGISTER_H
#define READ_WRITE_REGISTER_H
#include "api_define.h"
#include "api_global.h"

#include "api_debug.h"
#include "api.h"


#define UCSR1C_register_of_ATMEL_address 0xCA
#define UBRR1L_register_of_ATMEL_address 0xCC
#define UBRR1H_register_of_ATMEL_address 0xCD
#define UCSR1B_register_of_ATMEL_address 0xC9
#define UCSR1A_register_of_ATMEL_address 0xC8
#define UDR1_register_of_ATMEL_address 0xCE

#define UCSR1C_register_of_ATMEL_value 0x06
#define UBRR1L_register_of_ATMEL_value 0x00
#define UBRR1H_register_of_ATMEL_value 0x00
#define UCSR1B_register_of_ATMEL_value_transmit 0x08
#define UCSR1B_register_of_ATMEL_value_recieve 0x10
#define UCSR1A_register_of_ATMEL_value 0x22

#define CONTROL_REGISTER 0x00
#define DELAY1_MSB_REGISTER 0x01
#define DELAY1_MIDDLE_REGISTER 0x02
#define DELAY1_LSB_REGISTER 0x03
#define DELAY2_MSB_REGISTER 0x04
#define DELAY2_MIDDLE_REGISTER 0x05
#define DELAY2_LSB_REGISTER 0x06
#define SHOW_ALL 0xFF

#define DEBUG 0x00
#define RESET 0x40
#define READ 0xa0
#define PSEUDORANDOM 0x81
#define SQUARE 0x82
#define PULSE 0x83
#define PSEUDORAND_TIME_PULSE 0x84

#define step1 0
#define step2 1
#define step3 2
#define step4 3
#define step5 4
#define step6 5
#define step7 6
#define step8 7
#define step9 8
#define step10 9
#define step11 10
#define step12 11





void readRegister( struct uartStruct *ptr_uartStruct ); /*this function reads the status of a register with the of his adress */
void writeRegister( struct uartStruct *ptr_uartStruct );/* this function writes a value the status in the register with the of his adress */
void read_waveform_generator_Registers( struct uartStruct *ptr_uartStruct ); /*this function reads the status of a register with the of his adress */
void write_double_Register( struct uartStruct *ptr_uartStruct );/* this function writes a value the status in the register with the of his adress */

#endif
