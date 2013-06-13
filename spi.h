/*
 * spi.h
 *
 *  Created on: 23.05.2013
 *      Author: Peter Zumbruch, GSI, P.Zumbruch@gsi.de
 */

#ifndef SPI_H_
#define SPI_H_

#include "api_define.h"

#define SPI_SET TRUE
#define SPI_RELEASE FALSE
#define SPI_MAX_DATA_LENGTH 32
#define MAX_CS_CHANNELS 8

typedef struct spiByteArrayStruct
{
	uint8_t data[ MAX_LENGTH_COMMAND >> 1 ];
	uint16_t length;
} spiByteDataArray;

extern spiByteDataArray spiWriteData;
extern spiByteDataArray spiReadData;

typedef struct spiConfigStruct
{
	/* Register Bits [SPSR]  */
	/* SPI Status Register - SPSR */
	//#define    SPIF         7
	//#define    WCOL         6
	//#define    SPI2X        0
	/* End Register Bits */

	/* Register Bits [SPCR]  */
	/* SPI Control Register - SPCR */
	//#define    SPIE         7
	//#define    SPE          6
	//#define    DORD         5
	//#define    MSTR         4
	//#define    CPOL         3
	//#define    CPHA         2
	//#define    SPR1         1
	//#define    SPR0         0
	/* End Register Bits */

} spiConfig;

extern spiConfig spi_set_configuration;
extern spiConfig spi_current_status;

enum spi_chips
{
	CHIP1 = 0,
	CHIP2,
	CHIP3,
	CHIP_MAXIMUM
};

typedef struct spiPinStruct
{
	uint8_t registerAdress;
	uint8_t pinNumber;
} spiPin;

//extern uint8_t spi_set_control;
//extern uint8_t spi_set_status;

void spi_init(void);
void spi_enable(uint8_t spi_enable_flag);
void spi_set_chosen_chipSelect(uint8_t new_status, uint8_t chipSelect);
void spi_set_chipSelect_Register_Adresses(spiPin pins[MAX_CS_CHANNELS]);
void spi_set_chipSelect_Register_Mask(uint8_t mask);

void spi_write_without_cs      (uint8_t *ptr_data,    uint8_t data_length);
void spi_write_with_cs         (uint8_t *ptr_data,    uint8_t data_length, uint8_t chipSelect);
uint8_t spi_read_byte(void);

void spi_write_and_read_with_cs(void);
void spi_write_and_read_without_cs(void);

void set_control(uint16_t);
uint16_t get_control(void);

void spi_purge_write_data(void);
void spi_purge_read_data(void);

void spiWrite(void);
void spiRead(void);
void spiInit(void);
void spiEnable(void);

inline uint16_t spi_add_write_data(uint8_t value)
{
	spiWriteData.data[spiWriteData.length] = value;
	spiWriteData.length++;
	return spiWriteData.length;
}

#endif /* SPI_H_ */
