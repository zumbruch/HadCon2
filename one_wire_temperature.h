/* The one_wire_temperatur.h is a header file for the functions specific to read temperature of sensor via one wire bus*/
#ifndef ONE_WIRE_TEMPERATURE__H
#define ONE_WIRE_TEMPERATURE__H
#include "api_define.h"
#include "api_global.h"

#include "api_debug.h"#include "api.h"

#define DS1820_START_CONVERSION         0x44
#define DS1820_READ_SCRATCHPAD          0xbe
#define DS1820_WRITE_SCRATCHPAD         0x4E
#define DS1820_CONFIG_REGISTER          0x7f
#define DS1820_READ_POWER_SUPPLY        0xb4

extern uint16_t owiTemperatureTimeoutMask;
extern uint8_t owiUseCommonTemperatureConversion_flag;
extern uint8_t owiTemperatureConversionGoingOnCountDown;
extern uint8_t owiTemperatureIgnoreConversionResponse;
extern uint16_t owiTemperatureMaxConversionTime;
extern uint8_t owiTemperatureParasiticModeMask;
extern uint8_t owiTemperatureParasiticModeMask;
extern uint8_t owiTemperatureResolution_DS18B20;
extern uint8_t owiTemperatureSensorBySensorParasiticConversionFlag;

void owiTemperatureSensors( struct uartStruct *ptr_uartStruct );
void owiTemperatureReadSensors( struct uartStruct *ptr_uartStruct, uint8_t convert );/* read the temperature of sensor*/
uint8_t owiTemperatureReadSensorsCheckCommandSyntax(struct uartStruct *ptr_uartStruct);
uint32_t owiTemperatureReadSingleSensor( unsigned char bus, unsigned char * id );/* read the data of the scratchpad of device ID */
uint16_t owiTemperatureGetNumberOfDevicesAndSetTemperatureMask(struct uartStruct *ptr_uartStruct);

int owiTemperatureConversions( uint8_t *pins, uint8_t waitForResult, uint8_t spawnReleaseBlock );
void owiTemperatureConversionParasitic( unsigned char pins );
uint8_t owiTemperatureConversionEvaluateTimeoutFlag(const unsigned char timeout_flag, const uint8_t currentPins,
                                            const uint32_t maxcount, const uint32_t count, uint16_t *currentTimeoutMask,
                                            const uint16_t maxConversionTime);
void owiTemperatureSensorBySensorParasiticConversion(uint8_t currentPins, uint16_t selectedDeviceIndex);

void owiTemperatureMiscSubCommands( struct uartStruct *ptr_uartStruct );
void owiTemperatureMiscSubCommandGetSetForceParasiticMode(struct uartStruct *ptr_uartStruct);
void owiTemperatureMiscSubCommandGetSetMaxConversionTime(struct uartStruct *ptr_uartStruct);
void owiTemperatureMiscSubCommandGetSetFlag(struct uartStruct *ptr_uartStruct, uint8_t *flag, const prog_char *text, uint8_t invert);
void owiTemperatureMiscSubCommandGetSetResolution_DS18B20(struct uartStruct *ptr_uartStruct);
void owiTemperatureMiscSubCommandConvertOnly(struct uartStruct *ptr_uartStruct);
void owiTemperatureMiscSubCommandGetSetStepByStepParasiticConversion(struct uartStruct *ptr_uartStruct);

enum errorCodes
{
   eNoError = 0,
   eWrongCANAddress = 1, // bogus address set on hex-switches
   eCANDataSize = 2, // bogus amount of bytes arrived
   eI2CTimeout = 4, // I²C problem
   eI2CFatalError = 8, // I²C problem
   eI2CRetryFailed = 16, // I²C problem
   eADCTimeout = 32, // ADC conversion failed
   eCPLDAcknowledgeTimeout = 64, // CPLD communication problem
   eADCwrongAddress = 128
} ErrorCodes;

extern const char* owiTemperatureCommandKeywords[] PROGMEM;
enum owiTemperatureCommandKeyNumber
{
      owiTemperatureCommandKeyNumber_CONVERT_ONLY = 0,
      owiTemperatureCommandKeyNumber_NO_CONVERT,
      owiTemperatureCommandKeyNumber_IGNORE_CONVERSION_RESPONSE,
      owiTemperatureCommandKeyNumber_MAX_CONVERSION_TIME_MS,
      owiTemperatureCommandKeyNumber_FORCE_PARASITIC_MODE,
      owiTemperatureCommandKeyNumber_RESOLUTION_DS18B20,
      owiTemperatureCommandKeyNumber_SINGLE_SENSOR_PARSITIC_CONVERSION,
      owiTemperatureCommandKeyNumber_MAXIMUM_NUMBER
};

#endif
