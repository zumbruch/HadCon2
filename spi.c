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
#include <util/delay.h>

/*eclipse specific setting, not used during build process*/
#ifndef __AVR_AT90CAN128__
#include <avr/iocan128.h>
#endif


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
spiPin spiChipSelectArray[CHIPSELECT_MAXIMUM] = { { 0 , 0 , false  },
					    { 0 , 0 , false  },
					    { 0 , 0 , false  },
					    { 0 , 0 , false  },
					    { 0 , 0 , false  },
					    { 0 , 0 , false  },
					    { 0 , 0 , false  },
					    { 0 , 0 , false  } };

uint8_t spiInternalChipSelectMask = 0;

uint8_t spiGetChipSelectArrayStatus(void)
{
	uint8_t status = 0, i = 0;
	for (i = CHIPSELECT0; i < CHIPSELECT_MAXIMUM; i++)
	{
		if (spiChipSelectArray[i].isUsed)
		{
			status |= (1 << i);
		}
	}
	return status;
}

uint8_t spiAddChipSelect(volatile uint8_t *ptrCurrentPort, uint8_t currentPinNumber, uint8_t chipSelectNumber)
{
	uint8_t i;

	if (CHIPSELECT_MAXIMUM <= chipSelectNumber)
	{
		/* cs number exceeds range */
		CommunicationError_p(ERRA, SERIAL_ERROR_arguments_exceed_boundaries, true, NULL );
		return 30;
	}
	if (!spiChipSelectArray[chipSelectNumber].isUsed && (ptrCurrentPort > (uint8_t *) 1))
	{
		spiChipSelectArray[chipSelectNumber].ptrPort = ptrCurrentPort;
		spiChipSelectArray[chipSelectNumber].pinNumber = currentPinNumber;
		spiChipSelectArray[chipSelectNumber].isUsed = true;

		// initialize chipselect pins as output pins
		for (i = 0; i < 8; i++)
		{
			// PINA address 0x00
			// DDRA address 0x01
			// PORTA address 0x02
			if (spiChipSelectArray[i].isUsed == true)
			{
				// decrement address by one (now pointing to DDRx) and set the appropriate bit to one
				// pin becomes an output pin
				*(spiChipSelectArray[i].ptrPort - 1) |= (1 << spiChipSelectArray[i].pinNumber);
			}
		}
	}
	else
	{
		if (spiChipSelectArray[chipSelectNumber].isUsed)
		{
			// chipSelectNumber already in use
			CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, true, PSTR("chipSelect #%i already in use"),
					chipSelectNumber + 1);
			return 10;
		}
		if (ptrCurrentPort <= (uint8_t *) 1)
		{
			// invalid port address
			CommunicationError_p(ERRA, SERIAL_ERROR_arguments_exceed_boundaries, true, NULL );
			return 20;
		}
	}

	spiSetChipSelectInMask(chipSelectNumber);
	return 0;

}

uint8_t spiRemoveChipSelect(uint8_t chipSelectNumber)
{
	if (CHIPSELECT_MAXIMUM <= chipSelectNumber)
	{
		/* cs number exceeds range */
		CommunicationError_p(ERRA, SERIAL_ERROR_arguments_exceed_boundaries, true, NULL );
		return 1;
	}
	else
	{
		if (spiChipSelectArray[chipSelectNumber].isUsed == true)
		{
			// decrement address by one (now pointing to DDRx) and set the appropriate bit to zero
			// pin becomes an input pin again
			*(spiChipSelectArray[chipSelectNumber].ptrPort - 1) &=
					~(1 << spiChipSelectArray[chipSelectNumber].pinNumber);
		}
		spiChipSelectArray[chipSelectNumber].isUsed = false;
		spiReleaseChipSelectInMask(chipSelectNumber);
	}

	return 0;
}

void spiInit(void)
{
	uint8_t i;

	// DDRB logic 1 -> pin configured as output
	// set DDB2(MOSI) and DDB1(SCK) to output.
	// DDB3(MISO) is controlled by SPI logic
	DDRB |= (1 << DDB2) | (1 << DDB1);

	SPCR = (spiStandardConfiguration.data & 0x00FF);
	SPSR = ((spiStandardConfiguration.data >> 8) & 0x00FF);

	for (i = CHIPSELECT0; i < CHIPSELECT_MAXIMUM; i++)
	{
		spiRemoveChipSelect(i);
	}

	spiAddChipSelect(&PORTB, PB0, CHIPSELECT0);

	spiReleaseAllChipSelectLines();

	spiPurgeWriteData();
	spiPurgeReadData();
}

uint8_t spiWriteWithoutChipSelect(uint8_t data)
{
	volatile uint8_t counter = 0;
	SPDR = data;
	while ((!(SPSR & (1 << SPIF))) && (counter < SPI_MAX_WAIT_COUNT))
	{
		counter++;
	}
	if (counter >= SPI_MAX_WAIT_COUNT)
	{
		return 10; // write fail
	}
	return 0; // write succeed

}

uint8_t spiWriteAndReadWithoutChipSelect(uint8_t byteOrder)
{
	uint16_t i;
	uint8_t returnValue = 0;

	if ((spiWriteData.length + spiReadData.length) > (MAX_LENGTH_COMMAND >> 1))
	{
		return 20; // not enough space in readbuffer
	}

	if (SPI_MSBYTE_FIRST == byteOrder)
	{
		i = 0;
		while ((i < spiWriteData.length) && !returnValue)
		{
			returnValue = spiWriteWithoutChipSelect(spiWriteData.data[i]);
			spiReadData.data[i] = spiReadByte();
			spiReadData.length++;
			i++;
		}
	}
	else
	{
		i = 1;
		while ((i <= spiWriteData.length) && !returnValue)
		{
			returnValue = spiWriteWithoutChipSelect(spiWriteData.data[spiWriteData.length - i]);
			spiReadData.data[spiWriteData.length - i] = spiReadByte();
			spiReadData.length++;
			i++;
		}
	}
	return returnValue;
}

uint8_t spiWriteAndReadWithChipSelect(uint8_t byteOrder, uint8_t externalChipSelectMask)
{
	uint8_t returnValue = 0;
	spiSetChosenChipSelect(externalChipSelectMask);
	returnValue = spiWriteAndReadWithoutChipSelect(byteOrder);
	spiReleaseChosenChipSelect(externalChipSelectMask);
	return returnValue;
}

uint8_t spiReadByte(void)
{
	return SPDR ;
}

void spiReleaseAllChipSelectLines(void)
{
	uint8_t i;
	for (i = 0; i < 8; i++)
	{
		if (spiChipSelectArray[i].isUsed)
		{
			*(spiChipSelectArray[i].ptrPort) |= (1 << (spiChipSelectArray[i].pinNumber));
		}
	}
}

void spiReleaseChosenChipSelect(uint8_t externalChipSelectMask)
{
	uint8_t i = 0;
	for (i = 0; i < 8; i++)
	{
		if (((1 << i) & spiInternalChipSelectMask) & externalChipSelectMask)
		{
			*(spiChipSelectArray[i].ptrPort) |= (1 << spiChipSelectArray[i].pinNumber);
		}
	}
}

void spiSetChosenChipSelect(uint8_t externalChipSelectMask)
{
	uint8_t i = 0;
	//PORTB &= ~(1<<PB0); // selecting chip by putting the chip select line low
	for (i = 0; i < 8; i++)
	{
		if (((1 << i) & spiInternalChipSelectMask) & externalChipSelectMask)
		{
			*(spiChipSelectArray[i].ptrPort) &= ~(1 << spiChipSelectArray[i].pinNumber);
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
	currentConfig.data = ((SPSR << 8) & 0xFF00) | (SPCR & 0x00FF);
	return currentConfig;
}

void spiEnable(bool enable)
{
	if (enable)
	{
		SPCR |= (1 << SPE);
	}
	else
	{
		SPCR &= ~(1 << SPE);
	}
}


uint8_t spiGetCurrentChipSelectBarStatus(void)
{
	uint8_t currentChipSelectBarStatus = 0;
	uint8_t i;
	for (i = CHIPSELECT0; i < CHIPSELECT_MAXIMUM; i++)
	{
		if (spiChipSelectArray[i].isUsed)
		{
			// PINA address 0x00
			// DDRA address 0x01
			// PORTA address 0x02
			// decrement address by two (now pointing to DDRx) and set the appropriate bit to one
			// pin becomes an output pin
			if (*(spiChipSelectArray[i].ptrPort - 2) & (1 << spiChipSelectArray[i].pinNumber))
			{
				currentChipSelectBarStatus |= (1 << i);
			}
		}
	}
	return currentChipSelectBarStatus;
}

spiPin * spiGetCurrentChipSelectArray(void)
{
	return spiChipSelectArray;
}

volatile uint8_t * spiGetPortFromChipSelect(uint8_t chipSelectNumber)
{
	return spiChipSelectArray[chipSelectNumber].ptrPort;
}

uint8_t spiGetPinFromChipSelect(uint8_t chipSelectNumber)
{
	return spiChipSelectArray[chipSelectNumber].pinNumber;
}

