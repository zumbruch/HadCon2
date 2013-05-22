/*
 * VERSION 1.0 Januar 7th 2010 LATE  File: 'one_wire_adc.c'
 * Author: Linda Fouedjio  based on Alejandro Gil
 * modified (heavily rather rebuild): Peter Zumbruch
 */

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include <avr/iocan128.h>

#include "one_wire.h"
#include "one_wire_adc.h"
#include "one_wire_dualSwitch.h"
#include "one_wire_simpleSwitch.h"
#include "read_write_register.h"
//#include "one_wire_temperature.h"

#include "adc.h"
#include "api.h"
#include "api_define.h"
#include "api_global.h"

#include "api_debug.h"
#include "can.h"
#include "mem-check.h"

#include "OWIDeviceSpecific.h"
#include "OWIHighLevelFunctions.h"
#include "OWIBitFunctions.h"
#include "OWIcrc.h"

#define DS2450_CONVERT                  0x3C
#define DS2450_READ_MEMORY              0xAA
#define DS2450_WRITE_MEMORY             0x55

#define DS2450_ADDRESS_MEMORY_MAP_PAGE_0_CONVERSION_READOUT_INPUT_A_LSB     0x0000
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_0_CONVERSION_READOUT_INPUT_A_MSB     0x0001
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_0_CONVERSION_READOUT_INPUT_B_LSB     0x0002
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_0_CONVERSION_READOUT_INPUT_B_MSB     0x0003
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_0_CONVERSION_READOUT_INPUT_C_LSB     0x0004
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_0_CONVERSION_READOUT_INPUT_C_MSB     0x0005
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_0_CONVERSION_READOUT_INPUT_D_LSB     0x0006
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_0_CONVERSION_READOUT_INPUT_D_MSB     0x0007


#define DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_OC_OE_RESOLUTION_INPUT_A     0x0008
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_POR_ALARMS_RANGES_INPUT_A    0x0009
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_OC_OE_RESOLUTION_INPUT_B     0x000A
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_POR_ALARMS_RANGES_INPUT_B    0x000B
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_OC_OE_RESOLUTION_INPUT_C     0x000C
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_POR_ALARMS_RANGES_INPUT_C    0x000D
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_OC_OE_RESOLUTION_INPUT_D     0x000E
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_POR_ALARMS_RANGES_INPUT_D    0x000F


#define DS2450_ADDRESS_MEMORY_MAP_PAGE_2_ALARM_SETTINGS_LOW_INPUT_A      0x0010
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_2_ALARM_SETTINGS_HIGH_INPUT_A     0x0011
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_2_ALARM_SETTINGS_LOW_INPUT_B      0x0012
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_2_ALARM_SETTINGS_HIGH_INPUT_B     0x0013
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_2_ALARM_SETTINGS_LOW_INPUT_C      0x0014
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_2_ALARM_SETTINGS_HIGH_INPUT_C     0x0015
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_2_ALARM_SETTINGS_LOW_INPUT_D      0x0016
#define DS2450_ADDRESS_MEMORY_MAP_PAGE_2_ALARM_SETTINGS_HIGH_INPUT_D     0x0017

#define DS2450_ADDRESS_MEMORY_MAP_PAGE_3_VCC_CONTROL_BYTE     0x001C

#define DS2450_DATA_MEMORY_MAP_PAGE_3_VCC_CONTROL_BYTE     0x40 /*VCC powered mode*/

#define DS2450_OUTPUT_CONTROL_INPUT_A 0x0
#define DS2450_OUTPUT_CONTROL_INPUT_B 0x0
#define DS2450_OUTPUT_CONTROL_INPUT_C 0x0
#define DS2450_OUTPUT_CONTROL_INPUT_D 0x0

#warning TODO: make it changeable through API, by changing the resolution for DS2450, since max time is (4 x resolution * 80) us + 160us offset (max 5.3 ms)

#if 0
#define DS2450_RESOLUTION_INPUT_A     0x8 /* 8, bit resolution, only mask 0xF is taken, so 0x0 -> 16 bits, ..0x1 -> 1 bit */
#define DS2450_RESOLUTION_INPUT_B     0x8
#define DS2450_RESOLUTION_INPUT_C     0x8
#define DS2450_RESOLUTION_INPUT_D     0x8
#elif 1
#define DS2450_RESOLUTION_INPUT_A     0x10 /* 16, bit resolution, only mask 0xF is taken, so 0x0 -> 16 bits, ..0x1 -> 1 bit */
#define DS2450_RESOLUTION_INPUT_B     0x10
#define DS2450_RESOLUTION_INPUT_C     0x10
#define DS2450_RESOLUTION_INPUT_D     0x10
#else
#define DS2450_RESOLUTION_INPUT_A     0xA /* 10, bit resolution, only mask 0xF is taken, so 0x0 -> 16 bits, ..0x1 -> 1 bit */
#define DS2450_RESOLUTION_INPUT_B     0xA
#define DS2450_RESOLUTION_INPUT_C     0xA
#define DS2450_RESOLUTION_INPUT_D     0xA
#endif
#define DS2450_POWER_ON_RESET_INPUT_A 0x0
#define DS2450_POWER_ON_RESET_INPUT_B 0x0
#define DS2450_POWER_ON_RESET_INPUT_C 0x0
#define DS2450_POWER_ON_RESET_INPUT_D 0x0

#define DS2450_ALARM_ENABLE_INPUT_A 0x0 /*ALARM ENABLE LOW: 0x1, ALARM ENABLE HIGH: 0x2 or BOTH: 0x3*/
#define DS2450_ALARM_ENABLE_INPUT_B 0x0
#define DS2450_ALARM_ENABLE_INPUT_C 0x0
#define DS2450_ALARM_ENABLE_INPUT_D 0x0

#define DS2450_INPUT_RANGE_INPUT_A 0x1 /* 0: 2.55 V, 1: 5.10 V */
#define DS2450_INPUT_RANGE_INPUT_B 0x1 /* 0: 2.55 V, 1: 5.10 V */
#define DS2450_INPUT_RANGE_INPUT_C 0x1 /* 0: 2.55 V, 1: 5.10 V */
#define DS2450_INPUT_RANGE_INPUT_D 0x1 /* 0: 2.55 V, 1: 5.10 V */

#define DS2450_CONVERSION_CHANNEL_SELECT_MASK 0x0F
#define DS2450_CONVERSION_READOUT_CONTROL     0xAA

#ifndef OWI_ADC_MAX_CONVERSION_TIME_MILLISECONDS
#define OWI_ADC_MAX_CONVERSION_TIME_MILLISECONDS 8
#endif

#ifndef OWI_ADC_CONVERSION_DELAY_MILLISECONDS
#define OWI_ADC_CONVERSION_DELAY_MILLISECONDS 1
#endif

#define DS2450_TRIPLE_CONVERSION_MAX_LOOP_TURN 0x3

uint16_t owiAdcMask = 0;
uint16_t* p_owiAdcMask = &owiAdcMask;
uint16_t owiAdcTimeoutAndFailureBusMask = 0xFFFF; /*bit mask of non converted channels/pins 1:conversion timeout*/
uint8_t owiUseCommonAdcConversion_flag = TRUE;
//uint8_t owiAdcTripleReadout = TRUE;
uint8_t owiAdcTripleReadout = FALSE;

/* global address settings */
#warning TODO: move those to PROGMEM
uint8_t addressOutputAndResolution[4] =
{
		DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_OC_OE_RESOLUTION_INPUT_A,
		DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_OC_OE_RESOLUTION_INPUT_B,
		DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_OC_OE_RESOLUTION_INPUT_C,
		DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_OC_OE_RESOLUTION_INPUT_D
};

uint8_t addressPORandAlarmsAndInputRange[4] =
{
		DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_POR_ALARMS_RANGES_INPUT_A,
		DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_POR_ALARMS_RANGES_INPUT_B,
		DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_POR_ALARMS_RANGES_INPUT_C,
		DS2450_ADDRESS_MEMORY_MAP_PAGE_1_CONTROL_STATUS_DATA_POR_ALARMS_RANGES_INPUT_D
};

/*
 * this function contains all the functions that are necessary to
 * convert analog value to digital via one wire bus
 */

void owiReadADCs( struct uartStruct *ptr_uartStruct )
{
	uint8_t foundDevices = 0;

	/* check for syntax:
	 *    allowed arguments are:
	 *       - (empty)
	 *       - ID
	 *       - ID conversion_flag
	 */

	switch (ptr_uartStruct->number_of_arguments)
	{
	case 0:
		break;
	case 1:
	case 2:
	case 3:
		/* read single ID w/o adc conversion w/o initialization
		 * or
		 * read single ID w/ adc conversion of all buses but w/o initialization
		 * or
		 * read single ID w/ adc conversion of all buses and w/ initialization */
		if ( FALSE == ptr_owiStruct->idSelect_flag)
		{
			CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, TRUE, PSTR("invalid arguments"));
			return;
			break;
		}
		break;
	default:
		CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, TRUE, PSTR("write argument: too many arguments") );
		return;
		break;
	}

	/* find available devices and their IDs */
	NumDevicesFound = owiReadDevicesID(BUSES);

	if ( 0 < NumDevicesFound )
	{
		/* reset mask of busses with sensors*/
		owiAdcMask = 0;

		// scan for busses/pins connected to an ADC
		if ( 0 < owiScanIDS(OWI_FAMILY_DS2450_ADC,p_owiAdcMask))
		{
#warning TODO: is the Initialization always needed here, or is it just needed once at the beginning? MOVE IT TO the beginning !!!
			/* Initialization */

			switch (ptr_uartStruct->number_of_arguments)
			{
			case 0:
				/* read all */
				owiInitializeADCs(BUSES);
				break;
			case 3:
				/* read single ADC w/ conversion and initialization*/
				if (TRUE == ptr_owiStruct->init_flag) { owiInitializeADCs(BUSES); }
				break;
			}

			/* conversions */

			switch (ptr_uartStruct->number_of_arguments)
			{
			case 0:
				/* read all */
				ptr_owiStruct->conv_flag = TRUE;
				break;
			case 2:
				/* read single ID w/ adc conversion on all busses */
				/* during the filling it couldn't decide weather the 2nd argument
				 * is a flag or a value ... now we can, its a flag */
				ptr_owiStruct->conv_flag = ( 0 != ptr_owiStruct->value );
				break;
			case 3:
				/* read single ADC w/ conversion and initialization*/
				break;
			}

			if (TRUE == ptr_owiStruct->conv_flag) { owiMakeADCConversions(BUSES); }

			/*
			 * access values
			 */

			printDebug_p(debugLevelEventDebug, debugSystemOWIADC, __LINE__, PSTR(__FILE__), PSTR("call: FindFamilyDevicesAndAccessValues"));

			/*    - read DS2450 */
			foundDevices += owiFindFamilyDevicesAndAccessValues(BUSES, NumDevicesFound, OWI_FAMILY_DS2450_ADC, NULL );

			if ( TRUE == ptr_owiStruct->idSelect_flag && 0 == foundDevices)
			{
				generalErrorCode = CommunicationError_p(ERRG, dynamicMessage_ErrorIndex, TRUE, PSTR("no matching ID was found"));
			}

			printDebug_p(debugLevelEventDebug, debugSystemOWIADC, __LINE__, PSTR(__FILE__), PSTR("end"));
		}
	}
}//END of owiReadADCs function

/*
 *this function initializes the analog to digital converter
 */
int8_t owiInitializeADCs( uint8_t *pins )
{
#warning TODO: make it possible to access init settings for single and all devices and change the memory map addresses online

	static const uint8_t maxTrials = 3;

	uint8_t dataOutputAndResolution[4];
	uint8_t dataPORandAlarmsAndInputRange[4];

	/* presets */
	/*
	 * control/status, Memory Map Page 1
	 *
	 *
	 * The power-on default setting for the control/status data is
	 * 	08h
	 * for the first and
	 * 	8Ch
	 * for the second byte of each channel.
	 *
	 */

	/* * Output Control and Resolution */
	/*
	 * Memory Map Page 1, first byte */
	/*
	 * The control and status information for all channels is located in memory page 1 ... .
	 * As for the conversion read-out, each channel has assigned 16 bits.
	 * The four least significant bits, called RC3 to RC0, are an unsigned binary number
	 * that represents the number of bits to be converted.
	 * A code of 1111 (15 decimal) will generate a 15-bit result.
	 * For a full 16-bit conversion the code number needs to be 0000.
	 * The next two bits beyond RC3 will always read 0. They have no function and cannot be changed to 1s
	 *
	 * The next bits, OC (output control) and OE (enable output) control the alternate use of a channel as output.
	 * For normal operation as analog input the OE bit of a channel needs to be 0,
	 * rendering the OC bit to a don’t care.
	 * With OE set to 1, a 0 for OC will make the channel’s output transistor conducting,
	 * a 1 for OC will switch the transistor off. With a pullup resistor to a positive voltage, for example,
	 * the OC bit will directly translate into the voltage equivalent of its logic state.
	 * Enabling the output does not disable the analog input.
	 * Conversions remain possible, but will result in values close to 0 if the transistor is conducting.
	 *
	 *
	 *
	 */
	/*    - Channel A */
	dataOutputAndResolution[0] = 0x0;
	dataOutputAndResolution[0] |= (DS2450_OUTPUT_CONTROL_INPUT_A & 0x3) << 6;
	dataOutputAndResolution[0] |= (    DS2450_RESOLUTION_INPUT_A & 0xF) << 0;
	/*    - Channel B */
	dataOutputAndResolution[1] = 0x0;
	dataOutputAndResolution[1] |= (DS2450_OUTPUT_CONTROL_INPUT_B & 0x3) << 6;
	dataOutputAndResolution[1] |= (    DS2450_RESOLUTION_INPUT_B & 0xF) << 0;
	/*    - Channel C */
	dataOutputAndResolution[2] = 0x0;
	dataOutputAndResolution[2] |= (DS2450_OUTPUT_CONTROL_INPUT_C & 0x3) << 6;
	dataOutputAndResolution[2] |= (    DS2450_RESOLUTION_INPUT_C & 0xF) << 0;
	/*    - Channel D */
	dataOutputAndResolution[3] = 0x0;
	dataOutputAndResolution[3] |= (DS2450_OUTPUT_CONTROL_INPUT_D & 0x3) << 6;
	dataOutputAndResolution[3] |= (    DS2450_RESOLUTION_INPUT_D & 0xF) << 0;

	/* *  Power on Reset, Alarm enable, Input range */
	/*
	 * Memory Map Page 1, second byte
	 */
	/*
	 * The IR bit in the second byte of a channel’s control and status memory selects the input voltage range.
	 * With IR set to 0, the highest possible conversion result is reached at 2.55V.
	 * Setting IR to 1 requires an input voltage of 5.10V for the same result.
	 * The next bit beyond IR has no function. It will always read 0 and cannot be changed to 1
	 *
	 * The next two bits, AEL alarm enable low and AEH alarm enable high, control whether the device will
	 * respond to the Conditional Search command (see ROM Functions) if a conversion results in a value
	 * higher (AEH) than or lower (AEL) than the channel’s alarm threshold voltage as specified in the alarm
	 * settings. The alarm flags AFL (low) and AFH (high) tell the bus master whether the channel’s input
	 * voltage was beyond the low or high threshold at the latest conversion. These flags are cleared
	 * automatically if a new conversion reveals a non-alarming value.
	 * They can alternatively be written to 0 by the bus master without a conversion
	 *
	 * The next bit of a channel’s control and status memory always reads 0 and cannot be changed to 1.
	 *
	 * The POR bit (power on reset) is automatically set to 1 as the device performs a power-on reset cycle.
	 * As long as this bit is set the device will always respond to
	 * the Conditional Search command in order to notify the bus master that the control and threshold data
	 * is no longer valid. After powering-up the POR bit needs to be written to 0 by the bus master.
	 * This may be done together with restoring the control and threshold data.
	 * It is possible for the bus master to write the POR bit to a 1.
	 * This will make the device participate in the conditional search but will not generate a reset cycle.
	 * Since the POR bit is related to the device and not channel-specific
	 * the value written with the most recent setting of an input range or alarm enable applies.
	 *
	 */
	/*    -  Channel A */
	dataPORandAlarmsAndInputRange[0] = 0x0;
	dataPORandAlarmsAndInputRange[0] |= (DS2450_POWER_ON_RESET_INPUT_A & 0x1) << 7;
	dataPORandAlarmsAndInputRange[0] |= (  DS2450_ALARM_ENABLE_INPUT_A & 0x3) << 2;
	dataPORandAlarmsAndInputRange[0] |= (   DS2450_INPUT_RANGE_INPUT_A & 0x1) << 0;
	/*    -  Channel B */
	dataPORandAlarmsAndInputRange[1] = 0x0;
	dataPORandAlarmsAndInputRange[1] |= (DS2450_POWER_ON_RESET_INPUT_B & 0x1) << 7;
	dataPORandAlarmsAndInputRange[1] |= (  DS2450_ALARM_ENABLE_INPUT_B & 0x3) << 2;
	dataPORandAlarmsAndInputRange[1] |= (   DS2450_INPUT_RANGE_INPUT_B & 0x1) << 0;
	/*    -  Channel C */
	dataPORandAlarmsAndInputRange[2] = 0x0;
	dataPORandAlarmsAndInputRange[2] |= (DS2450_POWER_ON_RESET_INPUT_C & 0x1) << 7;
	dataPORandAlarmsAndInputRange[2] |= (  DS2450_ALARM_ENABLE_INPUT_C & 0x3) << 2;
	dataPORandAlarmsAndInputRange[2] |= (   DS2450_INPUT_RANGE_INPUT_C & 0x1) << 0;
	/*    -  Channel D */
	dataPORandAlarmsAndInputRange[3] = 0x0;
	dataPORandAlarmsAndInputRange[3] |= (DS2450_POWER_ON_RESET_INPUT_D & 0x1) << 7;
	dataPORandAlarmsAndInputRange[3] |= (  DS2450_ALARM_ENABLE_INPUT_D & 0x3) << 2;
	dataPORandAlarmsAndInputRange[3] |= (   DS2450_INPUT_RANGE_INPUT_D & 0x1) << 0;

	/*SET UP for non parasitic mode*/
	for ( int8_t b = 0 ; b < OWI_MAX_NUM_PIN_BUS ; b++ )
	{
		// continue if bus doesn't contain any ADCS
		if ( 0 == (owiAdcMask & (0x1 << b)))
		{
			continue;
		}

		if ( 0 == OWI_DetectPresence(pins[b]) )
		{ /* the "DetectPresence" function already sends a Reset*/
			continue; // Error
		}


		/*
		 * changing settings of channels  of all connected devices
		 */

		for (int channel = 0; channel < 4; channel++)
		{
			if ( 0 != owiADCMemoryWriteByte(pins[b], NULL, addressOutputAndResolution[channel], dataOutputAndResolution[0], maxTrials))
			{
				CommunicationError_p(ERRG, dynamicMessage_ErrorIndex, FALSE, PSTR("init 1-wire ADCs: failed to set: OUTPUT CONTROL, RESOLUTION"));
			}

			if ( 0 != owiADCMemoryWriteByte(pins[b], NULL, addressPORandAlarmsAndInputRange[channel], dataPORandAlarmsAndInputRange[channel], maxTrials))
			{
				CommunicationError_p(ERRG, dynamicMessage_ErrorIndex, FALSE, PSTR("init 1-wire ADCs: failed to set: POR, ALARM ENABLE, INPUT RANGE"));
			}
		}

		/*
		 * changing settings of VCC_CONTROL_BYTE of all connected devices
		 */
		if ( 0 != owiADCMemoryWriteByte(pins[b],NULL, DS2450_ADDRESS_MEMORY_MAP_PAGE_3_VCC_CONTROL_BYTE, DS2450_DATA_MEMORY_MAP_PAGE_3_VCC_CONTROL_BYTE, maxTrials))
		{
			CommunicationError_p(ERRG, dynamicMessage_ErrorIndex, FALSE, PSTR("init 1-wire ADCs: failed to set: VCC_CONTROL_BYTE"));
		}
	}
	return 1;
}//END of owiInitializeADCs function

/*
 * owiADCMemoryWriteByte (unsigned char bus_pattern, unsigned char * id, uint16_t address, uint8_t data, uint8_t maxTrials)
 *
 *
 * this function implements the writing of a data byte to an 16 byte address of an owi device
 * using the 0x55 command
 *
 * if a single id is specified also a data verification check is performed with a limited number of trials
 * CRC checks are not performed
 *
 * input:
 *    bus_pattern:
 *                 pattern of bus lines to send command to
 *    id         :
 *                 1-wire device id 8 byte char array
 *                    - if set, i.e. not NULL, only device with this id is addressed (MATCH_ROM)
 *                    - else every device is used (ROM_SKIP)
 *    address    :
 *                 16bit address to write data to
 *    data       :
 *                 data byte to be send
 *    maxTrials  :
 *                 number of trials to be performed before sending a failed, 0 will be increased to 1, give it at least a try
 *
 * return:
 *    0: ok
 *    1: failed
 *
 */

uint8_t owiADCMemoryWriteByte(unsigned char bus_pattern, unsigned char * id, uint16_t address, uint8_t data, uint8_t maxTrials)
{
	uint16_t receive_CRC;
	uint8_t flag = FALSE;
	uint8_t verificationData = 0x0;
	uint8_t trialsCounter = maxTrials;

	printDebug_p(debugLevelEventDebug, debugSystemOWIADC, __LINE__, PSTR(__FILE__), PSTR("writing to ADC memory address: 0x%x \tdata: 0x%x"),address, data);

	/* 0 trials, give it a chance */
	if (0 == trialsCounter) { trialsCounter++;}

	while ( FALSE == flag && trialsCounter != 0)
	{
		/* select one or all, depending if id is given */
		if ( id == NULL)
		{
			/*
			 * SKIP ROM [CCH]
			 *
			 * This command can save time in a single drop bus system by allowing the bus master to access the
			 * memory/ convert functions without providing the 64-bit ROM code. If more than one slave is present on
			 * the bus and a read command is issued following the Skip ROM command, data collision will occur on the
			 * bus as multiple slaves transmit simultaneously (open drain pulldowns will produce a wired-AND result).
			 */
			OWI_SendByte(OWI_ROM_SKIP, bus_pattern);
		}
		else
		{
			/*
			 * MATCH ROM [55H]
			 *
			 * The match ROM command, followed by a 64-bit ROM sequence, allows the bus master to address a
			 * specific DS2450 on a multidrop bus. Only the DS2450 that exactly matches the 64-bit ROM sequence
			 * will respond to the following memory/convert function command. All slaves that do not match the 64-bit
			 * ROM sequence will wait for a reset pulse. This command can be used with a single or multiple devices
			 * on the bus.
			 */
			OWI_MatchRom(id, bus_pattern); // Match id found earlier
		}

		/*
		 * WRITE MEMORY [55H]
		 *
		 * The Write Memory command is used to write to memory pages 1 and 2 in order to set the channel specific
		 * control data and alarm thresholds. The command can also be used to write the single control byte
		 * on page 3 at address 1Ch. The bus master will follow the command byte with a two byte starting address
		 * (TA1=(T7:T0), TA2=(T15:T8)) and a data byte of (D7:D0). A 16-bit CRC of the command byte, address
		 * bytes, and data byte is computed by the DS2450 and read back by the bus master to confirm that the
		 * correct command word, starting address, and data byte were received. Now the DS2450 copies the data
		 * byte to the specified memory location. With the next eight time slots the bus master receives a copy of
		 * the same byte but read from memory for verification. If the verification fails, a Reset Pulse should be
		 * issued and the current byte address should be written again.
		 * If the bus master does not issue a Reset Pulse and the end of memory was not yet reached, the DS2450
		 * will automatically increment its address counter to address the next memory location. The new two-byte
		 * address will also be loaded into the 16-bit CRC-generator as a starting value. The bus master will send
		 * the next byte using eight write time slots. As the DS2450 receives this byte it also shifts
		 * it into the CRCgenerator and the result is a 16-bit CRC of the new data byte and the new address.
		 * With the next sixteen read time slots the bus master
		 * will read this 16-bit CRC from the DS2450 to verify that the address
		 * incremented properly and the data byte was received correctly. Following the CRC the master receives
		 * the byte just written as read from the memory. If the CRC or read-back byte is incorrect, a Reset Pulse
		 * should be issued in order to repeat the Write Memory command sequence.
		 * Note that the initial pass through the Write Memory flow chart will generate a 16-bit CRC value that is
		 * the result of shifting the command byte into the CRC-generator, followed by the two address bytes, and
		 * finally the data byte. Subsequent passes through the Write Memory flow chart due to the DS2450
		 * automatically incrementing its address counter will generate a 16-bit CRC that is the result of loading (not
		 * shifting) the new (incremented) address into the CRC-generator and then shifting in the new data byte.
		 * The decision to continue after having received a bad CRC or if the verification fails is made entirely by
		 * the bus master. Write access to the conversion read-out registers is not possible. If a write attempt is
		 * made to a page 0 address the device will follow the Write Memory flow chart correctly but the
		 * verification of the data byte read back from memory will usually fail. The Write Memory command
		 * sequence can be ended at any point by issuing a Reset Pulse.
		 *
		 */

		OWI_SendByte(DS2450_WRITE_MEMORY, bus_pattern);
		OWI_SendWord(address, bus_pattern);

		OWI_SendByte(data, bus_pattern);

		/* receive 16bit CRC */
		receive_CRC = OWI_ReceiveWord(bus_pattern);           /* IMPORTANT AFTER EACH 'MEMORY WRITE' OPERATION to start memory writing*/

		/* only possible if there is not OWI_ROM_SKIP, so wait until this device specific is implemented*/
		if (id != NULL)
		{
#warning TODO: add a complex check on the CRC, including the correct address, if in single device mode

			/*verify written data*/
			verificationData = OWI_ReceiveByte(bus_pattern);
			if ( data == verificationData ) /* check passed */
			{
				flag = TRUE;
			}
			else
			{
				trialsCounter--;
			}
		}
		else
		{
			flag = TRUE;
		}

		/* ending the Write Memory command sequence by issuing a Reset Pulse*/
		OWI_DetectPresence(bus_pattern); /*the "DetectPresence" function includes sending a Reset Pulse*/

	} /*end of while loop */

	if (FALSE == flag)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/*
 *this function makes ADC conversation of all found devices
 */
int8_t owiMakeADCConversions( uint8_t *pins )
{
	static uint8_t initialCall_flag = TRUE;

	printDebug_p(debugLevelEventDebug, debugSystemOWIADC, __LINE__, PSTR(__FILE__), PSTR(""));

	if ( TRUE == initialCall_flag )
	{
		printDebug_p(debugLevelEventDebug, debugSystemOWIADC, __LINE__, PSTR(__FILE__), PSTR("inital call needs twice a conversion"));
		initialCall_flag = FALSE;
		owiMakeADCConversions(pins);
	}

	uint8_t commonPins = 0x0;
	uint8_t currentPins = 0x0;
	uint8_t busPatternIndexMax = 0;

	/* first loop checking busPattern against masks and creating common bus mask*/

	for ( int8_t busPatternIndex = 0 ; busPatternIndex < OWI_MAX_NUM_PIN_BUS ; busPatternIndex++ )
	{
		// continue if bus isn't active
		if ( 0 == ((owiBusMask & pins[busPatternIndex]) & 0xFF) )
		{
			printDebug_p(debugLevelEventDebug, debugSystemOWIADC, __LINE__, PSTR(__FILE__), PSTR("bus: %i differs (pin pattern 0x%x owiBusMask 0x%x)"), busPatternIndex, pins[busPatternIndex],owiBusMask);
			continue;
		}
		else
		{
			printDebug_p(debugLevelEventDebug, debugSystemOWIADC, __LINE__, PSTR(__FILE__), PSTR("bus: %i active  (pin pattern 0x%x owiBusMask 0x%x)"), busPatternIndex, pins[busPatternIndex],owiBusMask);
		}
		// continue if bus doesn't contain any ADCs
		if ( 0 == ((owiAdcMask & pins[busPatternIndex]) & 0xFF ) )
		{
			printDebug_p(debugLevelEventDebug, debugSystemOWIADC, __LINE__, PSTR(__FILE__), PSTR("bus: %i ADCs: NONE (pin pattern 0x%x owiAdcMask 0x%x)"), busPatternIndex, pins[busPatternIndex],owiAdcMask);
			continue;
		}
		else
		{
			printDebug_p(debugLevelEventDebug, debugSystemOWIADC, __LINE__, PSTR(__FILE__), PSTR("bus: %i ADCs: some (pin pattern 0x%x owiAdcMask 0x%x)"), busPatternIndex, pins[busPatternIndex],owiAdcMask);
		}

		if ( TRUE == owiUseCommonAdcConversion_flag)
		{
			commonPins |= pins[busPatternIndex];
			printDebug_p(debugLevelEventDebug, debugSystemOWIADC, __LINE__, PSTR(__FILE__), PSTR("bus: %i combining 0x%x to common set of pins 0x%x)"), busPatternIndex, pins[busPatternIndex],commonPins);
		}
	}

	if ( TRUE == owiUseCommonAdcConversion_flag)
	{
		busPatternIndexMax = 1; /*only once */
		printDebug_p(debugLevelEventDebug, debugSystemOWIADC, __LINE__, PSTR(__FILE__), PSTR("final common pins: 0x%x"), commonPins);
	}
	else
	{
		busPatternIndexMax = OWI_MAX_NUM_PIN_BUS;
	}

	for ( int8_t busPatternIndex = 0 ; busPatternIndex < busPatternIndexMax ; busPatternIndex++ )
	{
		if ( TRUE == owiUseCommonAdcConversion_flag)
		{
			currentPins = commonPins;
		}
		else
		{
			currentPins = pins[busPatternIndex];

			// continue if bus isn't active
			if (0 == ((owiBusMask & currentPins) & 0xFF))
			{
				continue;
			}

			// continue if bus doesn't contain any ADCs
			if (0 == ((owiAdcMask & currentPins) & 0xFF))
			{
				continue;
			}
		}

		/* now first access to bus, within the function  */
		if ( 0 == OWI_DetectPresence(currentPins) )
		{
			printDebug_p(debugLevelEventDebug, debugSystemOWIADC, __LINE__, PSTR(__FILE__), PSTR("bus: %i no Device present (pin pattern 0x%x)"), busPatternIndex, currentPins);
			continue;
		}
		else
		{
			printDebug_p(debugLevelEventDebug, debugSystemOWIADC, __LINE__, PSTR(__FILE__), PSTR("bus: %i some devices present (pin pattern 0x%x)"), busPatternIndex, currentPins);
		}

		/*starting conversion sequence on all IDs */
		owiADCConvert(currentPins, NULL);

	}//end of: 	for ( int8_t busPatternIndex = 0 ; busPatternIndex < busPatternIndexMax ; busPatternIndex++ )

	printDebug_p(debugLevelEventDebug, debugSystemOWIADC, __LINE__, PSTR(__FILE__), PSTR("make conversion finished (owiAdcTimeoutMask = 0x%x)"), owiAdcTimeoutAndFailureBusMask);
	return 1;
}//END of owiMakeADCConversions function


/*
 * owiADCConvert(unsigned char bus_pattern, unsigned char * id)
 *
 *
 * this function implements the convert function
 * using the 0x3C command
 *
 * CRC checks are not performed
 *
 * input:
 *    bus_pattern:
 *                 pattern of bus lines to send command to
 *    id         :
 *                 1-wire device id 8 byte char array
 *                    - if set, i.e. not NULL, only device with this id is addressed (MATCH_ROM)
 *                    - else every device is used (ROM_SKIP)
 *
 * return:
 *    0: ok
 *    1: failed
 *
 */
uint8_t owiADCConvert(unsigned char currentPins, unsigned char * id)
{
	printDebug_p(debugLevelEventDebug, debugSystemOWIADC, __LINE__, PSTR(__FILE__), PSTR("bus_pattern: 0x%x starting conversion sequence"), currentPins );

	static uint16_t maxConversionTime = OWI_ADC_MAX_CONVERSION_TIME_MILLISECONDS;
	static uint32_t count;
	static uint32_t maxcount;
	static unsigned char timeout_flag;
	uint8_t trialsCounter = 0;
	uint8_t result = RESULT_OK;
    uint16_t receive_CRC;

	maxcount = ( OWI_ADC_CONVERSION_DELAY_MILLISECONDS > 0 ) ? maxConversionTime / OWI_ADC_CONVERSION_DELAY_MILLISECONDS : 1;
	count = maxcount;
	timeout_flag = FALSE;

	/*
	 * CONVERT [3CH]
	 *
	 * The Convert command is used to initiate the analog to digital conversion for one or more channels at the
	 * resolution specified in memory page 1, control/status data. The conversion takes between 60 and 80 μs
	 * per bit plus an offset time of maximum 160 μs every time the convert command is issued. For four
	 * channels with 12-bit resolution each, as an example, the convert command will not take more than
	 * 4x12x80 μs plus 160 μs offset, which totals 4 ms. If the DS2450 gets its power through the VCC pin, the
	 * bus master may communicate with other devices on the 1-Wire bus while the DS2450 is busy with A/D
	 * conversions. If the device is powered entirely from the 1-Wire bus, the bus master must instead provide a
	 * strong pullup to 5V for the estimated duration of the conversion in order to provide sufficient energy.
	 *
	 * The conversion is controlled by the input select mask (Figure 7a) and the read-out control byte (Figure
	 * 7b). In the input select mask the bus master specifies which channels participate in the conversion. A
	 * channel is selected if the bit associated to the channel is set to 1. If more than one channel is selected, the
	 * conversion takes place one channel after another in the sequence A, B, C, D, skipping those channels that
	 * are not selected. The bus master can read the result of a channel’s conversion before the conversion of all
	 * the remaining selected channels is completed. In order to distinguish between the previous result and the
	 * new value the bus master uses the read-out control byte. This byte allows presetting the conversion readout
	 * registers for each selected channel to all 1’s or all 0’s. If the expected result is close to 0 then one
	 * should preset to all 1’s or to all 0’s if the conversion result will likely be a high number. In applications
	 * where the bus master can wait until all selected channels are converted before reading, a preset of the
	 * read-out registers is not necessary. Note that for a channel not selected in the input select mask, the
	 * channel’s read-out control setting has no effect. If a channel constantly yields conversion results close to
	 * 0 the channel’s output transistor may be conducting. See section Device Registers for details.
	 *
	 * INPUT SELECT MASK (CONVERSION COMMAND) Figure 7a
	 * bit 7 bit 6 bit 5 bit 4 bit 3 bit 2 bit 1 bit 0
	 * “don’t care” D C B A
	 *
	 * READ-OUT CONTROL (CONVERSION COMMAND) Figure 7b
	 * bit 7 bit 6 bit 5 bit 4 bit 3 bit 2 bit 1 bit 0
	 * Set D Clear D Set C Clear C Set B Clear B Set A Clear A
	 *
	 * Set Clear Explanation
	 * 0 0 no preset, leave as is
	 * 0 1 preset to all 0’s
	 * 1 0 preset to all 1’s
	 * 1 1 (illegal code)
	 *
	 * Following the Convert command byte the bus master transmits the input select mask and the read-out
	 * control byte. Now the bus master reads the CRC16 of the command byte, select mask and control byte.
	 * The conversion will start no earlier than 10 μs after the most significant bit of the CRC is received by the
	 * bus master.
	 *
	 * With a parasitic power supply the bus master must activate the strong pullup within this 10 μs window for
	 * a duration that is estimated as explained above. After that, the data line returns to an idle high state and
	 * communication on the bus can resume. The bus master would normally send a reset pulse to exit the
	 * Convert command. Read data time slots generated after the strong pullup has ended but before issuing a
	 * reset pulse should result in all 1’s if the conversion time was calculated correctly.
	 *
	 * With VCC power supply the bus master may either send a reset pulse to exit the Convert command or
	 * continuously generate read data time slots. As long as the DS2450 is busy with conversions the bus
	 * master will read 0’s. After the conversion is completed the bus master will receive 1’s instead. Since in a
	 * open-drain environment a single 0 overwrites multiple 1’s the bus master can monitor multiple devices
	 * converting simultaneously and immediately knows when the last one is ready. As in the parasite
	 * powered scenario the bus master finally has to exit the Convert command by issuing a rest pulse.
	 *
	 * from data sheet: http://datasheets.maximintegrated.com/en/ds/DS2450.pdf
	 */

	while( trialsCounter < OWI_SEND_BYTE_MAX_TRIALS )
	{
		trialsCounter++;
        receive_CRC = 0;
    	result = RESULT_OK;

		printDebug_p(debugLevelEventDebug, debugSystemOWIADC, __LINE__, PSTR(__FILE__), PSTR("bus_pattern: 0x%x sending command, mask, readout byte"),currentPins );
		/* select one or all, depending if id is given */
		if ( NULL == id)
		{
			/*
			 * SKIP ROM [CCH]
			 *
			 * This command can save time in a single drop bus system by allowing the bus master to access the
			 * memory/convert functions without providing the 64-bit ROM code. If more than one slave is present on
			 * the bus and a read command is issued following the Skip ROM command, data collision will occur on the
			 * bus as multiple slaves transmit simultaneously (open drain pulldowns will produce a wired-AND result).
			 */
			OWI_SkipRom(currentPins);

			result = owiSendBytesAndCheckCRC16(currentPins, 3,
					DS2450_CONVERT, DS2450_CONVERSION_CHANNEL_SELECT_MASK, DS2450_CONVERSION_READOUT_CONTROL);
			result = RESULT_OK; /*disable CRC check for skip rom*/
		}
		else
		{
			/*
			 * MATCH ROM [55H]
			 *
			 * The match ROM command, followed by a 64-bit ROM sequence, allows the bus master to address a
			 * specific DS2450 on a multidrop bus. Only the DS2450 that exactly matches the 64-bit ROM sequence
			 * will respond to the following memory/convert function command. All slaves that do not match the 64-bit
			 * ROM sequence will wait for a reset pulse. This command can be used with a single or multiple devices
			 * on the bus.
			 */
			OWI_MatchRom(id, currentPins); // Match id found earlier
			result = owiSendBytesAndCheckCRC16(currentPins, 3,
					DS2450_CONVERT, DS2450_CONVERSION_CHANNEL_SELECT_MASK, DS2450_CONVERSION_READOUT_CONTROL);
			result = RESULT_OK; /*disable CRC check for skip rom*/
		}

		//		printDebug_p(debugLevelEventDebug, debugSystemOWIADC, __LINE__, PSTR(__FILE__), PSTR("crc16 checksum 3 bytes 0x%x ?"), owiComputeCRC16(0x0000, 3, DS2450_CONVERT, DS2450_CONVERSION_CHANNEL_SELECT_MASK, DS2450_CONVERSION_READOUT_CONTROL) );
		//		printDebug_p(debugLevelEventDebug, debugSystemOWIADC, __LINE__, PSTR(__FILE__), PSTR("crc16 checksum 4 bytes 0x%x ?"), owiComputeCRC16(0x0000, 4, OWI_ROM_SKIP, DS2450_CONVERT, DS2450_CONVERSION_CHANNEL_SELECT_MASK, DS2450_CONVERSION_READOUT_CONTROL) );
		//		printDebug_p(debugLevelEventDebug, debugSystemOWIADC, __LINE__, PSTR(__FILE__), PSTR("crc16 checksum 3 bytes 0x%x ?"), owiComputeCRC16(0xFFFF, 3, DS2450_CONVERT, DS2450_CONVERSION_CHANNEL_SELECT_MASK, DS2450_CONVERSION_READOUT_CONTROL) );
		//		printDebug_p(debugLevelEventDebug, debugSystemOWIADC, __LINE__, PSTR(__FILE__), PSTR("crc16 checksum 4 bytes 0x%x ?"), owiComputeCRC16(0xFFFF, 4, OWI_ROM_SKIP, DS2450_CONVERT, DS2450_CONVERSION_CHANNEL_SELECT_MASK, DS2450_CONVERSION_READOUT_CONTROL) );


		if ( RESULT_OK == result )
		{
			break;
		}
		else
		{
			if (OWI_SEND_BYTE_MAX_TRIALS != trialsCounter)
			{
				printDebug_p(debugLevelEventDebug, debugSystemOWIADC, __LINE__, PSTR(__FILE__), PSTR("CRC16 check send byte failed - trial no. %i, computed 0x%x != received 0x%x"), trialsCounter);

				/* ending the Convert command sequence in any case by issuing a Reset Pulse*/
				OWI_DetectPresence(currentPins); /*the "DetectPresence" function includes sending a Reset Pulse*/
				continue;
			}
			else
			{
				CommunicationError_p(ERRG, dynamicMessage_ErrorIndex, TRUE, PSTR("CRC16 check reached max trials (%i) on send byte"), OWI_SEND_BYTE_MAX_TRIALS);
				result = RESULT_FAILURE;
				break;
			}
		}
	}

	//loop that waits for the conversion to be done
	if ( RESULT_OK == result )
	{
		result = RESULT_OK;
		while ( OWI_ReadBit(currentPins) == 0 )
		{
			_delay_ms(OWI_ADC_CONVERSION_DELAY_MILLISECONDS);

			/* timeout check */
			if ( 0 == --count)
			{
				timeout_flag = TRUE;
				result = RESULT_FAILURE;
				break;
			}
		}
		printDebug_p(debugLevelEventDebug, debugSystemOWIADC, __LINE__, PSTR(__FILE__), PSTR("waited %i times a delay of %i ms"), maxcount - count, OWI_ADC_CONVERSION_DELAY_MILLISECONDS);

		/* ending the Convert command sequence in any case by issuing a Reset Pulse*/
		OWI_DetectPresence(currentPins); /*the "DetectPresence" function includes sending a Reset Pulse*/
	}

	/* post conversion status analysis*/
	if (RESULT_OK == result)
	{
		owiAdcTimeoutAndFailureBusMask &= ~(currentPins);
		result = RESULT_OK;
		printDebug_p(debugLevelEventDebug, debugSystemOWIADC, __LINE__, PSTR(__FILE__), PSTR("OWI ADC Conversion failed (bus_pattern: 0x%x)"), currentPins);
	}
	else
	{
		result = RESULT_FAILURE;

		if ( NULL == id )
		{
			owiAdcTimeoutAndFailureBusMask |= currentPins;
			if ( FALSE != timeout_flag )
			{
				CommunicationError_p(ERRG, dynamicMessage_ErrorIndex, TRUE, PSTR("OWI ADC Conversion timeout (bus_pattern: 0x%x"), currentPins);
				printDebug_p(debugLevelEventDebug, debugSystemOWIADC, __LINE__, PSTR(__FILE__), PSTR("OWI Adc Conversion timeout (>%i ms) on bus_mask (0x%x)"),  maxConversionTime, currentPins);
			}
			CommunicationError_p(ERRG, dynamicMessage_ErrorIndex, TRUE, PSTR("OWI ADC Conversion failed (bus_pattern: 0x%x)"), currentPins);
		}
		else
		{
			owiCreateIdString(owi_id_string, id);
			if ( FALSE != timeout_flag )
			{
				CommunicationError_p(ERRG, dynamicMessage_ErrorIndex, TRUE, PSTR("OWI ADC Conversion timeout (id: %s"), owi_id_string);
				printDebug_p(debugLevelEventDebug, debugSystemOWIADC, __LINE__, PSTR(__FILE__), PSTR("OWI Adc Conversion timeout (>%i ms) (id: %s)"),  maxConversionTime, owi_id_string);
			}
			CommunicationError_p(ERRG, dynamicMessage_ErrorIndex, TRUE, PSTR("OWI ADC Conversion failed (id: %s)"), owi_id_string);
		}
	}

	return result;
}

/*
 *this function gets the ADC-value of all channels of one device
 */
uint32_t owiReadChannelsOfSingleADCs( unsigned char bus_pattern, unsigned char * id, uint16_t *array_chn, const int8_t size )
{
	static const uint8_t maxTrials = 3;
	uint8_t channelIndex = 0;
	uint16_t CRC;
	uint8_t flag = FALSE;
	uint8_t trialsCounter = maxTrials;
	uint32_t returnValue;
	uint8_t loopTurn = 0;
	uint16_t channelArray[DS2450_TRIPLE_CONVERSION_MAX_LOOP_TURN][4];
	uint8_t maxLoopTurn;
	/* 0 trials, give it a chance */
	if (0 == trialsCounter) { trialsCounter++;}
	if (TRUE == owiAdcTripleReadout)
	{
		maxLoopTurn = DS2450_TRIPLE_CONVERSION_MAX_LOOP_TURN;
	}
	else
	{
		maxLoopTurn = 1;
	}

	/*checks*/

	printDebug_p(debugLevelEventDebug, debugSystemOWIADC, __LINE__, PSTR(__FILE__), PSTR("begin"));

	if ( 0 == ((owiBusMask & bus_pattern) & 0xFF) )
	{
		printDebug_p(debugLevelEventDebug, debugSystemOWIADC, __LINE__, PSTR(__FILE__), PSTR("passive (bus pattern 0x%x owiBusMask 0x%x)"), bus_pattern,owiBusMask);
		return ((uint32_t) owiReadStatus_owi_bus_mismatch) << OWI_ADC_DS2450_MAX_RESOLUTION;
	}

	if ( 0 != ((owiAdcTimeoutAndFailureBusMask & bus_pattern) & 0xFF) )
	{
		//conversion went into timeout
		owiCreateIdString(owi_id_string, id);
		CommunicationError_p(ERRG, dynamicMessage_ErrorIndex, TRUE, PSTR("OWI ADC Conversion Error id: %s (bus_pattern 0x%x)"), owi_id_string, bus_pattern);

		return ((uint32_t) owiReadStatus_conversion_timeout) << OWI_ADC_DS2450_MAX_RESOLUTION;
	}
#warning TODO: consider the case that bus_pattern has more than one bit active, but the conversion failed/succeeded not on all the same way

	for ( loopTurn = 0; loopTurn < maxLoopTurn; loopTurn++)
	{
		/* Reset, presence */
		if ( 0 == OWI_DetectPresence(bus_pattern) )
		{
			return ((uint32_t) owiReadStatus_no_device_presence) << OWI_ADC_DS2450_MAX_RESOLUTION; // Error
		}

		/* Send READ MEMORY command
		 *
		 * READ MEMORY [AAH]
		 *
		 * The Read Memory command is used to read conversion results, control/status data and alarm settings.
		 * The bus master follows the command byte with a two byte address (TA1=(T7:T0), TA2=(T15:T8)) that
		 * indicates a starting byte location within the memory map.
		 *
		 * With every subsequent read data time slot the bus master receives data from the DS2450
		 * starting at the supplied address and continuing until the end of
		 * an eight-byte page is reached. At that point the bus master will receive a 16-bit CRC of the command byte,
		 * address bytes and data bytes. This CRC is computed by the DS2450 and read back by the bus master to check
		 * if the command word, starting address and data were received correctly. If the CRC read by the bus master
		 * is incorrect, a Reset Pulse must be issued and the entire sequence must be repeated.
		 *
		 * Note that the initial pass through the Read Memory flow chart will generate a 16-bit CRC value that is the
		 * result of clearing the CRC-generator and then shifting in the command byte followed by the two address
		 * bytes, and finally the data bytes beginning at the first addressed memory location and continuing through
		 * to the last byte of the addressed page. Subsequent passes through the Read Memory flow chart will
		 * generate a 16-bit CRC that is the result of clearing the CRC-generator and then shifting in the new data
		 * bytes starting at the first byte of the next page.
		 *
		 * (http://datasheets.maximintegrated.com/en/ds/DS2450.pdf)
		 * */

		while ( FALSE == flag && trialsCounter != 0)
		{
			/* Match id found earlier*/
			OWI_MatchRom(id, bus_pattern); // Match id found earlier

#warning TODO: is the CRC check described above done here? if this sequence is repeated implement a timeout counter

			OWI_SendByte(DS2450_READ_MEMORY, bus_pattern);
			/* set starting address for memory read */
			OWI_SendWord(DS2450_ADDRESS_MEMORY_MAP_PAGE_0_CONVERSION_READOUT_INPUT_A_LSB, bus_pattern); //Send two bytes address (ie: 0x00 & 0x00,0x08 & 0x00,0x10 & 0x00,0x18 & 0x00)

			for (channelIndex = 0; channelIndex < 4; channelIndex++)
			{
				// Read a word place it in the 16 bit channel variable.
				channelArray[loopTurn][channelIndex] = OWI_ReceiveWord(bus_pattern);
			}

			/* Receive CRC */
			CRC = OWI_ReceiveWord(bus_pattern);

			/* Check CRC */
			flag = TRUE; /*Pseudo check*/
#if 0
			if ( checkCRC(...))
			{
				flag = TRUE;
			}
			else
			{
				trialsCounter--;
				OWI_DetectPresence(bus_pattern);
			}
#endif
		}

		OWI_DetectPresence(bus_pattern);

		if (FALSE == flag) /*error*/
		{
			returnValue = ((uint32_t) 1 ) | (((uint32_t)owiReadWriteStatus_MAXIMUM_INDEX) << OWI_ADC_DS2450_MAX_RESOLUTION);
			break;
		}
		/*re-convert channel*/
//		owiADCConvert(bus_pattern, id);
		owiADCConvert(bus_pattern, 0);
	}

	if ( TRUE == flag )
	{
		clearString_p(resultString, BUFFER_SIZE);

		int32_t a, b, c;
		uint32_t m=0;
		char value[7];
		for (uint8_t channel = 0; channel < 4; channel++ )
		{
			if ( FALSE == owiAdcTripleReadout )
			{
				m = channelArray[0][channel];
			}
			else /*triple readout, take the average of the two closest values*/
			{
				a = channelArray[0][channel];
				b = channelArray[1][channel];
				c = channelArray[2][channel];

				if ( abs(a-b) <= abs(a-c) )
				{
					if ( abs(a-b) <= abs(b-c) )
					{
						m = (a+b)/2;
					}
					else
					{
						m = (b+c)/2;
					}
				}
				else
				{
					if ( abs(a-c) <= abs(b-c) )
					{
						m = (a+c)/2;
					}
					else
					{
						m = (b+c)/2;
					}
				}
			}

			snprintf(value, 7 - 1, " %.4lX", m);
			strncat(resultString, value, BUFFER_SIZE - 1);
		}
		if ( TRUE == owiAdcTripleReadout)
		{
			for ( loopTurn = 0; loopTurn < DS2450_TRIPLE_CONVERSION_MAX_LOOP_TURN; loopTurn++)
			{
				for (uint8_t channel = 0; channel < 4; channel++ )
				{
					snprintf(value, 7 - 1, " %.4X", channelArray[loopTurn][channel]);
					strncat(resultString, value, BUFFER_SIZE - 1);
				}
			}
		}
		printDebug_p(debugLevelEventDebug, debugSystemOWIADC, __LINE__, PSTR(__FILE__), PSTR("retrieved data and end"));
		returnValue = ((uint32_t) 0 ) | (((uint32_t)owiReadWriteStatus_OK) << OWI_ADC_DS2450_MAX_RESOLUTION);
	}

	return returnValue;
}//END of owiReadChannelsOfSingleADCs function


