/*
 * VERSION 1.0 Januar 7th 2010 LATE  File: 'api.c'
 * Author: Linda Fouedjio
 * modified (heavily rather rebuild): Peter Zumbruch
 * modified: Florian Feldbauer
 * modified: Peter Zumbruch, Oct 2011
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <stddef.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/iocan128.h>
#include <avr/iocanxx.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h>

#include "one_wire.h"
#include "one_wire_adc.h"
#include "one_wire_dualSwitch.h"
#include "one_wire_octalSwitch.h"
#include "one_wire_simpleSwitch.h"
#include "read_write_register.h"
#include "waveform_generator_registers.h"
#include "one_wire_temperature.h"
#include "one_wire_api_settings.h"
#include "one_wire_octalSwitch.h"
#include "relay.h"

#include "adc.h"
#include "api.h"
#include "api_define.h"
#include "api_global.h"

#include "api_debug.h"
#include "api_show.h"
#include "api_help.h"
#include "api_debug.h"
#include "jtag.h"
#include "can.h"
#include "mem-check.h"
#include "api_version.h"
#include "twi_master.h"


#warning TODO: combine responseKeyword and other responses error into one set of responses
static const char responseKeyword00[] PROGMEM = "RECV";

const char* responseKeywords[] PROGMEM = {
        responseKeyword00
};

/* those are the command keywords of the API*/
static const char commandKeyword00[] PROGMEM = "SEND";
static const char commandKeyword01[] PROGMEM = "SUBS";
static const char commandKeyword02[] PROGMEM = "USUB";
static const char commandKeyword03[] PROGMEM = "STAT";
static const char commandKeyword04[] PROGMEM = "RGWR";
static const char commandKeyword05[] PROGMEM = "RGRE";
static const char commandKeyword06[] PROGMEM = "RADC";
static const char commandKeyword07[] PROGMEM = "OWAD";
static const char commandKeyword08[] PROGMEM = "OWDS";
static const char commandKeyword09[] PROGMEM = "OWON";
static const char commandKeyword10[] PROGMEM = "OWLS";
static const char commandKeyword11[] PROGMEM = "OWSS";
static const char commandKeyword12[] PROGMEM = "RSET";
static const char commandKeyword13[] PROGMEM = "PING";
static const char commandKeyword14[] PROGMEM = "OWTP";
static const char commandKeyword15[] PROGMEM = "OWSP"; /*one-wire set active pins/bus mask*/
static const char commandKeyword16[] PROGMEM = "ADSP"; /*AVR's adcs set active pins/bus mask*/
static const char commandKeyword17[] PROGMEM = "RLSL"; /*relay set low  level*/
static const char commandKeyword18[] PROGMEM = "RLSH"; /*relay set high level*/
static const char commandKeyword19[] PROGMEM = "RLSI"; /*relay set ADC pin(s) to monitor "in" */
static const char commandKeyword20[] PROGMEM = "RLSO"; /*relay set output pin(s) to switch "out" */
static const char commandKeyword21[] PROGMEM = "DBGL"; /*set debug level*/
static const char commandKeyword22[] PROGMEM = "DBGM"; /*set debug system mask*/
static const char commandKeyword23[] PROGMEM = "JTAG"; /*toggle/set JTAG availability*/
static const char commandKeyword24[] PROGMEM = "HELP"; /*output some help*/
static const char commandKeyword25[] PROGMEM = "OWTR"; /*trigger one-wire device(s) for action, if possible*/
static const char commandKeyword26[] PROGMEM = "OWRP"; /*one-wire read active pins/bus mask*/
static const char commandKeyword27[] PROGMEM = "ADRP"; /*AVR's adcs read active pins/bus mask*/
static const char commandKeyword28[] PROGMEM = "DEBG"; /*set/get debug level and mask*/
static const char commandKeyword29[] PROGMEM = "PARA"; /*check parasitic power supply mode*/
static const char commandKeyword30[] PROGMEM = "SHOW"; /*show (internal) settings*/
static const char commandKeyword31[] PROGMEM = "OWMR"; /*one wire basics: match rom*/
static const char commandKeyword32[] PROGMEM = "OWPC"; /*one wire basics: presence check*/
static const char commandKeyword33[] PROGMEM = "OWRb"; /*one wire basics: receive bit, wait for it*/
static const char commandKeyword34[] PROGMEM = "OWRB"; /*one wire basics: receive byte*/
static const char commandKeyword35[] PROGMEM = "OWSC"; /*one wire basics: send command*/
static const char commandKeyword36[] PROGMEM = "OWSB"; /*one wire basics: send byte*/
static const char commandKeyword37[] PROGMEM = "OWSA"; /*one wire API settings: set/get 1-wire specific API settings*/
static const char commandKeyword38[] PROGMEM = "WDOG"; /*set/get watch dog status*/
static const char commandKeyword39[] PROGMEM = "EXIT"; /*exit*/
static const char commandKeyword40[] PROGMEM = "RLTH"; /* relay threshold */
static const char commandKeyword41[] PROGMEM = "CMD1"; /* command (dummy name) */
static const char commandKeyword42[] PROGMEM = "CMD2"; /* command (dummy name) */
static const char commandKeyword43[] PROGMEM = "CMD3"; /* command (dummy name) */
static const char commandKeyword44[] PROGMEM = "CMD4"; /* command (dummy name) */
static const char commandKeyword45[] PROGMEM = "GNWR"; /* send <address> <data> for waveform generator */
static const char commandKeyword46[] PROGMEM = "GNRE"; /* command (dummy name) */
static const char commandKeyword47[] PROGMEM = "OW8S"; /* set/read state of one wire octal switches */
static const char commandKeyword48[] PROGMEM = "TWIS"; /* command (dummy name) */
static const char commandKeyword49[] PROGMEM = "VERS"; /* command (dummy name) */


/* this is the corresponding command key array, beware of the same order*/
const char* commandKeywords[] PROGMEM = {
		commandKeyword00,
		commandKeyword01,
		commandKeyword02,
		commandKeyword03,
		commandKeyword04,
		commandKeyword05,
		commandKeyword06,
		commandKeyword07,
		commandKeyword08,
		commandKeyword09,
		commandKeyword10,
		commandKeyword11,
		commandKeyword12,
		commandKeyword13,
		commandKeyword14,
		commandKeyword15,
		commandKeyword16,
		commandKeyword17,
		commandKeyword18,
		commandKeyword19,
		commandKeyword20,
		commandKeyword21,
		commandKeyword22,
		commandKeyword23,
		commandKeyword24,
		commandKeyword25,
		commandKeyword26,
		commandKeyword27,
		commandKeyword28,
		commandKeyword29,
		commandKeyword30,
		commandKeyword31,
		commandKeyword32,
		commandKeyword33,
		commandKeyword34,
		commandKeyword35,
		commandKeyword36,
		commandKeyword37,
		commandKeyword38,
		commandKeyword39,
		commandKeyword40,
		commandKeyword41,
		commandKeyword42,
		commandKeyword43,
		commandKeyword44,
		commandKeyword45,
		commandKeyword46,
		commandKeyword47,
		commandKeyword48,
		commandKeyword49

};

/* those are the command keywords of the API*/
static const char commandShortDescription00[] PROGMEM = "CAN-ID ID-Range [RTR <nBytes> D0 .. D7]";
static const char commandShortDescription01[] PROGMEM = "CAN-ID ID-Range";
static const char commandShortDescription02[] PROGMEM = "CAN-ID ID-Range";
static const char commandShortDescription03[] PROGMEM = "[ID]";
static const char commandShortDescription04[] PROGMEM = "Register Value";
static const char commandShortDescription05[] PROGMEM = "Register";
static const char commandShortDescription06[] PROGMEM = "[<ADC Channel>]";
static const char commandShortDescription07[] PROGMEM = "[ID [flag_conv [flag_init]]]";
static const char commandShortDescription08[] PROGMEM = "[ID]";
static const char commandShortDescription09[] PROGMEM = "[ID]";
static const char commandShortDescription10[] PROGMEM = "[<Family Code>]";
static const char commandShortDescription11[] PROGMEM = "[ID]";
static const char commandShortDescription12[] PROGMEM = "";
static const char commandShortDescription13[] PROGMEM = "";
static const char commandShortDescription14[] PROGMEM = "[ID [flag_conv [flag_init]] | <command_keyword> [arguments]]";
static const char commandShortDescription15[] PROGMEM = "<bus mask>"; /*one-wire set/get active pins/bus mask*/
static const char commandShortDescription16[] PROGMEM = "<bus mask>"; /*AVR's adcs set active pins/bus mask*/
static const char commandShortDescription17[] PROGMEM = "<bus> [val]"; /*relay set low  level*/
static const char commandShortDescription18[] PROGMEM = "<bus> [val]"; /*relay set high level*/
static const char commandShortDescription19[] PROGMEM = "<bus> [pins]"; /*relay set ADC pin(s) to monitor "in" */
static const char commandShortDescription20[] PROGMEM = "<bus> [pins]"; /*relay set output pin(s) to switch "out" */
static const char commandShortDescription21[] PROGMEM = "[level]"; /*set debug level*/
static const char commandShortDescription22[] PROGMEM = "[mask]"; /*set debug system mask*/
static const char commandShortDescription23[] PROGMEM = ""; /*toggle/set JTAG availability*/
static const char commandShortDescription24[] PROGMEM = "[CMND]"; /*output some help*/
static const char commandShortDescription25[] PROGMEM = "[ID] ..."; /*trigger one-wire device(s) for action, if possible*/
static const char commandShortDescription26[] PROGMEM = ""; /*one-wire read active pins/bus mask*/
static const char commandShortDescription27[] PROGMEM = ""; /*AVR's adcs read active pins/bus mask*/
static const char commandShortDescription28[] PROGMEM = "[level [mask]]"; /*set/get debug level and mask*/
static const char commandShortDescription29[] PROGMEM = "parasitic device test [???]"; /*not yet figured out*/
static const char commandShortDescription30[] PROGMEM = "[key_word]"; /*show (internal) settings*/
static const char commandShortDescription31[] PROGMEM = "ID <pin_mask>"; /*one wire basics: match rom*/
static const char commandShortDescription32[] PROGMEM = "[<pin_mask>]"; /*one wire basics: presence check*/
static const char commandShortDescription33[] PROGMEM = "<pin_mask> <delay> <timeout: N (times delay)> "; /*one wire basics: receive bit, wait for it*/
static const char commandShortDescription34[] PROGMEM = "[<pin_mask>]"; /*one wire basics: receive byte*/
static const char commandShortDescription35[] PROGMEM = "<command_key_word> [<pin_mask> [arguments ...]] "; /*one wire basics: send command*/
static const char commandShortDescription36[] PROGMEM = "<byte> [<pin_mask>]"; /*one wire basics: send byte*/
static const char commandShortDescription37[] PROGMEM = "<command_key_word> [arguments] "; /*one wire API settings: set/get 1-wire specific API settings*/
static const char commandShortDescription38[] PROGMEM = "[???]"; /* watch dog, not yet figured out*/
static const char commandShortDescription39[] PROGMEM = ""; /* exit */
static const char commandShortDescription40[] PROGMEM = "[command_key_word] <value>"; /* dummy command */
static const char commandShortDescription41[] PROGMEM = "[???]"; /* dummy command */
static const char commandShortDescription42[] PROGMEM = "[???]"; /* dummy command */
static const char commandShortDescription43[] PROGMEM = "[???]"; /* dummy command */
static const char commandShortDescription44[] PROGMEM = "[???]"; /* dummy command */
static const char commandShortDescription45[] PROGMEM = "<address> <data>"; /* dummy command */
static const char commandShortDescription46[] PROGMEM = "<address>"; /* dummy command */
static const char commandShortDescription47[] PROGMEM = "[ID [value]]"; /* set/read state of one wire octal switches */
static const char commandShortDescription48[] PROGMEM = "<0|1> <I2C address> <data length> <data byte1 ... byte8>"; /* I2C access */
static const char commandShortDescription49[] PROGMEM = ""; /* code version */


/* this is the corresponding command key array, beware of the same order*/
const char* commandShortDescriptions[] PROGMEM= {
		commandShortDescription00,
		commandShortDescription01,
		commandShortDescription02,
		commandShortDescription03,
		commandShortDescription04,
		commandShortDescription05,
		commandShortDescription06,
		commandShortDescription07,
		commandShortDescription08,
		commandShortDescription09,
		commandShortDescription10,
		commandShortDescription11,
		commandShortDescription12,
		commandShortDescription13,
		commandShortDescription14,
		commandShortDescription15,
		commandShortDescription16,
		commandShortDescription17,
		commandShortDescription18,
		commandShortDescription19,
		commandShortDescription20,
		commandShortDescription21,
		commandShortDescription22,
		commandShortDescription23,
		commandShortDescription24,
		commandShortDescription25,
		commandShortDescription26,
		commandShortDescription27,
		commandShortDescription28,
		commandShortDescription29,
		commandShortDescription30,
		commandShortDescription31,
		commandShortDescription32,
		commandShortDescription33,
		commandShortDescription34,
		commandShortDescription35,
		commandShortDescription36,
		commandShortDescription37,
		commandShortDescription38,
		commandShortDescription39,
		commandShortDescription40,
		commandShortDescription41,
		commandShortDescription42,
		commandShortDescription43,
		commandShortDescription44,
		commandShortDescription45,
		commandShortDescription46,
		commandShortDescription47,
		commandShortDescription48,
		commandShortDescription49
};

/* those are the command keywords of the API*/
static const char commandImplementation00[] PROGMEM = "";
static const char commandImplementation01[] PROGMEM = "";
static const char commandImplementation02[] PROGMEM = "";
static const char commandImplementation03[] PROGMEM = "--- not implemented";
static const char commandImplementation04[] PROGMEM = "";
static const char commandImplementation05[] PROGMEM = "";
static const char commandImplementation06[] PROGMEM = "";
static const char commandImplementation07[] PROGMEM = "";
static const char commandImplementation08[] PROGMEM = "--- [ID] not implemented";
static const char commandImplementation09[] PROGMEM = "--- [ID] not implemented";
static const char commandImplementation10[] PROGMEM = "";
static const char commandImplementation11[] PROGMEM = "--- [ID] not implemented";
static const char commandImplementation12[] PROGMEM = "";
static const char commandImplementation13[] PROGMEM = "";
static const char commandImplementation14[] PROGMEM = "";
static const char commandImplementation15[] PROGMEM = ""; /*one-wire set/get active pins/bus mask*/
static const char commandImplementation16[] PROGMEM = "--- not implemented"; /*AVR's adcs set active pins/bus mask*/
static const char commandImplementation17[] PROGMEM = "--- not implemented"; /*relay set low  level*/
static const char commandImplementation18[] PROGMEM = "--- not implemented"; /*relay set high level*/
static const char commandImplementation19[] PROGMEM = "--- not implemented"; /*relay set ADC pin(s) to monitor "in" */
static const char commandImplementation20[] PROGMEM = "--- not implemented"; /*relay set output pin(s) to switch "out" */
static const char commandImplementation21[] PROGMEM = ""; /*set debug level*/
static const char commandImplementation22[] PROGMEM = ""; /*set debug system mask*/
static const char commandImplementation23[] PROGMEM = "--- not implemented"; /*toggle/set JTAG availability*/
static const char commandImplementation24[] PROGMEM = ""; /*output some help*/
static const char commandImplementation25[] PROGMEM = "--- not implemented"; /*trigger one-wire device(s) for action, if possible*/
static const char commandImplementation26[] PROGMEM = ""; /*one-wire read active pins/bus mask*/
static const char commandImplementation27[] PROGMEM = "--- not implemented"; /*AVR's adcs read active pins/bus mask*/
static const char commandImplementation28[] PROGMEM = ""; /*set/get debug level and mask*/
static const char commandImplementation29[] PROGMEM = "";
static const char commandImplementation30[] PROGMEM = "";
static const char commandImplementation31[] PROGMEM = "--- not implemented"; /*one wire basics: match rom*/
static const char commandImplementation32[] PROGMEM = "--- not implemented"; /*one wire basics: presence check*/
static const char commandImplementation33[] PROGMEM = "--- not implemented"; /*one wire basics: receive bit, wait for it*/
static const char commandImplementation34[] PROGMEM = "--- not implemented"; /*one wire basics: receive byte*/
static const char commandImplementation35[] PROGMEM = "--- not implemented"; /*one wire basics: send command*/
static const char commandImplementation36[] PROGMEM = "--- not implemented"; /*one wire basics: send byte*/
static const char commandImplementation37[] PROGMEM = "--- not implemented"; /*one wire API settings: set/get 1-wire specific API settings*/
static const char commandImplementation38[] PROGMEM = "--- not implemented"; /*wdog*/
static const char commandImplementation39[] PROGMEM = "--- not implemented"; /*exit*/
static const char commandImplementation40[] PROGMEM = ""; /*dummy command*/
static const char commandImplementation41[] PROGMEM = "--- not implemented"; /*dummy command*/
static const char commandImplementation42[] PROGMEM = "--- not implemented"; /*dummy command*/
static const char commandImplementation43[] PROGMEM = "--- not implemented"; /*dummy command*/
static const char commandImplementation44[] PROGMEM = "--- not implemented"; /*dummy command*/
static const char commandImplementation45[] PROGMEM = "Waveform Generator send data"; /*dummy command*/
static const char commandImplementation46[] PROGMEM = "Waveform Generator read data"; /*dummy command*/
static const char commandImplementation47[] PROGMEM = ""; /* set/read state of one wire octal switches */
static const char commandImplementation48[] PROGMEM = ""; /*I2C access*/
static const char commandImplementation49[] PROGMEM = ""; /*code version*/

/* this is the corresponding command key array, beware of the same order*/
const char* commandImplementations[] PROGMEM= {
		commandImplementation00,
		commandImplementation01,
		commandImplementation02,
		commandImplementation03,
		commandImplementation04,
		commandImplementation05,
		commandImplementation06,
		commandImplementation07,
		commandImplementation08,
		commandImplementation09,
		commandImplementation10,
		commandImplementation11,
		commandImplementation12,
		commandImplementation13,
		commandImplementation14,
		commandImplementation15,
		commandImplementation16,
		commandImplementation17,
		commandImplementation18,
		commandImplementation19,
		commandImplementation20,
		commandImplementation21,
		commandImplementation22,
		commandImplementation23,
		commandImplementation24,
		commandImplementation25,
		commandImplementation26,
		commandImplementation27,
		commandImplementation28,
		commandImplementation29,
		commandImplementation30,
		commandImplementation31,
		commandImplementation32,
		commandImplementation33,
		commandImplementation34,
		commandImplementation35,
		commandImplementation36,
		commandImplementation37,
		commandImplementation38,
		commandImplementation39,
		commandImplementation40,
		commandImplementation41,
		commandImplementation42,
		commandImplementation43,
		commandImplementation44,
		commandImplementation45,
		commandImplementation46,
		commandImplementation47,
		commandImplementation48,
		commandImplementation49
};


/* pointer of array for defined serial error number*/
static const char se00[] PROGMEM = "no valid command name";
static const char se01[] PROGMEM = "ID is too long";
static const char se02[] PROGMEM = "mask is too long";
static const char se03[] PROGMEM = "rtr is too long";
static const char se04[] PROGMEM = "length is  too long";
static const char se05[] PROGMEM = "data 0 is too long";
static const char se06[] PROGMEM = "data 1 is too long";
static const char se07[] PROGMEM = "data 2 is too long";
static const char se08[] PROGMEM = "data 3 is too long";
static const char se09[] PROGMEM = "data 4 is too long";
static const char se10[] PROGMEM = "data 5 is too long";
static const char se11[] PROGMEM = "data 6 is too long";
static const char se12[] PROGMEM = "data 7 is too long";
static const char se13[] PROGMEM = "command is too long";
static const char se14[] PROGMEM = "argument has invalid type";
static const char se15[] PROGMEM = "ID has invalid type";
static const char se16[] PROGMEM = "mask has invalid type";
static const char se17[] PROGMEM = "rtr has invalid type";
static const char se18[] PROGMEM = "length has invalid type";
static const char se19[] PROGMEM = "data 0 has invalid type";
static const char se20[] PROGMEM = "data 1 has invalid type";
static const char se21[] PROGMEM = "data 2 has invalid type";
static const char se22[] PROGMEM = "data 3 has invalid type";
static const char se23[] PROGMEM = "data 4 has invalid type";
static const char se24[] PROGMEM = "data 5 has invalid type";
static const char se25[] PROGMEM = "data 6 has invalid type";
static const char se26[] PROGMEM = "data 7 has invalid type";
static const char se27[] PROGMEM = "undefined error type";
static const char se28[] PROGMEM = "first value is too long";
static const char se29[] PROGMEM = "second value is too long";
static const char se30[] PROGMEM = "arguments have invalid type";
static const char se31[] PROGMEM = "argument(s) exceed(s) allowed boundary";
static const char se32[] PROGMEM = "too many arguments";

const char *serial_error[] PROGMEM = {
		se00, se01, se02, se03, se04,
		se05, se06, se07, se08, se09,
		se10, se11, se12, se13, se14,
		se15, se16, se17, se18, se19,
		se20, se21, se22, se23, se24,
		se25, se26, se27, se28, se29,
		se30, se31, se32 };


/* array for defined serial error number*/
const uint8_t serial_error_number[] = { 11, 12, 13, 14, 15, 16, 17, 18, 19, 110,
		111, 112, 113, 114, 115, 116, 117, 118, 119, 120,
		121, 122, 123, 124, 125, 126, 127, 128, 129, 130,
		131};

/* pointer of array for defined can error number*/

static const char ce00[] PROGMEM = "Can_Bus is off";
static const char ce01[] PROGMEM = "Can_Bus is passive";
static const char ce02[] PROGMEM = "Can_Bus is on";
static const char ce03[] PROGMEM = "Bit Error";
static const char ce04[] PROGMEM = "Stuff Error";
static const char ce05[] PROGMEM = "CRC Error";
static const char ce06[] PROGMEM = "Form Error";
static const char ce07[] PROGMEM = "Acknowledgment Error";
static const char ce08[] PROGMEM = "CAN was not successfully initialized";
static const char ce09[] PROGMEM = "timeout for CAN communication";

const char *can_error[] PROGMEM = { ce00, ce01, ce02, ce03, ce04, ce05, ce06, ce07, ce08, ce09 };

/* array for defined can error number*/
const uint8_t can_error_number[] = { 21, 22, 23, 24, 25, 26, 27, 28, 29, 0x2A };
/* pointer of array for defined mailbox error */

static const char me00[] PROGMEM = "all mailboxes already in use";
static const char me01[] PROGMEM = "message ID not found";
static const char me02[] PROGMEM = "this message already exists";

const char *mob_error[] PROGMEM = { me00, me01, me02 };

/* array for defined mailbox error number*/
const uint8_t mob_error_number[] = { 31, 32, 33 };

/* pointer of array for defined general error number*/

static const char ge00[] PROGMEM = "init for timer0 failed";
static const char ge01[] PROGMEM = "init for timer0A failed";
static const char ge02[] PROGMEM = "family code not found";
static const char ge03[] PROGMEM = "all message were read";
static const char ge04[] PROGMEM = "no device is connected to the bus";
static const char ge05[] PROGMEM = "undefined bus";
static const char ge06[] PROGMEM = "channel undefined";
static const char ge07[] PROGMEM = "value has invalid type";
static const char ge08[] PROGMEM = "address has invalid type";
static const char ge09[] PROGMEM = "undefined family code";
static const char ge10[] PROGMEM = "invalid argument";

const char *general_error[] PROGMEM = { ge00, ge01, ge02, ge03, ge04, ge05, ge06, ge07, ge08, ge09, ge10 };

/* array for defined general error number*/
const uint16_t general_error_number[] = { 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 0x4A };

static const char tw00[] PROGMEM = "Error Initiating TWI interface";
static const char tw01[] PROGMEM = "Could not start TWI Bus for WRITE";
static const char tw02[] PROGMEM = "Could not start TWI Bus for READ";
static const char tw03[] PROGMEM = "unknown command";
static const char tw04[] PROGMEM = "address_is_too_long";
static const char tw05[] PROGMEM = "data length is too long";
static const char tw06[] PROGMEM = "data 0 is too long";
static const char tw07[] PROGMEM = "data 1 is too long";
static const char tw08[] PROGMEM = "data 2 is too long";
static const char tw09[] PROGMEM = "data 3 is too long";
static const char tw10[] PROGMEM = "data 4 is too long";
static const char tw11[] PROGMEM = "data 5 is too long";
static const char tw12[] PROGMEM = "data 6 is too long";
static const char tw13[] PROGMEM = "data 7 is too long";
static const char tw14[] PROGMEM = "failed writing TWI_Bus";
static const char tw15[] PROGMEM = "failed reading TWI_Bus";
static const char tw16[] PROGMEM = "too few (numeric) arguments";
static const char tw17[] PROGMEM = "wrong length or number of data bytes";
const char *twi_error[] PROGMEM = { tw00, tw01, tw02, tw03, tw04,
		                            tw05, tw06, tw07, tw08, tw09,
		                            tw10, tw11, tw12, tw13, tw14,
		                            tw15, tw16, tw17 };

/* array for defined can error number*/
const uint8_t twi_error_number[] = { 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, 0x60};

/* pointer of array for defined general error number*/
static const char errorType00[] PROGMEM = "ERRG"; /*general*/
static const char errorType01[] PROGMEM = "ERRA"; /*api*/
static const char errorType02[] PROGMEM = "ERRC"; /*can protocol*/
static const char errorType03[] PROGMEM = "ERRM"; /*can hardware*/
static const char errorType04[] PROGMEM = "ERRT"; /*TWI / I2C*/
static const char errorType05[] PROGMEM = "ERRU"; /*undefined*/

/* this is the corresponding error Type array, beware of the same order*/
const char *errorTypes[] PROGMEM = {
		errorType00,
		errorType01,
		errorType02,
		errorType03,
		errorType04,
		errorType05
};


/*----------------------------------------------------------------------------------------------------*/

/*
 *this function initializes the UART0 register needed to allow the at90can128
 *USART transmissions
 */
int8_t UART0_Init( void )
{
	uint8_t intstate1 = SREG;/*save global interrupt flag*/
	cli();
	/*disable interrupt*/
	while ( 0 << TXC0 )
	{
#warning timeout needed
	}
	/* set baud rate*/
	UBRR0H = (uint8_t) ( BAUD >> 8 );
	UBRR0L = (uint8_t) ( BAUD );
	/* set frame format: i will set it now as 8 bits, no parity 1 stop bit, asynchronous*/
	UCSR0C = ( 0 << UMSEL0 ) | ( 0 << UPM0 ) | ( 0 << USBS0 ) | ( 3 << UCSZ0 );
	/*set double speed*/
	UCSR0A = ( 1 << U2X0 );
	/*enable receiver and transmitter; enable RX and Data register empty interrupt*/
	UCSR0B = ( 1 << TXEN0 ) | ( 1 << RXEN0 ) | ( 1 << RXCIE0 );
	SREG = intstate1; /*restore global interrupt flag*/
	return 1;
}//END of  UART0_Init




/*
 *this function initializes the 8 bit counter
 * the function has not input parameter
 */

int8_t Timer0_Init( void )
{
	uint8_t intstate3 = SREG; /* save global interrupt flag */
	cli(); /* disable interrupt */
	TCCR0A = ( 1 << CS02 ) | ( 0 << CS01 ) | ( 1 << CS00 ); /* set Prescaler 1024 */
	TIMSK0 |= ( 1 << TOIE0 ); /* Timer0 Overflow enable */
	SREG = intstate3; /* restore global interrupt flag */
	return 1;
}// END of Timer0_Init


/*
 *this function initializes the 8 bit counter with output compareA
 * the function has not input parameter
 * the output parameter is an integer
 * 1 -> the timer initializierung for timer 0 was successfully
 */

int8_t Timer0A_Init( void )
{
	uint8_t intstate4 = SREG; /* save global interrupt flag */
	cli();  /* disable interrupt */
	TCCR0A = ( 1 << FOC0A ) | ( 1 << CS02 ) | ( 0 << CS01 ) | ( 1 << CS00 ); /* set Prescaler 1024 */
	OCR0A = 255;
	TCNT0 = 20;
	TIMSK0 |= ( 1 << OCIE0A ); /* enable compare output Interrupt */
	SREG = intstate4; /*restore global interrupt flag */
	return 1;
}//END of int8_t Timer0A_Init

/* this function processes an UART event
 * i.e.
 * - parsing it
 * - finding command keywords
 * - converting command arguments into an structure
 * - check the parameter boundaries and types (integer)
 * - check for errors
 * - choose the corresponding function
 * - resetting data structures
 *
 * no direct input: use of global variable decrypt_uartStruct
 * no return value
 */

void Process_Uart_Event(void)
{
	/* split uart string into its elements */

	int8_t number_of_elements = -1;
	number_of_elements = Decrypt_Uart_String();

	if ( eventDebugVerbose <= debug && ( ( debugMask >> debugCommandKey ) & 1 ) )
	{
		snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) number of string elements found: %i"), __LINE__, __FILE__, number_of_elements);
		UART0_Send_Message_String(NULL,0);
	}

	if ( 0 < number_of_elements  )
	{
		/* set number of arguments */
		ptr_uartStruct->number_of_arguments = number_of_elements -1;

		/* Find matching command keyword */
		ptr_uartStruct->commandKeywordIndex = Parse_Keyword(setParameter[0]);

		if ( eventDebug <= debug && ( ( debugMask >> debugCommandKey ) & 1 ) )
		{
			snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) keywordIndex of %s is %i"), __LINE__, __FILE__, setParameter[0], ptr_uartStruct->commandKeywordIndex);
			UART0_Send_Message_String(NULL,0);
		}

		/* no matching keyword ?*/
		if ( 0 > ptr_uartStruct->commandKeywordIndex )
		{
			Check_Error(ptr_uartStruct);
		}
		else
		{
			switch(ptr_uartStruct->commandKeywordIndex)
			{
			case commandKeyNumber_HELP: /*use its own parameter handling*/
			case commandKeyNumber_SHOW: /*use its own parameter handling*/
			case commandKeyNumber_OWSA: /*use its own parameter handling*/
			case commandKeyNumber_OWSC: /*use its own parameter handling*/
			case commandKeyNumber_RLTH: /*use its own parameter handling*/
			case commandKeyNumber_TWIS: /*use its own parameter handling*/
			case commandKeyNumber_DEBG: /*use its own parameter handling*/
			   Choose_Function(ptr_uartStruct);
				break;
			case commandKeyNumber_OWTP: /*use its own parameter handling, ID access*/
			case commandKeyNumber_OWAD: /*use its own parameter handling, ID access*/
			case commandKeyNumber_OWDS: /*use its own parameter handling, ID access*/
			case commandKeyNumber_OWSS: /*use its own parameter handling, ID access*/
			case commandKeyNumber_OWON: /*use its own parameter handling, ID access*/
			case commandKeyNumber_OWMR: /*use its own parameter handling, ID access*/
			case commandKeyNumber_OW8S: /*use its own parameter handling, ID access*/ // flo
			   if ( 0 == ConvertUartDataToOwiStruct() )
			   {
			      Choose_Function(ptr_uartStruct);
			   }
			   break;
			default:
				/* call the function for the conversion frameformat in Canformat*/
				Convert_UartData_to_UartStruct(setParameter);
				/*call the function Check_Parameter*/
				/* TODO: improve this for different command keywords than CAN based,
				 * i.e. think to do checks maybe later when already having chosen the
				 * right function*/
				if ( TRUE == Check_Parameter(ptr_uartStruct) )
				{
					Choose_Function(ptr_uartStruct);
				}
				else
				{
					Check_Error(ptr_uartStruct);
				}
				break;
			}
		}
	}

	/*clear the variables*/
	Reset_SetParameter();
	Reset_UartStruct(ptr_uartStruct);
	clearString(decrypt_uartString, BUFFER_SIZE);

	return;
}

/*
 * this function splits the string to receive various parameters
 * no direct input : use of global variable decrypt_uartString
 * no direct output: use of global variable setParameter
 * return value:
 *          number of found elements,
 *          FALSE else
 */

int8_t Decrypt_Uart_String( void )
{
	uint8_t index_parameter;

	/* maximum length check of input */

	if ( MAX_LENGTH_COMMAND < strlen(decrypt_uartString) )
	{
		uart_errorCode = CommunicationError(ERRA, SERIAL_ERROR_command_is_too_long, 0, NULL, 0);

		/* reset */
		clearString(decrypt_uartString, BUFFER_SIZE);
		/* "reset": decrypt_uartString[0] = '\0'; */

		return 0;
	}

	/*
	 * disassemble the decrypt_uartString into stringlets
	 * separated by delimiters "delim"
	 * into elements of array setParameter
	 */

	const char *delims = " ";
	char *test = NULL;
	char *result = NULL; /* pointer init */

	/* initial iteration */
	result = strtok_r(decrypt_uartString, delims, &test); /*search spaces in string */
	index_parameter = 0; /* pointer of setParameter*/

	while ( result != NULL )
	{
		if (MAX_LENGTH_PARAMETER < strlen(result))
		{
			/* TODO: create correct error code*/
			uart_errorCode = CommunicationError(ERRA, SERIAL_ERROR_command_is_too_long, 0, NULL, 0);
		    clearString(decrypt_uartString, BUFFER_SIZE);
		    /* "reset": decrypt_uartString[0] = '\0'; */

			return 0;
		}

		strncpy(setParameter[index_parameter], result, MAX_LENGTH_PARAMETER);
		result = strtok_r(NULL, delims, &test);
		index_parameter++;

		if ( MAX_PARAMETER < index_parameter )
		{
			uart_errorCode = CommunicationError(ERRA, SERIAL_ERROR_too_many_arguments, 0, NULL, 0);
	        clearString(decrypt_uartString, BUFFER_SIZE);
	        /* "reset": decrypt_uartString[0] = '\0'; */
			return 0;
		}
	}

	if ( eventDebug <= debug && ( ( debugMask >> debugDecrypt ) & 0x1 ) )
	{
		snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) found %i arguments "), __LINE__, __FILE__, index_parameter-1);
		UART0_Send_Message_String(NULL,0);
	}

    clearString(decrypt_uartString, BUFFER_SIZE);
     /* "reset": decrypt_uartString[0] = '\0'; */

	return index_parameter;

}//END of Decrypt_Uart_String function


/*
 *this function assigns the various parameters of the string to the structure and reviewed the range of *individual  parameters.
 *the  function has a two dimensional array from string via serial interface as input and returns no parameter
 */

void Convert_UartData_to_UartStruct( char string[MAX_PARAMETER][MAX_LENGTH_PARAMETER] )
{
	/* convert char in Hexadezimal*/

	//   ptr_uartStruct->Name = setParameter[0];
	ptr_uartStruct->Uart_Message_ID = (uint32_t) strtoul(setParameter [1], &ptr_setParameter[1], 16);
	ptr_uartStruct->Uart_Mask       = (uint32_t) strtoul(setParameter [2], &ptr_setParameter[2], 16);
	ptr_uartStruct->Uart_Rtr        = (uint8_t)  strtoul(setParameter [3], &ptr_setParameter[3], 16);
	ptr_uartStruct->Uart_Length     = (uint8_t)  strtoul(setParameter [4], &ptr_setParameter[4], 16);
	ptr_uartStruct->Uart_Data[0]    = (uint16_t) strtoul(setParameter [5], &ptr_setParameter[5], 16);
	ptr_uartStruct->Uart_Data[1]    = (uint16_t) strtoul(setParameter [6], &ptr_setParameter[6], 16);
	ptr_uartStruct->Uart_Data[2]    = (uint16_t) strtoul(setParameter [7], &ptr_setParameter[7], 16);
	ptr_uartStruct->Uart_Data[3]    = (uint16_t) strtoul(setParameter [8], &ptr_setParameter[8], 16);
	ptr_uartStruct->Uart_Data[4]    = (uint16_t) strtoul(setParameter [9], &ptr_setParameter[9], 16);
	ptr_uartStruct->Uart_Data[5]    = (uint16_t) strtoul(setParameter[10], &ptr_setParameter[10], 16);
	ptr_uartStruct->Uart_Data[6]    = (uint16_t) strtoul(setParameter[11], &ptr_setParameter[11], 16);
	ptr_uartStruct->Uart_Data[7]    = (uint16_t) strtoul(setParameter[12], &ptr_setParameter[12], 16);

} // END of function Convert_UartFormat_to_CanFormat

/* this function parses for all valid command matching keyword
 * on the first MAX_LENGTH_PARAMETER characters of the string
 *
 * it has an char pointer as input
 * it returns
 *     the commandKeyNumber of the corresponding command key word
 *     -1 if not found
 *     -99 on error
 */

int Parse_Keyword(char string[])
{
	static int8_t keywordNumber;

	if (NULL == string   ) {return -99;}
	if (STRING_END  == string[0]) {return -99;}

	if ( eventDebug <= debug && ( ( debugMask >> debugCommandKey ) & 1 ) )
	{
		snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) Parse_Keyword %s"), __LINE__, __FILE__, string);
		UART0_Send_Message_String(NULL,0);
	}
	for ( keywordNumber = 0; keywordNumber < commandKeyNumber_MAXIMUM_NUMBER ; keywordNumber++ )
	{
       /*exclude list*/
       if ( commandKeyNumber_OWON == keywordNumber) { continue; }

		if ( 0 == strncmp_P(&string[0], (const char*) (pgm_read_word( &(commandKeywords[keywordNumber]))), MAX_LENGTH_PARAMETER) )
		{
           if ( eventDebug <= debug && ( ( debugMask >> debugCommandKey ) & 0x1 ) )
            {
               snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) fcn:ParseKeyword keyword %s matches "),
                          __LINE__, __FILE__, string);
               strncat_P(uart_message_string,(const char*) (pgm_read_word( &(commandKeywords[keywordNumber]))),BUFFER_SIZE -1);
               UART0_Send_Message_String(NULL,0);
            }

			return keywordNumber;
		}
        else
          {
             if ( eventDebugVerbose <= debug && ((debugMask >> debugCommandKey) & 0x1))
             {
                snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) fcn:ParseKeyworkd keyword %s doesn't match "),
                           __LINE__, __FILE__, string);
                strncat_P(uart_message_string,(const char*) (pgm_read_word( &(commandKeywords[keywordNumber]))),BUFFER_SIZE -1);
                UART0_Send_Message_String(NULL,0);
             }
          }

	}
	return -1;
}

/* this function checks whether all the received parameters are valid
 * the function has a pointer of the serial structure as input and
 * returns TRUE if all checks are passed
 * else FALSE
 */

int8_t Check_Parameter( struct uartStruct *ptr_uartStruct )
{
	/*check correct keyword*/
	/*check range of parameter*/
	return ( 0 <= ptr_uartStruct->commandKeywordIndex && TRUE == Check_Parameter_CanFormat(ptr_uartStruct) ) ? TRUE : FALSE;
}

/* this function checks whether all the received parameters are valid
 * the function has a pointer of the serial structure as input and returns no parameter
 * returns TRUE if all checks are passed
 * returns FALSE else
 */

int8_t Check_Parameter_CanFormat( struct uartStruct *ptr_uartStruct)
{
   /*check range of parameter*/

   for (uint8_t index = 1; index < MAX_PARAMETER; index ++)
   {
      if ( 0 != *ptr_setParameter[index] ) return FALSE;
   }

   if ( 0x7FFFFFF  < ptr_uartStruct->Uart_Message_ID ) return FALSE;
   if ( 0x7FFFFFF  < ptr_uartStruct->Uart_Mask       ) return FALSE;
   if ( 1          < ptr_uartStruct->Uart_Rtr        ) return FALSE;
   if ( 8          < ptr_uartStruct->Uart_Length     ) return FALSE;
   if ( 0XFF       < ptr_uartStruct->Uart_Data[0]    ) return FALSE;
   if ( 0XFF       < ptr_uartStruct->Uart_Data[1]    ) return FALSE;
   if ( 0XFF       < ptr_uartStruct->Uart_Data[2]    ) return FALSE;
   if ( 0XFF       < ptr_uartStruct->Uart_Data[3]    ) return FALSE;
   if ( 0XFF       < ptr_uartStruct->Uart_Data[4]    ) return FALSE;
   if ( 0XFF       < ptr_uartStruct->Uart_Data[5]    ) return FALSE;
   if ( 0XFF       < ptr_uartStruct->Uart_Data[6]    ) return FALSE;
   if ( 0XFF       < ptr_uartStruct->Uart_Data[7]    ) return FALSE;
   return TRUE;

}

/* this function clears the container setParameter
 * the function has no input
 */

void Reset_SetParameter()
{
   for ( uint8_t count_setParameter = 0 ; count_setParameter < MAX_PARAMETER ; count_setParameter++ )
   {
      setParameter[count_setParameter][0] = '\0';
      *ptr_setParameter[count_setParameter] = 0;

   }
}

/* this function clears the container pointed to by ptr_uartStruct
 * the function has as input a pointer to an uartStruct structure
 */

void Reset_UartStruct( struct uartStruct *ptr_uartStruct )
{
   if ( NULL != ptr_uartStruct )
   {
      ptr_uartStruct->Uart_Length = 0;
      ptr_uartStruct->Uart_Mask = 0;
      ptr_uartStruct->Uart_Message_ID = 0;
      ptr_uartStruct->Uart_Rtr = 0;
      ptr_uartStruct->commandKeywordIndex = -1;
      ptr_uartStruct->number_of_arguments = -1;
      for ( uint8_t i = 0 ; i < 8 ; i++ )
      {
         ptr_uartStruct->Uart_Data[i] = 0;
      }
   }
}

/* this function checks error for the received parameter
 * the function has a pointer of the serial structur as input and returns no parameter
 */

int8_t Check_Error( struct uartStruct *ptr_uartStruct )
{
   static int8_t error = FALSE;

   error = FALSE;

   if ( 0 > ptr_uartStruct->commandKeywordIndex )
   {
      uart_errorCode = CommunicationError(ERRA, SERIAL_ERROR_no_valid_command_name,0,NULL,0);
      error = TRUE;
   }
   else
   {
      for (uint8_t index = 1; index < MAX_PARAMETER; index ++)
      {
         if (0 != *ptr_setParameter[index])
         {
            uart_errorCode = CommunicationError(ERRA,SERIAL_ERROR_argument_has_invalid_type + index, 0, NULL, 0);
            error = TRUE;
         }
         else
         {
            switch (ptr_uartStruct->commandKeywordIndex)
            {
               case commandKeyNumber_SEND: /*CAN*/
               case commandKeyNumber_SUBS: /*CAN*/
               case commandKeyNumber_USUB: /*CAN*/
               {
                  //#error missing break statement for number of arguments
                  /* type check */
                  for ( uint8_t i = 0 ; i < MAX_PARAMETER ; i++ )
                  {
                     switch (i)
                     {
                        case 1:
                           if ( ( 0x7FFFFFF ) < ptr_uartStruct->Uart_Message_ID )
                           {
                              uart_errorCode = CommunicationError(ERRA, SERIAL_ERROR_ID_is_too_long, 0, NULL, 0);
                              error = TRUE;
                              break;
                           }
                           break;
                        case 2:
                           if ( ( 0x7FFFFFF ) < ptr_uartStruct->Uart_Mask )
                           {
                              uart_errorCode = CommunicationError(ERRA, SERIAL_ERROR_mask_is_too_long, 0, NULL, 0);
                              error = TRUE;
                              break;
                           }
                           break;
                        case 3:

                           if ( ( 1 ) < ptr_uartStruct->Uart_Rtr )
                           {
                              uart_errorCode = CommunicationError(ERRA, SERIAL_ERROR_rtr_is_too_long, 0, NULL, 0);
                              error = TRUE;
                              break;
                           }
                           break;
                        case 4:

                           if ( ( 8 ) < ptr_uartStruct->Uart_Length )
                           {
                              uart_errorCode = CommunicationError(ERRA, SERIAL_ERROR_length_is_too_long, 0, NULL, 0);
                              error = TRUE;
                              break;
                           }
                           break;
                        case 5:
                        case 6:
                        case 7:
                        case 8:
                        case 9:
                        case 10:
                        case 11:
                        case 12:
                           for ( uint8_t i = 0 ; i < 8 ; i++ )
                           {
                              if ( ( 0XFF ) < ptr_uartStruct->Uart_Data[i] )
                              {
                                 uart_errorCode = CommunicationError(ERRA, SERIAL_ERROR_data_0_is_too_long + i, 0, NULL, 0);
                                 ptr_uartStruct->Uart_Data[i] = 0;
                                 error = TRUE;
                                 break;
                              }
                           }
                           break;
                     }

                  }
               }
               break;
               case commandKeyNumber_TWIS: /*I2C*/
               {
                   error = twiMasterCheckError(ptr_uartStruct);
			   }
				break;
	               default: /*uncovered command keys*/
                  break;
            }
         }
         if ( TRUE == error )
         {
            break;
         }
      } //end for
   }//end else

   if ( TRUE == error )
   {
      Reset_UartStruct(ptr_uartStruct);
      Reset_SetParameter();
      return 1;
   }
   else
   {
      return 0;
   }
}//END of Check_Error function

/*
 *this function calls another function in relation to the first parameter of the string
 *the function has a pointer of the serial structure as input and returns no parameter
 */

void Choose_Function( struct uartStruct *ptr_uartStruct )
{
	switch (ptr_uartStruct->commandKeywordIndex)
	{
	case commandKeyNumber_SEND:
	case commandKeyNumber_SUBS:
	case commandKeyNumber_USUB:
	{
		Convert_UartData_to_UartStruct(setParameter);
		if (TRUE == Check_Parameter_CanFormat(ptr_uartStruct))
		{
			switch ( ptr_uartStruct->commandKeywordIndex )
			{
			case commandKeyNumber_SEND:
				/* command : SEND CAN-Message-ID ID-Range [RTR <Number of data bytes> Data0 ... Data7] */
				if ( 0 == ptr_uartStruct->Uart_Rtr )
				{
					/* command      : SEND CAN-Message-ID ID-Range 0 <Number of data bytes> Data0 ... Data7]
					 * response now : RECV SEND CAN-Message-ID "command will be carried out"*/
					/* response TODO: <nothing> */
					/* TODO: is this obsolete, debug level?*/
					Send_Message(ptr_uartStruct); /* call function with name  Send_Message */
				}
				else
				{
					/* command      : SEND CAN-Message-ID ID-Range 0 <Number of data bytes> Data0 ... Data7]
					 * response  now: RECV CAN_Mob CAN-Message-ID CAN-Length [Data0 ... Data7] */
					/* response TODO: RECV SEND CAN-Message-ID CAN-Length [Data0 ... Data7] */
					Receive_Message(ptr_uartStruct); /* call function with name Receive_message */
				}
				break;
			case commandKeyNumber_SUBS:
				/* command : SUBS CAN-Message-ID ID-Range
				 * response: */
				Subscribe_Message(ptr_uartStruct); /* call function with name Subscribe_Message */
				break;
			case commandKeyNumber_USUB:
				/* command : USUB CAN-Message-ID ID-Range
				 * response: */
				Unsubscribe_Message(ptr_uartStruct); /* call function with name Unsubscribe_Message */
				break;
			}
		}
		else { Check_Error(ptr_uartStruct);}
	}
	break;
	case commandKeyNumber_STAT:
		/* command  : STAT [ID]*/
		/* response : RECV STAT ID1 description1 status1\r
		 *            ...
		 *            RECV STAT IDX descriptionX statusX\n */
		/* originally ment to read the pressure in the bus*/
		//atmelControlLoopReadStatus(ptr_uartStruct);
		break;
	case commandKeyNumber_OWTP:
		/* command : OWTP [ID / keyword [keyword arguments]]
		 * [ID] response: RECV OWTP ID1 value1\r
		 *                ...
		 *                RECV OWTP IDx valueX\n
		 * keyword response: RECV keyword STATUS
         * command : OWTP
         */
		owiTemperatureSensors(ptr_uartStruct); /* call function with name owiTemperatureReadSensors */
		break;
	case commandKeyNumber_RGWR:
		/* command      : RGWR Register Value */
		/* response now : RECV the value %x has been written in Register */
		/* response TODO: RECV RGWR Register Value (OldValue) */
		writeRegister(ptr_uartStruct); /* call function with name writeRegister  */
		break;
	case commandKeyNumber_RGRE:
		/* command      : RGRE Register*/
		/* response now : RECV the value %x has been written in Register */
		/* response TODO: RECV RGWR Register Value */
		readRegister(ptr_uartStruct); /* call function with name  readRegister */
		break;
	case commandKeyNumber_RADC: /* read AVR's ADCs */
		atmelReadADCs(ptr_uartStruct);
		break;
	case commandKeyNumber_OWAD: /* one-wire adc */
		/* command : OWAD [ID]
		 * response: RECV OWAD ID1 value1.1 ... value1.n\r
		 *           ...
		 *           RECV OWAD IDx valueX.1 ... valueX.n\n */
		owiReadADCs(ptr_uartStruct);
		/* similar to OWTP*/
		break;
//	case commandKeyNumber_OWON:
//		dualSwitches_ON(ptr_uartStruct);
//		break;
	case commandKeyNumber_OWDS: /*set/get one wire dual switches*/
		owiDualSwitches(ptr_uartStruct);
		break;
	case commandKeyNumber_RSET:
        reset(ptr_uartStruct);
		break;
	case commandKeyNumber_OWSS:
		read_status_simpleSwitches(ptr_uartStruct); /* call function with name  readRegister */
		break;
	case commandKeyNumber_OWLS:
       /* command : OWLS
        * response: RECV OWLS 1: bus mask: 0xXX ID <owi ID>
        *           ...
        * response: RECV OWLS N: bus mask: 0xXX ID <owi ID>*/
	   owiShowDevicesID(ptr_uartStruct);
		break;
	case commandKeyNumber_PING:
		keep_alive(ptr_uartStruct); /* call function with name  readRegister */
		break;
	case commandKeyNumber_OWSP: /*one-wire set active pins/bus mask*/
		/* command : OWSP FC
		 * response: ... */
		setOneWireBusMask(ptr_uartStruct);
		break;
	case commandKeyNumber_OWRP: /*one-wire read active pins/bus mask*/
		/* command : OWRP
		 * response: RECV OWRP value */
		getOneWireBusMask(ptr_uartStruct);
		break;
	case commandKeyNumber_ADSP: /*AVR's adcs set active pins/bus mask*/
		break;
	case commandKeyNumber_RLSL: /*relay set low  level*/
		break;
	case commandKeyNumber_RLSH: /*relay set high level*/
		break;
	case commandKeyNumber_RLSI: /*relay set ADC pin(s) to monitor "in" */
		break;
	case commandKeyNumber_RLSO: /*relay set output pin(s) to switch "out" */
		break;
	case commandKeyNumber_PARA: /*parasitic devices*/
	   owiTemperatureFindParasiticlyPoweredDevices(TRUE);
		break;
//    case commandKeyNumber_WDOG: /*watch dog*/
//        break;
	case commandKeyNumber_SHOW: /*show (internal) settings*/
		show(ptr_uartStruct);
		break;
	case commandKeyNumber_DEBG: /*set/get debug level*/
		/* command : DEBG [level [mask]]
		 * set response: ...
		 * get response: RECV DEBG level mask*/
		apiDebug(ptr_uartStruct);
		/*readModifyDebugLevelAndMask(ptr_uartStruct);*/
		break;
	case commandKeyNumber_DBGL: /*set/get debug level*/
		/* command : DBGL [level]
		 * set response: ...
		 * get response: RECV DBGL level */
		apiDebug(ptr_uartStruct);
		/*readModifyDebugLevel(ptr_uartStruct);*/
		break;
	case commandKeyNumber_DBGM: /*set/get only debug system mask*/
		/* command : DBGM [mask]
		 * set response: ...
		 * get response: RECV DBGM mask*/
		apiDebug(ptr_uartStruct);
		/*readModifyDebugMask(ptr_uartStruct);*/
		break;
	case commandKeyNumber_JTAG: /*toggle/set JTAG availability*/
	   modifyJTAG(ptr_uartStruct);
		break;
	case commandKeyNumber_OWTR: /*trigger one-wire device(s) for action, if possible*/
		break;
	case commandKeyNumber_HELP: /*output some help*/
		help(ptr_uartStruct);
		break;
    case commandKeyNumber_OWMR: /*one wire basics: match rom*/
       break;
    case commandKeyNumber_OWPC: /*one wire basics: presence check*/
       break;
    case commandKeyNumber_OWRb: /*one wire basics: receive bit, wait for it*/
       break;
    case commandKeyNumber_OWRB: /*one wire basics: receive byte*/
       break;
    case commandKeyNumber_OWSB: /*one wire basics: send byte*/
       break;
    case commandKeyNumber_OWSC: /*one wire basics: send command*/
       break;
    case commandKeyNumber_OWSA: /*one wire API settings: set/get 1-wire specific API settings*/
       owiApi(ptr_uartStruct);
       break;
    case commandKeyNumber_WDOG: /*set/get watch dog status*/
       break;
//    case commandKeyNumber_EXIT: /*exit*/
//       break;
    case commandKeyNumber_RLTH: /* relay threshold */
       relayThreshold(ptr_uartStruct);
       break;
    case commandKeyNumber_CMD1: /* command (dummy name) */
#warning TODO combine PORT/PINA access to two general fcn, e.g. PORT/PIN [ABCDEF] [and|or|nor|nand] <val>
       PORTA |= 0x01;
       break;
    case commandKeyNumber_CMD2: /* command (dummy name) */
       PORTA &= ~(0x01);
       break;
    case commandKeyNumber_CMD3: /* command (dummy name) */
       PINA |= 0x01;
       break;
    case commandKeyNumber_CMD4: /* command (dummy name) */
       break;
    case commandKeyNumber_GNWR: /* command (dummy name) */
      /* command      : RGWR Register Value */
      /* response now : RECV the value %x has been written in Register */
      /* response TODO: RECV RGWR Register Value (OldValue) */
      /* waveformGeneratorWriteRegister(ptr_uartStruct); /* call function with name writeRegister  */
      waveformGeneratorWriteRegister(ptr_uartStruct); /* call function with name waveformGeneratorWriteRegister  */
       break;
    case commandKeyNumber_GNRE: /* command (dummy name) */
      /* command      : GNRE Register*/
      /* response now : RECV the value %x has been written in Register */
      /* response TODO: RECV GNWR Register Value */
      waveformGeneratorReadRegister(ptr_uartStruct); /* call function with name  waveformGeneratorReadRegister */
      break;
    case commandKeyNumber_OW8S: /* command (dummy name) */
    	owiOctalSwitches(ptr_uartStruct);
    	break;
    case commandKeyNumber_TWIS: /* i2c interface (twi) */
    	twiMaster(ptr_uartStruct);
       break;
    case commandKeyNumber_VERS: /* command (dummy name) */
    	version();
       break;
    default:
		ptr_uartStruct->commandKeywordIndex = -1;
		Check_Error(ptr_uartStruct);
		break;
	}
	return;
}//END of Choose function

/**************************/
/* interrupt for overflow with defined interrupt vector TIMER0_OVF_vect */

ISR(TIMER0_OVF_vect)
{

	static uint8_t counter0 = 0; /* counter for timer0 */
	static uint8_t counter1 = 0; /* counter for timer0 */
	counter0++;
	counter1++;

	if ( counter0 == 10 ) /* 10 overflow for timer0=0.5s */
	{

		timer0Ready = 1; /* mark, that we got an CAN_interrupt, to be handled by main */
		counter0 = 0;
	}
	if ( counter1 == 20 ) /* 10 overflow for timer1=1s */
	{

		timer1Ready = 1; /* mark, that we got an CAN_interrupt, to be handled by main */
		counter1 = 0;
	}
}//END of ISR(TIMER0_OVF_vect)


/* interrupt for output compare with defined interrupt vector SIG_OUTPUT_COMPARE0 */
ISR(SIG_OUTPUT_COMPARE0)
{
	static uint8_t counter0A = 0; /* counter for timer0 with output compareA */
	static uint8_t counter0AScheduler = 0; /* counter for timer0 with output compareA */
	counter0A++;
	counter0AScheduler++;

	if ( counter0A == 9 ) /*9 overflow for timer0A=0.6s*/
	{
		timer0AReady = 1; /* mark, that we got an CAN_interrupt, to be handled by main */
		counter0A = 0;
	}
	if ( counter0AScheduler == 40 ) /*9 overflow for timer0A=2s*/
	{
		timer0ASchedulerReady = 1; /* mark, that we got an CAN_interrupt, to be handled by main */
		counter0AScheduler = 0;
	}
}//END of ISR(SIG_OUTPUT_COMPARE0)


/* interrupt for receive  data via serial communication with defined interrupt vector SIG_UART0_RECV */
ISR (SIG_UART0_RECV)
{

	unsigned char c = UDR0;

	if ( c == '\n' ) /* the string is complete? */
	{

		uartString[nextCharPos] = '\0';
		uartReady = 1; /* mark, that we got an CAN_interrupt, to be handled by main */
		nextCharPos = 0;
	}

	else
	{
		uartString[nextCharPos] = c;
		nextCharPos++;
		uartString[nextCharPos] = '\0';
	}
}//END of ISR (SIG_UART0_RECV)


/* interrupt for receive data via CAN communication with defined interrupt vector CANIT_vect */
ISR(CANIT_vect)
{
	uint8_t save_canpage = CANPAGE;

	if ( (CANHPMOB & 0xF0) != 0xF0 )
	{
		// --- interrupt generated by a MOb

		mob = (CANHPMOB & 0xf0) >> 4;
		CANPAGE = (CANHPMOB & 0xf0);

		if (Get_CanError() == 1)
		{ // check if MOb has an error
			CANCDMOB = ( 1 << CONMOB0 ); /* reset receive mode*/
		}

		else if ( CANSTMOB & ( 1 << TXOK ) )
		{ // MOb finished transmission
			CANSTMOB &= ~( 1 << TXOK );  /* disable transmission mode*/
			CANCDMOB = ( 1 << CONMOB0 ); /* reset receive mode*/
		}
		else if ( CANSTMOB & ( 1 << RXOK ) )
		{ // MOb received message
			ptr_canStruct->Can_Length = ( CANCDMOB & ( 0xF << DLC0 ) );
			for ( uint8_t i = 0 ; i < ptr_canStruct->Can_Length ; i++ )
			{
				/* get data of selected MOb */
				ptr_canStruct->Can_Data[i] = CANMSG;
			}
			/*get identifier */
			ptr_canStruct->Can_Message_ID = 0;
			ptr_canStruct->Can_Message_ID = CANIDT2 >> 5;
			ptr_canStruct->Can_Message_ID |= CANIDT1 << 3;
			ptr_canStruct->Can_Mob = mob; /*get mailbox */
			CANSTMOB &= ~( 1 << RXOK ); /* clear interrupt */
			CANCDMOB = ( 1 << CONMOB1 ); /* reset receive mode*/
			canReady = 1; /* mark, that we got an CAN_interrupt, to be handled by main */
		}
		CANPAGE = save_canpage; /* restore CANPAGE*/
	}
	else {
		// --- general interrupt was generated
		Get_BusState();
		CANGIT |= 0;
	}
}//END of ISR(CANIT_vect)

/*
 *USART0 must be initialized before using this function
 *for use this function, be sure that data is no longer then 8 bit
 *(single character) this function will send  8 bits from the at90can128
 */

#warning add timeout ? Add check for USART0 init ?

void UART0_Transmit( uint8_t data )
{
	/* wait for empty transmit buffer */
	while ( !( UCSR0A & ( 1 << UDRE0 ) ) )
	{
		/* do nothing*/
	}
	/* put the data into the buffer */

	UDR0 = data;
}

/*
 *this function sends a string to serial communication
 * the input parameter is a defined global variable tmp_str
 * the output parameter is an integer
 * 0 -> the message is sent
 */

int16_t UART0_Send_Message_String( char *tmp_str, uint16_t maxSize )
{
	/* this function sends the string pointed to by tmp_str to UART
	 * this happens char by char until STRING_END is found
	 * afterwards the message string is cleared up to the size maxSize
	 *
	 * if tmp_str is NULL
	 *    - tmp_str is set to the global variable uart_message_string
	 *    - and maxSize is set to BUFFER_SIZE
	 * else if maxSize is 0
	 *    - maxSize is assumed to be BUFFER_SIZE
	 *
	 * returns
	 *   - in case of errors: -1
	 *   - else number of transfered characters (max INT16_MAX )
	 *
	 * */

	/* if tmp_str is NULL, take as default uart_message_string and its size BUFFER_SIZE */
	if (NULL == tmp_str)
	{
		if (NULL != uart_message_string)
		{
			tmp_str = uart_message_string;
			maxSize = BUFFER_SIZE;
		}
		else
		{
			return -1;
		}
	}

	/* if maxSize is set to 0, take default value BUFFER_SIZE */
	if ( 0 == maxSize)
	{
		maxSize = BUFFER_SIZE;
	}

	static uint16_t index;

	for ( index = 0; STRING_END != tmp_str[index] && index < maxSize ; index++ )
	{
		UART0_Transmit(tmp_str[index]);
	}
	UART0_Transmit('\n');
	clearString(tmp_str, maxSize); /*clear tmp_str variable*/

	return index;
}//END of UART0_Send_Message_String function


/*
 *this function sends a string to serial communication
 * the input parameter is a defined global variable tmp_str
 * the output parameter is an integer
 * 0 -> the message is sent
 * only use for the function owiFindFamilyDevicesAndAccessValues in one_wire.c
 */
int8_t UART0_Send_Message_String_woLF( char *tmp_str, uint32_t maxSize )
{
	for ( uint16_t j = 0 ; j < strlen((char *) tmp_str) && j < maxSize ; j++ )
	{
		UART0_Transmit(tmp_str[j]);
	}

	clearString(tmp_str, maxSize); /*clear tmp_str variable*/
	return 0;
}//END of UART0_Send_Message_String_woLF


uint8_t CommunicationError( uint8_t errorType, const int16_t errorIndex, const uint8_t flag_printCommand, const prog_char *alternativeErrorMessage, const int16_t alternativeErrorNumber)
{
/* This (new) Communication Error function
 * sends a ((partly) predefined) error message to UART
 *
 * output: ERRG/C/U/A/M <error number> <error message> [<alternative/extra Error> <alternative/extra number>]
 * output: ERRA/C/U/A/M "<command key> <command arguments>" --- <error number> <error message>
 *
 * arguments:
 *    - Error type: defines the error class the error belongs to (ERRG, ERRA, ERRM, ERRC, ERRU)
 *      - use enums as input
 *         - ERRG: general errors
 *         - ERRA: API specific
 *            - considered to be specific to a certain command call,
 *              therefore it delivers in addition to the error message, the calling command string
 *         - ERRC/ERRM: can (call/hardware) specific errors
 *            - if considered to be specific to a certain command call,
 *              then it delivers in addition to the error message, the calling command string
 *         - ERRT: TWI specific errors
 *            - if considered to be specific to a certain command call,
 *              then it delivers in addition to the error message, the calling command string
 *         - ERRU: undefined errors
 *      - if  Error_typ > ERRU, Error_typ = ERRU
 *    - Error_Index: selection index of predefined error messages
 *      - if index > MAX_INDEX or 0 > index, the next arguments are used
 *    - flag_printCommand
 *      - if 0 do nothing
 *      - else add command sequence to error message
 *    - alternativeErrorMessage: message to be printed/attached if Error_Index exceeds limits / else
 *       - by default prog_char, i.e. FLASH data (otherwise see below)
 *    - alternativeErrorNumber :  number to be printed/attached if Error_Index exceeds limits / else
 *       - if this value is less than COMMUNICATION_ERROR_USE_GLOBAL_MESSAGE_STRING_INDEX_THRESHOLD:
 *            value < COMMUNICATION_ERROR_USE_GLOBAL_MESSAGE_STRING_INDEX_THRESHOLD,
 *            then instead of the text the global variable message is used
 *
 * returns:
 *    - 0 , if all o.k.
 *    - <> 0, else
 * */

    unsigned char flag_UseOnlyAlternatives = FALSE;
    unsigned char errmax[5];

    errmax[ERRG] = GENERAL_ERROR_MAXIMUM_INDEX;
    errmax[ERRA] = SERIAL_ERROR_MAXIMUM_INDEX;
    errmax[ERRC] = CAN_ERROR_MAXIMUM_INDEX;
    errmax[ERRM] = MOB_ERROR_MAXIMUM_INDEX;
    errmax[ERRT] = TWI_ERROR_MAXIMUM_INDEX;

    /* checking error type */
    if ( ERR_MAXIMUM_NUMBER <= errorType )
    {
       errorType = ERRU; /* set to undefined error */
    }

/* checking error index */
    switch(errorType)
    {
       case ERRA:
       case ERRC:
       case ERRG:
       case ERRM:
       case ERRT:
          if (0 > errorIndex || errmax[errorType] <= errorIndex)
          { flag_UseOnlyAlternatives = TRUE; }
          break;
       case ERRU:
          flag_UseOnlyAlternatives = TRUE;
          break;
       default:
          if ( eventDebug <= debug )
          {
             for ( int clearIndex = 0 ; clearIndex < BUFFER_SIZE ; clearIndex++ )
             {
                message[clearIndex] = STRING_END;
             }
              snprintf_P(message, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) CommunicationError wrong error index ... returning"), __LINE__, __FILE__);
              UART0_Send_Message_String(message,BUFFER_SIZE);
          }
          return 1; /*shouldn't happen*/
          break;
    }

    /* check alternative pointer, return if empty*/
    if ( TRUE == flag_UseOnlyAlternatives && NULL == alternativeErrorMessage && COMMUNICATION_ERROR_USE_GLOBAL_MESSAGE_STRING_INDEX_THRESHOLD <= alternativeErrorNumber)
    {
        if ( eventDebug <= debug )
          {
             for ( int clearIndex = 0 ; clearIndex < BUFFER_SIZE ; clearIndex++ )
             {
                message[clearIndex] = STRING_END;
             }
             snprintf_P(message, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) CommunicationError alternatives: NULL ... returning"), __LINE__, __FILE__);
             UART0_Send_Message_String(message,BUFFER_SIZE);
          }
          return 1;
    }

    /* compose message
     * output: ERRG/C/U/A/M <error number> <error message> [<alternative/extra Error> <alternative/extra number>]
     * output: ERRA/C/U/A/M "<command key> <command arguments>" --- <error number> <error message>
     * */
    /* clear string*/
    clearString(uart_message_string, BUFFER_SIZE);

    /* cat Error Type */
    strncat_P(uart_message_string, (const char*) (pgm_read_word( &(errorTypes[errorType]))) ,BUFFER_SIZE - 1 );

    /* "<command key> <command arguments>" */
    if ( TRUE == flag_printCommand)
    {
       /* no command string parsing yet*/
       if ( STRING_END != decrypt_uartString[0]) /*assume decrypt_uartString  not to be empty */
       {
          strncat_P(uart_message_string, PSTR(" \""), BUFFER_SIZE -1 );
          strncat  (uart_message_string, decrypt_uartString, BUFFER_SIZE -1 );
          strncat_P(uart_message_string, PSTR("\""), BUFFER_SIZE -1 );
       }
       else /* command string already parsed: setParameter available */
       {
          uint8_t index=0;
          strncat_P(uart_message_string, PSTR(" \""), BUFFER_SIZE -1 );

          while (index < MAX_PARAMETER && NULL != setParameter[index] && STRING_END != setParameter[index][0])
          {
             snprintf_P(uart_message_string, BUFFER_SIZE -1 , PSTR("%s%s%s"), uart_message_string, (0 == index)?"":" ", &setParameter[index][0]);
             index++;
          }
          snprintf_P(uart_message_string, BUFFER_SIZE -1 , PSTR("%s\" ---"), uart_message_string);
       }
    }

    if ( FALSE == flag_UseOnlyAlternatives )
    {
       switch (errorType)
       {
          case ERRG:
             snprintf_P(uart_message_string, BUFFER_SIZE -1 , PSTR("%s %i "), uart_message_string, general_error_number[errorIndex]);
             strncat_P(uart_message_string, (const char*) (pgm_read_word( &(general_error[errorIndex]))), BUFFER_SIZE -1);
             break;
          case ERRA:
             snprintf_P(uart_message_string, BUFFER_SIZE -1 , PSTR("%s %i "), uart_message_string, serial_error_number[errorIndex]);
             strncat_P(uart_message_string, (const char*) (pgm_read_word( &(serial_error[errorIndex]))), BUFFER_SIZE -1);
             break;
          case ERRM:
             snprintf_P(uart_message_string, BUFFER_SIZE -1 , PSTR("%s %i "), uart_message_string, mob_error_number[errorIndex]);
             strncat_P(uart_message_string, (const char*) (pgm_read_word( &(mob_error[errorIndex]))), BUFFER_SIZE -1);
             break;
          case ERRC:
             snprintf_P(uart_message_string, BUFFER_SIZE -1 , PSTR("%s %i "), uart_message_string, can_error_number[errorIndex]);
             strncat_P(uart_message_string, (const char*) (pgm_read_word( &(can_error[errorIndex]))), BUFFER_SIZE -1);
             break;
          case ERRT:
              snprintf_P(uart_message_string, BUFFER_SIZE -1 , PSTR("%s %i "), uart_message_string, twi_error_number[errorIndex]);
              strncat_P(uart_message_string, (const char*) (pgm_read_word( &(twi_error[errorIndex]))), BUFFER_SIZE -1);
              break;
          default:
             if ( eventDebug <= debug )
             {
                for ( int clearIndex = 0 ; clearIndex < BUFFER_SIZE ; clearIndex++ )
                {
                   message[clearIndex] = STRING_END;
                }
                snprintf_P(message, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) CommunicationError wrong error type %i... returning"), __LINE__, __FILE__, errorType);
                UART0_Send_Message_String(message,BUFFER_SIZE);
             }
             return 1;
             break;
       }
    }

    if ( NULL != alternativeErrorMessage || COMMUNICATION_ERROR_USE_GLOBAL_MESSAGE_STRING_INDEX_THRESHOLD > alternativeErrorNumber)
    {
       if (TRUE == flag_UseOnlyAlternatives)
       {
          snprintf_P(uart_message_string,  BUFFER_SIZE - 1 , PSTR("%s %i "), uart_message_string, alternativeErrorNumber);
       }
       else
       {
          strncat_P(uart_message_string, PSTR(" *** "), BUFFER_SIZE -1);
       }

       /* if alternativeErrorIndex is smaller then COMMUNICATION_ERROR_USE_GLOBAL_MESSAGE_STRING_INDEX_THRESHOLD use global message variable instead*/
       if ( COMMUNICATION_ERROR_USE_GLOBAL_MESSAGE_STRING_INDEX_THRESHOLD > alternativeErrorNumber )
       {
          strncat(uart_message_string, message, BUFFER_SIZE -1);
       }
       else
       {
          strncat_P(uart_message_string, alternativeErrorMessage, BUFFER_SIZE -1);
       }
    }

    UART0_Send_Message_String(NULL,0);

    return 0;
}//END of CommunicationError function

/*
 this function initializes all init functions again and activates the interrupt
 */
void Initialization( void )
{
#warning TODO find a generalized way to check for correct init and a modular possibility to fail and still run, e.g. relay
   uint16_t status = 0;
   /* disable interrupts*/
   cli();

   UART0_Init();

   Init_Port();

   disableJTAG(FALSE);

   twim_init = Twim_Init (250000);

   if( FALSE == twim_init)
   {
	   twi_errorCode = CommunicationError(ERRT, TWI_ERROR_Error_in_initiating_TWI_interface, 0, NULL, 0);
   }

#if (HADCON_VERSION == 1)
   owi_init = TRUE;
#elif (HADCON_VERSION == 2)
   owi_init = OWI_Init(0x3f);
#else
   owi_init = TRUE;
#endif

   if( FALSE == owi_init)
   {
#warning create realistic error message
	//   owi_errorCode = CommunicationError(ERRA, , 0, NULL, 0);
   }

   can_init = CAN_Init(CAN_DEFAULT_BAUD_RATE); /* initialize can-controller with a baudrate 250kps */

   if ( -1 == can_init )
   {
      can_errorCode = CommunicationError(ERRC, CAN_ERROR_CAN_was_not_successfully_initialized, 0, NULL, 0);
      exit(0);
   }
   else
   {
      timer0_init = Timer0_Init(); /* initialize first timer */
   }

   if ( 1 != timer0_init )
   {
      general_errorCode = CommunicationError(ERRG, GENERAL_ERROR_init_for_timer0_failed, 0, NULL, 0);
      exit(0);
   }
   else
   {
      timer0A_init = Timer0A_Init(); /* initialize second timer*/
   }
   if ( 1 != timer0A_init )
   {
      general_errorCode = CommunicationError(ERRG, GENERAL_ERROR_init_for_timer0A_failed, 0, NULL, 0);
      exit(0);
   }

   /* enable interrupts*/
   sei();

   clearString(currentResponseKeyword,MAX_LENGTH_KEYWORD);
   strncat_P(currentResponseKeyword, (const char*) ( pgm_read_word( &(responseKeywords[responseKeyNumber_RECV])) ), MAX_LENGTH_KEYWORD - 1);

   // start relay init
   if ( FALSE != relayThresholdEnable_flag)
   {
	   status = relayThresholdInit();

	   if (FALSE == status)
	   {
		   snprintf_P(message, BUFFER_SIZE, PSTR("Relay Init Failed"));
		   CommunicationError(ERRA, -1, TRUE, message, COMMUNICATION_ERROR_USE_GLOBAL_MESSAGE_STRING_INDEX_THRESHOLD -1 );

#warning TODO: missing action in case of failure: either exit or deactivate relay
	   }
   }
# warning remove it
//PORTG |= (1<<PG2);

}//END of Initialization

/*This function initializes all input /output of the microcontroller
 *the  function has no input and output variable
 */

void Init_Port( void )
{
   uint8_t intstate = SREG; /* save global interrupt flag */
   cli();

#warning check right setting while merging, DDRA=0x00 could also be possible
   /* DDRX = 0xff =>  output ,  PORTX = 0x00 => disabled pullups*/
   DDRA  = 0xFF; //(1 << DDA0)| (1 << DDA1)| (1 << DDA2)| (1 << DDA3)| (1 << DDA4)| (1 << DDA5)| (1 << DDA6)| (1 << DDA7);
   PORTA = 0x00; //(0 << PA0) | (0 << PA1) | (0 << PA2) | (0 << PA3) | (0 << PA4) | (0 << PA5) | (0 << PA6) | (0 << PA7);
#warning probably n1 << Deded by MDC Relay RLTH
//   PORTA |= 0x08; (MDC Relay)

   /* use port B as input with activated pullups*/
   DDRB  = 0x00; //(0 << DDB0)| (0 << DDB1)| (0 << DDB2)| (0 << DDB3)| (0 << DDB4)| (0 << DDB5)| (0 << DDB6)| (0 << DDB7);
   PORTB = 0xFF; //(1 << PC0) | (1 << PC1) | (1 << PC2) | (1 << PC3) | (1 << PC4) | (1 << PC5) | (1 << PC6) | (1 << PC7);

# warning TODO: is this comment true?
   /* use port C as mixed I/O with input channels with activated pullups*/
   DDRC = 0xff & ( ~(1 << DDC2) ); //(1 << DDC0)| (1 << DDC1)| (0 << DDC2)| (1 << DDC3)| (1 << DDC4)| (1 << DDC5)| (1 << DDC6)| (1 << DDC7);
   PORTC = 0xff;                   //(1 << PC0) | (1 << PC1) | (1 << PC2) | (1 << PC3) | (1 << PC4) | (1 << PC5) | (1 << PC6) | (1 << PC7);

   /* use port E as input with activated pullups*/
   DDRE  = 0x00; //(0 << DDE0)| (0 << DDE1)| (0 << DDE2)| (0 << DDE3)| (0 << DDE4)| (0 << DDE5)| (0 << DDE6)| (0 << DDE7);
   PORTE = 0xFF; //(1 << PE0) | (1 << PE1) | (1 << PE2) | (1 << PE3) | (1 << PE4) | (1 << PE5) | (1 << PE6) | (1 << PE7);

   /* use port F as input with activated pullups*/
   DDRF  = 0x00; //(0 << DDF0)| (0 << DDF1)| (0 << DDF2)| (0 << DDF3)| (0 << DDF4)| (0 << DDF5)| (0 << DDF6)| (0 << DDF7);
   PORTF = 0xFF; //(1 << PF0) | (1 << PF1) | (1 << PF2) | (1 << PF3) | (1 << PF4) | (1 << PF5) | (1 << PF6) | (1 << PF7);

#if HADCON_VERSION == 2
   /* use port G as output with deactivated pullups for lower pins (3 LEDs and PG3 for JTAG pull-ups*/
   if ( eventDebug <= debug && ( ( debugMask >> debugApi ) & 0x1 ) )
   {
      snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s, %s): setting ports for JTAG: PING:0x%x, PORTG:0x%x"), __LINE__, __FILE__,__FUNCTION__, PING&0xFF, PORTG&0xFF);
      UART0_Send_Message_String(NULL,0);
   }

   DDRG  = (1 << DDG0)| (1 << DDG1)| (1 << DDG2)| (1 << DDG3) | (0 << DDG4);
   PORTG = (0 << PG0) | (0 << PG1) | (1 << PG2) | (1 << PG3)  | (1 << PG4);

   if ( eventDebug <= debug && ( ( debugMask >> debugApi ) & 0x1 ) )
   {
	   _delay_ms(1000);
	   snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s, %s): setting ports for JTAG: PING:0x%x, PORTG:0x%x"), __LINE__, __FILE__,__FUNCTION__, PING&0xFF, PORTG&0xFF);
	   UART0_Send_Message_String(NULL,0);
   }
#endif

   /* enable ADC and set clock prescale factor to 64 (p.280)*/
   ADCSRA = BIT (ADEN) | 0x06;
   /* 0x80 => ADEN => enable ADC*/

   /* low speed and manually triggered ADC conversion mode*/
   ADCSRB = 0;

   /* deactivate digital input buffer for used ADC pins*/
#warning during merge: different opinions to activate this #if 0/1
#if 1
   DIDR0 = 0x0F;
#else
#ifdef ALLOW_DISABLE_JTAG
   DIDR0 = 0xFF;
   /*MCUCR |= 0x80;   // deactivate whole JTAG interface */
#else
   DIDR0 = 0x0F;
#endif
#endif

   /* enable only compare match interrupt on timer 1, channel A*/
   TIMSK1 = 0x02;

   /* normal counter mode (no wave and/or PWM stuff) for timer 1, channel A*/
   TCCR1A = 0;
   TCCR1C = 0;

   /* snprintf_P(uart_message_string,BUFFER_SIZE-1, PSTR("init finished") );
    UART0_Send_Message_String(NULL,0);*/

   SREG = intstate; /*restore global interrupt flag */

}//END of Init function

/*this function checks the functionality of the software*/
void keep_alive( struct uartStruct *ptr_uartStruct )
{
	flag_pingActive = ( 0 != ptr_uartStruct->Uart_Message_ID );

	snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("RECV PING mechanism is %s"), ( flag_pingActive ) ? "enabled" : "disabled");
	UART0_Send_Message_String(NULL,0);
}//END of keep_alive

//void SwitchPinPeriodically()
//{
//   /*
//    for (int j=0;1;j++) {
//    for(i=1;i<=100;i++) {
//    _delay_us(1);
//    }
//    if (j && 0 == j%1000 )
//    {
//    snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("--- alive (line:%i) --- %i "),
//    __LINE__, j);
//    UART0_Send_Message_String(NULL,0);
//    }
//    //      PORTA ^=  0xFF ;
//    }
//    */
//}

void clearUartStruct( struct uartStruct *ptr_uartStruct ) /* resets all values of uartStruct*/
{ /*resets the uartStruct structure to "0" */
   if ( NULL != ptr_uartStruct )
   {
      ptr_uartStruct->Uart_Message_ID = 0;
      ptr_uartStruct->Uart_Mask = 0;
      ptr_uartStruct->Uart_Rtr = 0;
      ptr_uartStruct->Uart_Length = 0;
      ptr_uartStruct->commandKeywordIndex = 0;
      ptr_uartStruct->number_of_arguments = 0;
      for ( uint8_t clearIndex = 0 ; clearIndex < UART_MAX_DATA_ELEMENTS ; clearIndex++ )
      {
         ptr_uartStruct->Uart_Data[clearIndex] = 0;
      }
   }
}//END of clearuartStruct

/* creates the string "RECV <current command keyword> "
 * from the information of the uart struct
 * input:
 *    ptr_myUartStruct:
 *       pointer to the uart struct
 *       - if NULL, set to ptr_uartStruct global variable
 *    message_string:
 *       pointer to char array where the created message should be stored
 *       - if NULL, set to uart_message_string global variable
 *    size:
 *       maximum size of message_string
 *       - if 0, set to BUFFER_SIZE
 * return:
 *  always: 0
 */

uint8_t createReceiveHeader( struct uartStruct *ptr_myUartStruct, char message_string[], uint16_t size )
{
   if (NULL == ptr_myUartStruct) {ptr_myUartStruct = ptr_uartStruct;}
   if (NULL == message_string) { message_string = uart_message_string; }
   if (0 == size) { size = BUFFER_SIZE; }

   if ( eventDebug <= debug && ( ( debugMask >> debugApi ) & 1 ) )
   {
	   clearString(message_string, size);
	   snprintf_P(message_string, size -1,PSTR("DEBUG (%4i, %s) createReceiveHeader: commandKeywordIndex %i"), __LINE__, __FILE__,
			      ptr_myUartStruct->commandKeywordIndex);
	   UART0_Send_Message_String(message_string, size - 1);
   }

   clearString(message_string, size);
   strncat_P(message_string, (const char*) ( pgm_read_word( &(responseKeywords[responseKeyNumber_RECV])) ), size - 1);
   strncat_P(message_string, PSTR(" "), size - 1);

   strncat_P(message_string, (const char*) ( pgm_read_word( &(commandKeywords[ptr_myUartStruct->commandKeywordIndex])) ), size - 1);
   strncat_P(message_string, PSTR(" "), size - 1);
   return 0;
}

/*clear strings*/
uint16_t clearString( char mystring[], uint16_t length )
{
   if (NULL == mystring) {return 0;}
   uint16_t clearIndex = 0;
   for (; clearIndex < length ; clearIndex++) {mystring[clearIndex] = STRING_END;}
   return clearIndex;
}

/*
 *this function toggles endlessly ! PINA3
 */

#warning TODO finish this function (pin_number and timeout/duration)
void toggle_pin( unsigned char pin_number )
{
   long int i, c, d;
   _delay_ms(10);

   for ( i = 0; i <= 50 ; i++ )
   {
      _delay_ms(10);
      d = d + i * c;
      c = d + i;
   }
   /*toggle FrontLED*/
   PINA = 0x3;
}//END of toggle_pin function


/*
 * adds to the normal receive header created internally the additional command key word of sub command
 * input: uart container
 *  - key index number of primary command, or -1 for current
 *  - command Keyword of sub command
 */

void createExtendedSubCommandReceiveResponseHeader(struct uartStruct * ptr_uartStruct,
                                                   int8_t keyNumber, int8_t index,  const prog_char* commandKeywords[])
{
   /* make sure keyNumber is shown */
   int8_t keywordIndex = ptr_uartStruct->commandKeywordIndex;

   if (0 <= keyNumber) {ptr_uartStruct->commandKeywordIndex = keyNumber;}

   createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);

   /* reset index to original value */
   ptr_uartStruct->commandKeywordIndex = keywordIndex;

   if (-1 < index && NULL != commandKeywords)
   {
	   strncat_P(uart_message_string, (const char*) ( pgm_read_word( &(commandKeywords[index])) ), BUFFER_SIZE - 1);
	   strncat_P(uart_message_string, PSTR(" "), BUFFER_SIZE - 1);
   }
}

/*
 * getNumericLength(const char *string, const uint16_t maxLenght)
 *
 * return number of leading characters of string
 * being an digit (0x/0X of hex numbers are ignored) up to the maximum maxLength-1
 */

uint16_t getNumericLength(const char string[], const uint16_t maxLength)
{
    /* check if argument contains of digits */
    uint16_t index = 0;
    if (
             0 == strncmp_P( string, PSTR("0x"),2) ||
             0 == strncmp_P( string, PSTR("0X"),2)
        )
    {
       index = 2;
    }

    /* calculate length of argument and check if all of them are hex numbers */
    while (index < maxLength && isxdigit(string[index])) {index++;}
    if ( eventDebug <= debug && ( ( debugMask >> debugApi ) & 1 ) )
    {
       snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) getNumericLength: found length is %i"), __LINE__, __FILE__, index);
       UART0_Send_Message_String(NULL,0);
    }

    if (index+1 < maxLength)
    {
       if   ( ! ( isspace(string[index+1]) || ('\0' == string[index+1] ) ) )
       {
          if ( eventDebug <= debug && ( ( debugMask >> debugApi ) & 1 ) )
          {
             snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("DEBUG (%4i, %s) getNumericLength: length 0 - value is followed by non-space character ASCII: %i, '%c'"),
                        __LINE__, __FILE__, string[index+1], string[index+1]);
             UART0_Send_Message_String(NULL,0);
          }
          index = 0; /* numeric value is not separated from next word nor isn't followed by '\0', so it isn't a number, */
       }
    }

    return index;
}

int8_t getNumericValueFromParameter(uint8_t parameterIndex, uint32_t *ptr_value)
{
	if ( MAX_PARAMETER < parameterIndex || NULL == ptr_value)
	{
		CommunicationError(ERRG, -1, TRUE, PSTR("getNumericValueFromParameter: wrong input parameters"), -1);
		return -1;
	}

    /* T,F,t,f : T(RUE)/F(ALSE) */
    if ( 0 == strcmp_P(setParameter[parameterIndex], PSTR("TRUE")) || 0 == strcmp_P(setParameter[parameterIndex], PSTR("true")) )
    {
    	*ptr_value = TRUE;
    }
    else if ( 0 == strcmp_P(setParameter[parameterIndex], PSTR("FALSE")) || 0 == strcmp_P(setParameter[parameterIndex], PSTR("false")) )
    {
    	*ptr_value = FALSE;
    }
    else if ( 0 < getNumericLength(setParameter[parameterIndex], MAX_LENGTH_PARAMETER) )
	{
		*ptr_value = strtoul(setParameter[parameterIndex], &ptr_setParameter[parameterIndex], 16);
	}
	else
	{
		CommunicationError(ERRA, -1, TRUE, PSTR("command argument not a numeric value"), -1);
		return -1;
	}
	return 0;
}

void reset(struct uartStruct *ptr_uartStruct)
{
	createReceiveHeader(NULL, NULL, 0);
    UART0_Send_Message_String(NULL,0);

    Initialization();
}

