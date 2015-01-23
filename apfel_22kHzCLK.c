/*
 * apfel.c
 *
 *  Created on: 12.06.2013
 *      Author: Florian Brabetz, GSI, f.brabetz@gsi.de
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

/*eclipse specific setting, not used during build process*/
#ifndef __AVR_AT90CAN128__
#include <avr/iocan128.h>
#endif


void apfel_Inline()
{
#define APFEL_US_TO_DELAY_DEFAULT 0

	/* definitions */
#define APFEL_N_CommandBits  4
#define APFEL_N_ValueBits  10
#define APFEL_ValueBits_MASK 0x3FF
#define APFEL_N_ChipIdBits  8
#define APFEL_LITTLE_ENDIAN  0
#define APFEL_BIG_ENDIAN  1
#define APFEL_DEFAULT_ENDIANNESS APFEL_BIG_ENDIAN

#define APFEL_COMMAND_SetDac  0x0
#define APFEL_COMMAND_ReadDac  0x4
#define APFEL_COMMAND_AutoCalibration  0xC
#define APFEL_COMMAND_TestPulse  0x9
#define APFEL_COMMAND_SetAmplitude  0xE
#define APFEL_COMMAND_ResetAmplitude  0xB

#define APFEL_READ_N_HEADER_BITS 2
#define APFEL_READ_HEADER 0x2
#define APFEL_READ_HEADER_MASK 0x3
#define APFEL_READ_N_TRAILING_BITS 3
#define APFEL_READ_TRAILER 0x7
#define APFEL_READ_TRAILER_MASK 0x7

#define APFEL_READ_CHECK_MASK  ((APFEL_READ_HEADER_MASK << (APFEL_N_ValueBits + APFEL_READ_N_TRAILING_BITS)) | APFEL_READ_TRAILER_MASK)
#define APFEL_READ_CHECK_VALUE ((APFEL_READ_HEADER      << (APFEL_N_ValueBits + APFEL_READ_N_TRAILING_BITS)) | APFEL_READ_TRAILER)

#define APFEL_N_PINS 4
#define APFEL_PIN_DIN1 	PINA0
#define APFEL_PIN_DOUT1 PINA1
#define APFEL_PIN_CLK1 	PINA2
#define APFEL_PIN_SS1 	PINA3

#define APFEL_PIN_DIN2 	PINA4
#define APFEL_PIN_DOUT2 PINA5
#define APFEL_PIN_CLK2 	PINA6
#define APFEL_PIN_SS2 	PINA7

#define APFEL_PIN_MASK1 (0xFF & (1 << APFEL_PIN_CLK1 | 1 << APFEL_PIN_DOUT1 | 1 << APFEL_PIN_SS1 ))
#define APFEL_PIN_MASK2 (0xFF & (1 << APFEL_PIN_CLK2 | 1 << APFEL_PIN_DOUT2 | 1 << APFEL_PIN_SS2 ))
#define APFEL_PIN_MASK_DIN (0xFF & (1 << APFEL_PIN_DIN1 | 1 << APFEL_PIN_DIN2 ))

#define APFEL_writePort_CalcPattern(val,A,pinSetIndex)\
	    0xFF & (((0 == pinSetIndex-1) ? \
		(~(APFEL_PIN_MASK_DIN)) &  (( PIN##A & APFEL_PIN_MASK2) | (val & APFEL_PIN_MASK1)): \
		(~(APFEL_PIN_MASK_DIN)) &  (( PIN##A & APFEL_PIN_MASK1) | (val & APFEL_PIN_MASK2))))
#define APFEL_writePort(val,A,pinSetIndex) \
		    {PORT##A = (APFEL_writePort_CalcPattern(val,A,pinSetIndex));\
		               _delay_us(APFEL_US_TO_DELAY_DEFAULT);}
#define APFEL_readPort(A,pinSetIndex) ((PIN##A >> (APFEL_PIN_DIN##pinSetIndex )) & 0x1)

	static bool apfelOsziTestMode = 0;

	/* functions */
	inline int8_t apfelWritePort(uint8_t val, char port, uint8_t pinSetIndex, uint8_t sideSelection)
	{
		if (0 == pinSetIndex || pinSetIndex > 2)
		{
			return -1;
		}

		// add sideSelection bit
		if (sideSelection)
		{
			val = (0xFF & (val | 1 << ((1==pinSetIndex)?APFEL_PIN_SS1:APFEL_PIN_SS2)));
		}

		switch (port)
		{
			case 'A':
				APFEL_writePort(val, A, pinSetIndex);
				break;
			case 'B':
				APFEL_writePort(val, B, pinSetIndex);
				break;
			case 'C':
				APFEL_writePort(val, C, pinSetIndex);
				break;
			case 'D':
				APFEL_writePort(val, D, pinSetIndex);
				break;
			case 'E':
				APFEL_writePort(val, E, pinSetIndex);
				break;
			case 'F':
				APFEL_writePort(val, F, pinSetIndex);
				break;
			case 'G':
				APFEL_writePort(val, G, pinSetIndex);
				break;
			default:
				return -1;
				break;
		}
		return 0;
	}

	inline int8_t apfelReadPort(char port, uint8_t pinSetIndex, uint8_t sideSelection)
	{
		if (0 == pinSetIndex || pinSetIndex > 2)
		{
			return -1;
		}
		switch (port)
		{
			case 'A':
				return (1 == pinSetIndex) ? (APFEL_readPort(A, 1)) : (APFEL_readPort(A, 2));
				break;
			case 'B':
				return (1 == pinSetIndex) ? (APFEL_readPort(B, 1)) : (APFEL_readPort(B, 2));
				break;
			case 'C':
				return (1 == pinSetIndex) ? (APFEL_readPort(C, 1)) : (APFEL_readPort(C, 2));
				break;
			case 'D':
				return (1 == pinSetIndex) ? (APFEL_readPort(D, 1)) : (APFEL_readPort(D, 2));
				break;
			case 'E':
				return (1 == pinSetIndex) ? (APFEL_readPort(E, 1)) : (APFEL_readPort(E, 2));
				break;
			case 'F':
				return (1 == pinSetIndex) ? (APFEL_readPort(F, 1)) : (APFEL_readPort(F, 2));
				break;
			case 'G':
				return (1 == pinSetIndex) ? (APFEL_readPort(G, 1)) : (APFEL_readPort(G, 2));
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
	static char const apfelHigh[][5] =
	{
	{ (0 << APFEL_PIN_DOUT1 | 0 << APFEL_PIN_CLK1), (1 << APFEL_PIN_DOUT1 | 0 << APFEL_PIN_CLK1), (1 << APFEL_PIN_DOUT1
			| 1 << APFEL_PIN_CLK1), (0 << APFEL_PIN_DOUT1 | 1 << APFEL_PIN_CLK1), (0 << APFEL_PIN_DOUT1
			| 0 << APFEL_PIN_CLK1) },

	{ (0 << APFEL_PIN_DOUT2 | 0 << APFEL_PIN_CLK2), (1 << APFEL_PIN_DOUT2 | 0 << APFEL_PIN_CLK2), (1 << APFEL_PIN_DOUT2
			| 1 << APFEL_PIN_CLK2), (0 << APFEL_PIN_DOUT2 | 1 << APFEL_PIN_CLK2), (0 << APFEL_PIN_DOUT2
			| 0 << APFEL_PIN_CLK2) } };
	/* 1.data low + clock Low */
	/* 2.data low + clock low */
	/* 3.data low + clock High*/
	/* 4.data low + clock High*/
	/* 5.data low + clock low */
	static char const apfelLow[][5] =
	{
	{ (0 << APFEL_PIN_DOUT1 | 0 << APFEL_PIN_CLK1), (0 << APFEL_PIN_DOUT1 | 0 << APFEL_PIN_CLK1), (0 << APFEL_PIN_DOUT1
			| 1 << APFEL_PIN_CLK1), (0 << APFEL_PIN_DOUT1 | 1 << APFEL_PIN_CLK1), (0 << APFEL_PIN_DOUT1
			| 0 << APFEL_PIN_CLK1) },

	{ (0 << APFEL_PIN_DOUT2 | 0 << APFEL_PIN_CLK2), (0 << APFEL_PIN_DOUT2 | 0 << APFEL_PIN_CLK2), (0 << APFEL_PIN_DOUT2
			| 1 << APFEL_PIN_CLK2), (0 << APFEL_PIN_DOUT2 | 1 << APFEL_PIN_CLK2), (0 << APFEL_PIN_DOUT2
			| 0 << APFEL_PIN_CLK2) } };

	inline int8_t apfelWriteBit_Inline(uint8_t bit, char port, uint8_t pinSetIndex, uint8_t sideSelection)
	{
		if (0 == pinSetIndex || pinSetIndex > 2)
		{
			return -1;
		}
		sideSelection=(sideSelection)?1:0;

		if (0 == bit)
		{
			apfelWritePort(apfelLow [pinSetIndex - 1][0], port, pinSetIndex, sideSelection );
			apfelWritePort(apfelLow [pinSetIndex - 1][1], port, pinSetIndex, sideSelection );
			apfelWritePort(apfelLow [pinSetIndex - 1][2], port, pinSetIndex, sideSelection );
			apfelWritePort(apfelLow [pinSetIndex - 1][3], port, pinSetIndex, sideSelection );
			apfelWritePort(apfelLow [pinSetIndex - 1][4], port, pinSetIndex, sideSelection );
		}
		else
		{
			apfelWritePort(apfelHigh[pinSetIndex - 1][0], port, pinSetIndex, sideSelection );
			apfelWritePort(apfelHigh[pinSetIndex - 1][1], port, pinSetIndex, sideSelection );
			apfelWritePort(apfelHigh[pinSetIndex - 1][2], port, pinSetIndex, sideSelection );
			apfelWritePort(apfelHigh[pinSetIndex - 1][3], port, pinSetIndex, sideSelection );
			apfelWritePort(apfelHigh[pinSetIndex - 1][4], port, pinSetIndex, sideSelection );
		}
		return 0;
	}

	inline void apfelInit_Inline(void)
	{
		/* init IO ports */
		//configure I/O port A
		DDRA = APFEL_PIN_MASK1 | APFEL_PIN_MASK2;
		DDRC = APFEL_PIN_MASK1 | APFEL_PIN_MASK2;
		DDRF = APFEL_PIN_MASK1 | APFEL_PIN_MASK2;

		PORTA = 0;
		PORTC = 0;
		PORTF = 0;
	}

	inline uint16_t apfelReadBitSequence_Inline(char port, uint8_t pinSetIndex, uint8_t sideSelection, uint8_t nBits)
	{
		static uint16_t value = 0;
		static uint8_t bitIndex = 0;
		value = 0;
		bitIndex = 0;
		uint8_t pinClk;
		uint8_t pinDout;

		switch (pinSetIndex)
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
		apfelWritePort(((0 << pinClk) | (0 << pinDout)), port, pinSetIndex, sideSelection);

		// initial bit read differently !!! TNX^6 Peter Wieczorek ;-)
		// since apfel needs first falling clock edge to activate the output pad
		// therefore first cycle then read data.
		if (nBits)
		{
			nBits--;
		}
		// clock high
		apfelWritePort((1 << pinClk), port, pinSetIndex, sideSelection);
		// clock low
		apfelWritePort((0 << pinClk), port, pinSetIndex, sideSelection);
		// read data in
		value |= (apfelReadPort(port, pinSetIndex, sideSelection) << bitIndex);

		while (bitIndex < nBits)
		{
			bitIndex++;
			// clock high
			apfelWritePort((1 << pinClk), port, pinSetIndex, sideSelection);
			// read data in
			value |= (apfelReadPort(port, pinSetIndex, sideSelection) << bitIndex);
			// clock low
			apfelWritePort((0 << pinClk), port, pinSetIndex, sideSelection);
		}
		return value;
	}

	inline void apfelWriteClockSequence_Inline(char port, uint8_t pinSetIndex, uint8_t sideSelection, uint16_t nClk)
	{
		uint16_t a = nClk;
		while (a > 0)
		{
			a--;
			apfelWriteBit_Inline(0, port, pinSetIndex, sideSelection);
		}
	}

	/*  apfelClearDataInput
	 sends enough empty clock cycles to clear input buffer
	 after each byte add 3 0 writes */
	inline void apfelClearDataInput_Inline(char port, uint8_t pinSetIndex, uint8_t sideSelection)
	{
		uint8_t a = (APFEL_N_CommandBits + APFEL_N_ValueBits + APFEL_N_ChipIdBits);
		uint8_t i = 0;
		switch (pinSetIndex)
		{
			case 1:
				while (i < a)
				{
					i++;
					if (0 == i % 8)
					{
						apfelWritePort((0 << APFEL_PIN_DOUT1 | 0 << APFEL_PIN_CLK1), port, pinSetIndex, sideSelection);
						apfelWritePort((0 << APFEL_PIN_DOUT1 | 0 << APFEL_PIN_CLK1), port, pinSetIndex, sideSelection);
					}
					apfelWriteClockSequence_Inline(port, pinSetIndex, sideSelection, 1);
				}
				i = 10;
				while (i > 0)
				{
					i--;
					apfelWritePort((0 << APFEL_PIN_DOUT1 | 0 << APFEL_PIN_CLK1), port, pinSetIndex, sideSelection);
				}
				break;
			case 2:
				while (i < a)
				{
					i++;
					if (0 == i % 8)
					{
						apfelWritePort((0 << APFEL_PIN_DOUT2 | 0 << APFEL_PIN_CLK2), port, pinSetIndex, sideSelection);
						apfelWritePort((0 << APFEL_PIN_DOUT2 | 0 << APFEL_PIN_CLK2), port, pinSetIndex, sideSelection);
					}
					apfelWriteClockSequence_Inline(port, pinSetIndex, sideSelection, 1);
				}
				i = 10;
				while (i > 0)
				{
					i--;
					apfelWritePort((0 << APFEL_PIN_DOUT2 | 0 << APFEL_PIN_CLK2), port, pinSetIndex, sideSelection);
				}
				break;
			default:
				break;
		}
	}

	//mandatory header to all command sequences
	inline void apfelStartStreamHeader_Inline(char port, uint8_t pinSetIndex, uint8_t sideSelection)
	{
		apfelClearDataInput_Inline(port, pinSetIndex, sideSelection);

		//  1. clock Low
		//  2. clock Low  + data high
		//  3. clock High + data high
		//  4. clock low  + data high
		//  5. clock low  + data low

		uint8_t pinDout = 0xFF;
		uint8_t pinClk = 0xFF;
		switch (pinSetIndex)
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
		apfelWritePort((0 << pinDout | 0 << pinClk), port, pinSetIndex, sideSelection);
		/* header */
		apfelWritePort((0 << pinDout | 0 << pinClk), port, pinSetIndex, sideSelection);
		apfelWritePort((1 << pinDout | 0 << pinClk), port, pinSetIndex, sideSelection);
		apfelWritePort((1 << pinDout | 1 << pinClk), port, pinSetIndex, sideSelection);
		apfelWritePort((1 << pinDout | 0 << pinClk), port, pinSetIndex, sideSelection);
		apfelWritePort((0 << pinDout | 0 << pinClk), port, pinSetIndex, sideSelection);
	}

	/* writeBitSequence #bits #data #endianess (0: little, 1:big)*/
	inline void apfelWriteBitSequence_Inline(char port, uint8_t pinSetIndex, uint8_t sideSelection, int8_t nBits,
			uint16_t data, uint8_t endianness)
	{
		int8_t bitPos = 0;

		switch (endianness)
		{
			case APFEL_LITTLE_ENDIAN:
				while (bitPos < nBits)
				{
					apfelWriteBit_Inline(((uint8_t) ((data >> bitPos) & 0x1)), port, pinSetIndex, sideSelection);
					bitPos++;
				}
				break;
			case APFEL_BIG_ENDIAN:
				bitPos = nBits - 1;
				while (bitPos >= 0)
				{
					apfelWriteBit_Inline(((uint8_t) ((data >> bitPos) & 0x1)), port, pinSetIndex, sideSelection);
					bitPos--;
				}
				break;
			default:
				CommunicationError(ERRA, -1, 1, PSTR("wrong endianness: %i (%i,%i)"), endianness, APFEL_LITTLE_ENDIAN,
						APFEL_BIG_ENDIAN);
				return;
				break;
		}
	}

	/* #setDac value[ 0 ... 3FF ] dacNr[1..4] chipID[0 ... FF]	*/

	inline void apfelSetDac_Inline(char port, uint8_t pinSetIndex, uint8_t sideSelection, uint16_t value, uint8_t dacNr,
			uint16_t chipID)
	{
		apfelStartStreamHeader_Inline(port, pinSetIndex, sideSelection);
		;
		// dacNr
		apfelWriteBitSequence_Inline(port, pinSetIndex, sideSelection, APFEL_N_CommandBits,
				APFEL_COMMAND_SetDac + dacNr, APFEL_DEFAULT_ENDIANNESS);
		// value
		apfelWriteBitSequence_Inline(port, pinSetIndex, sideSelection, APFEL_N_ValueBits, value,
				APFEL_DEFAULT_ENDIANNESS);
		// chipId
		apfelWriteBitSequence_Inline(port, pinSetIndex, sideSelection, APFEL_N_ChipIdBits, chipID,
				APFEL_DEFAULT_ENDIANNESS);
		//3 intermediate clock cycles equiv. 3 writeDataLow
		apfelWriteClockSequence_Inline(port, pinSetIndex, sideSelection, 3);
	}

	/* #readDac dacNr[1..4] chipID[0 ... FF] */
	inline int16_t apfelReadDac_Inline(char port, uint8_t pinSetIndex, uint8_t sideSelection, uint8_t dacNr,
			uint16_t chipId, uint8_t quiet)
	{
		uint16_t value = 0;
		apfelStartStreamHeader_Inline(port, pinSetIndex, sideSelection);
		// command + dacNr
		apfelWriteBitSequence_Inline(port, pinSetIndex, sideSelection, APFEL_N_CommandBits,
				APFEL_COMMAND_ReadDac + dacNr, APFEL_DEFAULT_ENDIANNESS);
		// dummy value
		apfelWriteBitSequence_Inline(port, pinSetIndex, sideSelection, APFEL_N_ValueBits, 0, APFEL_DEFAULT_ENDIANNESS);
		// chipId
		apfelWriteBitSequence_Inline(port, pinSetIndex, sideSelection, APFEL_N_ChipIdBits, chipId,
				APFEL_DEFAULT_ENDIANNESS);
		// read 15 bits
		value = apfelReadBitSequence_Inline(port, pinSetIndex, sideSelection,
				(APFEL_READ_N_HEADER_BITS + APFEL_N_ValueBits + APFEL_READ_N_TRAILING_BITS));

		if (0 > value)
		{
			if (!quiet)
				CommunicationError_p(ERRA, -1, 1, PSTR("port/pinSet/side/dac/chipId:'%c/%i/%i/%i/%x': readDac failed"),
						port, pinSetIndex, sideSelection, dacNr, chipId);
			/* Error */
			return -1;
		}
		// check validity for correct header (10) and trailing bits (111)
		if ( APFEL_READ_CHECK_VALUE != (value & APFEL_READ_CHECK_MASK))
		{
			if (!quiet)
				CommunicationError_p(ERRA, -1, 1,
						PSTR("port/pinSet/side/dac/chipId:'%c/%i/%i/%i/%x': validity check failed"), port, pinSetIndex,
						sideSelection, dacNr, chipId);
			/* Error */
			return -10;
		}

		value = (value >> APFEL_READ_N_TRAILING_BITS) & APFEL_ValueBits_MASK;

		createExtendedSubCommandReceiveResponseHeader(ptr_uartStruct, commandKeyNumber_APFEL,
				apfelApiCommandKeyNumber_DAC, apfelApiCommandKeywords);
		snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%sport/pinSet/side/dac/chipId:'%c/%x/%x/%x/%x"),
				uart_message_string, port, pinSetIndex, sideSelection, dacNr, chipId);
		apiShowValue(uart_message_string, &value, apiVarType_UINT16);
		apfelApiSubCommandsFooter(apiCommandResult_SUCCESS_WITH_OUTPUT);

		return 0;
	}

	/* #autoCalibration chipId[0 ... FF] */
	inline void apfelAutoCalibration_Inline(char port, uint8_t pinSetIndex, uint8_t sideSelection, uint8_t chipId)
	{
		apfelStartStreamHeader_Inline(port, pinSetIndex, sideSelection);
		// command
		apfelWriteBitSequence_Inline(port, pinSetIndex, sideSelection, APFEL_N_CommandBits,
				APFEL_COMMAND_AutoCalibration, APFEL_DEFAULT_ENDIANNESS);
		// dummy value
		apfelWriteBitSequence_Inline(port, pinSetIndex, sideSelection, APFEL_N_ValueBits, 0, APFEL_DEFAULT_ENDIANNESS);
		// chipId
		apfelWriteBitSequence_Inline(port, pinSetIndex, sideSelection, APFEL_N_ChipIdBits, chipId,
				APFEL_DEFAULT_ENDIANNESS);

		// #calibration sequence clocks 4 * 10bit DAC
		apfelWriteClockSequence_Inline(port, pinSetIndex, sideSelection, (0x1 << APFEL_N_ValueBits) - 1);
	}

	/* #testPulseSequence pulseHeightPattern[0 ... 1F] chipId[0 ... FF] */
	inline void apfelTestPulseSequence_Inline(char port, uint8_t pinSetIndex, uint8_t sideSelection,
			uint8_t pulseHeightPattern, uint8_t chipId)
	{
		apfelStartStreamHeader_Inline(port, pinSetIndex, sideSelection);
		// command
		apfelWriteBitSequence_Inline(port, pinSetIndex, sideSelection, APFEL_N_CommandBits, APFEL_COMMAND_TestPulse,
				APFEL_DEFAULT_ENDIANNESS);
		// pulse height
		apfelWriteBitSequence_Inline(port, pinSetIndex, sideSelection, APFEL_N_ValueBits, pulseHeightPattern,
				APFEL_DEFAULT_ENDIANNESS);
		// chipId
		apfelWriteBitSequence_Inline(port, pinSetIndex, sideSelection, APFEL_N_ChipIdBits, chipId,
				APFEL_DEFAULT_ENDIANNESS);

		apfelWriteClockSequence_Inline(port, pinSetIndex, sideSelection, 0x1);
	}

	/* #testPulse pulseHeight[0 ... 1F] channel[1 .. 2 ] chipId[0 ... FF] */
	inline void apfelTestPulse_Inline(char port, uint8_t pinSetIndex, uint8_t sideSelection, uint8_t pulseHeightPattern,
			uint8_t channel, uint8_t chipId)
	{

		apfelTestPulseSequence_Inline(port, pinSetIndex, sideSelection,
				(pulseHeightPattern << 0x1) << (5 * (channel - 1)), chipId);
		apfelTestPulseSequence_Inline(port, pinSetIndex, sideSelection, 0, chipId);
	}

	/*#setAmplitude channelId[1 ... 2] chipId[0 ... FF]*/
	inline void apfelSetAmplitude_Inline(char port, uint8_t pinSetIndex, uint8_t sideSelection, uint8_t channel,
			uint8_t chipId)
	{
		apfelStartStreamHeader_Inline(port, pinSetIndex, sideSelection);
		// command for channel 1/2
		apfelWriteBitSequence_Inline(port, pinSetIndex, sideSelection, APFEL_N_CommandBits,
				((APFEL_COMMAND_SetAmplitude + 1) - (channel - 1)), APFEL_DEFAULT_ENDIANNESS);
		// dummy
		apfelWriteBitSequence_Inline(port, pinSetIndex, sideSelection, APFEL_N_ValueBits, 0, APFEL_DEFAULT_ENDIANNESS);
		// chipId
		apfelWriteBitSequence_Inline(port, pinSetIndex, sideSelection, APFEL_N_ChipIdBits, chipId,
				APFEL_DEFAULT_ENDIANNESS);
	}

	/*#resetAmplitude channelId[1 ... 2] chipId[0 ... FF]*/
	inline void apfelResetAmplitude_Inline(char port, uint8_t pinSetIndex, uint8_t sideSelection, uint8_t channel,
			uint8_t chipId)
	{
		apfelStartStreamHeader_Inline(port, pinSetIndex, sideSelection);
		// command for channel 1/2
		apfelWriteBitSequence_Inline(port, pinSetIndex, sideSelection, APFEL_N_CommandBits,
				((APFEL_COMMAND_ResetAmplitude + 1) - (channel - 1) * 2), APFEL_DEFAULT_ENDIANNESS);
		// dummy
		apfelWriteBitSequence_Inline(port, pinSetIndex, sideSelection, APFEL_N_ValueBits, 0, APFEL_DEFAULT_ENDIANNESS);
		// chipId
		apfelWriteBitSequence_Inline(port, pinSetIndex, sideSelection, APFEL_N_ChipIdBits, chipId,
				APFEL_DEFAULT_ENDIANNESS);
	}

	/*#list Ids by checking result for dacRead */
	inline void apfelListIds_Inline(char port, uint8_t pinSetIndex, uint8_t sideSelection, uint8_t all)
	{
		uint8_t chipId;
		uint16_t value;
		uint8_t ctr;
		for (chipId = 0; chipId < 0xFF; chipId++)
		{
			value = apfelReadDac_Inline(port, pinSetIndex, sideSelection, 1, chipId, 1);
			if (0 <= value)
			{
				ctr++;
			}
			if (0 <= value || all)
			{
				createExtendedSubCommandReceiveResponseHeader(ptr_uartStruct, commandKeyNumber_APFEL,
						apfelApiCommandKeyNumber_LIST, apfelApiCommandKeywords);
				snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%sport/pinSet/side %c/%x/%x %x%s"),
						uart_message_string, port, pinSetIndex, sideSelection, chipId);
				if (all)
					strncat_P(uart_message_string, (0 <= value) ? PSTR(" yes") : PSTR(" no"), BUFFER_SIZE - 1);
				UART0_Send_Message_String_p(NULL, 0);
			}
		}
	}

	inline void apfelApi_Inline(void)
	{
		uint32_t arg[7] =
		{ -1, -1, -1, -1, -1, -1, -1 };
		int8_t nArguments = ptr_uartStruct->number_of_arguments;
		int8_t nSubCommandsArguments = nArguments - 1;

		switch (nArguments)
		{
			case 0:
				return;
				break;
			default:
				for (uint8_t index=1; index <= min((uint8_t)(nArguments),sizeof(arg)/sizeof(uint32_t)); index++)
				{
					apiAssignParameterToValue(index, &(arg[index-1]),apiVarType_UINT32, 0, 0xFFFF);
				}
				break;
		}

		switch(arg[0])
		{
			case 0: /*apfelOscilloscopeTestFrameMode*/
			{
				PORTG =(1 << PG0 | 1 << PG1 | 0 << PG2) | (PORTG & 0x18);
				switch (nSubCommandsArguments /* arguments of argument */)
				{
					case 1:
						apfelOsziTestMode = arg[1];
					case 0:
						createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
						apiShowValue(uart_message_string, &apfelOsziTestMode, apiVarType_BOOL_OnOff);
						apfelApiSubCommandsFooter(apiCommandResult_SUCCESS_WITH_OUTPUT);
						break;
				}
			}
			break;
			case 1: /* write High*/
			{
				PORTG = (0 << PG0 | 1 << PG1 | 0 << PG2) | (PORTG & 0x18);
				switch (nSubCommandsArguments /* arguments of argument */)
				{
					case 0:
						apfelWriteBit_Inline(1, 'A', 1, 1);
						break;
				}
			}
			break;
			case 2: /* write Low*/
			{
				PORTG =(1 << PG0 | 0 << PG1 | 0 << PG2) | (PORTG & 0x18);
				switch (nSubCommandsArguments /* arguments of argument */)
				{
					case 0:
						apfelWriteBit_Inline(0, 'A', 1, 1);
						break;
				}
			}
			break;
			case 3: /* write High/Low*/
			{
				PORTG =(0 << PG0 | 0 << PG1 | 0 << PG2) | (PORTG & 0x18);
				switch (nSubCommandsArguments /* arguments of argument */)
				{
					case 0:
						apfelWriteBit_Inline(1, 'A', 1, 1);
						apfelWriteBit_Inline(0, 'A', 1, 1);
						break;
				}
			}
			break;
			case 4: /* read sequence 1x6bit*/
			{
				uint16_t value = 0;
				switch (nSubCommandsArguments /* arguments of argument */)
				{
					case 0:
					{
						value = apfelReadBitSequence_Inline('A', 1, 1, 6);
					}
					break;
					case 1:
					{
						value = apfelReadBitSequence_Inline('A', 1, 1, arg[1]);
					}
					break;
				}
				createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);
				apiShowValue(uart_message_string, &value, apiVarType_UINT16);
				apfelApiSubCommandsFooter(apiCommandResult_SUCCESS_WITH_OUTPUT);
			}
			break;
			case 5: /* write sequence */
			{
				switch (nSubCommandsArguments /* arguments of argument */)
				{
					case 0:
						apfelWriteClockSequence_Inline('A', 1, 1, 16);
						break;
					case 1:
						apfelWriteClockSequence_Inline('A', 1, 1, arg[1]);
						break;
				}
			}
			break;
			case 6:
			{
				switch (nSubCommandsArguments /* arguments of argument */)
				{
					case 0:
						apfelClearDataInput_Inline('A', 1, 1);
						break;
				}
			}
			break;
			case 7:
			{
				switch (nSubCommandsArguments /* arguments of argument */)
				{
					case 0:
						apfelStartStreamHeader_Inline('A', 1, 1);
						break;
				}
			}
			break;
			case 8:
			{
				switch (nSubCommandsArguments /* arguments of argument */)
				{
					case 0:
						apfelWriteBitSequence_Inline('A', 1, 1, 6, 0x12, APFEL_DEFAULT_ENDIANNESS);
						break;
					case 2:
						apfelWriteBitSequence_Inline('A', 1, 1, arg[1], arg[2], APFEL_DEFAULT_ENDIANNESS);
						break;
					case 3:
						apfelWriteBitSequence_Inline('A', 1, 1, arg[1], arg[2], arg[3]);
						break;
				}
			}
			break;
			case 9:
			{
				switch (nSubCommandsArguments /* arguments of argument */)
				{
					case 0:
						apfelSetDac_Inline('A', 1, 1, 0x2ee, 2, 30);
						break;
					case 1:
						apfelSetDac_Inline('A', 1, 1, arg[1], 1, 30);
						apfelSetDac_Inline('A', 1, 1, arg[1], 2, 30);
						apfelSetDac_Inline('A', 1, 1, arg[1], 3, 30);
						apfelSetDac_Inline('A', 1, 1, arg[1], 4, 30);
						break;
					case 2:
						apfelSetDac_Inline('A', 1, 1, arg[1], arg[2], 30);
						break;
					case 3:
						apfelSetDac_Inline('A', 1, 1, arg[1], arg[2], arg[3]);
						break;
					case 5:
						apfelSetDac_Inline('A', arg[4], arg[5], arg[1], arg[2], arg[3]);
						break;
					case 6:
						apfelSetDac_Inline(arg[6], arg[4], arg[5], arg[1], arg[2], arg[3]);
						break;
				}
			}
			break;
			case 0xA:
			{
				switch (nSubCommandsArguments /* arguments of argument */)
				{
					case 0:
						apfelReadDac_Inline('A', 1, 1, 2, 30, 0);
						break;
					case 1:
						apfelReadDac_Inline('A', 1, 1, arg[1], 30, 0);
						break;
					case 2:
						apfelReadDac_Inline('A', 1, 1, arg[1], arg[2], 0);
						break;
					case 4:
						apfelReadDac_Inline('A', arg[3], arg[4], arg[1], arg[2], 0);
						break;
					case 5:
						apfelReadDac_Inline(arg[5], arg[3], arg[4], arg[1], arg[2], 0);
						break;
				}
			}
			break;
			case 0xB:
			{
				switch (nSubCommandsArguments /* arguments of argument */)
				{
					case 0:
					{
#warning TODO implement readout of current watchdog timing from the WDT registers of ATMEL
						wdt_disable();
						int_fast16_t chipId = 0;
						for (chipId = 0; chipId < 0xFF; chipId++)
						{
							apfelAutoCalibration_Inline('A', 1, 0, chipId);
						}
						for (chipId = 0; chipId < 0xFF; chipId++)
						{
							apfelAutoCalibration_Inline('A', 1, 1, chipId);
						}
						wdt_enable(WDTO_2S);
					}
					break;
					case 1:
						apfelAutoCalibration_Inline('A', 1, 1, arg[1]);
						break;
					case 3:
						apfelAutoCalibration_Inline('A', arg[2], arg[3], arg[1]);
						break;
					case 4:
						apfelAutoCalibration_Inline(arg[4], arg[2], arg[3], arg[1]);
						break;
				}
			}
			break;
			case 0xC:
			{
				switch (nSubCommandsArguments /* arguments of argument */)
				{
					case 0:
						apfelTestPulseSequence_Inline('A', 1, 1, 0x10, 30);
						break;
					case 1:
						apfelTestPulseSequence_Inline('A', 1, 1, arg[1], 30);
						break;
					case 2:
						apfelTestPulseSequence_Inline('A', 1, 1, arg[1], arg[2]);
						break;
					case 4:
						apfelTestPulseSequence_Inline('A', arg[3], arg[4], arg[1], arg[2]);
						break;
					case 5:
						apfelTestPulseSequence_Inline(arg[5], arg[3], arg[4], arg[1], arg[2]);
						break;
				}
			}
			break;
			case 0xD:
			{
				switch (nSubCommandsArguments /* arguments of argument */)
				{
					case 0:
						apfelTestPulse_Inline('A', 1, 1, 0x10, 1, 30);
						break;
					case 1:
						apfelTestPulse_Inline('A', 1, 1, arg[1], 1, 30);
						break;
					case 2:
						apfelTestPulse_Inline('A', 1, 1, arg[1], arg[2], 30);
						break;
					case 3:
						apfelTestPulse_Inline('A', 1, 1, arg[1], arg[2], arg[3]);
						break;
					case 5:
						apfelTestPulse_Inline('A', arg[4], arg[5], arg[1], arg[2], arg[3]);
						break;
					case 6:
						apfelTestPulse_Inline(arg[6], arg[4], arg[5], arg[1], arg[2], arg[3]);
						break;
				}
			}
			break;
			case 0xE:
			{
				switch (nSubCommandsArguments /* arguments of argument */)
				{
					case 0:
						apfelSetAmplitude_Inline('A', 1, 1, 1, 30);
						break;
					case 1:
						apfelSetAmplitude_Inline('A', 1, 1, arg[1], 30);
						break;
					case 2:
						apfelSetAmplitude_Inline('A', 1, 1, arg[1], arg[2]);
						break;
					case 4:
						apfelSetAmplitude_Inline('A', arg[3], arg[4], arg[1], arg[2]);
						break;
					case 5:
						apfelSetAmplitude_Inline(arg[5], arg[3], arg[4], arg[1], arg[2]);
						break;
				}
			}
			break;
			case 0xF:
			{
				switch (nSubCommandsArguments /* arguments of argument */)
				{
					case 0:
						apfelResetAmplitude_Inline('A', 1, 1, 1, 30);
						break;
					case 1:
						apfelResetAmplitude_Inline('A', 1, 1, arg[1], 30);
						break;
					case 2:
						apfelResetAmplitude_Inline('A', 1, 1, arg[1], arg[2]);
						break;
					case 4:
						apfelResetAmplitude_Inline('A', arg[3], arg[4], arg[1], arg[2]);
						break;
					case 5:
						apfelResetAmplitude_Inline(arg[5], arg[3], arg[4], arg[1], arg[2]);
						break;
				}
			}
			break;
			default:
				CommunicationError_p(ERRA, -1, 1, PSTR("wrong first argument : %x "), arg[0]);
				return;
				break;
		}
	}

	//void (*apfelApi_Inline_p)(void) = apfelApi_Inline;
	/*----------------------------------------------------*/
	apfelInit_Inline();

	if (apfelOsziTestMode)
	{
//		_delay_us(0);
//		PINA = (APFEL_PIN_MASK1 | APFEL_PIN_MASK2);
//		_delay_us(0);
//		PINA = (APFEL_PIN_MASK1 | APFEL_PIN_MASK2);
//		_delay_us(0);
//		PINA = (APFEL_PIN_MASK1 | APFEL_PIN_MASK2);
//		_delay_us(10);
//		PINA = (APFEL_PIN_MASK1 | APFEL_PIN_MASK2);
//		_delay_us(1);
		apfelWritePort((1 << APFEL_PIN_DOUT1 | 1 << APFEL_PIN_CLK1), 'A', 1, 1);
		apfelWritePort((0 << APFEL_PIN_DOUT1 | 0 << APFEL_PIN_CLK1), 'A', 1, 1);
		apfelWritePort((1 << APFEL_PIN_DOUT1 | 0 << APFEL_PIN_CLK1), 'A', 1, 0);
		apfelWritePort((0 << APFEL_PIN_DOUT1 | 1 << APFEL_PIN_CLK1), 'A', 1, 0);
		apfelWritePort((0 << APFEL_PIN_DOUT1 | 0 << APFEL_PIN_CLK1), 'A', 1, 0);
	}

	apfelApi_Inline();
	//apfelApi_Inline_p();

	if (apfelOsziTestMode)
	{
		apfelWritePort((0 << APFEL_PIN_DOUT1 | 1 << APFEL_PIN_CLK1), 'A', 1, 0);
		apfelWritePort((1 << APFEL_PIN_DOUT1 | 0 << APFEL_PIN_CLK1), 'A', 1, 1);
		apfelWritePort((0 << APFEL_PIN_DOUT1 | 0 << APFEL_PIN_CLK1), 'A', 1, 1);
		apfelWritePort((1 << APFEL_PIN_DOUT1 | 1 << APFEL_PIN_CLK1), 'A', 1, 1);
		apfelWritePort((0 << APFEL_PIN_DOUT1 | 0 << APFEL_PIN_CLK1), 'A', 1, 1);
		apfelWritePort((0 << APFEL_PIN_DOUT1 | 0 << APFEL_PIN_CLK1), 'A', 1, 0);
		apfelWritePort((0 << APFEL_PIN_DOUT1 | 1 << APFEL_PIN_CLK1), 'A', 1, 0);
		apfelWritePort((0 << APFEL_PIN_DOUT1 | 0 << APFEL_PIN_CLK1), 'A', 1, 0);
		apfelWritePort((0 << APFEL_PIN_DOUT1 | 1 << APFEL_PIN_CLK1), 'A', 1, 0);
		apfelWritePort((0 << APFEL_PIN_DOUT1 | 0 << APFEL_PIN_CLK1), 'A', 1, 0);
		apfelWritePort((0 << APFEL_PIN_DOUT1 | 1 << APFEL_PIN_CLK1), 'A', 1, 0);
		apfelWritePort((0 << APFEL_PIN_DOUT1 | 0 << APFEL_PIN_CLK1), 'A', 1, 0);
		apfelWritePort((0 << APFEL_PIN_DOUT1 | 1 << APFEL_PIN_CLK1), 'A', 1, 0);
		apfelWritePort((0 << APFEL_PIN_DOUT1 | 0 << APFEL_PIN_CLK1), 'A', 1, 0);

//		_delay_us(0);
//		PINA = (APFEL_PIN_MASK1 | APFEL_PIN_MASK2);
//		_delay_us(10);
//		PINA = (APFEL_PIN_MASK1 | APFEL_PIN_MASK2);
//		_delay_us(0);
//		PINA = (APFEL_PIN_MASK1 | APFEL_PIN_MASK2);
//		_delay_us(0);
//		PINA = (APFEL_PIN_MASK1 | APFEL_PIN_MASK2);
	}
}

uint16_t apfelPortAddressSetMask = 0;

apfelPortAddressSet apfelPortAddressSets[APFEL_MAX_N_PORT_ADDRESS_SETS];

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

	static uint8_t nBits = APFEL_N_COMMAND_BITS + APFEL_N_VALUE_BITS + APFEL_N_CHIP_ID_BITS;

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