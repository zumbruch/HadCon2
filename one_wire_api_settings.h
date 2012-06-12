/*
 * one_wire_api_settings.h
 *
 *  Created on: May 3, 2010
 *      Author: zumbruch
 */

#ifndef ONE_WIRE_API_SETTINGS_H_
#define ONE_WIRE_API_SETTINGS_H_

#include <avr/pgmspace.h>
#include "api_define.h"
#include "api_global.h"

#include "api_debug.h"#include "api.h"

/*
typedef struct {
    int8_t (*func)(struct uartStruct);
    char* commandShowKeyword;
} showCommand_t;

extern const showCommand_t showCommands[] PROGMEM;
*/
extern unsigned short unusedMemoryStart;
extern const char* owiApiCommandKeywords[] PROGMEM;
enum owiApiCommandKeyNumber
{
      owiApiCommandKeyNumber_COMMON_TEMPERATURE_CONVERSION = 0,
      owiApiCommandKeyNumber_COMMON_ADC_CONVERSION,
      owiApiCommandKeyNumber_MAXIMUM_NUMBER
};

int8_t owiApi(struct uartStruct *ptr_uartStruct);
int8_t owiApiFlag(struct uartStruct * ptr_uartStruct, uint8_t index);

#endif /* ONE_WIRE_API_SETTINGS_H_ */
