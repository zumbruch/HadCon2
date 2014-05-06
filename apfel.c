/*
 * apfel.c
 *
 *  Created on: 12.06.2013
 *      Author: Florian Brabetz, GSI, f.brabetz@gsi.de
 */


#include <stdio.h>
#include "api.h"
#include "apfel.h"
#include <avr/io.h>
#include <stdbool.h>
#include <util/delay.h>

/*eclipse specific setting, not used during build process*/
#ifndef __AVR_AT90CAN128__
#include <avr/iocan128.h>
#endif


// struct arrays for storing data to transmit and received data
apfelByteDataArray apfelWriteData;
apfelByteDataArray apfelReadData;

// initial APFEL configuration
apfelConfigUnion apfelStandardConfiguration = { .bits.bSpr   = 0,
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
apfelPin apfelChipSelectArray[CHIPSELECT_MAXIMUM] = { { 0 , 0 , false  },
					    { 0 , 0 , false  },
					    { 0 , 0 , false  },
					    { 0 , 0 , false  },
					    { 0 , 0 , false  },
					    { 0 , 0 , false  },
					    { 0 , 0 , false  },
					    { 0 , 0 , false  } };

uint8_t apfelInternalChipSelectMask = 0;

uint8_t apfelGetChipSelectArrayStatus(void)
{
	uint8_t status = 0, i = 0;
	for (i = CHIPSELECT0; i < CHIPSELECT_MAXIMUM; i++)
	{
		if (apfelChipSelectArray[i].isUsed)
		{
			status |= (1 << i);
		}
	}
	return status;
}

uint8_t apfelAddChipSelect(volatile uint8_t *ptrCurrentPort, uint8_t currentPinNumber, uint8_t chipSelectNumber)
{
	uint8_t i;

	if (CHIPSELECT_MAXIMUM <= chipSelectNumber)
	{
		/* cs number exceeds range */
		CommunicationError_p(ERRA, SERIAL_ERROR_arguments_exceed_boundaries, true, NULL );
		return 30;
	}
	if (!apfelChipSelectArray[chipSelectNumber].isUsed && (ptrCurrentPort > (uint8_t *) 1))
	{
		apfelChipSelectArray[chipSelectNumber].ptrPort = ptrCurrentPort;
		apfelChipSelectArray[chipSelectNumber].pinNumber = currentPinNumber;
		apfelChipSelectArray[chipSelectNumber].isUsed = true;

		// initialize chipselect pins as output pins
		for (i = 0; i < 8; i++)
		{
			// PINA address 0x00
			// DDRA address 0x01
			// PORTA address 0x02
			if (apfelChipSelectArray[i].isUsed == true)
			{
				// decrement address by one (now pointing to DDRx) and set the appropriate bit to one
				// pin becomes an output pin
				*(apfelChipSelectArray[i].ptrPort - 1) |= (1 << apfelChipSelectArray[i].pinNumber);
			}
		}
	}
	else
	{
		if (apfelChipSelectArray[chipSelectNumber].isUsed)
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

	apfelSetChipSelectInMask(chipSelectNumber);
	return 0;

}

uint8_t apfelRemoveChipSelect(uint8_t chipSelectNumber)
{
	if (CHIPSELECT_MAXIMUM <= chipSelectNumber)
	{
		/* cs number exceeds range */
		CommunicationError_p(ERRA, SERIAL_ERROR_arguments_exceed_boundaries, true, NULL );
		return 1;
	}
	else
	{
		if (apfelChipSelectArray[chipSelectNumber].isUsed == true)
		{
			// decrement address by one (now pointing to DDRx) and set the appropriate bit to zero
			// pin becomes an input pin again
			*(apfelChipSelectArray[chipSelectNumber].ptrPort - 1) &=
					~(1 << apfelChipSelectArray[chipSelectNumber].pinNumber);
		}
		apfelChipSelectArray[chipSelectNumber].isUsed = false;
		apfelReleaseChipSelectInMask(chipSelectNumber);
	}

	return 0;
}

void apfelInit(void)
{
	uint8_t i;

	// DDRB logic 1 -> pin configured as output
	// set DDB2(MOSI) and DDB1(SCK) to output.
	// DDB3(MISO) is controlled by APFEL logic
	DDRB |= (1 << DDB2) | (1 << DDB1);

	SPCR = (apfelStandardConfiguration.data & 0x00FF);
	SPSR = ((apfelStandardConfiguration.data >> 8) & 0x00FF);

	for (i = CHIPSELECT0; i < CHIPSELECT_MAXIMUM; i++)
	{
		apfelRemoveChipSelect(i);
	}

	apfelAddChipSelect(&PORTB, PB0, CHIPSELECT0);

	apfelReleaseAllChipSelectLines();

	apfelPurgeWriteData();
	apfelPurgeReadData();
}

uint8_t apfelWriteWithoutChipSelect(uint8_t data)
{
	volatile uint8_t counter = 0;
	SPDR = data;
	while ((!(SPSR & (1 << SPIF))) && (counter < APFEL_MAX_WAIT_COUNT))
	{
		counter++;
	}
	if (counter >= APFEL_MAX_WAIT_COUNT)
	{
		return 10; // write fail
	}
	return 0; // write succeed

}

uint8_t apfelWriteAndReadWithoutChipSelect(uint8_t byteOrder)
{
	uint16_t i;
	uint8_t returnValue = 0;

	if ((apfelWriteData.length + apfelReadData.length) > (MAX_LENGTH_COMMAND >> 1))
	{
		return 20; // not enough space in readbuffer
	}

	if (APFEL_MSBYTE_FIRST == byteOrder)
	{
		i = 0;
		while ((i < apfelWriteData.length) && !returnValue)
		{
			returnValue = apfelWriteWithoutChipSelect(apfelWriteData.data[i]);
			apfelReadData.data[i] = apfelReadByte();
			apfelReadData.length++;
			i++;
		}
	}
	else
	{
		i = 1;
		while ((i <= apfelWriteData.length) && !returnValue)
		{
			returnValue = apfelWriteWithoutChipSelect(apfelWriteData.data[apfelWriteData.length - i]);
			apfelReadData.data[apfelWriteData.length - i] = apfelReadByte();
			apfelReadData.length++;
			i++;
		}
	}
	return returnValue;
}

uint8_t apfelWriteAndReadWithChipSelect(uint8_t byteOrder, uint8_t externalChipSelectMask)
{
	uint8_t returnValue = 0;
	apfelSetChosenChipSelect(externalChipSelectMask);
	returnValue = apfelWriteAndReadWithoutChipSelect(byteOrder);
	apfelReleaseChosenChipSelect(externalChipSelectMask);
	return returnValue;
}

uint8_t apfelReadByte(void)
{
	return SPDR ;
}

void apfelReleaseAllChipSelectLines(void)
{
	uint8_t i;
	for (i = 0; i < 8; i++)
	{
		if (apfelChipSelectArray[i].isUsed)
		{
			*(apfelChipSelectArray[i].ptrPort) |= (1 << (apfelChipSelectArray[i].pinNumber));
		}
	}
}

void apfelReleaseChosenChipSelect(uint8_t externalChipSelectMask)
{
	uint8_t i = 0;
	for (i = 0; i < 8; i++)
	{
		if (((1 << i) & apfelInternalChipSelectMask) & externalChipSelectMask)
		{
			*(apfelChipSelectArray[i].ptrPort) |= (1 << apfelChipSelectArray[i].pinNumber);
		}
	}
}

void apfelSetChosenChipSelect(uint8_t externalChipSelectMask)
{
	uint8_t i = 0;
	//PORTB &= ~(1<<PB0); // selecting chip by putting the chip select line low
	for (i = 0; i < 8; i++)
	{
		if (((1 << i) & apfelInternalChipSelectMask) & externalChipSelectMask)
		{
			*(apfelChipSelectArray[i].ptrPort) &= ~(1 << apfelChipSelectArray[i].pinNumber);
		}
	}
}

void apfelSetChipSelectInMask(uint8_t chipSelectNumber)
{
	apfelInternalChipSelectMask |= (1 << chipSelectNumber);
}

void apfelReleaseChipSelectInMask(uint8_t chipSelectNumber)
{
	apfelInternalChipSelectMask &= ~(1 << chipSelectNumber);
}

uint8_t apfelGetInternalChipSelectMask(void)
{
	return apfelInternalChipSelectMask;
}

apfelByteDataArray apfelGetReadData(void)
{
	return apfelReadData;
}

void apfelPurgeWriteData(void)
{
	apfelWriteData.length = 0;
}

void apfelPurgeReadData(void)
{
	apfelReadData.length = 0;
}

void apfelSetConfiguration(apfelConfigUnion newConfig)
{
	SPSR = ((newConfig.data) >> 8) & 0x00FF;
	SPCR = (newConfig.data) & 0xFF;
}

apfelConfigUnion apfelGetConfiguration(void)
{
	apfelConfigUnion currentConfig;
	currentConfig.data = ((SPSR << 8) & 0xFF00) | (SPCR & 0x00FF);
	return currentConfig;
}

void apfelEnable(bool enable)
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


uint8_t apfelGetCurrentChipSelectBarStatus(void)
{
	uint8_t currentChipSelectBarStatus = 0;
	uint8_t i;
	for (i = CHIPSELECT0; i < CHIPSELECT_MAXIMUM; i++)
	{
		if (apfelChipSelectArray[i].isUsed)
		{
			// PINA address 0x00
			// DDRA address 0x01
			// PORTA address 0x02
			// decrement address by two (now pointing to DDRx) and set the appropriate bit to one
			// pin becomes an output pin
			if (*(apfelChipSelectArray[i].ptrPort - 2) & (1 << apfelChipSelectArray[i].pinNumber))
			{
				currentChipSelectBarStatus |= (1 << i);
			}
		}
	}
	return currentChipSelectBarStatus;
}

apfelPin * apfelGetCurrentChipSelectArray(void)
{
	return apfelChipSelectArray;
}

volatile uint8_t * apfelGetPortFromChipSelect(uint8_t chipSelectNumber)
{
	return apfelChipSelectArray[chipSelectNumber].ptrPort;
}

uint8_t apfelGetPinFromChipSelect(uint8_t chipSelectNumber)
{
	return apfelChipSelectArray[chipSelectNumber].pinNumber;
}

