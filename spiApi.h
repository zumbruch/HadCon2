/*
 * spiApi.h
 *
 *  Created on: 22.05.2013
 *      Author: Peter Zumbruch, GSI, P.Zumbruch@gsi.de
 */

#ifndef SPIAPI_H_
#define SPIAPI_H_

#include "api.h"
#include "api_define.h"
#include "api_global.h"

#include "api_debug.h"
#include "spi.h"

extern const char* spiCommandKeywords[] PROGMEM;
enum spiCommandKeyNumber
{
      spiCommandKeyNumber_STATUS = 0,
      spiCommandKeyNumber_WRITE,
      spiCommandKeyNumber_ADD,
      spiCommandKeyNumber_TRANSMIT,
      spiCommandKeyNumber_READ,
      spiCommandKeyNumber_CS,
      spiCommandKeyNumber_CS_BAR,
      spiCommandKeyNumber_CS_SET,
      spiCommandKeyNumber_CS_RELEASE,
      spiCommandKeyNumber_CS_SELECT_MASK,
      spiCommandKeyNumber_CS_PINS,
      spiCommandKeyNumber_CS_AUTO_ENABLE,
      spiCommandKeyNumber_PURGE,
      spiCommandKeyNumber_PURGE_WRITE_BUFFER,
      spiCommandKeyNumber_PURGE_READ_BUFFER,
      spiCommandKeyNumber_SHOW_WRITE_BUFFER,
      spiCommandKeyNumber_SHOW_READ_BUFFER,
      spiCommandKeyNumber_CONTROL_BITS,
      spiCommandKeyNumber_SPI_ENABLE,
      spiCommandKeyNumber_DATA_ORDER,
      spiCommandKeyNumber_MASTER,
      spiCommandKeyNumber_CLOCK_POLARITY,
      spiCommandKeyNumber_CLOCK_PHASE,
      spiCommandKeyNumber_SPEED,
      spiCommandKeyNumber_SPEED_DIVIDER,
      spiCommandKeyNumber_DOUBLE_SPEED,
      spiCommandKeyNumber_TRANSMIT_BYTE_ORDER,
      spiCommandKeyNumber_COMPLETE_BYTE,
      spiCommandKeyNumber_MAXIMUM_NUMBER

};

void spiApi(struct uartStruct *ptr_uartStruct);
size_t spiApiFillWriteArray(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex);
size_t spiApiAddToWriteArray(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex);
void spiApiSubCommands(struct uartStruct *ptr_uartStruct, int16_t subCommandIndex);
int8_t spiAddNumericParameterToByteArray(const char string[], spiByteDataArray* data, uint8_t index);
int8_t spiAddNumericStringToByteArray(const char string[], spiByteDataArray* data);
void spiApiShowWriteBufferContent(void);


#endif /* SPIAPI_H_ */
