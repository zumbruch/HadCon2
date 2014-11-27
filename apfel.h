/*
 * apfel.h
 *
 *  Created on: 06.05.2014
 *      Author: P.Zumbruch, GSI, P.Zumbruch@gsi.de
 */


/*
 * Normal workflow:
 *-----------------
 * apfelInit(); // initialize APFEL and sets PORTB0 as APFEL_CHIPSELECT0 and activates it in the internal chipselect mask
 * apfelEnable(); // enables apfel
 * apfelPurgeWriteData(); // clean both data buffers for safety
 * apfelPurgeReadData();
 * apfelAddWriteData( value ) // adds one value to the write buffer
 * apfelWriteAndReadWithChipSelect(APFEL_MSBYTE_FIRST, ( 1 << APFEL_CHIPSELECT0 ) ); // sets APFEL_CHIPSELECT0 to active and transmits the write buffer
 * myData = apfelGetReadData(); // the read buffer is automatically filled by the WriteAndRead functions and can be accessed by apfelGetReadData();
 *-------------------------------------------------
 *
 * Setting new configuration:
 *---------------------------
 * apfelConfigUnion newConfig;
 * newConfig = apfelGetConfiguration(); // get old configuration
 * newConfig.bits.bSpi2x = 1; // sets bSpi2x bit for double speed operation
 * newConfig.data = 0xXXXX // for accessing the whole 16 bits at once. be careful because you can easily influence the master- and enable bit!
 * apfelSetConfiguration(newConfig); // set the new configration
 *-------------------------------------------------
 *
 * Add a new chipselect line to the chipSelectArray:
 *---------------------------
 * apfelAddChipSelect(&PORTC, PC3, APFEL_CHIPSELECT1); // adding PORTC3 as APFEL_CHIPSELECT1. PORTB0 is APFEL_CHIPSELECT0 by default but can be removed by apfelRemoveChipSelect(APFEL_CHIPSELECT0) and a new PORT and PIN can be assigned for APFEL_CHIPSELECT0
 *
 *
 * Which chipselects are used:
 *----------------------------
 * uint8_t currentStatus;
 * currentStatus = apfelChipSelectArrayStatus(); // returns one byte, if(bit0) -> chipselect0 is in use, if(bit1) -> chipselect1 is in use, etc...
 *-------------------------------------------------
 *
 * Which chipselects are masked by the internal mask:
 *---------------------------------------------------
 * uint8_t currentInternalMask;
 * currentInternalMask = apfelGetInternalChipSelectMask(); // returns one byte, if(bit0) -> chipselect0 is masked, if(bit1) -> chipselect1 is masked, etc...
 *-------------------------------------------------
 *
 * Manual influence of the chipselect lines:
 *--------------------------------------------
 * apfelReleaseAllChipSelectLines(); // releases all usable chipselect lines
 * // suppose CS0 to CS3 are internally masked and usable present in the chipSelectArray
 * apfelSetChosenChipSelect(APFEL_MASK_ALL_CHIPSELECTS); // all internally masked chipselects will be set to active
 * apfelReleaseChosenChipSelect( (1<<APFEL_CHIPSELECT3) | (1<<APFEL_CHIPSELECT0) ); // only CS0 and CS3 are released, CS1 and CS2 are not influenced
 *-------------------------------------------------
 *
 * Manual byte per byte transmission and reading of the one byte reception buffer:
 *---------------------------------------------------------------------------------
 * uint8_t receivedByte;
 * apfelSetChosenChipSelect( 1 << APFEL_CHIPSELECT0 ); // set APFEL_CHIPSELECT0 (PORTB0) active
 * apfelWriteWithoutChipSelect( dataValue );  // clock out 8 bit dataValue
 * receivedByte = apfelReadByte();
 * apfelReleaseChoseChipSelect( 1 << APFEL_CHIPSELECT0 ); // release APFEL_CHIPSELECT0
 *
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
