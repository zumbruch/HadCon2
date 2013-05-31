/*
 * spi.h
 *
 *  Created on: 23.05.2013
 *      Author: Peter Zumbruch, GSI, P.Zumbruch@gsi.de
 */

#ifndef SPI_H_
#define SPI_H_


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


#endif /* SPI_H_ */
