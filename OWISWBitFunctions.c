// This file has been prepared for Doxygen automatic documentation generation.
/*! \file ********************************************************************
*
* Atmel Corporation
*
* \li File:               OWISWBitFunctions.c
* \li Compiler:           IAR EWAAVR 3.20a
* \li Support mail:       avr@atmel.com
*
* \li Supported devices:  All AVRs.
*
* \li Application Note:   AVR318 - Dallas 1-Wire(R) master.
*                         
*
* \li Description:        Polled software only implementation of the basic 
*                         bit-level signalling in the 1-Wire(R) protocol.
*
*                         $Revision: 1.15 $
*                         $Date: 2011-12-05 15:47:08 $
****************************************************************************/

#include "OWIPolled.h"

#ifdef OWI_SOFTWARE_DRIVER

#include <stdio.h>
#include <stdarg.h>

#include <avr/io.h>
#include <avr/interrupt.h>//before changing it was inavr.h
#include <util/delay.h>

#include "OWIBitFunctions.h"
#include "twi_master.h"

#include "twi_mpx_functions.h"
#include "twi_ow_functions.h"

/* not used *
void OWI_ResetPulse(unsigned char pins)
{

    OWI_PULL_BUS_LOW(pins);
    _delay_us(100);
    _delay_us(100);
    _delay_us(100);
    _delay_us(100);
    _delay_us(100);
    _delay_us(100);
    _delay_us(100);
    _delay_us(100);
    _delay_us(100);
    
    OWI_RELEASE_BUS(pins);
    _delay_us(100);
    _delay_us(100);
    _delay_us(100);
    _delay_us(100);
    _delay_us(100);
    _delay_us(100);
    _delay_us(100);
    _delay_us(100);
    _delay_us(100);
    _delay_us(100);
    _delay_us(35);
}
*/

/*! \brief Initialization of the one wire bus(es). (Software only driver)
 *  
 *  This function initializes the 1-Wire bus(es) by releasing it and
 *  waiting until any presence sinals are finished.
 *
 *  \param  pins    A bitmask of the buses to initialize.
 */
uint8_t OWI_Init(unsigned char pins)
{
	uint8_t status = FALSE;
#if (HADCON_VERSION == 1)
    OWI_RELEASE_BUS(pins);
    // The first rising edge can be interpreted by a slave as the end of a
    // Reset pulse. Delay for the required reset recovery time (H) to be 
    // sure that the real reset is interpreted correctly.
    _delay_us(100);
    _delay_us(100);
    _delay_us(100);//DELAY H MODE
    _delay_us(100);
    _delay_us(100);
    _delay_us(100);
    _delay_us(100);
    _delay_us(100);

    status = TRUE;
	return status;
#elif (HADCON_VERSION == 2)
	uint8_t chan = TWI_MPX_CHAN5; // 5
	uint8_t address = TWI_OWI_DEVICE_0_ADDRESS; // 0
	uint8_t configuration_nibble = (0<<TWI_OWI_CONFIG_BIT_1WS)|(0<<TWI_OWI_CONFIG_BIT_SPU)|(1<<TWI_OWI_CONFIG_BIT_APU);

	for(uint8_t i = 0; i < PIN_BUS ; i++) {
		if(!(i%2) && i!=0) {
				chan++;
		}

		if(i & 0x01) {
			address = TWI_OWI_DEVICE_1_ADDRESS;
		}
		else {
			address = TWI_OWI_DEVICE_0_ADDRESS;
		}

		if( pins & (0x01 << i)) {
			Twim_Mpx_Switch_Channel(chan);
			Twim_Owi_Reset_Device(address);
			status = Twim_Owi_Set_Configuration(address, configuration_nibble);
			if(FALSE == status) {
				general_errorCode = CommunicationError(ERRG, -1001, FALSE, PSTR("Error while OWI_init()"), 101);
			}
		}
	}
	return status;
#else
	status = FALSE;
	return status;
#endif
}


/*! \brief  Write a '1' bit to the bus(es). (Software only driver)
 *
 *  Generates the waveform for transmission of a '1' bit on the 1-Wire
 *  bus.
 *
 *  \param  pins    A bitmask of the buses to write to.
 */

void OWI_WriteBit1(unsigned char pins)
{
#if (HADCON_VERSION == 1)
    uint8_t intState;
    
    // Disable interrupts.
    intState = SREG;
    cli();//cli() to disable, sei() to enable
    
    // Drive bus low and delay.
    OWI_PULL_BUS_LOW(pins);
    _delay_us(1);
    
    // Release bus and delay.
    OWI_RELEASE_BUS(pins);
    _delay_us(80);
    
    // Restore interrupts.
    SREG=intState;
#elif (HADCON_VERSION == 2)    
	uint8_t chan = TWI_MPX_CHAN5; // 5
	uint8_t address = TWI_OWI_DEVICE_0_ADDRESS; // 0

	for(uint8_t i = 0; i < PIN_BUS ; i++) {

		if(!(i%2) && i!=0)  {
			chan++;
		}

		if(i & 0x01) {
			address = TWI_OWI_DEVICE_1_ADDRESS;
		}
		else {
			address = TWI_OWI_DEVICE_0_ADDRESS;
		}

		if( pins & (0x01 << i)) {
			Twim_Mpx_Switch_Channel(chan);
			Twim_Owi_Single_Bit_High(address);
		}
	}

#else
#endif
}


/*! \brief  Write a '0' to the bus(es). (Software only driver)
 *
 *  Generates the waveform for transmission of a '0' bit on the 1-Wire(R)
 *  bus.
 *
 *  \param  pins    A bitmask of the buses to write to.
 */
void OWI_WriteBit0(unsigned char pins)
{
#if (HADCON_VERSION == 1)

    uint8_t intState;
    
    // Disable interrupts.
    intState = SREG;
    cli();
    
    // Drive bus low and delay.
    OWI_PULL_BUS_LOW(pins);
    _delay_us(70);
    
    // Release bus and delay.
    OWI_RELEASE_BUS(pins);
    _delay_us(30);

    // Restore interrupts.
    SREG=intState;
    
#elif (HADCON_VERSION == 2)
	uint8_t chan = TWI_MPX_CHAN5; // 5
	uint8_t address = TWI_OWI_DEVICE_0_ADDRESS; // 0

	for(uint8_t i = 0; i < PIN_BUS ; i++) {
		if(!(i%2) && i!=0) {
			chan++;
		}

		if(i & 0x01) {
			address = TWI_OWI_DEVICE_1_ADDRESS;
		}
		else {
			address = TWI_OWI_DEVICE_0_ADDRESS;
		}

		if( pins & (0x01 << i)) {
			Twim_Mpx_Switch_Channel(chan);
			Twim_Owi_Single_Bit_Low(address);
		}

	}

#else
#endif
}

/*! \brief  Write a '0' to the bus(es). (Software only driver)
 *
 *  Generates the waveform for transmission of a '0' bit on the 1-Wire(R)
 *  bus. just for conversion in parasitic mode (different delay)
 *
 *  \param  pins    A bitmask of the buses to write to.
 */
void OWI_WriteBit0ShortRelease(unsigned char pins)
{
    uint8_t intState;

    // Disable interrupts.
    intState = SREG;
    cli();

    // Drive bus low and delay.
    OWI_PULL_BUS_LOW(pins);
    _delay_us(70);

    // Release bus and delay.
    OWI_RELEASE_BUS(pins);
    _delay_us(1);
    
    // Restore interrupts.
    SREG=intState;
}


/*! \brief  Read a bit from the bus(es). (Software only driver)
 *
 *  Generates the waveform for reception of a bit on the 1-Wire(R) bus(es).
 *
 *  \param  pins    A bitmask of the bus(es) to read from.
 *
 *  \return A bitmask of the buses where a '1' was read.
 */
unsigned char OWI_ReadBit(unsigned char pins)
{
    unsigned char bitsRead = 0;
#if ( HADCON_VERSION == 1 )
    uint8_t intState;
    
    // Disable interrupts.
    intState = SREG;
    cli();
    
    // Drive bus low and delay.
    OWI_PULL_BUS_LOW(pins);
    _delay_us(1); // very short down!
    
    // Release bus and delay.
    OWI_RELEASE_BUS(pins);
    _delay_us(10);
    
    // Sample bus and delay.
    bitsRead = OWI_PIN & pins;
    _delay_us(80);
    
    // Restore interrupts.
    SREG = intState;

#elif ( HADCON_VERSION == 2 )
	uint8_t chan = TWI_MPX_CHAN5; // 5
	uint8_t address = TWI_OWI_DEVICE_0_ADDRESS; // 0
	uint8_t busy_counter;

	for(uint8_t i = 0; i < PIN_BUS ; i++) {
		busy_counter = 5;	// Reset busy_counter

		if(!(i%2) && i!=0) {
			chan++;
		}

		if(i & 0x01) {
			address = TWI_OWI_DEVICE_1_ADDRESS;
		}
		else {
			address = TWI_OWI_DEVICE_0_ADDRESS;
		}

		if( pins & (0x01 << i)) {
			Twim_Mpx_Switch_Channel(chan);
			Twim_Owi_Single_Bit_High(address);

			while(Twim_Owi_Busy_Return(address) &&  0 != busy_counter) {
				busy_counter--;
			} // Status Register is in twi_data[0] after this block

			if(0 == busy_counter) {
				CommunicationError_p(ERRG,-1,1,PSTR("OW read bit busy time out"), 1000);
			}

			if( 0x20 == (0x20 & twi_data[0]) ) { // read bit result?
				bitsRead = bitsRead | (0x01<<i);
			}
		}
	}

#else
#endif
    return bitsRead;
}


/*! \brief  Send a Reset signal and listen for Presence signal. (software
 *  only driver)
 *
 *  Generates the waveform for transmission of a Reset pulse on the 
 *  1-Wire(R) bus and listens for presence signals.
 *
 *  \param  pins    A bitmask of the buses to send the Reset signal on.
 *
 *  \return A bitmask of the buses where a presence signal was detected.
 */
unsigned char OWI_DetectPresence(unsigned char pins)
{
    unsigned char presenceDetected = 0;
#if ( HADCON_VERSION == 1 )
    uint8_t intState;
    
    // Disable interrupts.
    intState = SREG;
    cli();
    
    // Drive bus low and delay.
    OWI_PULL_BUS_LOW(pins);
    _delay_us(100);
    _delay_us(100);
    _delay_us(100);//DELAY H MODE
    _delay_us(100);
    _delay_us(100);
    _delay_us(100);
    _delay_us(100);
    _delay_us(100);
    _delay_us(100);

    
    // Release bus and delay.
    OWI_RELEASE_BUS(pins);
    /*    
     During the initialization sequence the bus master transmits (TX) the reset pulse by pulling the 1-Wire bus
     low for a minimum of 480μs. The bus master then releases the bus and goes into receive mode (RX).
     When the bus is released, the 5kΩ pullup resistor pulls the 1-Wire bus high. When the DS18B20 detects
     this rising edge, it waits 15μs to 60μs and then transmits a presence pulse by pulling the 1-Wire bus low
     for 60μs to 240μs.
    */
    _delay_us(65); 



    // Sample bus to detect presence signal and delay.
    presenceDetected = ((~OWI_PIN) & pins);
    _delay_us(100);
    _delay_us(100);
    _delay_us(100);
    _delay_us(100);
    _delay_us(100);
    _delay_us(100);
    _delay_us(100);
    _delay_us(100);
    
    // Restore interrupts.

    SREG = intState;
    
#elif ( HADCON_VERSION == 2 )
	uint8_t chan = TWI_MPX_CHAN5;
	uint8_t address = TWI_OWI_DEVICE_0_ADDRESS;
	uint8_t busy_counter;

	for(uint8_t i = 0; i < PIN_BUS ; i++) {
		busy_counter = 5;	// Reset busy_counter

		if(!(i%2) && i!=0) {
			chan++;
		}

		if(i & 0x01) {
			address = TWI_OWI_DEVICE_1_ADDRESS;
		}
		else {
			address = TWI_OWI_DEVICE_0_ADDRESS;
		}

		if( pins & (0x01 << i)) {
			Twim_Mpx_Switch_Channel(chan);
			Twim_Owi_Ow_Reset(address);

			while(Twim_Owi_Busy_Return(address) &&  0 != busy_counter) {
				busy_counter--;
			} // Status Register is in twi_data[0] after this block

			if(0 == busy_counter) {
				CommunicationError_p(ERRG,-1,1,PSTR("OW detect presence busy time out"), 1000);
			}

			if( 2 == (0x02 & twi_data[0]) ) { //presence detected?
				presenceDetected = presenceDetected | (0x01<<i);
			}
		}
	}

#else
#endif
    return presenceDetected;

}


#endif
