/*
   Licensed under the EUPL V.1.1, Lizenziert unter EUPL V.1.1 
*/
#include <api.h>

/* TWI addresses of OW-Masters */
#define TWI_OWI_DEVICE_0_ADDRESS 0x18	// DS2482-101 with AD0 pin low
#define TWI_OWI_DEVICE_1_ADDRESS 0x19	// DS2482-101 with AD0 pin high

/* Command codes for the OWI-devices */
#define TWI_OWI_DEVICE_RESET		0xf0			// Global reset, terminates owi communication. Configuration register 0x00 after reset.
#define TWI_OWI_SET_READ_POINTER	0xe1		// Sets the read pointer. See pointer codes below.
#define TWI_OWI_WRITE_CONFIGURATION 0xd2	// Writes new configuration byte. When writing: upper nibble must be one's complement of the lower nibble. When read: upper nibble always 0x00.
#define TWI_OWI_OW_RESET			0xb4				// Generates a OW reset/presence-detect cycle.
#define TWI_OWI_OW_SINGLE_BIT		0x87			// Generates a single OW time slot with a bit value "V" of the passed bit byte: 0bVxxx.xxxx.
#define TWI_OWI_OW_WRITE_BYTE		0xa5			// Writes a single data byte to the OW line.
#define TWI_OWI_OW_READ_BYTE 		0x96			// Generates eight read-data time slots and stores result in the Read Data Register.
#define TWI_OWI_OW_TRIPLET 			0x78				// Geneates three time slots. See datasheet for more information.

/* Pointer codes for the OWI-devices */
#define TWI_OWI_STATUS_REGISTER 0xf0
#define TWI_OWI_READ_DATA_REGISTER 0xe1
#define TWI_OWI_CONFIGURATION_REGISTER 0xc3

/* Bit byte values for bitwise OW-transfer */
#define TWI_OWI_BIT_BYTE_HIGH	0x80;
#define TWI_OWI_BIT_BYTE_LOW	0x00;

/* Bits in configuration register of the OWI-devices */
#define TWI_OWI_CONFIG_BIT_1WS 3
#define TWI_OWI_CONFIG_BIT_SPU 2
#define TWI_OWI_CONFIG_BIT_APU 0

/* Function-declaration for TWI_OW_Functions */
void Twim_Owi_Single_Bit_High(uint8_t Device);
void Twim_Owi_Single_Bit_Low(uint8_t Device);
void Twim_Owi_Reset_Device(uint8_t Device);
uint8_t Twim_Owi_Set_Configuration(uint8_t Device, uint8_t configuration_nibble);
void Twim_Owi_Read_Status(uint8_t Device);
void Twim_Owi_Ow_Reset(uint8_t Device);
uint8_t Twim_Owi_Busy_Return(uint8_t Device);
