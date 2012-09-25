/* The canapi.h is a header file, the functions and structure of the variables specific to the serial interface between the CPU and micro controller.
 *the baudrate is 115200bit/s
 */
#ifndef API__H
#define API__H

#include "api_define.h"
#include "api_global.h"

#include "api_debug.h"
#include "can.h"
#include <stdint.h>
#include <stdarg.h>

#define UART_MAX_DATA_ELEMENTS 8
/* Structure for CPU commands */
struct uartStruct
{
      uint32_t Uart_Message_ID;
      uint32_t Uart_Mask;
      uint8_t Uart_Rtr;
      uint8_t Uart_Length;
      uint16_t Uart_Data[UART_MAX_DATA_ELEMENTS];
      int8_t commandKeywordIndex;
      int8_t number_of_arguments;
};

extern struct uartStruct uartFrame;
extern struct uartStruct *ptr_uartStruct;


/* Implemented functions */
void Process_Uart_Event( void );

int8_t Check_Error( struct uartStruct *ptr_uartStruct ); /*this function checks error for the received parameter */

int8_t Check_Parameter( struct uartStruct *ptr_uartStruct );/* this function checks whether all the received parameters are valid*/
int8_t Check_Parameter_CanFormat( struct uartStruct *ptr_uartStruct ); /* this function checks whether all the received parameters are valid*/

void Choose_Function( struct uartStruct *ptr_uartStruct );/* in terms of the command name is the competent function */

void Convert_pack_canFrame_to_UartFormat( struct canStruct *ptr_canStruct );/* this function collects the various CAN data in a string */

void Convert_UartData_to_UartStruct( char string[MAX_PARAMETER][MAX_LENGTH_PARAMETER] ); /*converting the decomposed CPU format in CAN-format */
//void Convert_UartFormat_to_CanFormat( char *string[MAX_LENGTH_PARAMETER]);

void keep_alive( struct uartStruct *PtrFrame ); /*this function checks the functionality of the software*/

int8_t Decrypt_Uart_String( void ); /*CPU-cutting format in various parameters */

void Receive_Message( struct uartStruct *PtrFrame ); /* function for the SEND command name and RTR set, initialization of the registers with elements of the structure uartStruct*/

void Initialization( void ); /*this function initialize all init functions again and actives the interrupt*/

void Init_Port( void );/*this function initializes all input /output of the microcontroller*/

void Send_Message( struct uartStruct *PtrFrame );/*function for the SEND command name and RTR is not set, initialization of the registers with elements of the structure uartStruct*/

void Subscribe_Message( struct uartStruct *PtrFrame ); /* function for the command name SUBS, initialization of the registers with elements of the structure uartStruct */

int8_t Timer0_Init( void ); /*initializieren  bit Timer0 */

int8_t Timer0A_Init( void ); /* initializieren 8 bit Timermit output compareA*/

void Unsubscribe_Message( struct uartStruct *PtrFrame ); /* function for the command name USUB, initialization of the registers with elements of the structure uartStruct */

int8_t UART0_Init( void ); /* initializieren serial communication */

void UART0_Transmit( unsigned char c );/* function send  data direction to cpu */

int16_t UART0_Send_Message_String( char *tmp_str, uint16_t maxSize );/* help function for the output of register contents in CPU */

int8_t UART0_Send_Message_String_woLF( char *tmp_str, uint32_t maxSize );/* help function for the output of register contents in CPU */

int Parse_Keyword( char string[] ); /* find matching command keyword and return its index*/

void Reset_SetParameter( void ); /* resets/clears all values of setParameter */

void Reset_UartStruct( struct uartStruct *ptr_uartStruct ); /* resets all values of uartStruct*/

void clearUartStruct( struct uartStruct *ptr_uartStruct ); /* resets all values of uartStruct*/

uint8_t CommunicationError( uint8_t errorType, const int16_t errorIndex, const uint8_t flag_printCommand, const prog_char *alternativeErrorMessage, ... );

void printDebug( uint8_t debugLevel, uint32_t debugMaskIndex, uint32_t line, const prog_char* file, const prog_char *format, ...);

uint8_t createReceiveHeader( struct uartStruct *ptr_uartStruct, char message_string[], uint16_t size );

uint16_t clearString( char mystring[], uint16_t length );

void toggle_pin( unsigned char pin_number );

void createExtendedSubCommandReceiveResponseHeader(struct uartStruct * ptr_uartStruct, int8_t keyNumber, int8_t index,  const prog_char* commandKeyword[]);

uint16_t getNumericLength(const char string[], const uint16_t maxLenght);

int8_t getNumericValueFromParameter(uint8_t parameterIndex, uint32_t *ptr_value);

void reset(struct uartStruct *ptr_uartStruct);

uint8_t initUartStruct(struct uartStruct *ptr_myUartStruct);

#ifndef API_CONSTANTS_H_

extern const char *general_error[] PROGMEM;
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
   GENERAL_ERROR_adress_has_invalid_type,
   GENERAL_ERROR_undefined_family_code,
   GENERAL_ERROR_invalid_argument,
   GENERAL_ERROR_MAXIMUM_INDEX
};


extern const char *serial_error[] PROGMEM;
enum se_index
{
   SERIAL_ERROR_no_valid_command_name = 0,
   SERIAL_ERROR_ID_is_too_long,
   SERIAL_ERROR_mask_is_too_long,
   SERIAL_ERROR_rtr_is_too_long,
   SERIAL_ERROR_length_is_too_long,
   SERIAL_ERROR_data_0_is_too_long,
   SERIAL_ERROR_data_1_is_too_long,
   SERIAL_ERROR_data_2_is_too_long,
   SERIAL_ERROR_data_3_is_too_long,
   SERIAL_ERROR_data_4_is_too_long,
   SERIAL_ERROR_data_5_is_too_long,
   SERIAL_ERROR_data_6_is_too_long,
   SERIAL_ERROR_data_7_is_too_long,
   SERIAL_ERROR_command_is_too_long,
   SERIAL_ERROR_argument_has_invalid_type,
   SERIAL_ERROR_ID_has_invalid_type,
   SERIAL_ERROR_mask_has_invalid_type,
   SERIAL_ERROR_rtr_has_invalid_type,
   SERIAL_ERROR_length_has_invalid_type,
   SERIAL_ERROR_data_0_has_invalid_type,
   SERIAL_ERROR_data_1_has_invalid_type,
   SERIAL_ERROR_data_2_has_invalid_type,
   SERIAL_ERROR_data_3_has_invalid_type,
   SERIAL_ERROR_data_4_has_invalid_type,
   SERIAL_ERROR_data_5_has_invalid_type,
   SERIAL_ERROR_data_6_has_invalid_type,
   SERIAL_ERROR_data_7_has_invalid_type,
   SERIAL_ERROR_undefined_error_type,
   SERIAL_ERROR_first_value_is_too_long,
   SERIAL_ERROR_second_value_is_too_long,
   SERIAL_ERROR_arguments_have_invalid_type,
   SERIAL_ERROR_arguments_exceed_boundaries,
   SERIAL_ERROR_too_many_arguments,
   SERIAL_ERROR_MAXIMUM_INDEX
};

extern const char *can_error[] PROGMEM;
enum ce_index
{
   CAN_ERROR_Can_Bus_is_off = 0,
   CAN_ERROR_Can_Bus_is_passive,
   CAN_ERROR_Can_Bus_is_on,
   CAN_ERROR_Bit_Error,
   CAN_ERROR_Stuff_Error,
   CAN_ERROR_CRC_Error,
   CAN_ERROR_Form_Error,
   CAN_ERROR_Acknowledgement_Error,
   CAN_ERROR_CAN_was_not_successfully_initialized,
   CAN_ERROR_timeout_for_CAN_communication,
   CAN_ERROR_MAXIMUM_INDEX
};

extern const char *mob_error[] PROGMEM;
enum me_index
{
   MOB_ERROR_all_mailboxes_already_in_use = 0,
   MOB_ERROR_message_ID_not_found,
   MOB_ERROR_this_message_already_exists,
   MOB_ERROR_MAXIMUM_INDEX
};

extern const char *twi_error[] PROGMEM;
enum te_index
{
   TWI_ERROR_Error_in_initiating_TWI_interface = 0,
   TWI_ERROR_Could_not_start_TWI_Bus_for_WRITE,
   TWI_ERROR_Could_not_start_TWI_Bus_for_READ,
   TWI_ERROR_unknown_command,
   TWI_ERROR_address_is_too_long,
   TWI_ERROR_data_length_is_too_long,
   TWI_ERROR_data_0_is_too_long,
   TWI_ERROR_data_1_is_too_long,
   TWI_ERROR_data_2_is_too_long,
   TWI_ERROR_data_3_is_too_long,
   TWI_ERROR_data_4_is_too_long,
   TWI_ERROR_data_5_is_too_long,
   TWI_ERROR_data_6_is_too_long,
   TWI_ERROR_data_7_is_too_long,
   TWI_ERROR_failed_writing_TWI_Bus,
   TWI_ERROR_failed_reading_TWI_Bus,
   TWI_ERROR_too_few_numeric_arguments,
   TWI_ERROR_wrong_length_or_number_of_data_bytes,
   TWI_ERROR_MAXIMUM_INDEX
};

enum dynamicMessage_index
{
    dynamicMessage_ErrorIndex = -100,
};

extern const char* responseKeywords[] PROGMEM;
/* those are the corresponding command key numbers to commandKeywords, beware of the same order*/
enum responseKeyNumber
{
      responseKeyNumber_RECV = 0,
      responseKeyNumber_MAXIMUM_INDEX
};

extern const char* commandKeywords[] PROGMEM;
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
               commandKeyNumber_OWON,
               commandKeyNumber_OWLS,
               commandKeyNumber_OWSS,
               commandKeyNumber_RSET,
               commandKeyNumber_PING,
               commandKeyNumber_OWTP,
               commandKeyNumber_OWSP,
               commandKeyNumber_ADSP,
               commandKeyNumber_RLSL,
               commandKeyNumber_RLSH,
               commandKeyNumber_RLSI,
               commandKeyNumber_RLSO,
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
               commandKeyNumber_WDOG,
               commandKeyNumber_EXIT,
               commandKeyNumber_RLTH,
               commandKeyNumber_CMD1,
               commandKeyNumber_CMD2,
               commandKeyNumber_CMD3,
               commandKeyNumber_CMD4,
               commandKeyNumber_GNWR,
               commandKeyNumber_GNRE,
               commandKeyNumber_OW8S,
               commandKeyNumber_TWIS,
               commandKeyNumber_VERS,
               commandKeyNumber_MAXIMUM_NUMBER
};

extern const char* commandShortDescriptions[] PROGMEM;
extern const char* commandImplementations[] PROGMEM;

extern const char *errorTypes[] PROGMEM;
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

extern const char *debugLevelNames[] PROGMEM;
extern const char *debugSystemNames[] PROGMEM;

/*function pointers*/
extern void (*printDebug_p)(uint8_t, uint32_t, uint32_t, const prog_char*, const prog_char*, ...);
extern uint8_t (*CommunicationError_p)(uint8_t, const int16_t, const uint8_t, const prog_char*, ...);
extern int16_t (*UART0_Send_Message_String_p)( char *, uint16_t );
extern void (*UART0_Transmit_p)( uint8_t );
extern void (*Process_Uart_Event_p)( void );
extern uint16_t (*clearString_p)( char[], uint16_t );

#endif
#endif
