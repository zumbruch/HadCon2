/*
 * spi.h
 *
 *  Created on: 23.05.2013
 *      Author: Peter Zumbruch, GSI, P.Zumbruch@gsi.de
 */

#ifndef SPI_H_
#define SPI_H_

#include "api_define.h"

typedef struct spiByteArrayStruct
{
	uint8_t data[ MAX_LENGTH_COMMAND >> 1 ];
	size_t length;
} spiByteDataArray;

extern spiByteDataArray spiWriteData;

typedef struct spiConfigStruct
{


} spiConfig;

extern spiConfig spiConfiguration;

void spiWrite(void);
void spiRead(void);
void spiInit(void);
void spiEnable(void);



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

#define SPI_SET TRUE
#define SPI_RELEASE FALSE
#define SPI_MAX_DATA_LENGTH 32

enum spi_chips { CHIP1 = 1, CHIP2, CHIP3 };

extern uint8_t spi_tx_data[SPI_MAX_DATA_LENGTH];
extern uint8_t spi_rx_data[SPI_MAX_DATA_LENGTH];
extern uint8_t spi_data_length;
extern uint8_t spi_control;
extern uint8_t spi_status;


void spi_init(void);
void spi_enable(uint8_t spi_enable_flag);
void spi_write_without_cs(uint8_t *ptr_data, uint8_t data_length);
uint8_t spi_read_byte(void);
void spi_set_chipSelect(uint8_t new_status, uint8_t chipSelect);
void spi_write_with_cs(uint8_t *ptr_data, uint8_t data_length, uint8_t chipSelect);
void spi_write_and_read_with_cs(uint8_t *ptr_tx_data, uint8_t data_length, uint8_t *ptr_rx_data, uint8_t chipSelect);


#endif /* SPI_H_ */
