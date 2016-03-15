/*
   Licensed under the EUPL V.1.1, Lizenziert unter EUPL V.1.1
*/
#ifndef API_DAC__H
#define API_DAC__H

#define DAC_VCC		0xCE4//3300 // 3.3V * 1000

// Commands
#define DAC_I2C_MUX_ADDRESS	0x70
#define DAC_I2C_MUX_DAC_ADDRESS 0x08
#define DAC_MUX_DAC_CHANNEL 0x00


#define DAC_I2C_DAC1_ADDRESS	0x4D
#define DAC_I2C_DAC2_ADDRESS	0x4C

/* Update selected DAC with I2C data. Most commonly utilized mode. The contents of MS-BYTE and
LS-BYTE (or power down information) are stored in the temporary register and in the DAC register of
the selected channel. This mode changes the DAC output of the selected channel with the new data*/
#define DAC_I2C_DAC5574_CONTROL_BYTE	0x10

#define DAC_UNDEFINED_DATA 0x100

#define DAC_CHANNEL_COUNT 	8


#define DAC_MAX_Vin	0xFF
#define DAC_Vcc 	DAC_VCC
#define DAC_MAX_CH 	7
#define DAC_Vper_bit  12.941 //(DAC_Vcc / 0x100)
// Function Prototypes

uint8_t DAC_Write(uint8_t port, uint16_t Vout);
uint8_t DAC_PortRead(uint8_t port, uint16_t *data);
void DAC(struct uartStruct *ptrUART);
void DAC_Read(uint8_t channel);
uint8_t DAC_Init(void);
#endif
