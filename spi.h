/*
   Licensed under the EUPL V.1.1, Lizenziert unter EUPL V.1.1 
*/
/*
 * spi.h
 *
 *  Created on: 12.06.2013
 *      Author: Florian Brabetz, GSI, f.brabetz@gsi.de
 */


/*
 * Normal workflow:
 *-----------------
 * spiInit(); // initialize SPI and sets PORTB0 as SPI_CHIPSELECT0 and activates it in the internal chipselect mask
 * spiEnable(); // enables spi
 * spiPurgeWriteData(); // clean both data buffers for safety
 * spiPurgeReadData();
 * spiAddWriteData( value ) // adds one value to the write buffer
 * spiWriteAndReadWithChipSelect(SPI_MSBYTE_FIRST, ( 1 << SPI_CHIPSELECT0 ) ); // sets SPI_CHIPSELECT0 to active and transmits the write buffer
 * myData = spiGetReadData(); // the read buffer is automatically filled by the WriteAndRead functions and can be accessed by spiGetReadData();
 *-------------------------------------------------
 *
 * Setting new configuration:
 *---------------------------
 * spiConfigUnion newConfig;
 * newConfig = spiGetConfiguration(); // get old configuration
 * newConfig.bits.bSpi2x = 1; // sets bSpi2x bit for double speed operation
 * newConfig.data = 0xXXXX // for accessing the whole 16 bits at once. be careful because you can easily influence the master- and enable bit!
 * spiSetConfiguration(newConfig); // set the new configration
 *-------------------------------------------------
 *
 * Add a new chipselect line to the chipSelectArray:
 *---------------------------
 * spiAddChipSelect(&PORTC, PC3, SPI_CHIPSELECT1); // adding PORTC3 as SPI_CHIPSELECT1. PORTB0 is SPI_CHIPSELECT0 by default but can be removed by spiRemoveChipSelect(SPI_CHIPSELECT0) and a new PORT and PIN can be assigned for SPI_CHIPSELECT0
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
 * currentInternalMask = spiGetInternalChipSelectMask(); // returns one byte, if(bit0) -> chipselect0 is masked, if(bit1) -> chipselect1 is masked, etc...
 *-------------------------------------------------
 *
 * Manual influence of the chipselect lines:
 *--------------------------------------------
 * spiReleaseAllChipSelectLines(); // releases all usable chipselect lines
 * // suppose CS0 to CS3 are internally masked and usable present in the chipSelectArray
 * spiSetChosenChipSelect(SPI_MASK_ALL_CHIPSELECTS); // all internally masked chipselects will be set to active
 * spiReleaseChosenChipSelect( (1<<SPI_CHIPSELECT3) | (1<<SPI_CHIPSELECT0) ); // only CS0 and CS3 are released, CS1 and CS2 are not influenced
 *-------------------------------------------------
 *
 * Manual byte per byte transmission and reading of the one byte reception buffer:
 *---------------------------------------------------------------------------------
 * uint8_t receivedByte;
 * spiSetChosenChipSelect( 1 << SPI_CHIPSELECT0 ); // set SPI_CHIPSELECT0 (PORTB0) active
 * spiWriteWithoutChipSelect( dataValue );  // clock out 8 bit dataValue
 * receivedByte = spiReadByte();
 * spiReleaseChoseChipSelect( 1 << SPI_CHIPSELECT0 ); // release SPI_CHIPSELECT0
 *
 */


#ifndef SPI_H_
#define SPI_H_

#include <stdbool.h>
#include <stdint.h>

#define SPI_SET true
#define SPI_RELEASE false
#define SPI_MSBYTE_FIRST 0
#define SPI_LSBYTE_FIRST 1
#define SPI_MASK_ALL_CHIPSELECTS 0xFF
#define SPI_MAX_WAIT_COUNT 120 // measured with osci

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
	SPI_CHIPSELECT0 = 0,
	SPI_CHIPSELECT1,
	SPI_CHIPSELECT2,
	SPI_CHIPSELECT3,
	SPI_CHIPSELECT4,
	SPI_CHIPSELECT5,
	SPI_CHIPSELECT6,
	SPI_CHIPSELECT7,
	SPI_CHIPSELECT_MAXIMUM
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


// adds a chipselect line on the passed port and pin number e.g. (&PORTB, PB0, SPI_CHIPSELECT0)
// by default PORTB PB0 is active as SPI_CHIPSELECT0 by default when spi is initialized and enabled
uint8_t spiAddChipSelect(volatile uint8_t *ptrCurrentPort, uint8_t currentPinNumber, uint8_t chipSelectNumber);
// removes the chipselect line and sets corresponding pin as input pin
uint8_t spiRemoveChipSelect(uint8_t chipSelectNumber);
// returns one byte showing used status of the chipselect array. if bit0 is set so chipselect0 in the array is in use
uint8_t spiGetChipSelectArrayStatus(void);


// sets chipSelectNumber in internal mask active
void spiSetChipSelectInMask(uint8_t chipSelectNumber);
// sets chipSelectNumber in internal mask inactive
void spiReleaseChipSelectInMask(uint8_t chipSelectNumber);
// returns the current internal ChipSelectMask
uint8_t spiGetInternalChipSelectMask(void);


// set all chipselect lines in chipSelectArray to high
void spiReleaseAllChipSelectLines(void);
// set all chipselectlines to high which are masked by internal mask & external mask(external = 0xff/SPI_MASK_ALL_CHIPSELECTS for all internal masked chipselects)
void spiReleaseChosenChipSelect(uint8_t externalChipSelectMask);
// set chipselectlines low which are masked by internal mask & external mask(external mask = 0xff/SPI_MASK_ALL_CHIPSELECTS for all internal masked chipselects)
void spiSetChosenChipSelect(uint8_t externalChipSelectMask);


// just clocking out 8 bits submitted in data
uint8_t spiWriteWithoutChipSelect(uint8_t data);
// reads the one byte read register of the internal SPI logic
uint8_t spiReadByte(void);
// transmits the write buffer to chipselects which are masked by internalMask & externalMask. fills the read buffer. byteOrder = SPI_MSBYTE_FIRST, SPI_LSBYTE_FIRST
uint8_t spiWriteAndReadWithChipSelect(uint8_t byteOrder, uint8_t externalChipSelectMask);
// transmits the write buffer and fills the read buffer. byteOrder = SPI_MSBYTE_FIRST, SPI_LSBYTE_FIRST
uint8_t spiWriteAndReadWithoutChipSelect(uint8_t byteOrder);


// sets SPI Status and SPI Config register according to spiConfigUnion
void spiSetConfiguration(spiConfigUnion);
// returns current values of SPI Status and SPI Config register as spiConfigUnion
spiConfigUnion spiGetConfiguration(void);


// fills the write buffer which can be transmitted by spiWriteAndReadWithChipSelect()
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
      CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, 0,  PSTR("write buffer full (max: %i)"), (MAX_LENGTH_COMMAND >> 1));
    }
  return spiWriteData.length;
}
// returns read buffer
spiByteDataArray spiGetReadData(void);


// erase write buffer
void spiPurgeWriteData(void);
// erase read buffer
void spiPurgeReadData(void);


// returns the current status of the chipselect output pins
uint8_t spiGetCurrentChipSelectBarStatus(void);

// returns a pointer to the internal ChipSelectArray
spiPin * spiGetCurrentChipSelectArray(void);

// returns the port pointer of one chipselect
volatile uint8_t * spiGetPortFromChipSelect(uint8_t chipSelectNumber);

// returns the pin number of one chipselect
uint8_t spiGetPinFromChipSelect(uint8_t chipSelectNumber);


#endif /* SPI_H_ */
