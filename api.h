/* The canapi.h is a header file, the functions and structure of the variables specific to the serial interface between the CPU and micro controller.
 *the baudrate is 115200bit/s
 */
#ifndef API__H
#define API__H

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

#include "api_define.h"
#include "api_global.h"

#include "api_debug.h"

#define UART_MAX_DATA_ELEMENTS 8
/* Structure for CPU commands */
typedef struct uartStruct
{
      uint32_t Uart_Message_ID;
      uint32_t Uart_Mask;
      uint8_t Uart_Rtr;
      uint8_t Uart_Length;
      uint16_t Uart_Data[UART_MAX_DATA_ELEMENTS];
      int8_t commandKeywordIndex;
      int8_t number_of_arguments;
} uartMessage;

#include "can.h"

extern struct uartStruct uartFrame;
extern struct uartStruct *ptr_uartStruct;

extern bool uartInputBufferExceeded;

/* Implemented functions
 * and
 * corresponding function pointers*/

void Process_Uart_Event( void );
extern void (*Process_Uart_Event_p)( void );

int8_t Check_Error( struct uartStruct *ptr_uartStruct ); /*this function checks error for the received parameter */
int8_t Check_Parameter( struct uartStruct *ptr_uartStruct );/* this function checks whether all the received parameters are valid*/

void Choose_Function( struct uartStruct *ptr_uartStruct );/* in terms of the command name is the competent function */

void apiConvertUartDataToCanUartStruct( uint8_t offset ); /*converting the decomposed CPU format in CAN-format */

void keep_alive( struct uartStruct *PtrFrame ); /*this function checks the functionality of the software*/

int8_t uartSplitUartString( char inputUartString[] ); /*CPU-cutting format in various parameters */

void Initialization( void ); /*this function initialize all init functions again and actives the interrupt*/

void InitIOPorts( void );/*this function initializes all input /output of the microcontroller*/

int8_t Timer0_Init( void ); /*initialize  bit Timer0 */

int8_t Timer0A_Init( void ); /* initialize 8 bit Timer with output compareA*/

int8_t UART0_Init( void ); /* initialize serial communication */

void UART0_Transmit( unsigned char c );/* function send  data direction to cpu */
extern void (*UART0_Transmit_p)( uint8_t );

int16_t UART0_Send_Message_String( char *outputString, uint16_t maxSize );/* help function for the output of register contents in CPU */
extern int16_t (*UART0_Send_Message_String_p)( char *, uint16_t );

int8_t UART0_Send_Message_String_woLF( char *outputString, uint32_t maxSize );/* help function for the output of register contents in CPU */

int8_t apiFindCommandKeywordIndex(const char string[], PGM_P const commandKeywords[], size_t commandMaximumIndex ); /* find matching command keyword and return its index*/

static inline bool isKeywordIndex( int index, int maximumIndex )
{
	 return ( -1 < index && index < maximumIndex);
}

void Reset_SetParameter( void ); /* resets/clears all values of setParameter */

void Reset_UartStruct( struct uartStruct *ptr_uartStruct ); /* resets all values of uartStruct*/

void clearUartStruct( struct uartStruct *ptr_uartStruct ); /* resets all values of uartStruct*/

uint8_t initUartStruct(struct uartStruct *ptr_myUartStruct);

uint8_t CommunicationError( uint8_t errorType, const int16_t errorIndex, const uint8_t flag_printCommand, PGM_P alternativeErrorMessage, ... );
extern uint8_t (*CommunicationError_p)(uint8_t, const int16_t, const uint8_t, PGM_P, ...);

void printDebug( uint8_t debugLevel, uint32_t debugMaskIndex, int16_t line, PGM_P file, PGM_P format, ...);
extern void (*printDebug_p)(uint8_t, uint32_t, int16_t, PGM_P, PGM_P, ...);

uint16_t clearString( char mystring[], uint16_t length );
extern uint16_t (*clearString_p)( char[], uint16_t );

void toggle_pin( unsigned char pin_number );

uint8_t createReceiveHeader( struct uartStruct *ptr_uartStruct, char message_string[], uint16_t size );

void createExtendedSubCommandReceiveResponseHeader(struct uartStruct * ptr_uartStruct, int8_t keyNumber, int8_t index,  PGM_P const commandKeyword[]);

uint16_t getNumberOfHexDigits(const char string[], const uint16_t maxLenght);
bool isNumericArgument(const char string[], const uint16_t maxLength);

int8_t getUnsignedNumericValueFromParameterIndex(uint8_t parameterIndex, uint64_t *ptr_value);
int8_t getUnsignedNumericValueFromParameterString(const char string[], uint64_t *ptr_value);

void reset(struct uartStruct *ptr_uartStruct);
void init(struct uartStruct *ptr_uartStruct);

void startMessage(void);
size_t getMaximumStringArrayLength_P(PGM_P const array[], size_t maxIndex, size_t maxResult);
size_t getMaximumStringArrayLength(const char* array[], size_t maxIndex, size_t maxResult);

void determineAndHandleResetSource(void);

bool isNumericalConstantOne(const char string[]);
bool isNumericalConstantZero(const char string[]);

uint8_t apiShowOrAssignParameterToValue(int16_t nArgumentArgs, uint8_t parameterIndex, void *value, uint8_t type, uint64_t min, uint64_t max, bool report, char message[]);
uint8_t apiAssignParameterToValue(uint8_t parameterIndex, void *value, uint8_t type, uint64_t min, uint64_t max);
uint8_t apiShowValue(char string[], void *value, uint8_t type );

typedef uint8_t apiCommandResult;

#ifndef API_CONSTANTS_H_

extern const char* const general_error[] PROGMEM;
enum ge_index
{
   GENERAL_ERROR_init_for_timer0_failed = 0,
   GENERAL_ERROR_init_for_timer0A_failed,
   GENERAL_ERROR_family_code_not_found,
   GENERAL_ERROR_all_message_were_read,
   GENERAL_ERROR_no_device_is_connected_to_the_bus,
   GENERAL_ERROR_undefined_bus,
   GENERAL_ERROR_channel_undefined,
   GENERAL_ERROR_value_has_invalid_type,
   GENERAL_ERROR_address_has_invalid_type,
   GENERAL_ERROR_undefined_family_code,
   GENERAL_ERROR_invalid_argument,
   GENERAL_ERROR_MAXIMUM_INDEX
};


extern const char* const serial_error[] PROGMEM;
enum se_index
{
   SERIAL_ERROR_no_valid_command_name = 0,
   SERIAL_ERROR_command_is_too_long,
   SERIAL_ERROR_argument_has_invalid_type,
   SERIAL_ERROR_undefined_error_type,
   SERIAL_ERROR_arguments_have_invalid_type,
   SERIAL_ERROR_arguments_exceed_boundaries,
   SERIAL_ERROR_too_many_arguments,
   SERIAL_ERROR_too_few_arguments,
   SERIAL_ERROR_invalid_sub_command_name,
   SERIAL_ERROR_argument_string_too_long,
   SERIAL_ERROR_MAXIMUM_INDEX
};

enum dynamicMessage_index
{
    dynamicMessage_ErrorIndex = -100,
};

extern const char* const responseKeywords[] PROGMEM;
/* those are the corresponding command key numbers to commandKeywords, beware of the same order*/
enum responseKeyNumber
{
      responseKeyNumber_RECV = 0,
      responseKeyNumber_CANR,
      responseKeyNumber_SYST,
      responseKeyNumber_MAXIMUM_INDEX
};

extern const char* const commandKeywords[] PROGMEM;
/* those are the corresponding command key numbers to commandKeywords, beware of the same order*/
enum cmdKeyNumber
{
               commandKeyNumber_SEND = 0,
               commandKeyNumber_SUBS,
               commandKeyNumber_USUB,
               commandKeyNumber_STAT,
               commandKeyNumber_RGWR,
               commandKeyNumber_RGRE,
               commandKeyNumber_RADC,
               commandKeyNumber_OWAD,
               commandKeyNumber_OWDS,
               commandKeyNumber_INIT,
               commandKeyNumber_OWLS,
               commandKeyNumber_OWSS,
               commandKeyNumber_RSET,
               commandKeyNumber_PING,
               commandKeyNumber_OWTP,
               commandKeyNumber_OWSP,
               commandKeyNumber_CANT,
               commandKeyNumber_CANS,
               commandKeyNumber_CANU,
               commandKeyNumber_CANP,
               commandKeyNumber_CAN,
               commandKeyNumber_DBGL,
               commandKeyNumber_DBGM,
               commandKeyNumber_JTAG,
               commandKeyNumber_HELP,
               commandKeyNumber_OWTR,
               commandKeyNumber_OWRP,
               commandKeyNumber_ADRP,
               commandKeyNumber_DEBG,
               commandKeyNumber_PARA,
               commandKeyNumber_SHOW,
               commandKeyNumber_OWMR,
               commandKeyNumber_OWPC,
               commandKeyNumber_OWRb,
               commandKeyNumber_OWRB,
               commandKeyNumber_OWSC,
               commandKeyNumber_OWSB,
               commandKeyNumber_OWSA,
               commandKeyNumber_TWIS,
               commandKeyNumber_I2C,
               commandKeyNumber_RLTH,
               commandKeyNumber_CMD1,
               commandKeyNumber_CMD2,
               commandKeyNumber_CMD3,
               commandKeyNumber_SPI,
               commandKeyNumber_GNWR,
               commandKeyNumber_GNRE,
               commandKeyNumber_OW8S,
               commandKeyNumber_WDOG,
               commandKeyNumber_VERS,
               commandKeyNumber_IDN,
               commandKeyNumber_APWI,
               commandKeyNumber_CMD7,
               commandKeyNumber_CMD8,
               commandKeyNumber_MAXIMUM_NUMBER
};

extern const char*    const commandSyntaxes[] PROGMEM;
extern const char*    const commandSyntaxAlternatives[] PROGMEM;
extern const char*    const commandShortDescriptions[] PROGMEM;
extern const uint8_t* const commandImplementations[] PROGMEM;

extern const uint8_t* const commandImpls[] PROGMEM;

extern const char* const errorTypes[] PROGMEM;
/* those are the corresponding ERRx numbers to errorTypes, beware of the same order*/
enum ERRs
{
   ERRG = 0,
   ERRA,
   ERRC,
   ERRM,
   ERRT,
   ERRU,
   ERR_MAXIMUM_NUMBER
};

extern const char* const debugLevelNames[] PROGMEM;
extern const char* const debugSystemNames[] PROGMEM;

enum resetSources
{
	resetSource_JTAG,
	resetSource_WATCHDOG,
	resetSource_BROWN_OUT,
	resetSource_EXTERNAL,
	resetSource_POWER_ON,
	resetSource_UNKNOWN_REASON,
	resetSource_UNDEFINED_REASON,
	resetSource_MAXIMUM_NUMBER
};

enum apiCommandResults
{
	apiCommandResult_SUCCESS_WITH_OUTPUT 			= 0,
	apiCommandResult_SUCCESS_WITH_OPTIONAL_OUTPUT,
	apiCommandResult_SUCCESS_QUIET,
	apiCommandResult_FAILURE 						= 100,
	apiCommandResult_FAILURE_NOT_A_SUB_COMMAND,
	apiCommandResult_FAILURE_QUIET,
	apiCommandResult_UNDEFINED,
	apiCommandResult_MAXIMUM_INDEX
};

enum apiVarTypes
{
	apiVarType_BOOL,
	apiVarType_BOOL_TrueFalse,
	apiVarType_BOOL_OnOff,
	apiVarType_BOOL_HighLow,
	apiVarType_UINT8,
	apiVarType_UINT16,
	apiVarType_UINT32,
	apiVarType_UINT64,
	apiVarType_UINTPTR,
	apiVarType_MAXIMUM_INDEX,
};

#endif
#endif
