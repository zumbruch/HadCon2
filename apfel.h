/*
 * apfel.h
 *
 *  Created on: 06.05.2014
 *      Author: P.Zumbruch, GSI, P.Zumbruch@gsi.de
 */




#ifndef APFEL_H_
#define APFEL_H_

#include <stdbool.h>
#include <stdint.h>
#include "read_write_register.h"

enum apfelChipSelects
{
	APFEL_PORT_ADDRESS_SET_0 = 0,
	APFEL_PORT_ADDRESS_SET_1,
	APFEL_PORT_ADDRESS_SET_2,
	APFEL_PORT_ADDRESS_SET_3,
	APFEL_PORT_ADDRESS_SET_4,
	APFEL_PORT_ADDRESS_SET_5,
	APFEL_PORT_ADDRESS_SET_MAXIMUM
};

typedef struct apfelAddressStruct
{
	char port;
	uint8_t pinSetIndex;
	uint8_t sideSelection;
} apfelAddress;

#warning struct should go to api.h since spiApi also uses this
typedef struct apfelPinStruct
{
  volatile uint8_t *ptrPort;
  uint8_t pinNumber;
  bool isUsed;
} apfelPin;


void	 apfelApi_Inline(void);
void 	 apfelAutoCalibration_Inline(char port, uint8_t pinSetIndex, uint8_t sideSelection, uint8_t chipId);
void 	 apfelClearDataInput_Inline(char port, uint8_t pinSetIndex, uint8_t sideSelection);
void 	 apfelInit_Inline(void);
void 	 apfelListIds_Inline(char port, uint8_t pinSetIndex, uint8_t sideSelection, bool all, uint8_t max);
uint16_t apfelReadBitSequence_Inline(char port, uint8_t pinSetIndex, uint8_t sideSelection, uint8_t nBits);
int16_t	 apfelReadDac_Inline(char port, uint8_t pinSetIndex, uint8_t sideSelection, uint8_t dacNr, uint16_t chipId, uint8_t quiet);
int8_t	 apfelReadPort(char port, uint8_t pinSetIndex, uint8_t sideSelection);
void	 apfelResetAmplitude_Inline(char port, uint8_t pinSetIndex, uint8_t sideSelection, uint8_t channel, uint8_t chipId);
void     apfelSendCommandValueChipIdSequence(uint8_t command, uint16_t value, uint16_t chipId, char port, uint8_t pinSetIndex, uint8_t sideSelection);
void     apfelSetAmplitude_Inline(char port, uint8_t pinSetIndex, uint8_t sideSelection, uint8_t channel, uint8_t chipId);
void     apfelSetDac_Inline(char port, uint8_t pinSetIndex, uint8_t sideSelection, uint16_t value, uint8_t dacNr, uint16_t chipId);
void     apfelStartStreamHeader_Inline(char port, uint8_t pinSetIndex, uint8_t sideSelection);
void     apfelTestPulse_Inline(char port, uint8_t pinSetIndex, uint8_t sideSelection, uint16_t pulseHeight, uint8_t channel, uint8_t chipId);
void     apfelTestPulseSequence_Inline(char port, uint8_t pinSetIndex, uint8_t sideSelection, uint16_t pulseHeightPattern, uint8_t chipId);
int8_t   apfelWriteBit_Inline(uint8_t bit, char port, uint8_t pinSetIndex, uint8_t sideSelection);
void     apfelWriteBitSequence_Inline(char port, uint8_t pinSetIndex, uint8_t sideSelection, int8_t nBits, uint16_t data, uint8_t endianness);
void     apfelWriteClockSequence_Inline(char port, uint8_t pinSetIndex, uint8_t sideSelection, uint16_t nClk);
int8_t   apfelWritePort(uint8_t val, char port, uint8_t pinSetIndex, uint8_t sideSelection);

void apfelEnable(bool enable);


/*---*/

#define APFEL_MAX_N_PORT_ADDRESS_SETS APFEL_PORT_ADDRESS_SET_MAXIMUM
#define APFEL_DEFAULT_US_TO_DELAY 1.0

#define APFEL_N_COMMAND_BITS  4
#define APFEL_N_VALUE_BITS    10
#define APFEL_N_CHIP_ID_BITS  8

#define APFEL_COMMAND_SET_DAC         0x0
#define APFEL_COMMAND_READ_DAC        0x4
#define APFEL_COMMAND_AUTOCALIBRATION 0xc
#define APFEL_COMMAND_TESTPULSE       0x9
#define APFEL_COMMAND_SET_AMPLITUDE   0xe
#define APFEL_COMMAND_RESET_AMPLITUDE 0xb

#define APFEL_TEST_PULSE_HEIGHT_PATTERN_MAX 0x1F

typedef struct apfelPinSetStruct
{
  unsigned bPinDIN:4;
  unsigned bPinDOUT:4;
  unsigned bPinCLK:4;
  unsigned bPinSS:4;
} apfelPinSet;

typedef union
{
  apfelPinSet bits;
  uint16_t data;
} apfelPinSetUnion;

typedef struct apfelPortAddressStatusStruct
{
  unsigned bIsEnabled:1;
  unsigned bIsInitialized:1;
  unsigned bIsSet:1;
  unsigned unused:5;
} apfelPortAddressStatus;

typedef union
{
  apfelPortAddressStatus bits;
  uint8_t data;
} apfelPortAddressStatusUnion;

typedef struct apfelPortAddressSetStruct
{
  volatile uint8_t *ptrPort;
  apfelPinSetUnion pins;
  apfelPortAddressStatusUnion status;
} apfelPortAddressSet;

typedef struct apfelChipAddressStruct
{
  uint8_t chipID;
  uint8_t ss;
  apfelPortAddressSet portAddress;
} apfelChipAddressStruct;


void apfel_Inline(void);

uint8_t apfelSetClockAndDataLine( uint8_t portAddress, uint8_t value, uint8_t mask);

inline uint8_t apfelGetDataInLine(uint8_t portAddress, uint8_t pinDIN)
{
	return (REGISTER_READ_FROM_8BIT_REGISTER(portAddress) & (1 << pinDIN)) >> pinDIN;
}

apiCommandResult apfelReadBitSequence(uint8_t nBits, uint32_t* bits, uint8_t portAddress, uint8_t ss, uint8_t pinDIN, uint8_t pinCLK, uint8_t pinSS);

apiCommandResult apfelWriteBit(uint8_t bit,                          uint8_t portAddress, uint8_t ss, uint8_t pinCLK, uint8_t pinDOUT, uint8_t pinSS);
apiCommandResult apfelWriteClockSequence(uint32_t num,               uint8_t portAddress, uint8_t ss, uint8_t pinCLK, uint8_t pinDOUT, uint8_t pinSS);

apiCommandResult apfelClearDataInput(                                uint8_t portAddress, uint8_t ss, uint8_t pinCLK, uint8_t pinDOUT, uint8_t pinSS);
apiCommandResult apfelWriteBitSequence(uint8_t num, uint32_t data,   uint8_t portAddress, uint8_t ss, uint8_t pinCLK, uint8_t pinDOUT, uint8_t pinSS);
apiCommandResult apfelStartStreamHeader(                             uint8_t portAddress, uint8_t ss, uint8_t pinCLK, uint8_t pinDOUT, uint8_t pinSS);

#warning TODO: combine common part into an address structure
apiCommandResult apfelCommandSet(uint16_t command, uint16_t value,    uint8_t chipID, uint8_t portAddress, uint8_t ss, uint8_t pinCLK, uint8_t pinDOUT, uint8_t pinSS);
apiCommandResult apfelSetDac(uint16_t value, uint8_t dacNr,           uint8_t chipID, uint8_t portAddress, uint8_t ss, uint8_t pinCLK, uint8_t pinDOUT, uint8_t pinSS);
apiCommandResult apfelReadDac(uint32_t* dacValue, uint8_t dacNr,      uint8_t chipID, uint8_t portAddress, uint8_t ss, uint8_t pinCLK, uint8_t pinDOUT, uint8_t pinSS, uint8_t pinDIN);
apiCommandResult apfelAutoCalibration(                                uint8_t chipID, uint8_t portAddress, uint8_t ss, uint8_t pinCLK, uint8_t pinDOUT, uint8_t pinSS);
apiCommandResult apfelTestPulseSequence(uint16_t pulseHeightPattern,  uint8_t chipID, uint8_t portAddress, uint8_t ss, uint8_t pinCLK, uint8_t pinDOUT, uint8_t pinSS);
apiCommandResult apfelTestPulse(uint8_t pulseHeight, uint8_t channel, uint8_t chipID, uint8_t portAddress, uint8_t ss, uint8_t pinCLK, uint8_t pinDOUT, uint8_t pinSS);
apiCommandResult apfelSetAmplitude(uint8_t channelId,                 uint8_t chipID, uint8_t portAddress, uint8_t ss, uint8_t pinCLK, uint8_t pinDOUT, uint8_t pinSS);
apiCommandResult apfelResetAmplitude(uint8_t channelId,               uint8_t chipID, uint8_t portAddress, uint8_t ss, uint8_t pinCLK, uint8_t pinDOUT, uint8_t pinSS);

void apfelInit(void);
apiCommandResult apfelAddOrModifyPortAddressSet(uint8_t portAddressSetIndex, volatile uint8_t *ptrCurrentPort, uint8_t pinIndexDIN, uint8_t pinIndexDOUT, uint8_t pinIndexCLK, uint8_t pinIndexSS);
apiCommandResult apfelRemovePortAddressSet(uint8_t portAddressSetIndex);
apiCommandResult apfelInitPortAddressSet(uint8_t portAddressSetIndex);
apiCommandResult apfelEnablePortAddressSet(uint8_t portAddressSetIndex);
apiCommandResult apfelDisablePortAddressSet(uint8_t portAddressSetIndex);
void apfelSetUsToDelay(double us);
double apfelGetUsToDelay(void);
volatile uint8_t * apfelGetPortFromPortAddressSet(uint8_t portAddressSetIndex);
apfelPortAddressStatusUnion apfelGetStatusFromPortAddressSet(uint8_t portAddressSetIndex);
apfelPinSetUnion apfelGetPinsFromPortAddressSet(uint8_t portAddressSetIndex);
#endif /* APFEL_H_ */
