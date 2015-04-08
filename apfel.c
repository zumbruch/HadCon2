/*
   Licensed under the EUPL V.1.1, Lizenziert unter EUPL V.1.1
*/
/* apfel.c
 *
 *  Created on: 12.06.2013
 *      Author: P.Zumbruch, GSI, p.zumbruch@gsi.de
 */


#include <stdio.h>
#include "api.h"
#include "apfel.h"
#include "apfelApi.h"
#include "read_write_register.h"
#include <avr/io.h>
#include <avr/wdt.h>
#include <stdbool.h>
#define __DELAY_BACKWARD_COMPATIBLE__
#include <util/delay.h>
#include <string.h>

/*eclipse specific setting, not used during build process*/
#ifndef __AVR_AT90CAN128__
#include <avr/iocan128.h>
#endif

static const char const filename[] 		                      PROGMEM = __FILE__;
static const char const string_address[]                      PROGMEM = "port:%c pinSet:%x side:%x chip:%x";
static const char const string_blank[]                        PROGMEM = " ";

bool apfelOscilloscopeTestFrameMode = false;
bool apfelEnableTrigger = false;
apfelPin apfelTrigger = {&PORTE, 'E', PINE6 + 1, 0};

/* functions */
int8_t apfelWritePort(uint8_t val, apfelAddress *address)
{
	if (0 == address->pinSetIndex || address->pinSetIndex > 2)
	{
		return -1;
	}

	// add sideSelection bit
	if (address->sideSelection)
	{
		val = (0xFF & (val | 1 << ((1==address->pinSetIndex)?APFEL_PIN_SS1:APFEL_PIN_SS2)));
	}

	switch (address->port)
	{
		case 'A':
			APFEL_writePort(val, A, address->pinSetIndex);
			break;
		case 'C':
			APFEL_writePort(val, C, address->pinSetIndex);
			break;
		case 'F':
			APFEL_writePort(val, F, address->pinSetIndex);
			break;
		default:
			return -1;
			break;
	}
	return 0;
}

int8_t apfelReadPort(apfelAddress *address)
{
	if (0 == address->pinSetIndex || address->pinSetIndex > 2)
	{
		return -1;
	}
	switch (address->port)
	{
		case 'A':
			return (1 == address->pinSetIndex) ? (APFEL_readPort(A, 1)) : (APFEL_readPort(A, 2));
			break;
		case 'C':
			return (1 == address->pinSetIndex) ? (APFEL_readPort(C, 1)) : (APFEL_readPort(C, 2));
			break;
		case 'F':
			return (1 == address->pinSetIndex) ? (APFEL_readPort(F, 1)) : (APFEL_readPort(F, 2));
			break;
		default:
			return -1;
			break;
	}
	return -1;
}

/* 1. data low  + clock Low  */
/* 2. data high + clock Low  */
/* 3. data high + clock High */
/* 4. data low  + clock High */
/* 5. data low  + clock low  */
static const char const apfelHigh[][5] =
{
		{ (0 << APFEL_PIN_DOUT1 | 0 << APFEL_PIN_CLK1),
		  (1 << APFEL_PIN_DOUT1 | 0 << APFEL_PIN_CLK1),
		  (1 << APFEL_PIN_DOUT1	| 1 << APFEL_PIN_CLK1),
		  (0 << APFEL_PIN_DOUT1 | 1 << APFEL_PIN_CLK1),
		  (0 << APFEL_PIN_DOUT1	| 0 << APFEL_PIN_CLK1) },

		{ (0 << APFEL_PIN_DOUT2 | 0 << APFEL_PIN_CLK2),
		  (1 << APFEL_PIN_DOUT2 | 0 << APFEL_PIN_CLK2),
		  (1 << APFEL_PIN_DOUT2 | 1 << APFEL_PIN_CLK2),
		  (0 << APFEL_PIN_DOUT2 | 1 << APFEL_PIN_CLK2),
		  (0 << APFEL_PIN_DOUT2 | 0 << APFEL_PIN_CLK2) } };

/* 1.data low + clock Low */
/* 2.data low + clock low */
/* 3.data low + clock High*/
/* 4.data low + clock High*/
/* 5.data low + clock low */
static const char const apfelLow[][5] =
{
		{ (0 << APFEL_PIN_DOUT1 | 0 << APFEL_PIN_CLK1),
		  (0 << APFEL_PIN_DOUT1 | 0 << APFEL_PIN_CLK1),
		  (0 << APFEL_PIN_DOUT1 | 1 << APFEL_PIN_CLK1),
		  (0 << APFEL_PIN_DOUT1 | 1 << APFEL_PIN_CLK1),
		  (0 << APFEL_PIN_DOUT1 | 0 << APFEL_PIN_CLK1) },

		{ (0 << APFEL_PIN_DOUT2 | 0 << APFEL_PIN_CLK2),
		  (0 << APFEL_PIN_DOUT2 | 0 << APFEL_PIN_CLK2),
		  (0 << APFEL_PIN_DOUT2	| 1 << APFEL_PIN_CLK2),
		  (0 << APFEL_PIN_DOUT2 | 1 << APFEL_PIN_CLK2),
		  (0 << APFEL_PIN_DOUT2	| 0 << APFEL_PIN_CLK2) } };

int8_t apfelWriteBit_Inline(uint8_t bit, apfelAddress *address)
{
	if (0 == address->pinSetIndex || address->pinSetIndex > 2)
	{
		return -1;
	}
//	sideSelection=(sideSelection)?1:0;

	if (0 == bit)
	{
		apfelWritePort(apfelLow [address->pinSetIndex - 1][0], address );
		apfelWritePort(apfelLow [address->pinSetIndex - 1][1], address );
		apfelWritePort(apfelLow [address->pinSetIndex - 1][2], address );
		apfelWritePort(apfelLow [address->pinSetIndex - 1][3], address );
		apfelWritePort(apfelLow [address->pinSetIndex - 1][4], address );
	}
	else
	{
		apfelWritePort(apfelHigh[address->pinSetIndex - 1][0], address );
		apfelWritePort(apfelHigh[address->pinSetIndex - 1][1], address );
		apfelWritePort(apfelHigh[address->pinSetIndex - 1][2], address );
		apfelWritePort(apfelHigh[address->pinSetIndex - 1][3], address );
		apfelWritePort(apfelHigh[address->pinSetIndex - 1][4], address );
	}
	return 0;
}

void apfelInit_Inline(void)
{
	/* init IO ports */
	//configure I/O port A
	DDRA = APFEL_PIN_MASK1 | APFEL_PIN_MASK2;
	DDRC = APFEL_PIN_MASK1 | APFEL_PIN_MASK2;
	DDRF = APFEL_PIN_MASK1 | APFEL_PIN_MASK2;

	/*trigger data direction register*/
	(*(apfelTrigger.ptrPort - 1)) &= (0xFF & (0x1 << (apfelTrigger.pinNumber -1)));

	PORTA = 0;
	PORTC = 0;
	PORTF = 0;
}

uint16_t apfelReadBitSequence_Inline(apfelAddress *address, uint8_t nBits)
{
	/* bits are supplied in BIG ENDIAN */
	static uint16_t value = 0;
	static uint8_t bitIndex = 0;
	value = 0;
	bitIndex = 0;
	uint8_t pinClk;
	uint8_t pinDout;

	switch (address->pinSetIndex)
	{
		case 1:
			pinClk = APFEL_PIN_CLK1;
			pinDout = APFEL_PIN_DOUT1;
			break;
		case 2:
			pinClk = APFEL_PIN_CLK2;
			pinDout = APFEL_PIN_DOUT2;
			break;
		default:
			pinClk = 0xFF;
			pinDout = 0xFF;
			return -1;
			break;
	}

	// make sure we start from clock 0 + data 0
	apfelWritePort(((0 << pinClk) | (0 << pinDout)), address);

	// initial bit read differently !!! TNX^6 Peter Wieczorek ;-)
	// since apfel needs first falling clock edge to activate the output pad
	// therefore first cycle then read data.
	if (nBits)
	{
		nBits--;
	}
	// clock high
	apfelWritePort((1 << pinClk), address);
	// clock low
	apfelWritePort((0 << pinClk), address);
	// read data in
	value |= (apfelReadPort(address) << (nBits - bitIndex));

	while (bitIndex < nBits)
	{
		bitIndex++;
		// clock high
		apfelWritePort((1 << pinClk), address);
		// read data in
		value |= (apfelReadPort(address) << (nBits - bitIndex));
		// clock low
		apfelWritePort((0 << pinClk), address);
	}
	return value;
}

void apfelWriteClockSequence_Inline(apfelAddress *address, uint16_t nClk)
{
	uint16_t a = nClk;
	while (a > 0)
	{
		a--;
		apfelWriteBit_Inline(0, address);
	}
}

/*  apfelClearDataInput
	 sends enough empty clock cycles to clear input buffer
	 after each byte add 3 0 writes */
void apfelClearDataInput_Inline(apfelAddress *address)
{
	uint8_t i = 0;
	static const uint8_t const val[2] =
	{
			(0 << APFEL_PIN_DOUT1 | 0 << APFEL_PIN_CLK1),
			(0 << APFEL_PIN_DOUT2 | 0 << APFEL_PIN_CLK2)
	};

	switch (address->pinSetIndex)
	{
		case 1:
		case 2:
			while (i < APFEL_N_Bits)
			{
				i++;
				if (0 == i % 8)
				{
					apfelWritePort(val[address->pinSetIndex], address);
					apfelWritePort(val[address->pinSetIndex], address);
				}
				apfelWriteClockSequence_Inline(address,  1);
			}
			i = 10;
			while (i > 0)
			{
				i--;
				apfelWritePort(val[address->pinSetIndex], address);
			}
			break;
		default:
			break;
	}
}

//mandatory header to all command sequences
void apfelStartStreamHeader_Inline(apfelAddress *address)
{
	apfelClearDataInput_Inline(address);

	//  1. clock Low
	//  2. clock Low  + data high
	//  3. clock High + data high
	//  4. clock low  + data high
	//  5. clock low  + data low

	uint8_t pinDout = 0xFF;
	uint8_t pinClk = 0xFF;
	switch (address->pinSetIndex)
	{
		case 1:
			pinDout = APFEL_PIN_DOUT1;
			pinClk = APFEL_PIN_CLK1;
			break;
		case 2:
			pinDout = APFEL_PIN_DOUT2;
			pinClk = APFEL_PIN_CLK2;
			break;
		default:
			return;
			break;
	}
	/*sideSelection*/
	apfelWritePort((0 << pinDout | 0 << pinClk), address);
	/* header */
	apfelWritePort((0 << pinDout | 0 << pinClk), address);
	apfelWritePort((1 << pinDout | 0 << pinClk), address);
	apfelWritePort((1 << pinDout | 1 << pinClk), address);
	apfelWritePort((1 << pinDout | 0 << pinClk), address);
	apfelWritePort((0 << pinDout | 0 << pinClk), address);
}

/* writeBitSequence #bits #data #endianess (0: little, 1:big)*/
void apfelWriteBitSequence_Inline(apfelAddress *address, int8_t nBits, uint16_t data, uint8_t endianness)
{
	int8_t bitPos = 0;

	switch (endianness)
	{
		case APFEL_LITTLE_ENDIAN:
			while (bitPos < nBits)
			{
				apfelWriteBit_Inline(((uint8_t) ((data >> bitPos) & 0x1)), address);
				bitPos++;
			}
			break;
		case APFEL_BIG_ENDIAN:
			bitPos = nBits - 1;
			while (bitPos >= 0)
			{
				apfelWriteBit_Inline(((uint8_t) ((data >> bitPos) & 0x1)), address);
				bitPos--;
			}
			break;
		default:
			CommunicationError_p(ERRA, -1, 1, PSTR("wrong endianness: %i (%i,%i)"), endianness, APFEL_LITTLE_ENDIAN,
					APFEL_BIG_ENDIAN);
			return;
			break;
	}
}

void apfelSendCommandValueChipIdClockSequence(uint8_t command, uint16_t value, uint16_t clockCycles, apfelAddress *address)
{
	apfelStartStreamHeader_Inline(address);
	// command
	apfelWriteBitSequence_Inline(address, APFEL_N_CommandBits, command, APFEL_DEFAULT_ENDIANNESS);
	// pulse height
	apfelWriteBitSequence_Inline(address, APFEL_N_ValueBits, value, APFEL_DEFAULT_ENDIANNESS);
	// chipId
	apfelWriteBitSequence_Inline(address, APFEL_N_ChipIdBits, address->chipId, APFEL_DEFAULT_ENDIANNESS);

	// add  external trigger on opposite clock pin//
	if (apfelEnableTrigger)
	{
		_delay_us(0);
		*(apfelTrigger.ptrPort) = *(apfelTrigger.ptrPort) | (0xFF &  (0x1 << (apfelTrigger.pinNumber - 1)));
		_delay_us(0);
		*(apfelTrigger.ptrPort) = *(apfelTrigger.ptrPort) & (0xFF & ~(0x1 << (apfelTrigger.pinNumber - 1)));
		_delay_us(0);
	}

	if (0 < clockCycles)
	{
		apfelWriteClockSequence_Inline(address, clockCycles);
	}
}

void (*apfelSendCommandValueChipIdClockSequence_p)(uint8_t command, uint16_t value, uint16_t clockCycles, apfelAddress *address) = apfelSendCommandValueChipIdClockSequence;

/* #setDac value[ 0 ... 3FF ] dacNr[1..4] chipID[0 ... FF]	*/

void apfelSetDac_Inline(apfelAddress *address, uint16_t value, uint8_t dacNr, uint8_t quiet)
{
	apfelSendCommandValueChipIdClockSequence_p(APFEL_COMMAND_SetDac + dacNr, value, APFEL_COMMAND_SetDac_CommandClockCycles, address);
	//inkl. 3 intermediate clock cycles equiv. 3 writeDataLow

	if (! quiet) {
			apfelReadDac_Inline(address, dacNr, false);
	}
}

/* #readDac dacNr[1..4] chipID[0 ... FF] */
int16_t apfelReadDac_Inline(apfelAddress *address, uint8_t dacNr, uint8_t quiet)
{
	uint16_t value = 0;
	apfelSendCommandValueChipIdClockSequence_p(APFEL_COMMAND_ReadDac + dacNr, 0, APFEL_COMMAND_ReadDac_CommandClockCycles, address);

	// read 15 bits
	value = apfelReadBitSequence_Inline(address, (APFEL_READ_N_HEADER_BITS + APFEL_N_ValueBits + APFEL_READ_N_TRAILING_BITS));


	apfelWriteClockSequence_Inline(address,  APFEL_COMMAND_ReadDac_CommandClockCycles_Trailer);

#if 0
	if (0 > value)
	{
		if (!quiet)
			CommunicationError_p(ERRA, -1, 1, PSTR("%S %S: readDac failed"), string_address, string_dac_,
					address->port, address->pinSetIndex, address->sideSelection, dacNr, address->chipId);
		/* Error */
		return -1;
	}
#endif
	// check validity for correct header (10) and trailing bits (111)
	/* Error */
	if ( APFEL_READ_CHECK_VALUE != (value & APFEL_READ_CHECK_MASK))
	{
		if (0 == quiet)
		{
			snprintf_P(resultString, BUFFER_SIZE - 1, string_address,
					resultString, address->port, address->pinSetIndex, address->sideSelection, address->chipId);
			snprintf_P(resultString, BUFFER_SIZE - 1, PSTR("%s dac:%x"), resultString, dacNr);
			CommunicationError_p(ERRA, -1, 1,
					PSTR("%s -read validity check failed, raw value:0x%x"), resultString, value);
		}
		return -10;
	}
	else
	{
		if ( 0 == quiet )
		{
			value = (value >> APFEL_READ_N_TRAILING_BITS) & APFEL_ValueBits_MASK;

			createExtendedSubCommandReceiveResponseHeader(ptr_uartStruct, commandKeyNumber_APFEL,
					apfelApiCommandKeyNumber_DAC, apfelApiCommandKeywords);
    		snprintf_P(resultString, BUFFER_SIZE -1, string_address, address->port, address->pinSetIndex, address->sideSelection, address->chipId );
			strncat(uart_message_string, resultString, BUFFER_SIZE - 1);
			snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s dac:%x "), uart_message_string, dacNr);
			apiShowValue(uart_message_string, &value, apiVarType_UINT16);
			apiSubCommandsFooter(apiCommandResult_SUCCESS_WITH_OUTPUT);
		}
		return value;
	}
}

/* #autoCalibration chipId[0 ... FF] */
void apfelAutoCalibration_Inline(apfelAddress *address)//char port, uint8_t pinSetIndex, uint8_t sideSelection, uint8_t chipId)
{
	if (0xFF==address->chipId)
	{
#warning TODO (implement readout of current watchdog timing from the WDT registers of ATMEL) or even better create kind of callback routine, and lock mechanism, while calibration is running
		bool watchdogStatus = 1 << WDE & WDTCR;
		uint8_t watchdogTiming;
		if (watchdogStatus)
		{
			watchdogTiming = (1 << WDP0 | 1 << WDP1 | 1<< WDP2) & WDTCR;
			wdt_disable();
		}
		for (uint8_t id = 0; id < 0xFF; id++)
		{
			apfelSendCommandValueChipIdClockSequence_p(APFEL_COMMAND_AutoCalibration, 0, APFEL_COMMAND_AutoCalibration_CommandClockCycles, address);
			// inkl. calibration sequence clocks 4 * 10bit DAC
		}
		if (watchdogStatus)
		{
			wdt_enable(watchdogTiming);
		}
	}
	else
	{
		apfelSendCommandValueChipIdClockSequence_p(APFEL_COMMAND_AutoCalibration, 0, APFEL_COMMAND_AutoCalibration_CommandClockCycles, address);
		// inkl. calibration sequence clocks 4 * 10bit DAC
	}
}

/* #testPulseSequence pulseHeightPattern[0 ... 1F] chipId[0 ... FF] */
void apfelTestPulseSequence_Inline(apfelAddress *address, uint16_t pulseHeightPattern)
{
	apfelSendCommandValueChipIdClockSequence_p(APFEL_COMMAND_TestPulse, pulseHeightPattern, APFEL_COMMAND_TestPulseSequence_CommandClockCycles, address);
}

/* #testPulse pulseHeight[0 ... 1F] channel[1 .. 2 ] chipId[0 ... FF] */
void apfelTestPulse_Inline(apfelAddress *address, uint16_t pulseHeight, uint8_t channel)
{
	apfelTestPulseSequence_Inline(address, 0x3FF & (pulseHeight << ((channel==1)?1:6)));
	apfelTestPulseSequence_Inline(address, 0);
}

/*#setAmplitude channelId[1 ... 2] chipId[0 ... FF]*/
void apfelSetAmplitude_Inline(apfelAddress *address, uint8_t channel)
{
	apfelSendCommandValueChipIdClockSequence_p(APFEL_COMMAND_SetAmplitude + ((channel==1)?1:0), 0, APFEL_COMMAND_SetAmplification_CommandClockCycles, address);
}

/*#resetAmplitude channelId[1 ... 2] chipId[0 ... FF]*/
void apfelResetAmplitude_Inline(apfelAddress *address, uint8_t channel)
{
	apfelSendCommandValueChipIdClockSequence_p(APFEL_COMMAND_ResetAmplitude + ((channel==2)?2:0), 0, APFEL_COMMAND_ResetAmplification_CommandClockCycles, address);
}

/*#list Ids by checking result for dacRead */
/* all lists all chipIds independent of presence*/
void apfelListIds_Inline(apfelAddress *address, bool all, uint8_t nElements, uint8_t min)
{
    uint8_t result[32];
    memset(result,0,sizeof(result));
    uint16_t max = 0;

    if (0 == nElements)
    {
    	nElements = 0xFF;
    }
    if (0 == min)
    {
    	min = 1;
    }

    max = min((uint16_t)(min + nElements),0x00FFU);

    /*check store result in an bit array*/
    for (address->chipId = min; address->chipId < max; (address->chipId)++)
	{
		if ( 0 <= apfelReadDac_Inline(address, 1, true) )
		{
			result[(address->chipId) >> 3] |= (1 << ((address->chipId) % 8));
		}
	}

    /* print out */

    // disable watchdog, since printout would take too, long.
	bool watchdogStatus = 1 << WDE & WDTCR;
	uint8_t watchdogTiming;
	if (watchdogStatus)
	{
		watchdogTiming = (1 << WDP0 | 1 << WDP1 | 1<< WDP2) & WDTCR;
		wdt_disable();
	}

    for (uint8_t chipId = min; chipId < max; chipId++)
	{
    	if ( all || (result[chipId >> 3] & (1 << (chipId % 8 ))))
    	{
			createExtendedSubCommandReceiveResponseHeader(ptr_uartStruct, commandKeyNumber_APFEL,
					apfelApiCommandKeyNumber_LIST, apfelApiCommandKeywords);
    		snprintf_P(resultString, BUFFER_SIZE -1, string_address, address->port, address->pinSetIndex, address->sideSelection, chipId );
			snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s%s "), uart_message_string, resultString);
			strncat_P(uart_message_string, (result[chipId >> 3] & 1 << (chipId % 8 )) ? PSTR("yes") : PSTR("no"), BUFFER_SIZE - 1);
			UART0_Send_Message_String_p(NULL, 0);
    	}
	}
    // re-enable watchdog
	if (watchdogStatus)
	{
    	wdt_enable(watchdogTiming);
    }
}

apiCommandResult apfelTriggerCommand(uint8_t nSubCommandsArguments)
{
	switch (nSubCommandsArguments /* arguments of argument */)
	{
		case 3:
			/* pin */
			if (apiCommandResult_FAILURE_QUIET == apiAssignParameterToValue(4, &apfelTrigger.pinNumber, apiVarType_UINT8, 1, 8))
			{
				return apiCommandResult_FAILURE_QUIET;
			}

			/*port*/
			switch (setParameter[3][0])
			{
				case 'A':
					apfelTrigger.ptrPort = &PORTA;
					break;
				case 'C':
					apfelTrigger.ptrPort = &PORTC;
					break;
				case 'D':
					apfelTrigger.ptrPort = &PORTD;
					break;
				case 'E':
					apfelTrigger.ptrPort = &PORTE;
					break;
				case 'F':
					apfelTrigger.ptrPort = &PORTF;
					break;
				default:
					CommunicationError_p(ERRA, SERIAL_ERROR_arguments_exceed_boundaries, true,
							PSTR("[A,C,D,E,F] %c"), setParameter[3][0]);
					return apiCommandResult_FAILURE_QUIET;
					break;
			}

			apfelTrigger.portLetter = setParameter[3][0];

			/* access DDR register by decrementing the address by 1 and set the trigger pin to be an output */
			*(apfelTrigger.ptrPort - 1) = *(apfelTrigger.ptrPort - 1)
										| (0xFF & (0x1 << (apfelTrigger.pinNumber - 1)));
		case 1: /*enable/disable trigger */
			apiAssignParameterToValue(2, &apfelEnableTrigger, apiVarType_BOOL, 0, 1);
			apfelTrigger.isUsed = apfelEnableTrigger;
			if (apfelEnableTrigger)
			{
				/*trigger data direction register*/
				if (!((*(apfelTrigger.ptrPort - 1)) & (0xFF & (0x1 << (apfelTrigger.pinNumber - 1)))))
				{
					(*(apfelTrigger.ptrPort - 1)) &= (0xFF & (0x1 << (apfelTrigger.pinNumber - 1)));
				}
				/* set trigger to low */
				*(apfelTrigger.ptrPort) = *(apfelTrigger.ptrPort) & (0xFF & ~(0x1 << (apfelTrigger.pinNumber - 1)));
			}
		case 0: /* status */
			createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
			strncat_P(uart_message_string, PSTR("trigger "), BUFFER_SIZE - 1);
			apiShowValue(uart_message_string, &apfelEnableTrigger, apiVarType_BOOL_OnOff);
			snprintf(uart_message_string, BUFFER_SIZE -1 , "%s %c ", uart_message_string, apfelTrigger.portLetter);
			apiShowValue(uart_message_string, &apfelTrigger.pinNumber, apiVarType_UINT8);
			apiSubCommandsFooter(apiCommandResult_SUCCESS_WITH_OUTPUT);
			break;
	}
	return apiCommandResult_SUCCESS_QUIET;
}


void apfel_Inline()
{
	apfelInit_Inline();

	static const apfelAddress address={.port='A',.pinSetIndex=1,.sideSelection=0};
	if (apfelOscilloscopeTestFrameMode)
	{
		apfelWritePort((1 << APFEL_PIN_DOUT1 | 1 << APFEL_PIN_CLK1), (apfelAddress*) &address);
		apfelWritePort((1 << APFEL_PIN_DOUT1 | 0 << APFEL_PIN_CLK1), (apfelAddress*) &address);
		apfelWritePort((1 << APFEL_PIN_DOUT1 | 1 << APFEL_PIN_CLK1), (apfelAddress*) &address);
		apfelWritePort((0 << APFEL_PIN_DOUT1 | 0 << APFEL_PIN_CLK1), (apfelAddress*) &address);
	}

	apfelApi_Inline();
	//apfelApi_Inline_p();

	if (apfelOscilloscopeTestFrameMode)
	{
		apfelWritePort((1 << APFEL_PIN_DOUT1 | 1 << APFEL_PIN_CLK1), (apfelAddress*) &address);
		apfelWritePort((1 << APFEL_PIN_DOUT1 | 0 << APFEL_PIN_CLK1), (apfelAddress*) &address);
		apfelWritePort((1 << APFEL_PIN_DOUT1 | 1 << APFEL_PIN_CLK1), (apfelAddress*) &address);
		apfelWritePort((1 << APFEL_PIN_DOUT1 | 0 << APFEL_PIN_CLK1), (apfelAddress*) &address);
		apfelWritePort((1 << APFEL_PIN_DOUT1 | 1 << APFEL_PIN_CLK1), (apfelAddress*) &address);
		apfelWritePort((0 << APFEL_PIN_DOUT1 | 0 << APFEL_PIN_CLK1), (apfelAddress*) &address);

	}
}

uint16_t apfelPortAddressSetMask = 0;

#if 0
apfelPortAddressSet apfelPortAddressSets[APFEL_MAX_N_PORT_ADDRESS_SETS];
#endif

//----------------
void apfelInit(void)
{
#if 0
	uint8_t i;

	for (i = APFEL_PORT_ADDRESS_SET_0; i < APFEL_PORT_ADDRESS_SET_MAXIMUM; i++)
	{
		apfelRemovePortAddressSet(i);
	}

	apfelAddOrModifyPortAddressSet(APFEL_PORT_ADDRESS_SET_0, &PORTA, PA1, PA2, PA3, PA4);
	apfelAddOrModifyPortAddressSet(APFEL_PORT_ADDRESS_SET_1, &PORTA, PA4, PA5, PA6, PA7);
	apfelAddOrModifyPortAddressSet(APFEL_PORT_ADDRESS_SET_2, &PORTC, PC1, PC2, PC3, PC4);
	apfelAddOrModifyPortAddressSet(APFEL_PORT_ADDRESS_SET_3, &PORTC, PC4, PC5, PC6, PC7);
	apfelAddOrModifyPortAddressSet(APFEL_PORT_ADDRESS_SET_4, &PORTF, PF1, PF2, PF3, PF4);
	apfelAddOrModifyPortAddressSet(APFEL_PORT_ADDRESS_SET_5, &PORTF, PF4, PF5, PF6, PF7);

	for (i = APFEL_PORT_ADDRESS_SET_0; i < APFEL_PORT_ADDRESS_SET_MAXIMUM; i++)
	{
		apfelInitPortAddressSet(i);
	}

	for (i = APFEL_PORT_ADDRESS_SET_0; i < APFEL_PORT_ADDRESS_SET_MAXIMUM; i++)
	{
		apfelEnablePortAddressSet(i);
	}
#endif
}

#if 0
apfelPortAddressSet * apfelGetPortAddressSetArray(void)
{
	return apfelPortAddressSets;
}
volatile uint8_t * apfelGetPortFromPortAddressSet(uint8_t portAddressSetIndex)
{
	if ( APFEL_PORT_ADDRESS_SET_MAXIMUM <= portAddressSetIndex)
	{
		return NULL;
	}
	return (apfelPortAddressSets[portAddressSetIndex]).ptrPort;
}

apfelPinSetUnion apfelGetPinsFromPortAddressSet(uint8_t portAddressSetIndex)
{
	return apfelPortAddressSets[portAddressSetIndex].pins;
}

apfelPortAddressStatusUnion apfelGetStatusFromPortAddressSet(uint8_t portAddressSetIndex)
{
	return apfelPortAddressSets[portAddressSetIndex].status;
}


apiCommandResult apfelAddOrModifyPortAddressSet(uint8_t portAddressSetIndex, volatile uint8_t *ptrCurrentPort,
		                                        uint8_t pinIndexDIN, uint8_t pinIndexDOUT,
		                                        uint8_t pinIndexCLK, uint8_t pinIndexSS)
{
	if ( APFEL_PORT_ADDRESS_SET_MAXIMUM <= portAddressSetIndex)
	{
		return apiCommandResult_FAILURE;
	}
	if (NULL == ptrCurrentPort)
	{
		return apiCommandResult_FAILURE;
	}
    if (pinIndexDIN > 7 || pinIndexDOUT > 7|| pinIndexCLK > 7|| pinIndexSS > 7)
    {
    	return apiCommandResult_FAILURE;
    }
    if (pinIndexDIN < 1 || pinIndexDOUT < 1|| pinIndexCLK < 1|| pinIndexSS < 1)
    {
    	return apiCommandResult_FAILURE;
    }

	(apfelPortAddressSets[portAddressSetIndex]).pins.bits.bPinCLK = pinIndexCLK  - 1;
	(apfelPortAddressSets[portAddressSetIndex]).pins.bits.bPinDIN = pinIndexDIN  - 1;
	(apfelPortAddressSets[portAddressSetIndex]).pins.bits.bPinDOUT= pinIndexDOUT - 1;
	(apfelPortAddressSets[portAddressSetIndex]).pins.bits.bPinSS  = pinIndexSS   - 1;
	(apfelPortAddressSets[portAddressSetIndex]).ptrPort = ptrCurrentPort;
	(apfelPortAddressSets[portAddressSetIndex]).status.bits.bIsEnabled = 0;
	(apfelPortAddressSets[portAddressSetIndex]).status.bits.bIsInitialized = 0;
	(apfelPortAddressSets[portAddressSetIndex]).status.bits.bIsSet         = 1;

	return apiCommandResult_SUCCESS_QUIET;
}

apiCommandResult apfelRemovePortAddressSet(uint8_t portAddressSetIndex)
{
	if ( APFEL_PORT_ADDRESS_SET_MAXIMUM <= portAddressSetIndex)
	{
		return apiCommandResult_FAILURE;
	}

	apfelPortAddressSets[portAddressSetIndex].pins.bits.bPinCLK          = 0;
	apfelPortAddressSets[portAddressSetIndex].pins.bits.bPinDIN          = 0;
	apfelPortAddressSets[portAddressSetIndex].pins.bits.bPinDOUT         = 0;
	apfelPortAddressSets[portAddressSetIndex].pins.bits.bPinSS           = 0;
	apfelPortAddressSets[portAddressSetIndex].ptrPort                    = NULL;
	apfelPortAddressSets[portAddressSetIndex].status.bits.bIsEnabled     = 0;
	apfelPortAddressSets[portAddressSetIndex].status.bits.bIsInitialized = 0;
	apfelPortAddressSets[portAddressSetIndex].status.bits.bIsSet         = 0;

	return apiCommandResult_SUCCESS_QUIET;
}

apiCommandResult apfelEnablePortAddressSet(uint8_t portAddressSetIndex)
{
	if ( APFEL_PORT_ADDRESS_SET_MAXIMUM <= portAddressSetIndex)
	{
		return apiCommandResult_FAILURE;
	}
	if (0 == apfelPortAddressSets[portAddressSetIndex].status.bits.bIsSet)
	{
		return apiCommandResult_FAILURE;
	}
	if (0 == apfelPortAddressSets[portAddressSetIndex].status.bits.bIsInitialized)
	{
		return apiCommandResult_FAILURE;
	}

	apfelPortAddressSetMask |= (0x1 << portAddressSetIndex);
	apfelPortAddressSets[portAddressSetIndex].status.bits.bIsEnabled = 1;

	return apiCommandResult_SUCCESS_QUIET;
}

apiCommandResult apfelDisblePortAddressSet(uint8_t portAddressSetIndex)
{
	if ( APFEL_PORT_ADDRESS_SET_MAXIMUM <= portAddressSetIndex)
	{
		return apiCommandResult_FAILURE;
	}

	apfelPortAddressSetMask &= ~(0x1 << portAddressSetIndex);
	apfelPortAddressSets[portAddressSetIndex].status.bits.bIsEnabled = 0;

	return apiCommandResult_SUCCESS_QUIET;
}


apiCommandResult apfelInitPortAddressSet(uint8_t portAddressSetIndex)
{
	if ( APFEL_PORT_ADDRESS_SET_MAXIMUM <= portAddressSetIndex)
	{
		return apiCommandResult_FAILURE;
	}

	if (0 == apfelPortAddressSets[portAddressSetIndex].status.bits.bIsSet)
	{
		return apiCommandResult_FAILURE;
	}


	// PINA address 0x00
	// DDRA address 0x01
	// PORTA address 0x02
	// decrement address by two (now pointing to DDRx) and set the appropriate bit to one
	// pin becomes an output pin

	// input pin / no pull up
	*(apfelPortAddressSets[portAddressSetIndex].ptrPort - 2) &= ~(0x1 << apfelPortAddressSets[portAddressSetIndex].pins.bits.bPinDIN);
    *(apfelPortAddressSets[portAddressSetIndex].ptrPort    ) &=  (0x1 << apfelPortAddressSets[portAddressSetIndex].pins.bits.bPinDIN);

    // output pins
	*(apfelPortAddressSets[portAddressSetIndex].ptrPort - 2) |= (0x1 << apfelPortAddressSets[portAddressSetIndex].pins.bits.bPinCLK);
	*(apfelPortAddressSets[portAddressSetIndex].ptrPort - 2) |= (0x1 << apfelPortAddressSets[portAddressSetIndex].pins.bits.bPinDOUT);
	*(apfelPortAddressSets[portAddressSetIndex].ptrPort - 2) |= (0x1 << apfelPortAddressSets[portAddressSetIndex].pins.bits.bPinSS);

	//port address set is initialized
	apfelPortAddressSets[portAddressSetIndex].status.bits.bIsInitialized = 1;

	return apiCommandResult_SUCCESS_QUIET;
}

/* read/write bits*/
apiCommandResult apfelWriteBit(uint8_t bit, uint8_t portAddress, uint8_t ss, uint8_t pinCLK, uint8_t pinDOUT, uint8_t pinSS)
{
	if (pinCLK > 7 || pinDOUT > 7 || pinSS > 7)
	{
		return apiCommandResult_FAILURE_QUIET;
	}

	uint8_t mask = (1 << pinCLK | 1 << pinDOUT | 1 << pinSS);

	bit = bit?1:0;
    ss  = ss?1:0;

	//clock Low + data low
	if ( false == apfelSetClockAndDataLine( portAddress, ((uint8_t)(0 << pinCLK | 0   << pinDOUT | ss << pinSS)), mask)) { return apiCommandResult_FAILURE_QUIET; }
	//# clock Low  + data "bit"
	if ( false == apfelSetClockAndDataLine( portAddress, ((uint8_t)(0 << pinCLK | bit << pinDOUT | ss << pinSS)), mask)) { return apiCommandResult_FAILURE_QUIET; }
	//# clock High + data "bit"
	if ( false == apfelSetClockAndDataLine( portAddress, ((uint8_t)(1 << pinCLK | bit << pinDOUT | ss << pinSS)), mask)) { return apiCommandResult_FAILURE_QUIET; }
	//# clock High + data low
	if ( false == apfelSetClockAndDataLine( portAddress, ((uint8_t)(1 << pinCLK | 0   << pinDOUT | ss << pinSS)), mask)) { return apiCommandResult_FAILURE_QUIET; }
	//# clock low  + data low
	if ( false == apfelSetClockAndDataLine( portAddress, ((uint8_t)(0 << pinCLK | 0   << pinDOUT | ss << pinSS)), mask)) { return apiCommandResult_FAILURE_QUIET; }

	return apiCommandResult_SUCCESS_QUIET;
}

apiCommandResult apfelReadBitSequence(uint8_t nBits, uint32_t* bits, uint8_t portAddress, uint8_t ss, uint8_t pinDIN, uint8_t pinCLK, uint8_t pinSS)
{
	if (pinCLK > 7 || pinDIN > 7 || pinSS > 7)
	{
		return apiCommandResult_FAILURE_QUIET;
	}
	if (NULL == bits)
	{
		return apiCommandResult_FAILURE_QUIET;
	}
	if (32 < nBits)
	{
		return apiCommandResult_FAILURE_QUIET;
	}

	ss = ss?1:0;
    *bits = 0;

	//make sure we start from clock 0
	if ( false == apfelSetClockAndDataLine(portAddress, (uint8_t)(0 << pinCLK | ss << pinSS), (uint8_t)(1 << pinCLK)))
	{
		return apiCommandResult_FAILURE_QUIET;
	}

	// initial bit read differently !!! TNX^6 Peter Wieczorek ;-)
	// since apfel needs first falling clock edge to activate the output pad
	// therefore first cycle then read data.
	//clock high
	if ( false == apfelSetClockAndDataLine(portAddress, (uint8_t)(1 << pinCLK | ss << pinSS), (uint8_t)(1 << pinCLK)))
	{
		return apiCommandResult_FAILURE_QUIET;
	}
	//clock low
	if ( false == apfelSetClockAndDataLine(portAddress, (uint8_t)(0 << pinCLK | ss << pinSS), (uint8_t)(1 << pinCLK)))
	{
		return apiCommandResult_FAILURE_QUIET;
	}

	//read first data bit in
    //LSB first

	*bits |= apfelGetDataInLine( portAddress, pinDIN );

	#warning check correct output
    //MSB first
	//*bits |= apfelGetDataInLine( portAddress, pinDIN ) << (nBits -1) ;

	//read remaining (max 31 bits)
	for (uint8_t n=1; n<nBits-1; n++)
	{
		//clock high
		if ( false == apfelSetClockAndDataLine(portAddress, (uint8_t)(1 << pinCLK | ss << pinSS), (uint8_t)(1 << pinCLK)))
		{
			return apiCommandResult_FAILURE_QUIET;
		}

		//read data bit in
	    //LSB first

		*bits |= apfelGetDataInLine( portAddress, pinDIN ) << n;
#warning check correct output
		//MSB first
		//*bits |= apfelGetDataInLine( portAddress, pinDIN ) << (nBits -1 - n) ;
		//_delay_us(apfelUsToDelay);

		//clock low
		if ( false == apfelSetClockAndDataLine(portAddress, (uint8_t)(0 << pinCLK | ss << pinSS), (uint8_t)(1 << pinCLK)))
		{
			return apiCommandResult_FAILURE_QUIET;
		}
	}

	return apiCommandResult_SUCCESS_QUIET;
}

apiCommandResult apfelWriteClockSequence(uint32_t num, uint8_t portAddress, uint8_t ss, uint8_t pinCLK, uint8_t pinDOUT, uint8_t pinSS)
{
	while (num)
	{
		if (apiCommandResult_FAILURE < apfelWriteBit(0, portAddress, ss, pinCLK, pinDOUT, pinSS))
		{
			return apiCommandResult_FAILURE_QUIET;
		}
		num--;
	}
	return apiCommandResult_SUCCESS_QUIET;
}
uint8_t apfelSetClockAndDataLine( uint8_t portAddress, uint8_t value, uint8_t mask)
{
	uint8_t set = ((REGISTER_READ_FROM_8BIT_REGISTER(portAddress) & (~mask)) | (value & mask ));
	if (set && mask != mask && (REGISTER_WRITE_INTO_8BIT_REGISTER_AND_READBACK(portAddress, set)))
	{
		return false;
	}
	_delay_us(0);
	return true;
}

apiCommandResult apfelClearDataInput(uint8_t portAddress, uint8_t ss, uint8_t pinCLK, uint8_t pinDOUT, uint8_t pinSS)
{
	if (pinCLK > 7 || pinDOUT > 7 || pinSS > 7)
	{
		return apiCommandResult_FAILURE_QUIET;
	}

	uint8_t mask = (1 << pinCLK | 1 << pinDOUT | 1 << pinSS);

	static const uint8_t nBits = APFEL_N_COMMAND_BITS + APFEL_N_VALUE_BITS + APFEL_N_CHIP_ID_BITS;

    ss  = ss?1:0;

    for (int i = 0; i < nBits; i++)
    {
    	//clock Low + data low
    	if ( false == apfelSetClockAndDataLine( portAddress, ((uint8_t)(0 << pinCLK | 0   << pinDOUT | ss << pinSS)), mask)) { return apiCommandResult_FAILURE_QUIET; }

    	//clock Low + data low
    	if ( false == apfelSetClockAndDataLine( portAddress, ((uint8_t)(0 << pinCLK | 0   << pinDOUT | ss << pinSS)), mask)) { return apiCommandResult_FAILURE_QUIET; }
    	_delay_us(0);

    	apfelWriteClockSequence(1, portAddress, ss, pinCLK, pinDOUT, pinSS);
    }

    // tuning
    for (int i = 0; i < 10; i++)
    {
    	//clock Low + data low
    	if ( false == apfelSetClockAndDataLine( portAddress, ((uint8_t)(0 << pinCLK | 0   << pinDOUT | ss << pinSS)), mask)) { return apiCommandResult_FAILURE_QUIET; }
    	_delay_us(0);
    }

	return apiCommandResult_SUCCESS_QUIET;
}

//writeBitSequence #bits #data #endianess (0: little, 1:big)
apiCommandResult apfelWriteBitSequence(uint8_t num, uint32_t data, uint8_t portAddress, uint8_t ss, uint8_t pinCLK, uint8_t pinDOUT, uint8_t pinSS)
{
	if (pinCLK > 7 || pinDOUT > 7 || pinSS > 7)
	{
		return apiCommandResult_FAILURE_QUIET;
	}

	while (num)
	{
		if ( apiCommandResult_FAILURE > apfelWriteBit( (data >> ( num -1 ) ) & 0x1 , portAddress, ss, pinCLK, pinDOUT, pinSS))
		{
			return apiCommandResult_FAILURE_QUIET;
		}
		num--;
	}

	return apiCommandResult_SUCCESS_QUIET;
}

//mandatory header to all command sequences
apiCommandResult apfelStartStreamHeader(uint8_t portAddress, uint8_t ss, uint8_t pinCLK, uint8_t pinDOUT, uint8_t pinSS)
{
	if (pinCLK > 7 || pinDOUT > 7 || pinSS > 7)
	{
		return apiCommandResult_FAILURE_QUIET;
	}

	uint8_t mask = (1 << pinCLK | 1 << pinDOUT | 1 << pinSS);

	ss  = ss?1:0;

	if ( apiCommandResult_FAILURE > apfelClearDataInput(portAddress, ss, pinCLK, pinDOUT, pinSS)){ return apiCommandResult_FAILURE_QUIET; }

	//clock Low + data low
	if ( false == apfelSetClockAndDataLine( portAddress, ((uint8_t)(0 << pinCLK | 0 << pinDOUT | ss << pinSS)), mask)) { return apiCommandResult_FAILURE_QUIET; }
	//# clock Low  + data high
	if ( false == apfelSetClockAndDataLine( portAddress, ((uint8_t)(0 << pinCLK | 1 << pinDOUT | ss << pinSS)), mask)) { return apiCommandResult_FAILURE_QUIET; }
	//# clock High + data high
	if ( false == apfelSetClockAndDataLine( portAddress, ((uint8_t)(1 << pinCLK | 1 << pinDOUT | ss << pinSS)), mask)) { return apiCommandResult_FAILURE_QUIET; }
	//# clock low + data high
	if ( false == apfelSetClockAndDataLine( portAddress, ((uint8_t)(0 << pinCLK | 1 << pinDOUT | ss << pinSS)), mask)) { return apiCommandResult_FAILURE_QUIET; }
	//# clock low  + data low
	if ( false == apfelSetClockAndDataLine( portAddress, ((uint8_t)(0 << pinCLK | 0 << pinDOUT | ss << pinSS)), mask)) { return apiCommandResult_FAILURE_QUIET; }

	return apiCommandResult_SUCCESS_QUIET;
}

/*
 * High Level commands
 */
apiCommandResult apfelCommandSet(uint16_t command, uint16_t value, uint8_t chipID, uint8_t portAddress, uint8_t ss, uint8_t pinCLK, uint8_t pinDOUT, uint8_t pinSS)
{
	if (pinCLK > 7 || pinDOUT > 7 || pinSS > 7)
	{
		return apiCommandResult_FAILURE_QUIET;
	}

	apfelStartStreamHeader(portAddress, ss, pinCLK, pinDOUT, pinSS);
	//# command + ...
	apfelWriteBitSequence( APFEL_N_COMMAND_BITS, command, portAddress, ss, pinCLK, pinDOUT, pinSS);
	// value
	apfelWriteBitSequence( APFEL_N_VALUE_BITS, value, portAddress, ss, pinCLK, pinDOUT, pinSS);
	//# chipId
	apfelWriteBitSequence( APFEL_N_CHIP_ID_BITS, chipID, portAddress, ss, pinCLK, pinDOUT, pinSS);

	return apiCommandResult_SUCCESS_QUIET;
}


//#setDac dacNr[1..4] value[ 0 ... 3FF ] chipID[0 ... FF]
apiCommandResult apfelSetDac(uint16_t value, uint8_t dacNr, uint8_t chipID, uint8_t portAddress, uint8_t ss, uint8_t pinCLK, uint8_t pinDOUT, uint8_t pinSS)
{
	if ( 1 > dacNr || dacNr > 4 )
	{
		return apiCommandResult_FAILURE_QUIET;
	}

	if (value > (0x1 << APFEL_N_VALUE_BITS) -1 )
	{
		return apiCommandResult_FAILURE_QUIET;
	}

    apfelCommandSet(APFEL_COMMAND_SET_DAC + dacNr, value, chipID, portAddress, ss, pinCLK, pinDOUT, pinSS);

	//#3 intermediate clock cycles equiv. 3 writeDataLow
	apfelWriteClockSequence(3, portAddress, ss, pinCLK, pinDOUT, pinSS);

	return apiCommandResult_SUCCESS_QUIET;
}

//#readDac dacNr[1..4] chipID[0 ... FF]
apiCommandResult apfelReadDac(uint32_t* dacValue, uint8_t dacNr, uint8_t chipID,  uint8_t portAddress, uint8_t ss, uint8_t pinCLK, uint8_t pinDOUT, uint8_t pinSS, uint8_t pinDIN)
{
	if ( NULL == dacValue )
	{
		return apiCommandResult_FAILURE_QUIET;
	}

	if (pinDIN > 7)
	{
		return apiCommandResult_FAILURE_QUIET;
	}

	if ( 1 > dacNr || dacNr > 4 )
	{
		return apiCommandResult_FAILURE_QUIET;
	}

    apfelCommandSet(APFEL_COMMAND_READ_DAC + dacNr, 0, chipID, portAddress, ss, pinCLK, pinDOUT, pinSS);

	apfelReadBitSequence( 2 + APFEL_N_VALUE_BITS + 3, dacValue, portAddress, ss, pinDIN, pinCLK, pinSS);
    //cut out real dacValue
	*dacValue = (*dacValue & ((0x1 << APFEL_N_VALUE_BITS) -1)) >> 3;

	return apiCommandResult_SUCCESS_QUIET;
}

//#autoCalibration chipId[0 ... FF]
apiCommandResult apfelAutoCalibration(uint8_t chipID, uint8_t portAddress, uint8_t ss, uint8_t pinCLK, uint8_t pinDOUT, uint8_t pinSS)
{

    apfelCommandSet(APFEL_COMMAND_AUTOCALIBRATION, 0, chipID, portAddress, ss, pinCLK, pinDOUT, pinSS);

    //#calibration sequence clocks 4 * 10bit DAC
    apfelWriteClockSequence( (1 << APFEL_N_VALUE_BITS) << 2, portAddress, ss, pinCLK, pinDOUT, pinSS);

	return apiCommandResult_SUCCESS_QUIET;
}

//#testPulseSequence pulseHeightPattern[0 ... 1F](<<1/5) chipId[0 ... FF]
apiCommandResult apfelTestPulseSequence(uint16_t pulseHeightPattern, uint8_t chipID, uint8_t portAddress, uint8_t ss, uint8_t pinCLK, uint8_t pinDOUT, uint8_t pinSS)
{
	if (	(((pulseHeightPattern >> 1) & ((0x1 << 5)-1)) > APFEL_TEST_PULSE_HEIGHT_PATTERN_MAX) ||
		    (((pulseHeightPattern >> 5) & ((0x1 << 5)-1)) > APFEL_TEST_PULSE_HEIGHT_PATTERN_MAX))
	{
		return apiCommandResult_FAILURE_QUIET;
	}

    apfelCommandSet(APFEL_COMMAND_TESTPULSE, pulseHeightPattern, chipID, portAddress, ss, pinCLK, pinDOUT, pinSS);

    apfelWriteClockSequence( 1, portAddress, ss, pinCLK, pinDOUT, pinSS);

	return apiCommandResult_SUCCESS_QUIET;
}

//#testPulse pulseHeight[0 ... 1F] channel[1 .. 2 ] chipId[0 ... FF]
apiCommandResult apfelTestPulse(uint8_t pulseHeight, uint8_t channel, uint8_t chipID, uint8_t portAddress, uint8_t ss, uint8_t pinCLK, uint8_t pinDOUT, uint8_t pinSS)
{
	if (pulseHeight > APFEL_TEST_PULSE_HEIGHT_PATTERN_MAX)
	{
		return apiCommandResult_FAILURE_QUIET;
	}

	if (1 > channel || channel > 2)
	{
		return apiCommandResult_FAILURE_QUIET;
	}

	apfelTestPulseSequence( pulseHeight << (1 + (channel == 1)?0:5), chipID, portAddress, ss, pinCLK, pinDOUT, pinSS);
	apfelTestPulseSequence( 0                                      , chipID, portAddress, ss, pinCLK, pinDOUT, pinSS);

	return apiCommandResult_SUCCESS_QUIET;
}

//#setAmplitude channelId[1 ... 2] chipId[0 ... FF]
apiCommandResult apfelSetAmplitude(uint8_t channelId, uint8_t chipID, uint8_t portAddress, uint8_t ss, uint8_t pinCLK, uint8_t pinDOUT, uint8_t pinSS)
{
	if (1 > channelId || channelId > 2)
	{
		return apiCommandResult_FAILURE_QUIET;
	}

    return apfelCommandSet(APFEL_COMMAND_SET_AMPLITUDE + 2 - channelId, 0, chipID, portAddress, ss, pinCLK, pinDOUT, pinSS);
}

//#resetAmplitude channelId[1 ... 2] chipId[0 ... FF]
apiCommandResult apfelResetAmplitude(uint8_t channelId, uint8_t chipID, uint8_t portAddress, uint8_t ss, uint8_t pinCLK, uint8_t pinDOUT, uint8_t pinSS)
{
	if (1 > channelId || channelId > 2)
	{
		return apiCommandResult_FAILURE_QUIET;
	}

    return apfelCommandSet(APFEL_COMMAND_RESET_AMPLITUDE + ((channelId -1)*2), 0, chipID, portAddress, ss, pinCLK, pinDOUT, pinSS);
}
#endif
