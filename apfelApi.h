/*
 * apfelApi.h
 *
 *  Created on: 06.05.2014
 *      Author: Peter Zumbruch, GSI, P.Zumbruch@gsi.de
 */

#ifndef APFELAPI_H_
#define APFELAPI_H_

#include <stdbool.h>
#include "api.h"
#include "api_define.h"
#include "api_global.h"

#include "api_debug.h"
#include "apfel.h"

typedef uint8_t apiCommandResult;

extern const char* apfelApiCommandKeywords[] PROGMEM;
enum apfelApiCommandKeyNumber
{
	apfelApiCommandKeyNumber_ADD= 0,
	apfelApiCommandKeyNumber_A  ,
	apfelApiCommandKeyNumber_CONTROL_BITS,
	apfelApiCommandKeyNumber_C,
	apfelApiCommandKeyNumber_PURGE,
	apfelApiCommandKeyNumber_P,
	apfelApiCommandKeyNumber_READ,
	apfelApiCommandKeyNumber_R,
	apfelApiCommandKeyNumber_STATUS,
	apfelApiCommandKeyNumber_S,
	apfelApiCommandKeyNumber_TRANSMIT,
	apfelApiCommandKeyNumber_T,
	apfelApiCommandKeyNumber_WRITE,
	apfelApiCommandKeyNumber_W,
	apfelApiCommandKeyNumber_WRITE_BUFFER,
	apfelApiCommandKeyNumber_WB,

	apfelApiCommandKeyNumber_PURGE_WRITE_BUFFER,
	apfelApiCommandKeyNumber_PW,
	apfelApiCommandKeyNumber_PURGE_READ_BUFFER,
	apfelApiCommandKeyNumber_PR,
	apfelApiCommandKeyNumber_SHOW_WRITE_BUFFER,
	apfelApiCommandKeyNumber_SW,
	apfelApiCommandKeyNumber_SHOW_READ_BUFFER,
	apfelApiCommandKeyNumber_SR,

	apfelApiCommandKeyNumber_CS,
	apfelApiCommandKeyNumber_CS_BAR,
	apfelApiCommandKeyNumber_CSB,
	apfelApiCommandKeyNumber_CS_SET,
	apfelApiCommandKeyNumber_CSS,
	apfelApiCommandKeyNumber_CS_RELEASE,
	apfelApiCommandKeyNumber_CSR,
	apfelApiCommandKeyNumber_CS_SELECT_MASK,
	apfelApiCommandKeyNumber_CS_PINS,
	apfelApiCommandKeyNumber_CS_ADD_PIN,
	apfelApiCommandKeyNumber_CSAP,
	apfelApiCommandKeyNumber_CS_REMOVE_PIN,
	apfelApiCommandKeyNumber_CSRP,


	apfelApiCommandKeyNumber_APFEL_ENABLE,
	apfelApiCommandKeyNumber_DATA_ORDER,
	apfelApiCommandKeyNumber_MASTER,
	apfelApiCommandKeyNumber_CLOCK_POLARITY,
	apfelApiCommandKeyNumber_CLOCK_PHASE,
	apfelApiCommandKeyNumber_SPEED,
	apfelApiCommandKeyNumber_SPEED_DIVIDER,
	apfelApiCommandKeyNumber_DOUBLE_SPEED,
	apfelApiCommandKeyNumber_TRANSMIT_BYTE_ORDER,
	apfelApiCommandKeyNumber_RESET,
	apfelApiCommandKeyNumber_TRANSMIT_REPORT,
	apfelApiCommandKeyNumber_AUTO_PURGE_WRITE_BUFFER,
	apfelApiCommandKeyNumber_AUTO_PURGE_READ_BUFFER,
	apfelApiCommandKeyNumber_MAXIMUM_NUMBER

};

enum apfelApiCommandResults
{
	apfelApiCommandResult_SUCCESS_WITH_OUTPUT 			= 0,
	apfelApiCommandResult_SUCCESS_WITH_OPTIONAL_OUTPUT,
	apfelApiCommandResult_SUCCESS_QUIET,
	apfelApiCommandResult_FAILURE 						= 100,
	apfelApiCommandResult_FAILURE_NOT_A_SUB_COMMAND,
	apfelApiCommandResult_FAILURE_QUIET,
	apfelApiCommandResult_UNDEFINED,
	apfelApiCommandResult_MAXIMUM_INDEX
};

enum apfelApiVarTypes
{
	apiVarType_BOOL,
	apiVarType_BOOL_TrueFalse,
	apiVarType_BOOL_OnOff,
	apiVarType_BOOL_HighLow,
	apiVarType_UINT8,
	apiVarType_UINT16,
	apiVarType_UINT32,
	apiVarType_UINT64,
	apiVarType_UINTPTR,
	apfelApiVarType_MAXIMUM_INDEX,
};

enum apfelApiTransmitByteOrders
{
	apfelApiTransmitByteOrder_MSB = APFEL_MSBYTE_FIRST,
	apfelApiTransmitByteOrder_LSB = APFEL_LSBYTE_FIRST,
	apfelApiTransmitByteOrder_MAXIMUM_INDEX
};

enum apfelApiByteCompletions
{
	apfelApiByteCompletion_LEADING,
	apfelApiByteCompletion_TRAILING,
	apfelApiByteCompletion_MAXIMUM_INDEX
};

typedef struct apfelApiConfig
{
	uint8_t transmitByteOrder; /* MSB first 0 / LSB !0 */
	bool reportTransmit;    /* show write buffer content on transmit*/
	uint8_t csExternalSelectMask;     /* selection of pins to set when sending, subset of available cs channels */
	bool autoPurgeWriteBuffer;        /* auto purge write buffer at the end of write commands */
	bool autoPurgeReadBuffer;         /* auto purge read buffer  at the beginning of write commands */
	apfelConfigUnion apfelConfiguration;   /* hardware configuration */
	bool hardwareInit;
} apfelApiConfig;

extern apfelApiConfig apfelApiConfiguration;
extern apfelApiConfig* ptr_apfelApiConfiguration;

void apfelApi(struct uartStruct *ptr_uartStruct);
uint8_t apfelApiSubCommands(struct uartStruct *ptr_uartStruct, int16_t subCommandIndex, uint8_t parameterIndex);
void apfelApiSubCommandsFooter( uint16_t result );
void apfelApiShowStatus( uint8_t status[], uint8_t size );
void apfelApiShowStatusApiSettings(void);
void apfelApiShowStatusBuffer(void);
void apfelApiShowStatusChipSelect(void);
void apfelApiShowStatusControlBits(void);
void apfelApiShowStatusControls(void);
void apfelApiShowStatusSpeed(void);


void apfelApiInit(void);
uint8_t apfelApiPurgeAndFillWriteArray(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex);
uint8_t apfelApiAddToWriteArray(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex);
uint8_t apfelApiAddNumericParameterToByteArray(const char string[], uint8_t index);
uint8_t apfelApiAddNumericStringToByteArray(const char string[]);

uint8_t apfelApiShowBufferContent(struct uartStruct *ptr_uartStruct, apfelByteDataArray *buffer, int16_t nBytes, int8_t subCommandKeywordIndex);
uint8_t apfelApiShowBuffer(struct uartStruct *ptr_uartStruct, apfelByteDataArray *buffer, int8_t subCommandKeywordIndex);

uint8_t apfelApiShowWriteBufferContent(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex);
/**/
uint8_t apfelApiSubCommandShowStatus(void);
uint8_t apfelApiSubCommandTransmit(void);
uint8_t apfelApiSubCommandWriteBuffer(void);
uint8_t apfelApiSubCommandPurge(void);
uint8_t apfelApiSubCommandReset(void);
uint8_t apfelApiSubCommandPurgeWriteBuffer(void);
uint8_t apfelApiSubCommandPurgeReadBuffer(void);
uint8_t apfelApiSubCommandCsSet(void);
uint8_t apfelApiSubCommandCsRelease(void);
uint8_t apfelApiSubCommandRead(void);

uint8_t apfelApiSubCommandWrite(struct uartStruct *ptr_uartStruct, uint16_t parameterIndex);
uint8_t apfelApiSubCommandAdd(struct uartStruct *ptr_uartStruct);
uint8_t apfelApiSubCommandCsStatus(struct uartStruct *ptr_uartStruct);
uint8_t apfelApiSubCommandCsBarStatus(struct uartStruct *ptr_uartStruct);
uint8_t apfelApiSubCommandCsSelectMask(struct uartStruct *ptr_uartStruct);
uint8_t apfelApiSubCommandCsPins(struct uartStruct *ptr_uartStruct);
uint8_t apfelApiSubCommandShowWriteBuffer(struct uartStruct *ptr_uartStruct);
uint8_t apfelApiSubCommandShowReadBuffer(struct uartStruct *ptr_uartStruct);
uint8_t apfelApiSubCommandControlBits(struct uartStruct *ptr_uartStruct);
uint8_t apfelApiSubCommandSpiEnable(struct uartStruct *ptr_uartStruct);
uint8_t apfelApiSubCommandDataOrder(struct uartStruct *ptr_uartStruct);
uint8_t apfelApiSubCommandMaster(struct uartStruct *ptr_uartStruct);
uint8_t apfelApiSubCommandClockPolarity(struct uartStruct *ptr_uartStruct);
uint8_t apfelApiSubCommandClockPhase(struct uartStruct *ptr_uartStruct);
uint8_t apfelApiSubCommandSpeed(struct uartStruct *ptr_uartStruct);
uint8_t apfelApiSubCommandSpeedDivider(struct uartStruct *ptr_uartStruct);
uint8_t apfelApiSubCommandDoubleSpeed(struct uartStruct *ptr_uartStruct);
uint8_t apfelApiSubCommandTransmitByteOrder(struct uartStruct *ptr_uartStruct);
uint8_t apfelApiSubCommandTransmitReport(struct uartStruct *ptr_uartStruct);
uint8_t apfelApiSubCommandAutoPurgeReadBuffer(struct uartStruct *ptr_uartStruct);
uint8_t apfelApiSubCommandAutoPurgeWriteBuffer(struct uartStruct *ptr_uartStruct);
uint8_t apfelApiSubCommandCsAddPin(struct uartStruct *ptr_uartStruct);
uint8_t apfelApiSubCommandCsRemovePin(struct uartStruct *ptr_uartStruct);

uint8_t apiShowOrAssignParameterToValue(int16_t nArgumentArgs, uint8_t parameterIndex, void *value, uint8_t type, uint64_t min, uint64_t max, bool report, char message[]);
uint8_t apiAssignParameterToValue(uint8_t parameterIndex, void *value, uint8_t type, uint64_t min, uint64_t max);
uint8_t apiShowValue(char string[], void *value, uint8_t type );

uint8_t apfelApiShowChipSelectAddress(int8_t chipSelectIndex);
void apfelApiShowChipSelectStatus(uint8_t mask, bool invert);
uint8_t apfelApiCsStatus(struct uartStruct *ptr_uartStruct, bool invert);
uint8_t apfelApiCsSetOrCsRelease( bool set );

#endif /* APFELAPI_H_ */
