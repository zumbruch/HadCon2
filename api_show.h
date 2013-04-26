/*
 * api-show.h
 *
 *  Created on: Apr 12, 2010
 *      Author: P.Zumbruch@gsi.de
 */

#ifndef APISHOW_H_
#define APISHOW_H_
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
extern const char* commandShowKeywords[] PROGMEM;
enum cmdShowKeyNumber
{
      commandShowKeyNumber_UNUSED_MEM_START = 0,
      commandShowKeyNumber_UNUSED_MEM_NOW ,
      commandShowKeyNumber_FREE_MEM_NOW ,
      commandShowKeyNumber_MEM ,
      commandShowKeyNumber_ALL_ERRORS ,
      commandShowKeyNumber_RESET_SOURCE ,
      commandShowKeyNumber_WATCHDOG_COUNTER,
      commandShowKeyNumber_MAXIMUM_NUMBER
};

int8_t show(struct uartStruct *ptr_uartStruct);
/*void showReceiveHeader(char *string, struct uartStruct * ptr_uartStruct, uint8_t index);*/
int8_t showMem(struct uartStruct * ptr_uartStruct, uint8_t index);

int8_t showFreeMemNow(struct uartStruct * ptr_uartStruct);
int8_t showUnusedMemNow(struct uartStruct * ptr_uartStruct);
int8_t showUnusedMemStart(struct uartStruct * ptr_uartStruct);
/*void help_show(void);*/
void showErrors(struct uartStruct * ptr_uartStruct, uint8_t index);
void showResetSource(uint8_t startup_flag);
void showWatchdogIncarnationsCounter(uint8_t startup_flag);

#endif /* APISHOW_H_ */
