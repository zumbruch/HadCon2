/*
 * spi.h
 *
 *  Created on: 12.06.2013
 *      Author: Florian Brabetz, GSI, f.brabetz@gsi.de
 */


/*
 * Normal workflow:
 *-----------------
 * spiInit(); // initialize SPI and sets PORTB0 as CHIPSELECT0 and activates it in the internal chipselect mask
 * spiEnable(); // enables spi
 * spiPurgeWriteData(); // clean both data buffers for safety
 * spiPurgeReadData();
 * spiAddWriteData( value ) // adds one value to the writebuffer
 * spiWriteAndReadWithChipSelect(SPI_MSBYTE_FIRST, ( 1 << CHIPSELECT0 ) ); // sets CHIPSELECT0 to active and transmits the writebuffer
 * myData = spiGetReadData(); // the readbuffer is automatically filled by the WriteAndRead functions and can be accessed by spiGetReadData();
 *-------------------------------------------------
 *
 * Setting new configuration:
 *---------------------------
 * spiConfigUnion newConfig;
 * newConfig = spiGetConfiguration(); // get old configuration
 * newConfig.bits.bSpi2x = 1; // sets bSpi2x bit for double speed operation
 * newConfig.data = 0xXXXX // for accessing the whole 16 bits at once. be careful because you can easily influence the master- and enable bit!
 * spiSetConfiguration(newConfig); // set the new configuration
 *-------------------------------------------------
 *
 * Add a new chipselect line to the chipSelectArray:
 *---------------------------
 * spiAddChipSelect(&PORTC, PC3, CHIPSELECT1); // adding PORTC3 as CHIPSELECT1. PORTB0 is CHIPSELECT0 by default but can be removed by spiRemoveChipSelect(CHIPSELECT0) and a new PORT and PIN can be assigned for CHIPSELECT0
 * spiSetChipSelectInMask(CHIPSELECT1); // sets new chipselect active in the internal chipselect mask
 *
 *
 * Which chipselects are used:
 *----------------------------
 * uint8_t currentStatus;
 * currentStatus = spiChipSelectArrayStatus(); // returns one byte, if(bit0) -> chipselect0 is in use, if(bit1) -> chipselect1 is in use, etc...
 *-------------------------------------------------
 *
 * Which chipselects are masked by the internal mask:
 *---------------------------------------------------
 * uint8_t currentInternalMask;
 * curreentInternalMask = spiGetInternalChipSelectMask(); // returns one byte, if(bit0) -> chipselect0 is masked, if(bit1) -> chipselect1 is masked, etc...
 *-------------------------------------------------
 *
 * Manual influence of the chipselect lines:
 *--------------------------------------------
 * spiReleaseAllChipSelectLines(); // releases all usable chipselect lines
 * // suppose CS0 to CS3 are internally masked and usable present in the chipSelectArray
 * spiSetChosenChipSelect(SPI_MASK_ALL_CHIPSELECTS); // all internally masked chipselects will be set to active
 * spiReleaseChosenChipSelect( (1<<CHIPSELECT3) | (1<<CHIPSELECT0) ); // only CS0 and CS3 are released, CS1 and CS2 are not influenced
 *-------------------------------------------------
 *
 * Manual byte per byte transmission and reading of the one byte reception buffern:
 *---------------------------------------------------------------------------------
 * uint8_t receivedByte;
 * spiSetChosenChipSelect( 1 << CHIPSELECT0 ); // set CHIPSELECT0 (PORTB0) active
 * spiWriteWithoutChipSelect( dataValue );  // clock out 8 bit dataValue
 * receivedByte = spiReadByte();
 * spiReleaseChosenChipSelect( 1 << CHIPSELECT0 ); // release CHIPSELECT0
 *
 */


#ifndef SPI_H_
#define SPI_H_

#include <stdbool.h>

#define SPI_SET true
#define SPI_RELEASE false
#define SPI_MSBYTE_FIRST 0
#define SPI_LSBYTE_FIRST 1
#define SPI_MASK_ALL_CHIPSELECTS 0xFF

typedef struct spiByteArrayStruct
{
	uint8_t data[ MAX_LENGTH_COMMAND >> 1 ];
	uint16_t length;
} spiByteDataArray;

spiByteDataArray spiWriteData;
spiByteDataArray spiReadData;

typedef struct spiConfigStruct
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
} spiConfig;

typedef union
{
  spiConfig bits;
  uint16_t data;
} spiConfigUnion;

enum spiChipSelects
{
	CHIPSELECT0 = 0,
	CHIPSELECT1,
	CHIPSELECT2,
	CHIPSELECT3,
	CHIPSELECT4,
	CHIPSELECT5,
	CHIPSELECT6,
	CHIPSELECT7,
	CHIP_MAXIMUM
};

typedef struct spiPinStruct
{
  volatile uint8_t *ptrPort;
  uint8_t pinNumber;
  bool isUsed;
} spiPin;


// initialize SPI, remove all chipselect pins except PORTB0 and sets standard configuration
// should be called once at startup or later for reset to standard
void spiInit(void);
//spi_enables(true) enables spi
void spiEnable(bool enable);


// adds a chipselect line on the passed port and pin number e.g. (&PORTB, PB0, CHIPSELECT0)
// by default PORTB PB0 is active as CHIPSELECT0 by default when spi is initialized and enabled
uint8_t spiAddChipSelect(volatile uint8_t *ptrCurrentPort, uint8_t currentPinNumber, uint8_t chipSelectNumber);
// removes the chipselect line and sets corresponding pin as input pin
uint8_t spiRemoveChipSelect(uint8_t chipSelectNumber);
// returns one byte showing used status of the chipselect array. if bit0 is set so chipselect0 in the array is in use
uint8_t getChipSelectArrayStatus(void);


// sets chipSelectNumber in internal mask active
void spiSetChipSelectInMask(uint8_t chipSelectNumber);
// sets chipSelectNumber in internal mask inactive
void spiReleaseChipSelectInMask(uint8_t chipSelectNumber);
// returns the current internal ChipSelectMask
uint8_t spiGetInternalChipSelectMask(void);


// set all chipselect lines in chipSelectArray to high
void spiReleaseAllChipSelectLines(void);
// set all chipselect lines to high which are masked by internal mask & external mask(external = 0xff/SPI_MASK_ALL_CHIPSELECTS for all internal masked chipselects)
void spiReleaseChosenChipSelect(uint8_t externalChipSelectMask);
// set chipselect lines low which are masked by internal mask & external mask(external mask = 0xff/SPI_MASK_ALL_CHIPSELECTS for all internal masked chipselects)
void spiSetChosenChipSelect(uint8_t externalChipSelectMask);


// just clocking out 8 bits submitted in data
void spiWriteWithoutChipSelect(uint8_t data);
// reads the one byte read register of the internal SPI logic
uint8_t spiReadByte(void);
// transmits the writebuffer to chipselects which are masked by internalMask & externalMask. fills the readbuffer. byteOrder = SPI_MSBYTE_FIRST, SPI_LSBYTE_FIRST
void spiWriteAndReadWithChipSelect(uint8_t byteOrder, uint8_t externalChipSelectMask);
// transmits the writebuffer and fills the readbuffer. byteOrder = SPI_MSBYTE_FIRST, SPI_LSBYTE_FIRST
void spiWriteAndReadWithoutChipSelect(uint8_t byteOrder);


// sets SPI Status and SPI Config register according to spiConfigUnion
void spiSetConfiguration(spiConfigUnion);
// returns current values of SPI Status and SPI Config register as spiConfigUnion
spiConfigUnion spiGetConfiguration(void);


// fills the writebuffer which can be transmitted by spiWriteAndReadWithChipSelect()
// first byte added is the most significant byte, last byte the least significant
static inline uint16_t spiAddWriteData(uint8_t value)
{
  if( spiWriteData.length < (MAX_LENGTH_COMMAND >> 1) )
    {
      spiWriteData.data[spiWriteData.length] = value;
      spiWriteData.length++;
    }
  else
    {
      // Error writebuffer full
    }
  return spiWriteData.length;
}
// returns readbuffer
spiByteDataArray spiGetReadData(void);


// erase writebuffer
void spiPurgeWriteData(void);
// erase readbuffer
void spiPurgeReadData(void);


/* // for testing purposes */
/* void spiTest(void); */


#endif /* SPI_H_ */
