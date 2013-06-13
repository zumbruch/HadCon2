/*
 * spi.c
 *
 *  Created on: 12.06.2013
 *      Author: Florian Brabetz, GSI, f.brabetz@gsi.de
 */


#include <stdio.h>
#include "api.h"
#include "spi.h"
#include <avr/io.h>
#include <stdbool.h>
#include "api_define.h"
#include <avr/iocanxx.h>

// struct arrays for storing data to transmit and received data
spiByteDataArray spiWriteData;
spiByteDataArray spiReadData;

// initial SPI configuration
spiConfigUnion spiStandardConfiguration = { .bits.bSpr   = 0,
					    .bits.bCpha  = 0,
					    .bits.bCpol  = 0,
					    .bits.bMstr  = 1,
					    .bits.bDord  = 0,
					    .bits.bSpe   = 0,
					    .bits.bSpie  = 0,
					    .bits.bSpi2x = 0,
					    .bits.bWcol  = 0,
					    .bits.bSpif  = 0  };

// initial configuration for chipselectarray -> all chipselects are unused
spiPin spiChipSelectArray[CHIP_MAXIMUM] = { { 0 , 0 , false  },
					    { 0 , 0 , false  },
					    { 0 , 0 , false  },
					    { 0 , 0 , false  },
					    { 0 , 0 , false  },
					    { 0 , 0 , false  },
					    { 0 , 0 , false  },
					    { 0 , 0 , false  } };

uint8_t spiInternalChipSelectMask = 0;

uint8_t getChipSelectArrayStatus(void)
{
  uint8_t status = 0, i = 0;
  for( i = CHIPSELECT0 ; i < CHIP_MAXIMUM; i++)
    {
      if( spiChipSelectArray[i].isUsed )
	{
	  status |= ( 1 << i );
	}
    }
  return status;
}

uint8_t spiAddChipSelect(volatile uint8_t *ptrCurrentPort, uint8_t currentPinNumber, uint8_t chipSelectNumber)
{
  uint8_t i;

  if ( CHIP_MAXIMUM <= chipSelectNumber )
  {
	  /* cs number exceeds range */
	  //CommunicationError
	  return 30;
  }
  if( !spiChipSelectArray[chipSelectNumber].isUsed && (ptrCurrentPort > (uint8_t *)1))
    {
      spiChipSelectArray[chipSelectNumber].ptrPort = ptrCurrentPort;
      spiChipSelectArray[chipSelectNumber].pinNumber = currentPinNumber;
      spiChipSelectArray[chipSelectNumber].isUsed = true;

      // initialize chipselect pins as output pins
      for( i = 0; i < 8; i++ )
      {
    	  // PINA address 0x00
    	  // DDRA address 0x01
    	  // PORTA address 0x02
    	  if( spiChipSelectArray[i].isUsed == true )
    	  {
    		  // decrement address by one (now pointing to DDRx) and set the appropriate bit to one
    		  // pin becomes an output pin
    		  *(spiChipSelectArray[i].ptrPort-1) |= ( 1 << spiChipSelectArray[i].pinNumber );
    	  }
      }
    }
  else
    {
	  if( spiChipSelectArray[chipSelectNumber].isUsed)
	  {
		// Warning - chipSelectNumber already in use
		  return 10;
	  }
	  if( ptrCurrentPort <= (uint8_t *)1)
	  {
		// Warning invalid port address
		  return 20;
	  }
    }

  return 0;

}

<<<<<<< HEAD
uint8_t spiRemoveChipSelect(uint8_t chipSelectNumber)
=======
/* TODO: byte order ergänzen */
void spi_write_without_cs(uint8_t *ptr_data, uint8_t data_length)
>>>>>>> continued merge FB / PZ
{
	if ( CHIP_MAXIMUM <= chipSelectNumber )
	{
		/* cs number exceeds range */
		//CommunicationError
		return 1;
	}
	else
	{
		if( spiChipSelectArray[chipSelectNumber].isUsed == true )
		{
			// decrement address by one (now pointing to DDRx) and set the appropriate bit to zero
			// pin becomes an input pin again
			*(spiChipSelectArray[chipSelectNumber].ptrPort-1) &= ~( 1 << spiChipSelectArray[chipSelectNumber].pinNumber );
		}
		spiChipSelectArray[chipSelectNumber].isUsed = false;
		spiReleaseChipSelectInMask(chipSelectNumber);
	}

	return 0;
}


void spiInit(void) {
  uint8_t i;

  // DDRB logic 1 -> pin configured as output
  // set DDB2(MOSI) and DDB1(SCK) to output.
  // DDB3(MISO) is controlled by SPI logic
  DDRB |= (1<<DDB2) | (1<<DDB1);

  SPCR = (spiStandardConfiguration.data & 0x00FF);
  SPSR = ((spiStandardConfiguration.data >> 8) & 0x00FF);

  for( i = CHIPSELECT0 ; i < CHIP_MAXIMUM ; i++)
    {
      spiRemoveChipSelect(i);
    }

  spiAddChipSelect(&PORTB, PB0, CHIPSELECT0);
  spiSetChipSelectInMask(CHIPSELECT0);

  spiReleaseAllChipSelectLines();

  spiPurgeWriteData();
  spiPurgeReadData();
}


void spiWriteWithoutChipSelect(uint8_t data)
{
<<<<<<< HEAD
  SPDR = data;
  while( ! (SPSR & (1<<SPIF) ) );
}


void spiWriteAndReadWithoutChipSelect(uint8_t byteOrder)
=======
  spi_set_chosen_chipSelect(SPI_SET, chipSelect);
  spi_write_without_cs(ptr_data, data_length);
  spi_set_chosen_chipSelect(SPI_RELEASE, chipSelect);
}


<<<<<<< HEAD
void spi_write_and_read_with_cs(uint8_t *ptr_tx_data, uint8_t data_length, uint8_t *ptr_rx_data, uint8_t chipSelect)
>>>>>>> continued merge FB / PZ
{
  uint16_t i;
  if(SPI_MSBYTE_FIRST == byteOrder)
=======
void spi_write_and_read_with_cs(void)
{
#if 0
  uint8_t i;
  if( NULL != ptr_rx_data )
>>>>>>> debugging spi.c/h
    {
<<<<<<< HEAD
      i = 0;
      while( i < spiWriteData.length )
=======
      spi_set_chosen_chipSelect(SPI_SET, chipSelect);
      for( i=0 ; i<data_length ; i++ )
>>>>>>> continued merge FB / PZ
	{
	  spiWriteWithoutChipSelect(spiWriteData.data[i]);
	  spiReadData.data[i] = spiReadByte();
	  spiReadData.length++;
	  i++;
	}
<<<<<<< HEAD
=======
      spi_set_chosen_chipSelect(SPI_RELEASE, chipSelect);
>>>>>>> continued merge FB / PZ
    }
  else
    {
      i = 1;
      while( i <= spiWriteData.length )
	{
	  spiWriteWithoutChipSelect(spiWriteData.data[spiWriteData.length - i]);
	  spiReadData.data[spiWriteData.length - i] = spiReadByte();
	  spiReadData.length++;
	  i++;
	}
    }
<<<<<<< HEAD
 
}

void spiWriteAndReadWithChipSelect(uint8_t byteOrder, uint8_t externalChipSelectMask)
{
  spiSetChosenChipSelect(externalChipSelectMask);
  spiWriteAndReadWithoutChipSelect(byteOrder);
  spiReleaseChosenChipSelect(externalChipSelectMask);
=======
#endif
>>>>>>> debugging spi.c/h
}

uint8_t spiReadByte(void)
{
  return SPDR;
}

<<<<<<< HEAD
void spiReleaseAllChipSelectLines(void)
=======
void spi_set_chosen_chipSelect(uint8_t new_status, uint8_t chipSelect)
>>>>>>> continued merge FB / PZ
{
  uint8_t i;
  for( i = 0 ; i < 8 ; i++ )
    {
<<<<<<< HEAD
      if( spiChipSelectArray[i].isUsed )
      	{
	  *(spiChipSelectArray[i].ptrPort) |= ( 1 << (spiChipSelectArray[i].pinNumber) );
      	}
    }
}

void spiReleaseChosenChipSelect(uint8_t externalChipSelectMask)
{
  uint8_t i = 0;
  for( i = 0 ; i < 8 ; i++)
    {
      if( ((1 << i) & spiInternalChipSelectMask) & externalChipSelectMask )
      	{
	   *(spiChipSelectArray[i].ptrPort) |=  ( 1 << spiChipSelectArray[i].pinNumber);
      	}
    }
}

void spiSetChosenChipSelect(uint8_t externalChipSelectMask)
{
  uint8_t i = 0;
  //PORTB &= ~(1<<PB0); // selecting chip by putting the chip select line low
  for( i = 0 ; i < 8 ; i++)
    {
      if( ((1 << i) & spiInternalChipSelectMask) & externalChipSelectMask )
      	{
	   *(spiChipSelectArray[i].ptrPort) &=  ~( 1 << spiChipSelectArray[i].pinNumber);
      	}
    }
}

void spiSetChipSelectInMask(uint8_t chipSelectNumber)
{
  spiInternalChipSelectMask |= (1 << chipSelectNumber);
}

void spiReleaseChipSelectInMask(uint8_t chipSelectNumber)
{
  spiInternalChipSelectMask &= ~(1 << chipSelectNumber);
}

uint8_t spiGetInternalChipSelectMask(void)
{
  return spiInternalChipSelectMask;
}

spiByteDataArray spiGetReadData(void)
{
  return spiReadData;
}

void spiPurgeWriteData(void)
{
  spiWriteData.length = 0;
}

void spiPurgeReadData(void)
{
  spiReadData.length = 0;
}

void spiSetConfiguration(spiConfigUnion newConfig)
{
  SPSR = ((newConfig.data) >> 8) & 0x00FF;
  SPCR = (newConfig.data) & 0xFF;
}

spiConfigUnion spiGetConfiguration(void)
{
  spiConfigUnion currentConfig;
  currentConfig.data = ( (SPSR << 8) & 0xFF00 ) | (SPCR & 0x00FF) ;
  return currentConfig;
}

void spiEnable(bool enable)
{
  if(enable)
    {
      SPCR |= (1<<SPE);
    }
  else
    {
      SPCR &= ~(1<<SPE);
    }
}
  
/* // for testing purposes */
/* void spiTest(void) */
/* { */

/*   spiConfigUnion newConfig; */
/*   newConfig = spiGetConfiguration(); */

/*   newConfig.bits.bSpi2x = 0; */
/*   spiSetConfiguration(newConfig); */

/*   spiPurgeWriteData(); */

/*   spiAddWriteData((spiStandardConfiguration.data) & 0x00FF); */
/*   spiAddWriteData( ((spiStandardConfiguration.data) >> 8) & 0x00FF); */

/*   spiAddWriteData( (spiGetConfiguration().data) & 0x00FF); */
/*   spiAddWriteData( ((spiGetConfiguration().data) >> 8) & 0x00FF); */

/*   spiAddWriteData( getChipSelectArrayStatus() ); */

/*   spiAddWriteData( spiGetInternalChipSelectMask() ); */

/*   spiWriteAndReadWithChipSelect(SPI_MSBYTE_FIRST, SPI_MASK_ALL_CHIPSELECTS); */

/*   spiWriteData.data[0] = spiGetReadData().data[0]; */
/*   spiWriteData.data[1] = spiGetReadData().data[1]; */
/*   spiWriteData.data[2] = spiGetReadData().data[2]; */
/*   spiWriteData.data[3] = 0x55; */
/*   spiWriteData.data[4] = spiGetReadData().data[4]; */
/*   spiWriteData.data[5] = spiGetReadData().data[5]; */
/*   spiWriteData.length = spiGetReadData().length; */

/*   newConfig.bits.bSpi2x = 1; */
/*   spiSetConfiguration(newConfig); */

/*   spiWriteAndReadWithChipSelect(SPI_MSBYTE_FIRST, SPI_MASK_ALL_CHIPSELECTS); */
  
/*   spiPurgeReadData(); */
/* } */
=======
    case CHIP1:
      if( SPI_SET == new_status )
	{
	  PORTB &= ~(1<<PB0); // selecting chip by putting the chip select line low
	}
      else
	{
	  PORTB |= (1<<PB0);
	}
      break;
    case CHIP2:
      break;
    case CHIP3:
      break;
    default:
      break;
    }
}


void spi_purge_write_data(void)
{
	spiWriteData.length = 0;
}

void spi_purge_read_data(void)
{
	spiReadData.length = 0;
}
>>>>>>> continued merge FB / PZ
