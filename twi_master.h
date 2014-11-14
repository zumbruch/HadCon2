#ifndef _TWIS_MASTER
#define _TWIS_MASTER

#include "api.h"
#define TWIM_READ    1
#define TWIM_WRITE   0
/*minimum number of arguments to TWIS: 1/0, address, length*/
#warning READ/WRITE mins differ!
#define TWIS_MIN_NARGS 3

extern const char* const twi_error[] PROGMEM;
enum te_index
{
   TWI_ERROR_Error_in_initiating_TWI_interface = 0,
   TWI_ERROR_Could_not_start_TWI_Bus_for_WRITE,
   TWI_ERROR_Could_not_start_TWI_Bus_for_READ,
   TWI_ERROR_unknown_command,
   TWI_ERROR_address_is_too_long,
   TWI_ERROR_data_length_is_too_long,
   TWI_ERROR_data_0_is_too_long,
   TWI_ERROR_data_1_is_too_long,
   TWI_ERROR_data_2_is_too_long,
   TWI_ERROR_data_3_is_too_long,
   TWI_ERROR_data_4_is_too_long,
   TWI_ERROR_data_5_is_too_long,
   TWI_ERROR_data_6_is_too_long,
   TWI_ERROR_data_7_is_too_long,
   TWI_ERROR_failed_writing_TWI_Bus,
   TWI_ERROR_failed_reading_TWI_Bus,
   TWI_ERROR_too_few_numeric_arguments,
   TWI_ERROR_wrong_length_or_number_of_data_bytes,
   TWI_ERROR_MAXIMUM_INDEX
};

/* Elements necessary for TWI communication */
#define TWI_MAX_DATA_ELEMENTS 8					// Maximum data elements in one transfer
extern uint16_t twi_data[TWI_MAX_DATA_ELEMENTS];	// Data array
extern uint8_t twi_bytes_to_transceive;			// How much bytes to send/receive.


/* Function-declaration for writing/reading TWI */
uint8_t Twim_Write_Data(uint8_t Address, uint8_t twi_bytes_to_transceive, uint16_t twi_data[TWI_MAX_DATA_ELEMENTS]);
uint8_t Twim_Read_Data(uint8_t Address, uint8_t twi_bytes_to_transceive, uint16_t twi_data[TWI_MAX_DATA_ELEMENTS]);

uint8_t Twim_Init (uint32_t TWI_Bitrate);
void Write_to_slave( struct uartStruct *ptr_uartStruct );
void Read_from_slave( struct uartStruct *ptr_uartStruct );
void Twim_Stop (void);
uint8_t Twim_Start (uint8_t Address, uint8_t TWIM_Type);
uint8_t Twim_Write (uint8_t byte);
uint8_t Twim_ReadAck (void);
uint8_t Twim_ReadNack (void);

void    twiMaster(struct uartStruct *ptr_uartStruct);
uint8_t twiMasterParseUartStruct(struct uartStruct *ptr_uartStruct);
uint8_t twiMasterCheckParameterFormatAndRanges(struct uartStruct *ptr_uartStruct);
uint8_t twiMasterCheckError(struct uartStruct *ptr_uartStruct);
uint8_t twiCreateRecvString(struct uartStruct *ptr_uartStruct, uint32_t length, uint16_t array[TWI_MAX_DATA_ELEMENTS]);
void TWI_errorAnalysis(uint8_t status);

enum twi_index {
	TWI_success = 0,
	TWI_data_write_transfer_fail,
	TWI_data_read_transfer_fail,
	TWI_start_condition_write_fail,
	TWI_start_condition_read_fail
};

#endif
