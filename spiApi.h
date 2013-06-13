/*
 * spiApi.h
 *
 *  Created on: 22.05.2013
 *      Author: Peter Zumbruch, GSI, P.Zumbruch@gsi.de
 */

#ifndef SPIAPI_H_
#define SPIAPI_H_

#include <stdbool.h>
#include "api.h"
#include "api_define.h"
#include "api_global.h"

#include "api_debug.h"
#include "spi.h"

extern const char* spiApiCommandKeywords[] PROGMEM;
enum spiApiCommandKeyNumber
{
	spiApiCommandKeyNumber_ADD= 0,
	spiApiCommandKeyNumber_A  ,
	spiApiCommandKeyNumber_CONTROL_BITS,
	spiApiCommandKeyNumber_C,
	spiApiCommandKeyNumber_PURGE,
	spiApiCommandKeyNumber_P,
	spiApiCommandKeyNumber_READ,
	spiApiCommandKeyNumber_R,
	spiApiCommandKeyNumber_STATUS,
	spiApiCommandKeyNumber_S,
	spiApiCommandKeyNumber_TRANSMIT,
	spiApiCommandKeyNumber_T,
	spiApiCommandKeyNumber_WRITE,
	spiApiCommandKeyNumber_W,

	spiApiCommandKeyNumber_PURGE_WRITE_BUFFER,
	spiApiCommandKeyNumber_PW,
	spiApiCommandKeyNumber_PURGE_READ_BUFFER,
	spiApiCommandKeyNumber_PR,
	spiApiCommandKeyNumber_SHOW_WRITE_BUFFER,
	spiApiCommandKeyNumber_SW,
	spiApiCommandKeyNumber_SHOW_READ_BUFFER,
	spiApiCommandKeyNumber_SR,

	spiApiCommandKeyNumber_CS,
	spiApiCommandKeyNumber_CS_BAR,
	spiApiCommandKeyNumber_CSB,
	spiApiCommandKeyNumber_CS_SET,
	spiApiCommandKeyNumber_CSS,
	spiApiCommandKeyNumber_CS_RELEASE,
	spiApiCommandKeyNumber_CSR,
	spiApiCommandKeyNumber_CS_SELECT_MASK,
	spiApiCommandKeyNumber_CS_PINS,
	spiApiCommandKeyNumber_CS_AUTO_ENABLE,

	spiApiCommandKeyNumber_SPI_ENABLE,
	spiApiCommandKeyNumber_DATA_ORDER,
	spiApiCommandKeyNumber_MASTER,
	spiApiCommandKeyNumber_CLOCK_POLARITY,
	spiApiCommandKeyNumber_CLOCK_PHASE,
	spiApiCommandKeyNumber_SPEED,
	spiApiCommandKeyNumber_SPEED_DIVIDER,
	spiApiCommandKeyNumber_DOUBLE_SPEED,
	spiApiCommandKeyNumber_TRANSMIT_BYTE_ORDER,
	spiApiCommandKeyNumber_COMPLETE_BYTE,
	spiApiCommandKeyNumber_RESET,
	spiApiCommandKeyNumber_TRANSMIT_REPORT,
	spiApiCommandKeyNumber_MAXIMUM_NUMBER

};

enum spiApiCommandResults
{
	spiApiCommandResult_SUCCESS_WITH_OUTPUT 			= 0,
	spiApiCommandResult_SUCCESS_WITH_OPTIONAL_OUTPUT,
	spiApiCommandResult_SUCCESS_WITH_OUTPUT_ELSEWHERE,
	spiApiCommandResult_FAILURE 						= 100,
	spiApiCommandResult_FAILURE_NOT_A_SUB_COMMAND,
	spiApiCommandResult_FAILURE_QUIET,
	spiApiCommandResult_MAXIMUM_INDEX
};

typedef struct spiApiConfig
{
	uint8_t transmitByteOrder; /* MSB first 0 / LSB !0 */
	uint8_t byteCompletion;    /* MSB (0xf == 0x0f) / LSB (0xf == 0xf0) */
	bool reportTransmit;    /* show write buffer content on transmit*/
	bool csWriteAutoEnable; /* enable Automatic CS setting on write*/
	uint8_t csExternalSelectMask;      /* selection of pins to set when sending, subset of available cs channels */
	spiConfigUnion spiConfiguraton; /* hardware configuration */
	bool hardwareInit;
} spiApiConfig;

extern spiApiConfig spiApiConfiguration;
extern spiApiConfig* ptr_spiApiConfiguration;

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
uint8_t spiApiSubCommandCsStatus(struct uartStruct *ptr_uartStruct);
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
uint8_t spiApiSubCommandTransmitReport(struct uartStruct *ptr_uartStruct);

void spiApiGetChipSelectStatus(uint8_t mask);

#define max(a,b) \
  ({ __typeof__ (a) _a = (a); \
      __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b; })

#endif /* SPIAPI_H_ */
