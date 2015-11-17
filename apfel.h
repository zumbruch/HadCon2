/*
   Licensed under the EUPL V.1.1, Lizenziert unter EUPL V.1.1
*/
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

typedef struct apfelAddressStruct
{
	char port;
	uint8_t pinSetIndex;
	uint8_t sideSelection;
	uint8_t chipId;
} apfelAddress;

#warning struct should go to api.h since spiApi also uses this
typedef struct apfelPinStruct
{
  volatile uint8_t *ptrPort;
  char portLetter;
  uint8_t pinNumber;
  bool isUsed;
} apfelPin;

enum apfelTestPulseTriggerPosition
{
	APFEL_TestPulse_TRIGGER_POSITION_AFTER_SET = 1,
	APFEL_TestPulse_TRIGGER_POSITION_AFTER_RESET,
	APFEL_TestPulse_TRIGGER_POSITION_MAXIMUM
};

void 	 apfelAutoCalibration_Inline(apfelAddress *address);
void     apfelClearDataInput_Inline(apfelAddress *address);
void 	 apfelInit_Inline(void);
void 	 apfelListIds_Inline(apfelAddress *address, bool all, uint8_t nElements, uint8_t min);
uint16_t apfelReadBitSequence_Inline(apfelAddress *address, uint8_t nBits);
int16_t	 apfelReadDac_Inline(apfelAddress *address, uint8_t dacNr, uint8_t quiet);
int8_t	 apfelReadPort(apfelAddress *address);
void	 apfelResetAmplification_Inline(apfelAddress *address, uint8_t channel);
void     apfelSendCommandValueChipIdClockSequence(uint8_t command, uint16_t value, uint16_t clockCycles, apfelAddress *address);
void     apfelSetAmplification_Inline(apfelAddress *address, uint8_t channel);
void     apfelSetDac_Inline(apfelAddress *address, uint16_t value, uint8_t dacNr, uint8_t quiet);
void     apfelStartStreamHeader_Inline(apfelAddress *address);
void     apfelTestPulse_Inline(apfelAddress *address, uint16_t pulseHeight, uint8_t channel);
void     apfelTestPulseSequence_Inline(apfelAddress *address, uint16_t pulseHeightPattern);
int8_t   apfelWriteBit_Inline(uint8_t bit, apfelAddress *address);
void     apfelWriteBitSequence_Inline(apfelAddress *address, int8_t nBits, uint16_t data, uint8_t endianness);
void     apfelWriteClockSequence_Inline(apfelAddress *address, uint16_t nClk);
int8_t   apfelWritePort(uint8_t val, apfelAddress *address);

void apfelEnable(bool enable);
apiCommandResult apfelTriggerCommand(uint8_t nSubCommandsArguments);



	/* definitions */
//#define DEBUG_APFEL
#define APFEL_US_TO_DELAY_DEFAULT 0

#define APFEL_N_CommandBits  4
#define APFEL_N_ValueBits  10
#define APFEL_ValueBits_MASK 0x3FF
#define APFEL_N_ChipIdBits  8
#define APFEL_N_Bits ({static const uint8_t a=APFEL_N_CommandBits + APFEL_N_ValueBits + APFEL_N_ChipIdBits;a;})
#define APFEL_LITTLE_ENDIAN  0
#define APFEL_BIG_ENDIAN  1
#define APFEL_DEFAULT_ENDIANNESS APFEL_BIG_ENDIAN

#define APFEL_COMMAND_SetDac  0x0
#define APFEL_COMMAND_ReadDac  0x4
#define APFEL_COMMAND_AutoCalibration  0xC
#define APFEL_COMMAND_TestPulse  0x9
#define APFEL_COMMAND_SetAmplitude  0xE
#define APFEL_COMMAND_ResetAmplitude  0xB

#define APFEL_COMMAND_SetDac_CommandClockCycles 3
#define APFEL_COMMAND_ReadDac_CommandClockCycles 0
#define APFEL_COMMAND_ReadDac_DataClockCycles (APFEL_READ_N_HEADER_BITS + APFEL_N_ValueBits + APFEL_READ_N_TRAILING_BITS)
#define APFEL_COMMAND_ReadDac_CommandClockCycles_Trailer 3
#define APFEL_COMMAND_AutoCalibration_CommandClockCycles ((0x1 << APFEL_N_ValueBits)<<2)
#define APFEL_COMMAND_TestPulseSequence_CommandClockCycles 3
#define APFEL_COMMAND_SetAmplification_CommandClockCycles 3
#define APFEL_COMMAND_ResetAmplification_CommandClockCycles 3

#define APFEL_READ_N_HEADER_BITS 2
#define APFEL_READ_HEADER 0x2
#define APFEL_READ_HEADER_MASK 0x3
#define APFEL_READ_N_TRAILING_BITS 3
#define APFEL_READ_TRAILER 0x7
#define APFEL_READ_TRAILER_MASK 0x7
#define APFEL_READ_CHECK_MASK  (( {static const uint16_t a = (((APFEL_READ_HEADER_MASK << (APFEL_N_ValueBits + APFEL_READ_N_TRAILING_BITS)) | APFEL_READ_TRAILER_MASK));a;}))
#define APFEL_READ_CHECK_VALUE (( {static const uint16_t a = (((APFEL_READ_HEADER      << (APFEL_N_ValueBits + APFEL_READ_N_TRAILING_BITS)) | APFEL_READ_TRAILER));a;}))

#define APFEL_N_PINS 4
#define APFEL_PIN_DIN1 	PINA0
#define APFEL_PIN_DOUT1 PINA1
#define APFEL_PIN_CLK1 	PINA2
#define APFEL_PIN_SS1 	PINA3

#define APFEL_PIN_DIN2 	PINA4
#define APFEL_PIN_DOUT2    PINA5
#define APFEL_PIN_CLK2 	PINA6
#define APFEL_PIN_SS2 	PINA7

#define APFEL_PIN_MASK1    ( {static const uint8_t a = (0xFF & (1 << APFEL_PIN_DOUT1 | 1 << APFEL_PIN_CLK1 | 1 << APFEL_PIN_SS1 ));a;})
#define APFEL_PIN_MASK2    ( {static const uint8_t a = (0xFF & (1 << APFEL_PIN_DOUT2 | 1 << APFEL_PIN_CLK2 | 1 << APFEL_PIN_SS2 ));a;})
#define APFEL_PIN_MASK_DIN ( {static const uint8_t a = (0xFF & (1 << APFEL_PIN_DIN1 | 1 << APFEL_PIN_DIN2 ))                      ;a;})

#define APFEL_writePort_CalcPattern(val,A,pinSetIndex)\
	    0xFF & (((0 == pinSetIndex-1) ? \
		(~(APFEL_PIN_MASK_DIN)) &  (( PIN##A & APFEL_PIN_MASK2) | (val & APFEL_PIN_MASK1)): \
		(~(APFEL_PIN_MASK_DIN)) &  (( PIN##A & APFEL_PIN_MASK1) | (val & APFEL_PIN_MASK2))))
#define APFEL_writePort(val,A,pinSetIndex) \
		    {PORT##A = (APFEL_writePort_CalcPattern(val,A,pinSetIndex));\
		               _delay_us(APFEL_US_TO_DELAY_DEFAULT);}
#define APFEL_readPort(A,pinSetIndex) ((PIN##A >> (APFEL_PIN_DIN##pinSetIndex )) & 0x1)





void apfelInit(void);

/*---*/
/*
#define APFEL_MAX_N_PORT_ADDRESS_SETS APFEL_PORT_ADDRESS_SET_MAXIMUM

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
*/
#endif /* APFEL_H_ */
