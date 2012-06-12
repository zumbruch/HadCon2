/* The one_wire_ADC.h is a header file for the functions specific to the analog to digital converter via one_wire bus*/
#ifndef ONE_WIRE__H
#define ONE_WIRE__H
#include "api_define.h"
#include "api_global.h"

#include "api_debug.h"#include "api.h"
#include "can.h"

/* structure for owi arguments */
struct owiStruct
{
      uint8_t id[8]; /* holds the 1-wire ID*/
      uint8_t idSelect_flag; /* flag indicating id is not empty */
      void* ptr_value;  /* pointer to the value */
      uint16_t value;  /* value to be written/stored */
      uint8_t conv_flag; /* request conversion before reading */
      uint8_t init_flag; /* request initialization before reading */
      char command[MAX_LENGTH_PARAMETER]; /* holds command string*/
};

extern struct owiStruct owiFrame;
extern struct owiStruct *ptr_owiStruct;

#define OWI_ID_LENGTH 17 /*(16+1)*/
extern char owi_id_string[OWI_ID_LENGTH];

uint8_t owiInitOwiStruct(struct owiStruct *ptr_owiStruct);
uint8_t ConvertUartDataToOwiStruct(void);
void owiDualSwitches( struct uartStruct *ptr_uartStruct );
int16_t owiFindIdAndGetIndex(uint8_t id[]);

int8_t owiReadDevicesID(uint8_t *pins);/*this function list of all devices found in the one wire bus*/

int8_t owiFindFamilyDevicesAndAccessValues( uint8_t *pins, uint8_t countDev, uint8_t familyCode, void *ptr_value);

void list_all_devices( struct uartStruct *ptr_uartStruct );/* this function contains all the functions that are necessary to
 list ID on all devices connected to one wire bus*/

void setOneWireBusMask( struct uartStruct *ptr_uartStruct );

uint32_t getOneWireBusMask( struct uartStruct *ptr_uartStruct );

uint8_t owiScanIDS(uint16_t id, uint16_t *mask);

uint8_t owiCheckReadWriteReturnStatus( uint32_t status );

int8_t owiShowDevicesID( struct uartStruct* ptr_myuartStruct);

uint8_t checkBusAndDeviceActivityMasks(uint8_t pins, int8_t busPatternIndex, uint16_t owiBusMask, uint16_t owiDeviceMask, uint8_t verbose);

uint8_t generateCommonPinsPattern(uint8_t *pins, const uint16_t owiBusMask, const uint16_t owiDeviceMask);

uint16_t isParameterIDThenFillOwiStructure(uint8_t parameterIndex);


enum owiReadWriteStatus
{
   owiReadWriteStatus_OK = 0x0,
   owiReadStatus_owi_bus_mismatch = 0x1,
   owiReadStatus_conversion_timeout = 0x2,
   owiReadStatus_no_device_presence = 0x4,
   owiWriteStatus_Timeout = 0x8,
   owiReadWriteStatus_MAXIMUM_INDEX
};

/* Structure for OWI IDs commands */
//struct owiIdStruct
//{
//    uint8_t owi_IDs[8];
//    uint8_t busPin;
//};
//
//extern struct owiIdStruct owiIDs[NUM_DEVICES];
//extern struct owiIdStruct *ptr_owiIDs[NUM_DEVICES];

#endif
