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
      spiCommandKeyNumber_S,
      spiCommandKeyNumber_WRITE,
      spiCommandKeyNumber_W,
      spiCommandKeyNumber_ADD,
      spiCommandKeyNumber_A,
      spiCommandKeyNumber_TRANSMIT,
      spiCommandKeyNumber_T,
      spiCommandKeyNumber_READ,
      spiCommandKeyNumber_R,
      spiCommandKeyNumber_CS,
      spiCommandKeyNumber_CS_BAR,
      spiCommandKeyNumber_CS_SET,
      spiCommandKeyNumber_CS_RELEASE,
      spiCommandKeyNumber_CS_SELECT_MASK,
      spiCommandKeyNumber_CS_PINS,
      spiCommandKeyNumber_CS_AUTO_ENABLE,
      spiCommandKeyNumber_PURGE,
      spiCommandKeyNumber_P,
      spiCommandKeyNumber_PURGE_WRITE_BUFFER,
      spiCommandKeyNumber_PW,
      spiCommandKeyNumber_PURGE_READ_BUFFER,
      spiCommandKeyNumber_PR,
      spiCommandKeyNumber_SHOW_WRITE_BUFFER,
      spiCommandKeyNumber_SW,
      spiCommandKeyNumber_SHOW_READ_BUFFER,
      spiCommandKeyNumber_SR,
      spiCommandKeyNumber_CONTROL_BITS,
      spiCommandKeyNumber_C,
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
      spiCommandKeyNumber_RESET,
      spiCommandKeyNumber_MAXIMUM_NUMBER

};

void spiApi(struct uartStruct *ptr_uartStruct);
void spiApiSubCommands(struct uartStruct *ptr_uartStruct, int16_t subCommandIndex);

size_t spiApiFillWriteArray(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex);
size_t spiApiAddToWriteArray(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex);
int8_t spiAddNumericParameterToByteArray(const char string[], uint8_t index);
int8_t spiAddNumericStringToByteArray(const char string[]);


/*pure show commands*/
void spiApiSubCommandShowStatus(struct uartStruct *ptr_uartStruct);
void spiApiShowWriteBufferContent(void);

/**/
void spiApiSubCommandWrite(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex);
void spiApiSubCommandAdd(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex);
void spiApiSubCommandTransmit(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex);
void spiApiSubCommandRead(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex);
void spiApiSubCommandCs(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex);
void spiApiSubCommandCsBar(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex);
void spiApiSubCommandCsSet(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex);
void spiApiSubCommandCsRelease(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex);
void spiApiSubCommandCsSelectMask(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex);
void spiApiSubCommandCsPins(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex);
void spiApiSubCommandCsAutoEnable(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex);
void spiApiSubCommandPurge(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex);
void spiApiSubCommandPurgeWriteBuffer(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex);
void spiApiSubCommandPurgeReadBuffer(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex);
void spiApiSubCommandShowWriteBuffer(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex);
void spiApiSubCommandShowReadBuffer(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex);
void spiApiSubCommandControlBits(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex);
void spiApiSubCommandSpiEnable(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex);
void spiApiSubCommandDataOrder(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex);
void spiApiSubCommandMaster(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex);
void spiApiSubCommandClockPolarity(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex);
void spiApiSubCommandClockPhase(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex);
void spiApiSubCommandSpeed(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex);
void spiApiSubCommandSpeedDivider(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex);
void spiApiSubCommandDoubleSpeed(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex);
void spiApiSubCommandTransmitByteOrder(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex);
void spiApiSubCommandCompleteByte(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex);
void spiApiSubCommandReset(struct uartStruct *ptr_uartStruct);



#endif /* SPIAPI_H_ */
