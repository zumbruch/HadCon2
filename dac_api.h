#ifndef DAC_API__H
#define DAC_API__H

#define DAC_VCC		0xCE4//3300 // 3.3V * 1000

// Commands
#define MUX_ADDRESS	0x70
#define MUX_DAC_ADDRESS 0x08
#define MUX_DAC_CHANNEL 0x00


#define DAC1_ADDRESS	0x4D
#define DAC2_ADDRESS	0x4C

#define DAC_CHANNEL	0x20 // A

#define CHANNEL_COUNT 	8


#define MAX_Vin		0xFF
#define DAC_Vcc 	3300
#define DAC_MAX_CH 	7
#define DAC_Vper_bit  12.9 //(DAC_Vcc / 0x100)
// Function Prototypes

uint8_t DAC_SetUP(uint8_t port, uint16_t Vout);
uint8_t DAC_PortRead(uint8_t port, uint8_t *data);
void DAC(struct uartStruct *ptrUART);
void DAC_Read(uint8_t channel);


#endif
