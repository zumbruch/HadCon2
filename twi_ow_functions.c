#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include "twi_master.h"
#include "twi_ow_functions.h"

void Twim_Owi_Single_Bit_High(uint8_t Device)
{
	uint8_t status = 0;
	twi_data[0] = TWI_OWI_OW_SINGLE_BIT;
	twi_data[1] = TWI_OWI_BIT_BYTE_HIGH;
	twi_bytes_to_transceive = 2;
	status = Twim_Write_Data(Device, twi_bytes_to_transceive, twi_data);

	if ( TWI_success != status )
	{
		TWI_errorAnalysis(status);
	}
}

void Twim_Owi_Single_Bit_Low(uint8_t Device)
{
	uint8_t status = 0;
	twi_data[0] = TWI_OWI_OW_SINGLE_BIT;
	twi_data[1] = TWI_OWI_BIT_BYTE_LOW;
	twi_bytes_to_transceive = 2;
	status = Twim_Write_Data(Device, twi_bytes_to_transceive, twi_data);

	if ( TWI_success != status )
	{
		TWI_errorAnalysis(status);
	}
}

void Twim_Owi_Reset_Device(uint8_t Device)
{
	uint8_t status = 0;
	twi_data[0] = TWI_OWI_DEVICE_RESET;
	twi_bytes_to_transceive = 1;
	status = Twim_Write_Data(Device, twi_bytes_to_transceive, twi_data);

	if ( TWI_success != status )
	{
		TWI_errorAnalysis(status);
	}
}

uint8_t Twim_Owi_Set_Configuration(uint8_t Device, uint8_t configuration_nibble)
{
	uint8_t status = 0;
	twi_data[0] = TWI_OWI_WRITE_CONFIGURATION;
	twi_data[1] = configuration_nibble | (((~configuration_nibble)&0x0F)<<4); // upper nibble has to be the one's complement of the lower nibble
	twi_bytes_to_transceive = 2;
	status = Twim_Write_Data(Device, twi_bytes_to_transceive, twi_data);

	if ( TWI_success != status )
	{
		TWI_errorAnalysis(status);
	}

	// Read back the configuration register to validate writing process
	twi_bytes_to_transceive = 1;
	status = Twim_Read_Data(Device, twi_bytes_to_transceive, twi_data);

	if ( TWI_success != status )
	{
		TWI_errorAnalysis(status);
	}

	if(twi_data[0] != configuration_nibble)
	{
		return FALSE;
	}
	else {
		return TRUE;
	}
}

void Twim_Owi_Read_Status(uint8_t Device)
{
	uint8_t status = 0;
	twi_bytes_to_transceive = 2;
	twi_data[0] = TWI_OWI_SET_READ_POINTER;
	twi_data[1] = TWI_OWI_STATUS_REGISTER;
	status = Twim_Write_Data(Device, twi_bytes_to_transceive, twi_data);

	if ( TWI_success != status )
	{
		TWI_errorAnalysis(status);
	}

	twi_bytes_to_transceive = 1;
	status = Twim_Read_Data(Device, twi_bytes_to_transceive, twi_data);

	if ( TWI_success != status )
	{
		TWI_errorAnalysis(status);
	}
}

void Twim_Owi_Ow_Reset(uint8_t Device)
{
	uint8_t status = 0;
	twi_bytes_to_transceive = 1;
	twi_data[0] = TWI_OWI_OW_RESET;
	status = Twim_Write_Data(Device, twi_bytes_to_transceive, twi_data);

	if ( TWI_success != status )
	{
		TWI_errorAnalysis(status);
	}
}

uint8_t Twim_Owi_Busy_Return(uint8_t Device)
{
	Twim_Owi_Read_Status(Device);
	return (twi_data[0] & 0x01); // return busy flag, 1 == busy
}
