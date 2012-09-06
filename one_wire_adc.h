/* The one_wire_ADC.h is a header file for the functions specific to analog to digital converter via one wire bus*/
#ifndef ONE_WIRE_ADC__H
#define ONE_WIRE_ADC__H
#include "api_define.h"
#include "api_global.h"

#include "api_debug.h"
#include "api.h"

extern uint16_t owiAdcMask;
extern uint16_t* p_owiAdcMask;
extern uint16_t owiAdcTimeoutMask;
extern uint8_t owiUseCommonAdcConversion_flag;

void owiReadADCs( struct uartStruct *ptr_uartStruct ); /*this function contains all the functions that are necessary to
 * convert analog value to digital via one wire bus*/

int8_t owiInitializeADCs( uint8_t *pins );/*this function initializes the analog to digital converter*/

int8_t owiMakeADCConversions( uint8_t *pins );/*this function makes ADC conversion o all found devices*/

uint32_t owiReadChannelsOfSingleADCs( unsigned char bus_pattern, unsigned char * id, uint16_t *array_chn, const int8_t size );

uint8_t owiADCMemoryWriteByte(unsigned char bus_pattern, unsigned char * id, uint16_t address, uint8_t data, uint8_t maxTrials);

#endif

