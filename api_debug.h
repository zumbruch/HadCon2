/*
 * api_debug.h
 *
 *  Created on: Jul 7, 2011
 *      Author: Peter Zumbruch, GSI, P.Zumbruch@gsi.de
 *      Modified: Peter Zumbruch, July 2011
 */

#ifndef API_DEBUG_H_
#define API_DEBUG_H_
#include "api.h"
#include <stdint.h>

struct uartStruct;

extern uint8_t debug;
extern uint32_t debugMask;

enum debugLevels
{
   noDebug = 0,
   verboseDebug,
   eventDebug,
   eventDebugVerbose,
   periodicDebug,
   levelsDebug_MAXIMUM_INDEX
};

enum debugSystems
{
      debugMain,
      debugApi,
      debugApiMisc,
      debugUART,
      debugCAN,
      debugADC,
      debugRELAY,
      debugCommandKey,
      debugOWI,
      debugOWITemperatures,
      debugDecrypt,
      debugDEBUG,
      debugOWIADC,
      debugOWIDualSwitches,
      debugOWISingleSwitches,
      debugOWIOctalSwitches,
      debugSHOW,
      debugOWIApiSettings,
      debugTIMER0,
      debugTIMER1,
      debugTIMER0A,
      debugTIMER0AScheduler,
      debugTWI,
      debugSystems_MAXIMUM_INDEX
};

extern const char* commandDebugKeywords[] PROGMEM;

enum cmdDebugKeyNumber
{
      commandDebugKeyNumber_ALL = 0,
      commandDebugKeyNumber_LEVEL,
      commandDebugKeyNumber_MASK,
      commandDebugKeyNumber_HELP,
      commandDebugKeyNumber_MAXIMUM_NUMBER
};

int8_t apiDebug(struct uartStruct *ptr_uartStruct);
int8_t apiDebugSubCommands(struct uartStruct *ptr_uartStruct, int16_t subCommandIndex);
int8_t apiDebugSetDebugLevel(uint32_t value);
int8_t apiDebugSetDebugMask(uint32_t value);

void readModifyDebugLevelAndMask( struct uartStruct *ptr_uartStruct ); /* set/get debugLevel/debugMask*/
void readModifyDebugLevel( struct uartStruct *ptr_uartStruct ); /* set/get debugLevel*/
void readModifyDebugMask( struct uartStruct *ptr_uartStruct ); /* set/get debugMask*/

#endif /* API_DEBUG_H_ */
