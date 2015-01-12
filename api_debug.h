/*
   Licensed under the EUPL V.1.1, Lizenziert unter EUPL V.1.1 
*/
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

extern uint8_t globalDebugLevel;
extern uint32_t globalDebugSystemMask;

enum debugLevels
{
   debugLevelNoDebug = 0,
   debugLevelVerboseDebug,
   debugLevelEventDebug,
   debugLevelEventDebugVerbose,
   debugLevelPeriodicDebug,
   debugLevelPeriodicDebugVerbose,
   debugLevel_MAXIMUM_INDEX
};

enum debugSystems
{
      debugSystemMain,
      debugSystemApi,
      debugSystemApiMisc,
      debugSystemUART,
      debugSystemCAN,
      debugSystemADC,
      debugSystemRELAY,
      debugSystemCommandKey,
      debugSystemOWI,
      debugSystemOWITemperatures,
      debugSystemDecrypt,
      debugSystemDEBUG,
      debugSystemOWIADC,
      debugSystemOWIDualSwitches,
      debugSystemOWISingleSwitches,
      debugSystemOWIOctalSwitches,
      debugSystemSHOW,
      debugSystemOWIApiSettings,
      debugSystemTIMER0,
      debugSystemTIMER1,
      debugSystemTIMER0A,
      debugSystemTIMER0AScheduler,
      debugSystemTWI,
      debugSystemSPI,
      debugSystem_MAXIMUM_INDEX
};

extern const char* const commandDebugKeywords[] PROGMEM;

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

void apiDebugReadModifyDebugLevelAndMask( struct uartStruct *ptr_uartStruct ); /* set/get debugLevel/debugMask*/
void apiDebugReadModifyDebugLevel( struct uartStruct *ptr_uartStruct ); /* set/get debugLevel*/
void apiDebugReadModifyDebugMask( struct uartStruct *ptr_uartStruct ); /* set/get debugMask*/

#endif /* API_DEBUG_H_ */
