/* TWI addresses of TW-Multiplexer */
#define TWI_MPX_ADDRESS 0x70			// PCA9547

/* Command codes for the MPX-device (switching to the appropriate channel) */
#define TWI_MPX_CHAN0 0x08
#define TWI_MPX_CHAN1 0x09
#define TWI_MPX_CHAN2 0x0a
#define TWI_MPX_CHAN3 0x0b
#define TWI_MPX_CHAN4 0x0c
#define TWI_MPX_CHAN5 0x0d
#define TWI_MPX_CHAN6 0x0e
#define TWI_MPX_CHAN7 0x0f

void Twim_Mpx_Switch_Channel(uint8_t Channel);
