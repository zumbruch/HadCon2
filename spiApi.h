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
	spiCommandKeyNumber_ADD= 0,
	spiCommandKeyNumber_A  ,
	spiCommandKeyNumber_CONTROL_BITS,
	spiCommandKeyNumber_C,
	spiCommandKeyNumber_PURGE,
	spiCommandKeyNumber_P,
	spiCommandKeyNumber_READ,
	spiCommandKeyNumber_R,
	spiCommandKeyNumber_STATUS,
	spiCommandKeyNumber_S,
	spiCommandKeyNumber_TRANSMIT,
	spiCommandKeyNumber_T,
	spiCommandKeyNumber_WRITE,
	spiCommandKeyNumber_W,

	spiCommandKeyNumber_PURGE_WRITE_BUFFER,
	spiCommandKeyNumber_PW,
	spiCommandKeyNumber_PURGE_READ_BUFFER,
	spiCommandKeyNumber_PR,
	spiCommandKeyNumber_SHOW_WRITE_BUFFER,
	spiCommandKeyNumber_SW,
	spiCommandKeyNumber_SHOW_READ_BUFFER,
	spiCommandKeyNumber_SR,

	spiCommandKeyNumber_CS,
	spiCommandKeyNumber_CS_BAR,
	spiCommandKeyNumber_CSB,
	spiCommandKeyNumber_CS_SET,
	spiCommandKeyNumber_CSS,
	spiCommandKeyNumber_CS_RELEASE,
	spiCommandKeyNumber_CSR,
	spiCommandKeyNumber_CS_SELECT_MASK,
	spiCommandKeyNumber_CS_PINS,
	spiCommandKeyNumber_CS_AUTO_ENABLE,

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

enum spiCommandResults
{
	spiCommandResult_SUCCESS_WITH_OUTPUT 		= 0,
	spiCommandResult_SUCCESS_WITHOUT_OUTPUT 	= 1,
	spiCommandResult_FAILURE 					= 100,
	spiCommandResult_FAILURE_NOT_A_SUB_COMMAND 	= 110,
	spiCommandResult_FAILURE_QUIET 				= 120
};

void spiApi(struct uartStruct *ptr_uartStruct);
void spiApiSubCommands(struct uartStruct *ptr_uartStruct, int16_t subCommandIndex, uint8_t parameterIndex);

size_t spiApiFillWriteArray(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex);
size_t spiApiAddToWriteArray(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex);
int8_t spiAddNumericParameterToByteArray(const char string[], uint8_t index);
int8_t spiAddNumericStringToByteArray(const char string[]);

uint8_t spiApiShowBufferContent(struct uartStruct *ptr_uartStruct, spiByteDataArray *buffer, int16_t nBytes, int8_t subCommandKeywordIndex);

uint8_t spiApiShowWriteBufferContent(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex);
/**/
uint8_t spiApiSubCommandShowStatus(void);
uint8_t spiApiSubCommandTransmit(void);
uint8_t spiApiSubCommandPurge(void);
uint8_t spiApiSubCommandReset(void);
uint8_t spiApiSubCommandPurgeWriteBuffer(void);
uint8_t spiApiSubCommandPurgeReadBuffer(void);
uint8_t spiApiSubCommandCsSet(void);
uint8_t spiApiSubCommandCsRelease(void);
uint8_t spiApiSubCommandRead(void);

uint8_t spiApiSubCommandWrite(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex);
uint8_t spiApiSubCommandAdd(struct uartStruct *ptr_uartStruct);
uint8_t spiApiSubCommandCs(struct uartStruct *ptr_uartStruct);
uint8_t spiApiSubCommandCsBar(struct uartStruct *ptr_uartStruct);
uint8_t spiApiSubCommandCsSelectMask(struct uartStruct *ptr_uartStruct);
uint8_t spiApiSubCommandCsPins(struct uartStruct *ptr_uartStruct);
uint8_t spiApiSubCommandCsAutoEnable(struct uartStruct *ptr_uartStruct);
uint8_t spiApiSubCommandShowWriteBuffer(struct uartStruct *ptr_uartStruct);
uint8_t spiApiSubCommandShowReadBuffer(struct uartStruct *ptr_uartStruct);
uint8_t spiApiSubCommandControlBits(struct uartStruct *ptr_uartStruct);
uint8_t spiApiSubCommandSpiEnable(struct uartStruct *ptr_uartStruct);
uint8_t spiApiSubCommandDataOrder(struct uartStruct *ptr_uartStruct);
uint8_t spiApiSubCommandMaster(struct uartStruct *ptr_uartStruct);
uint8_t spiApiSubCommandClockPolarity(struct uartStruct *ptr_uartStruct);
uint8_t spiApiSubCommandClockPhase(struct uartStruct *ptr_uartStruct);
uint8_t spiApiSubCommandSpeed(struct uartStruct *ptr_uartStruct);
uint8_t spiApiSubCommandSpeedDivider(struct uartStruct *ptr_uartStruct);
uint8_t spiApiSubCommandDoubleSpeed(struct uartStruct *ptr_uartStruct);
uint8_t spiApiSubCommandTransmitByteOrder(struct uartStruct *ptr_uartStruct);
uint8_t spiApiSubCommandCompleteByte(struct uartStruct *ptr_uartStruct);


#define max(a,b) \
  ({ __typeof__ (a) _a = (a); \
      __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b; })

#endif /* SPIAPI_H_ */
