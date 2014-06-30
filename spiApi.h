/*
   Licensed under the EUPL V.1.1, Lizenziert unter EUPL V.1.1 
*/
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

extern const char* const spiApiCommandKeywords[] PROGMEM;
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
	spiApiCommandKeyNumber_WRITE_BUFFER,
	spiApiCommandKeyNumber_WB,

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
	spiApiCommandKeyNumber_CS_ADD_PIN,
	spiApiCommandKeyNumber_CSAP,
	spiApiCommandKeyNumber_CS_REMOVE_PIN,
	spiApiCommandKeyNumber_CSRP,


	spiApiCommandKeyNumber_SPI_ENABLE,
	spiApiCommandKeyNumber_DATA_ORDER,
	spiApiCommandKeyNumber_MASTER,
	spiApiCommandKeyNumber_CLOCK_POLARITY,
	spiApiCommandKeyNumber_CLOCK_PHASE,
	spiApiCommandKeyNumber_SPEED,
	spiApiCommandKeyNumber_SPEED_DIVIDER,
	spiApiCommandKeyNumber_DOUBLE_SPEED,
	spiApiCommandKeyNumber_TRANSMIT_BYTE_ORDER,
	spiApiCommandKeyNumber_RESET,
	spiApiCommandKeyNumber_TRANSMIT_REPORT,
	spiApiCommandKeyNumber_AUTO_PURGE_WRITE_BUFFER,
	spiApiCommandKeyNumber_AUTO_PURGE_READ_BUFFER,
	spiApiCommandKeyNumber_MAXIMUM_NUMBER

};

enum spiApiTransmitByteOrders
{
	spiApiTransmitByteOrder_MSB = SPI_MSBYTE_FIRST,
	spiApiTransmitByteOrder_LSB = SPI_LSBYTE_FIRST,
	spiApiTransmitByteOrder_MAXIMUM_INDEX
};

enum spiApiByteCompletions
{
	spiApiByteCompletion_LEADING,
	spiApiByteCompletion_TRAILING,
	spiApiByteCompletion_MAXIMUM_INDEX
};

typedef struct spiApiConfig
{
	uint8_t transmitByteOrder; /* MSB first 0 / LSB !0 */
	bool reportTransmit;    /* show write buffer content on transmit*/
	uint8_t csExternalSelectMask;     /* selection of pins to set when sending, subset of available cs channels */
	bool autoPurgeWriteBuffer;        /* auto purge write buffer at the end of write commands */
	bool autoPurgeReadBuffer;         /* auto purge read buffer  at the beginning of write commands */
	spiConfigUnion spiConfiguration;   /* hardware configuration */
	bool hardwareInit;
} spiApiConfig;

extern spiApiConfig spiApiConfiguration;
extern spiApiConfig* ptr_spiApiConfiguration;

void spiApi(struct uartStruct *ptr_uartStruct);
#warning space needed for APFEL command
#if 0
uint8_t spiApiSubCommands(struct uartStruct *ptr_uartStruct, int16_t subCommandIndex, uint8_t parameterIndex);
void spiApiSubCommandsFooter( uint16_t result );
void spiApiShowStatus( uint8_t status[], uint8_t size );
void spiApiShowStatusApiSettings(void);
void spiApiShowStatusBuffer(void);
void spiApiShowStatusChipSelect(void);
void spiApiShowStatusControlBits(void);
void spiApiShowStatusControls(void);
void spiApiShowStatusSpeed(void);


void spiApiInit(void);
uint8_t spiApiPurgeAndFillWriteArray(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex);
uint8_t spiApiAddToWriteArray(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex);
uint8_t spiApiAddNumericParameterToByteArray(const char string[], uint8_t index);
uint8_t spiApiAddNumericStringToByteArray(const char string[]);

uint8_t spiApiShowBufferContent(struct uartStruct *ptr_uartStruct, spiByteDataArray *buffer, int16_t nBytes, int8_t subCommandKeywordIndex);
uint8_t spiApiShowBuffer(struct uartStruct *ptr_uartStruct, spiByteDataArray *buffer, int8_t subCommandKeywordIndex);

uint8_t spiApiShowWriteBufferContent(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex);
/**/
uint8_t spiApiSubCommandShowStatus(void);
uint8_t spiApiSubCommandTransmit(void);
uint8_t spiApiSubCommandWriteBuffer(void);
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
uint8_t spiApiSubCommandCsBarStatus(struct uartStruct *ptr_uartStruct);
uint8_t spiApiSubCommandCsSelectMask(struct uartStruct *ptr_uartStruct);
uint8_t spiApiSubCommandCsPins(struct uartStruct *ptr_uartStruct);
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
uint8_t spiApiSubCommandTransmitReport(struct uartStruct *ptr_uartStruct);
uint8_t spiApiSubCommandAutoPurgeReadBuffer(struct uartStruct *ptr_uartStruct);
uint8_t spiApiSubCommandAutoPurgeWriteBuffer(struct uartStruct *ptr_uartStruct);
uint8_t spiApiSubCommandCsAddPin(struct uartStruct *ptr_uartStruct);
uint8_t spiApiSubCommandCsRemovePin(struct uartStruct *ptr_uartStruct);

uint8_t spiApiShowChipSelectAddress(int8_t chipSelectIndex);
void spiApiShowChipSelectStatus(uint8_t mask, bool invert);
uint8_t spiApiCsStatus(struct uartStruct *ptr_uartStruct, bool invert);
uint8_t spiApiCsSetOrCsRelease( bool set );

#endif
#endif /* SPIAPI_H_ */
