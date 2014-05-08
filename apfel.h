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

#define APFEL_SET true
#define APFEL_RELEASE false
#define APFEL_MSBYTE_FIRST 0
#define APFEL_LSBYTE_FIRST 1
#define APFEL_MASK_ALL_CHIPSELECTS 0xFF
#define APFEL_MAX_WAIT_COUNT 120 // measured with osci

typedef struct apfelByteArrayStruct
{
	uint8_t data[ MAX_LENGTH_COMMAND >> 1 ];
	uint16_t length;
} apfelByteDataArray;

apfelByteDataArray apfelWriteData;
apfelByteDataArray apfelReadData;

typedef struct apfelConfigStruct
{
  unsigned bSpr:2;
  unsigned bCpha:1;
  unsigned bCpol:1;
  unsigned bMstr:1;
  unsigned bDord:1;
  unsigned bSpe:1;
  unsigned bSpie:1;

  unsigned bSpi2x:1;
  unsigned unused:5;
  unsigned bWcol:1;
  unsigned bSpif:1;
} apfelConfig;

typedef union
{
  apfelConfig bits;
  uint16_t data;
} apfelConfigUnion;

enum apfelChipSelects
{
	APFEL_CHIPSELECT0 = 0,
	APFEL_CHIPSELECT1,
	APFEL_CHIPSELECT2,
	APFEL_CHIPSELECT3,
	APFEL_CHIPSELECT4,
	APFEL_CHIPSELECT5,
	APFEL_CHIPSELECT6,
	APFEL_CHIPSELECT7,
	APFEL_CHIPSELECT_MAXIMUM
};

typedef struct apfelPinStruct
{
  volatile uint8_t *ptrPort;
  uint8_t pinNumber;
  bool isUsed;
} apfelPin;


// initialize APFEL, remove all chipselect pins except PORTB0 and sets standard configuration
// should be called once at startup or later for reset to standard
void apfelInit(void);
//apfel_enables(true) enables apfel
void apfelEnable(bool enable);


// adds a chipselect line on the passed port and pin number e.g. (&PORTB, PB0, APFEL_CHIPSELECT0)
// by default PORTB PB0 is active as APFEL_CHIPSELECT0 by default when apfel is initialized and enabled
uint8_t apfelAddChipSelect(volatile uint8_t *ptrCurrentPort, uint8_t currentPinNumber, uint8_t chipSelectNumber);
// removes the chipselect line and sets corresponding pin as input pin
uint8_t apfelRemoveChipSelect(uint8_t chipSelectNumber);
// returns one byte showing used status of the chipselect array. if bit0 is set so chipselect0 in the array is in use
uint8_t apfelGetChipSelectArrayStatus(void);


// sets chipSelectNumber in internal mask active
void apfelSetChipSelectInMask(uint8_t chipSelectNumber);
// sets chipSelectNumber in internal mask inactive
void apfelReleaseChipSelectInMask(uint8_t chipSelectNumber);
// returns the current internal ChipSelectMask
uint8_t apfelGetInternalChipSelectMask(void);


// set all chipselect lines in chipSelectArray to high
void apfelReleaseAllChipSelectLines(void);
// set all chipselectlines to high which are masked by internal mask & external mask(external = 0xff/APFEL_MASK_ALL_CHIPSELECTS for all internal masked chipselects)
void apfelReleaseChosenChipSelect(uint8_t externalChipSelectMask);
// set chipselectlines low which are masked by internal mask & external mask(external mask = 0xff/APFEL_MASK_ALL_CHIPSELECTS for all internal masked chipselects)
void apfelSetChosenChipSelect(uint8_t externalChipSelectMask);


// just clocking out 8 bits submitted in data
uint8_t apfelWriteWithoutChipSelect(uint8_t data);
// reads the one byte read register of the internal APFEL logic
uint8_t apfelReadByte(void);
// transmits the write buffer to chipselects which are masked by internalMask & externalMask. fills the read buffer. byteOrder = APFEL_MSBYTE_FIRST, APFEL_LSBYTE_FIRST
uint8_t apfelWriteAndReadWithChipSelect(uint8_t byteOrder, uint8_t externalChipSelectMask);
// transmits the write buffer and fills the read buffer. byteOrder = APFEL_MSBYTE_FIRST, APFEL_LSBYTE_FIRST
uint8_t apfelWriteAndReadWithoutChipSelect(uint8_t byteOrder);


// sets APFEL Status and APFEL Config register according to apfelConfigUnion
void apfelSetConfiguration(apfelConfigUnion);
// returns current values of APFEL Status and APFEL Config register as apfelConfigUnion
apfelConfigUnion apfelGetConfiguration(void);


// fills the write buffer which can be transmitted by apfelWriteAndReadWithChipSelect()
// first byte added is the most significant byte, last byte the least significant
static inline uint16_t apfelAddWriteData(uint8_t value)
{
  if( apfelWriteData.length < (MAX_LENGTH_COMMAND >> 1) )
    {
      apfelWriteData.data[apfelWriteData.length] = value;
      apfelWriteData.length++;
    }
  else
    {
      CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, 0,  PSTR("write buffer full (max: %i)"), (MAX_LENGTH_COMMAND >> 1));
    }
  return apfelWriteData.length;
}
// returns read buffer
apfelByteDataArray apfelGetReadData(void);


// erase write buffer
void apfelPurgeWriteData(void);
// erase read buffer
void apfelPurgeReadData(void);


// returns the current status of the chipselect output pins
uint8_t apfelGetCurrentChipSelectBarStatus(void);

// returns a pointer to the internal ChipSelectArray
apfelPin * apfelGetCurrentChipSelectArray(void);

// returns the port pointer of one chipselect
volatile uint8_t * apfelGetPortFromChipSelect(uint8_t chipSelectNumber);

// returns the pin number of one chipselect
uint8_t apfelGetPinFromChipSelect(uint8_t chipSelectNumber);

/*---*/
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

double apfelUsToDelay;

uint8_t apfelSetClockAndDataLine( uint8_t portAddress, uint8_t value, uint8_t mask);

inline uint8_t apfelGetDataInLine(uint8_t portAddress, uint8_t pinDIN)
{
	return (REGISTER_READ_FROM_8BIT_REGISTER(portAddress) & (1 << pinDIN)) >> pinDIN;
}

apiCommandResult apfelWritePortA(uint8_t value, uint8_t mask);
apiCommandResult apfelWritePort(uint8_t value, uint8_t portAddress, uint8_t mask);

apiCommandResult apfelReadPortA(uint8_t* value);
apiCommandResult apfelReadPort(uint8_t* value, uint8_t portAddress);

apiCommandResult apfelWriteBit(uint8_t bit, uint8_t portAddress, uint8_t ss, uint8_t pinCLK, uint8_t pinDOUT, uint8_t pinSS);
apiCommandResult apfelWriteClockSequence(uint8_t num, uint8_t portAddress, uint8_t ss, uint8_t pinCLK, uint8_t pinDOUT, uint8_t pinSS);

#endif /* APFEL_H_ */
