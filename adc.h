/*
   Licensed under the EUPL V.1.1, Lizenziert unter EUPL V.1.1 
*/
#ifndef ADC__H
#define ADC__H
#include "api_define.h"
#include "api_global.h"

#include "api_debug.h"#include "api.h"

void atmelReadADCs( struct uartStruct *ptr_uartStruct ); /* this function takes the value of the voltage read from the ADC-channel and convert it in the form of the API */
uint8_t atmelCollectSingleADCChannel( int8_t channelIndex, uint8_t quiet );

int8_t atmelReadSingleADCChannelVoltage( unsigned int channel_nr );/*this function reads and converts the voltage of the ADC-4 channels */

void atmelControlLoopReadStatus( struct uartStruct *ptr_uartStruct ); /*this function reads the pressure in the bus */

#endif

