/*
 * apfelApi.h
 *
 *  Created on: 06.05.2014
 *      Author: Peter Zumbruch, GSI, P.Zumbruch@gsi.de
 */

#ifndef APFELAPI_H_
#define APFELAPI_H_

#include <stdbool.h>
#include "api.h"
#include "api_define.h"
#include "api_global.h"

#include "api_debug.h"
#include "apfel.h"

extern const char* const apfelApiCommandKeywords[] PROGMEM;
enum apfelApiCommandKeyNumber
{
	   apfelApiCommandKeyNumber_DAC                  = 0,
	   apfelApiCommandKeyNumber_D                    ,
	   apfelApiCommandKeyNumber_TESTPULSE            ,
	   apfelApiCommandKeyNumber_T                    ,
	   apfelApiCommandKeyNumber_AUTOCALIB            ,
	   apfelApiCommandKeyNumber_A                    ,
	   apfelApiCommandKeyNumber_AMPL                 ,
	   apfelApiCommandKeyNumber_LIST                 ,
	   apfelApiCommandKeyNumber_L                    ,
	   apfelApiCommandKeyNumber_STATUS               ,
	   apfelApiCommandKeyNumber_S                    ,
	   apfelApiCommandKeyNumber_PORT_ADDRESS_SET_ENABLE_MASK,
	   apfelApiCommandKeyNumber_PASEM,
	   apfelApiCommandKeyNumber_ENABLE_PORT_ADDRESS_SET,
	   apfelApiCommandKeyNumber_EPAS,
	   apfelApiCommandKeyNumber_DISABLE_PORT_ADDRESS_SET,
	   apfelApiCommandKeyNumber_DPAS,
	   apfelApiCommandKeyNumber_ADD_PORT_ADDRESS_SET,
	   apfelApiCommandKeyNumber_APAS,
	   apfelApiCommandKeyNumber_REMOVE_PORT_ADDRESS_SET,
	   apfelApiCommandKeyNumber_RPAS,
	   apfelApiCommandKeyNumber_APFEL_ENABLE         ,
	   apfelApiCommandKeyNumber_RESET                ,
	   apfelApiCommandKeyNumber_MAXIMUM_NUMBER
};


typedef struct apfelApiConfig
{
	//apfelConfigUnion apfelConfiguration;   /* hardware configuration */
	bool hardwareInit;
} apfelApiConfig;

extern apfelApiConfig apfelApiConfiguration;
extern apfelApiConfig* ptr_apfelApiConfiguration;

void apfelApi(struct uartStruct *ptr_uartStruct);
uint8_t apfelApiSubCommands(struct uartStruct *ptr_uartStruct, int16_t subCommandIndex, uint8_t parameterIndex);
void apfelApiShowStatus( uint8_t status[], uint8_t size );
void apfelApiShowStatusApiSettings(void);

void apfelApiInit(void);

apiCommandResult apfelApiSubCommandShowStatus              (void);
apiCommandResult apfelApiSubCommandDac                     (void);
apiCommandResult apfelApiSubCommandTestPulse               (void);
apiCommandResult apfelApiSubCommandAutoCalib               (void);
apiCommandResult apfelApiSubCommandAmplification           (void);
apiCommandResult apfelApiSubCommandListIds                 (void);
apiCommandResult apfelApiSubCommandChipIdIgnoreMask        (void);
apiCommandResult apfelApiSubCommandAddPortAddressSet       (void);
apiCommandResult apfelApiSubCommandRemovePortAddressSet    (void);
apiCommandResult apfelApiSubCommandApfelEnable             (void);
apiCommandResult apfelApiSubCommandReset                   (void);
apiCommandResult apfelApiSubCommandPortAddressSetEnableMask(void);
apiCommandResult apfelApiSubCommandEnablePortAddressSet    (void);
apiCommandResult apfelApiSubCommandDisablePortAddressSet   (void);

#endif /* APFELAPI_H_ */
