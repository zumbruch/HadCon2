/*
   Licensed under the EUPL V.1.1, Lizenziert unter EUPL V.1.1 
*/
/*
 * spiTest.c
 *
 *  Created on: 18.06.2013
 *      Author: Peter Zumbruch, GSI, P.Zumbruch@gsi.de
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>

#include "api.h"
#include "spi.h"
#include "spiTest.h"

#ifdef FB_SPI_TEST
// for testing purposes
void spiTest(void)
{

  spiConfigUnion newConfig;
  newConfig = spiGetConfiguration();

  newConfig.bits.bSpi2x = 0;
  newConfig.bits.bSpr = 3;
  spiSetConfiguration(newConfig);


  spiAddChipSelect(&PORTE, PE6, SPI_CHIPSELECT1);
  spiSetChipSelectInMask(SPI_CHIPSELECT1);

  spiPurgeWriteData();

  spiAddWriteData((spiStandardConfiguration.data) & 0x00FF);
  spiAddWriteData( ((spiStandardConfiguration.data) >> 8) & 0x00FF);

  spiAddWriteData( (spiGetConfiguration().data) & 0x00FF);
  spiAddWriteData( ((spiGetConfiguration().data) >> 8) & 0x00FF);

  spiAddWriteData( spiGetChipSelectArrayStatus() );

  spiAddWriteData( spiGetInternalChipSelectMask() );

  spiWriteAndReadWithChipSelect(SPI_MSBYTE_FIRST, SPI_MASK_ALL_CHIPSELECTS);

  spiWriteData.data[0] = spiGetReadData().data[0];
  spiWriteData.data[1] = spiGetReadData().data[1];
  spiWriteData.data[2] = spiGetPinFromChipSelect(SPI_CHIPSELECT0);
  spiWriteData.data[3] = (uint8_t)spiGetPortFromChipSelect(SPI_CHIPSELECT0);
  spiWriteData.data[4] = spiGetCurrentChipSelectBarStatus();
  spiWriteData.data[5] = (uint8_t)spiGetCurrentChipSelectArray()[1].ptrPort;
  spiWriteData.data[6] = (uint8_t)spiGetCurrentChipSelectArray()[0].ptrPort;

  spiWriteData.length = spiGetReadData().length + 1;

  newConfig.bits.bSpi2x = 1;
  spiSetConfiguration(newConfig);

  spiWriteAndReadWithChipSelect(SPI_MSBYTE_FIRST, SPI_MASK_ALL_CHIPSELECTS);

  spiPurgeReadData();
}
#endif



