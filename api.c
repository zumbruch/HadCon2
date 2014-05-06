/*
   Licensed under the EUPL V.1.1, Lizenziert unter EUPL V.1.1 
*/
/*
 * VERSION 1.0 Januar 7th 2010 LATE  File: 'api.c'
 * Author: Linda Fouedjio
 * modified (heavily rather rebuild): Peter Zumbruch
 * modified: Florian Feldbauer
 * modified: Peter Zumbruch, Oct 2011
 * modified: Peter Zumbruch, May 2013
 */

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <stddef.h>
#define __DELAY_BACKWARD_COMPATIBLE__
#include <util/delay.h>
#include <util/atomic.h>
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
#include "spiApi.h"
#include "apfelApi.h"

#include "api_debug.h"
#include "api_show.h"
#include "api_help.h"
#include "api_debug.h"
#include "jtag.h"
#include "can.h"
#include "mem-check.h"
#include "api_version.h"
#include "api_identification.h"
#include "twi_master.h"

#include "adc.h"
#include "api_define.h"
#include "api_global.h"
#include "api.h"
#ifdef TESTING_ENABLE
#include "testing.h"
#endif

static const char filename[] PROGMEM = __FILE__;
static const char string_s_i_[] PROGMEM = "%s %i ";
static const char string_sX[]   	PROGMEM = "%s%X";
//static const char string_sx[]     PROGMEM = "%s%x";

#warning TODO: combine responseKeyword and other responses error into one set of responses

static const char responseKeyword00[] PROGMEM = "RECV";
static const char responseKeyword01[] PROGMEM = "CANR";
static const char responseKeyword02[] PROGMEM = "SYST";

const char* const responseKeywords[] PROGMEM = {
        responseKeyword00,
        responseKeyword01,
        responseKeyword02
};


/* those are the
 * 		command keywords
 *      command short descriptions
 *      command implementation status
 * of the API*/

// index: 00
static const char commandKeyword00[]           PROGMEM = "SEND";
static const char commandSyntax00[]            PROGMEM = "CAN-ID ID-Range [RTR <nBytes> D0 .. D7]";
static const char commandSyntaxAlternative00[] PROGMEM = "";
static const char commandShortDescription00[]  PROGMEM = "send can message";
static const uint8_t commandImplementation00   PROGMEM = TRUE;

// index: 01
static const char commandKeyword01[]           PROGMEM = "SUBS";
static const char commandSyntax01[]            PROGMEM = "CAN-ID ID-Range";
static const char commandSyntaxAlternative01[] PROGMEM = "";
static const char commandShortDescription01[]  PROGMEM = "unsubscribe can id/mask";
static const uint8_t commandImplementation01   PROGMEM = TRUE;

// index: 02
static const char commandKeyword02[]           PROGMEM = "USUB";
static const char commandSyntax02[]            PROGMEM = "CAN-ID ID-Range";
static const char commandSyntaxAlternative02[] PROGMEM = "";
static const char commandShortDescription02[]  PROGMEM = "unsubscribe can id/mask";
static const uint8_t commandImplementation02   PROGMEM = TRUE;

// index: 03
static const char commandKeyword03[]           PROGMEM = "STAT";
static const char commandSyntax03[]            PROGMEM = "[ID]";
static const char commandSyntaxAlternative03[] PROGMEM = "";
static const char commandShortDescription03[]  PROGMEM = "";
static const uint8_t commandImplementation03   PROGMEM = FALSE;

// index: 04
static const char commandKeyword04[]           PROGMEM = "RGWR";
static const char commandSyntax04[]            PROGMEM = "<Register> <Value>";
static const char commandSyntaxAlternative04[] PROGMEM = "";
static const char commandShortDescription04[]  PROGMEM = "write register";
static const uint8_t commandImplementation04   PROGMEM = TRUE;

// index: 05
static const char commandKeyword05[]           PROGMEM = "RGRE";
static const char commandSyntax05[]            PROGMEM = "<Register>";
static const char commandSyntaxAlternative05[] PROGMEM = "";
static const char commandShortDescription05[]  PROGMEM = "read register";
static const uint8_t commandImplementation05   PROGMEM = TRUE;

// index: 06
static const char commandKeyword06[]           PROGMEM = "RADC";
static const char commandSyntax06[]            PROGMEM = "[<ADC Channel>]";
static const char commandSyntaxAlternative06[] PROGMEM = "";
static const char commandShortDescription06[]  PROGMEM = "AVR ADCs";
static const uint8_t commandImplementation06   PROGMEM = TRUE;

// index: 07
static const char commandKeyword07[]           PROGMEM = "OWAD";
static const char commandSyntax07[]            PROGMEM = "[ID [flag_conv [flag_init]]]";
static const char commandSyntaxAlternative07[] PROGMEM = "";
static const char commandShortDescription07[]  PROGMEM = "1-wire ADC";
static const uint8_t commandImplementation07   PROGMEM = TRUE;

// index: 08
static const char commandKeyword08[]           PROGMEM = "OWDS";
static const char commandSyntax08[]            PROGMEM = "[ID]";
static const char commandSyntaxAlternative08[] PROGMEM = "";
static const char commandShortDescription08[]  PROGMEM = "1-wire double switch";
static const uint8_t commandImplementation08   PROGMEM = TRUE;

// index: 09
static const char commandKeyword09[]           PROGMEM = "INIT";
static const char commandSyntax09[]            PROGMEM = "";
static const char commandSyntaxAlternative09[] PROGMEM = "";
static const char commandShortDescription09[]  PROGMEM = "(re)init of system";
static const uint8_t commandImplementation09   PROGMEM = TRUE;

// index: 10
static const char commandKeyword10[]           PROGMEM = "OWLS";
static const char commandSyntax10[]            PROGMEM = "[<Family Code>]";
static const char commandSyntaxAlternative10[] PROGMEM = "";
static const char commandShortDescription10[]  PROGMEM = "1-wire list devices";
static const uint8_t commandImplementation10   PROGMEM = TRUE;

// index: 11
static const char commandKeyword11[]           PROGMEM = "OWSS";
static const char commandSyntax11[]            PROGMEM = "[ID]";
static const char commandSyntaxAlternative11[] PROGMEM = "";
static const char commandShortDescription11[]  PROGMEM = "1-wire single switch ([ID] not implemented)";
static const uint8_t commandImplementation11   PROGMEM = TRUE;

// index: 12
static const char commandKeyword12[]           PROGMEM = "RSET";
static const char commandSyntax12[]            PROGMEM = "";
static const char commandSyntaxAlternative12[] PROGMEM = "";
static const char commandShortDescription12[]  PROGMEM = "reset via watchdog";
static const uint8_t commandImplementation12   PROGMEM = TRUE;

// index: 13
static const char commandKeyword13[]           PROGMEM = "PING";
static const char commandSyntax13[]            PROGMEM = "";
static const char commandSyntaxAlternative13[] PROGMEM = "";
static const char commandShortDescription13[]  PROGMEM = "";
static const uint8_t commandImplementation13   PROGMEM = TRUE;

// index: 14
static const char commandKeyword14[]           PROGMEM = "OWTP";
static const char commandSyntax14[]            PROGMEM = "[ID [flag_conv [flag_init]]";
static const char commandSyntaxAlternative14[] PROGMEM = "<command_keyword> [arguments]]";
static const char commandShortDescription14[]  PROGMEM = "1-wire temperature";
static const uint8_t commandImplementation14   PROGMEM = TRUE;

// index: 15
#warning TODO combine with OWRP
static const char commandKeyword15[]           PROGMEM = "OWSP"; /*one-wire set active pins/bus mask*/
static const char commandSyntax15[]            PROGMEM = "<bus mask>";
static const char commandSyntaxAlternative15[] PROGMEM = "";
static const char commandShortDescription15[]  PROGMEM = "one-wire set active pins/bus mask";
static const uint8_t commandImplementation15   PROGMEM = TRUE;

// index: 16
static const char commandKeyword16[]           PROGMEM = "CANT"; /*CAN transmit*/
static const char commandSyntax16[]            PROGMEM = "CAN-ID ID-Range [RTR <nBytes> D0 .. D7]";
static const char commandSyntaxAlternative16[] PROGMEM = "";
static const char commandShortDescription16[]  PROGMEM = "CAN send message";
static const uint8_t commandImplementation16   PROGMEM = TRUE;


// index: 17
static const char commandKeyword17[]           PROGMEM = "CANS"; /* CAN subscribe */
static const char commandSyntax17[]            PROGMEM = "CAN-ID ID-Range";
static const char commandSyntaxAlternative17[] PROGMEM = "";
static const char commandShortDescription17[]  PROGMEM = "CAN subscribe";
static const uint8_t commandImplementation17   PROGMEM = TRUE;

// index: 18
static const char commandKeyword18[]           PROGMEM = "CANU"; /* CAN unsubscribe */
static const char commandSyntax18[]            PROGMEM = "CAN-ID ID-Range";
static const char commandSyntaxAlternative18[] PROGMEM = "";
static const char commandShortDescription18[]  PROGMEM = "CAN unsubscribe";
static const uint8_t commandImplementation18   PROGMEM = TRUE;

// index: 19
static const char commandKeyword19[]           PROGMEM = "CANP"; /* CAN properties */
static const char commandSyntax19[]            PROGMEM = "[<keyword> [value[s]]]";
static const char commandSyntaxAlternative19[] PROGMEM = "";
static const char commandShortDescription19[]  PROGMEM = "CAN properties ";
static const uint8_t commandImplementation19   PROGMEM = FALSE;

// index: 20
static const char commandKeyword20[]           PROGMEM = "CAN";
static const char commandSyntax20[]            PROGMEM = "<key> <arguments>";
static const char commandSyntaxAlternative20[] PROGMEM = "";
static const char commandShortDescription20[]  PROGMEM = "CAN operations: send/receive/unsubscribe/properties";
static const uint8_t commandImplementation20   PROGMEM = FALSE;

// index: 21
static const char commandKeyword21[]           PROGMEM = "DBGL"; /*set/get debug level*/
static const char commandSyntax21[]            PROGMEM = "[level]";
static const char commandSyntaxAlternative21[] PROGMEM = "";
static const char commandShortDescription21[]  PROGMEM = "set/get debug level";
static const uint8_t commandImplementation21   PROGMEM = TRUE;

// index: 22
static const char commandKeyword22[]           PROGMEM = "DBGM"; /*set/get debug system mask*/
static const char commandSyntax22[]            PROGMEM = "[mask]";
static const char commandSyntaxAlternative22[] PROGMEM = "";
static const char commandShortDescription22[]  PROGMEM = "set/get debug system mask";
static const uint8_t commandImplementation22   PROGMEM = TRUE;

// index: 23
static const char commandKeyword23[]           PROGMEM = "JTAG"; /*get/set JTAG availability*/
static const char commandSyntax23[]            PROGMEM = "[0|1]";
static const char commandSyntaxAlternative23[] PROGMEM = "";
static const char commandShortDescription23[]  PROGMEM = "set/get JTAG availability, switch off/enable 4 more ADC channels";
static const uint8_t commandImplementation23   PROGMEM = TRUE;

// index: 24
static const char commandKeyword24[]           PROGMEM = "HELP";
static const char commandSyntax24[]            PROGMEM = "[CMND]";
static const char commandSyntaxAlternative24[] PROGMEM = "<mode>";
static const char commandShortDescription24[]  PROGMEM = "help";
static const uint8_t commandImplementation24   PROGMEM = TRUE;

// index: 25
static const char commandKeyword25[]           PROGMEM = "OWTR"; /*trigger one-wire device(s) for action, if possible*/
static const char commandSyntax25[]            PROGMEM = "[ID] ...";
static const char commandSyntaxAlternative25[] PROGMEM = "";
static const char commandShortDescription25[]  PROGMEM = "trigger one-wire device(s) for action, if possible";
static const uint8_t commandImplementation25   PROGMEM = FALSE;

// index: 26
#warning TODO combine with OWSP
static const char commandKeyword26[]           PROGMEM = "OWRP";
static const char commandSyntax26[]            PROGMEM = "";
static const char commandSyntaxAlternative26[] PROGMEM = "";
static const char commandShortDescription26[]  PROGMEM = "1-wire read active pins/bus mask";
static const uint8_t commandImplementation26   PROGMEM = TRUE;

// index: 27
#warning obsolete functions, included in RLTH/RADC
static const char commandKeyword27[]           PROGMEM = "ADRP"; /*AVR's adcs read active pins/bus mask*/
static const char commandSyntax27[]            PROGMEM = "";
static const char commandSyntaxAlternative27[] PROGMEM = "";
static const char commandShortDescription27[]  PROGMEM = "AVR's adcs read active pins/bus mask";
static const uint8_t commandImplementation27   PROGMEM = FALSE;

// index: 28
static const char commandKeyword28[]           PROGMEM = "DEBG"; /*set/get debug level and mask*/
static const char commandSyntax28[]            PROGMEM = "[level [mask]]";
static const char commandSyntaxAlternative28[] PROGMEM = "";
static const char commandShortDescription28[]  PROGMEM = "set/get debug level and mask";
static const uint8_t commandImplementation28   PROGMEM = TRUE;

// index: 29
static const char commandKeyword29[]           PROGMEM = "PARA"; /*check parasitic power supply mode*/
static const char commandSyntax29[]            PROGMEM = "";
static const char commandSyntaxAlternative29[] PROGMEM = "";
static const char commandShortDescription29[]  PROGMEM = "parasitic device presence test";
static const uint8_t commandImplementation29   PROGMEM = TRUE;

// index: 30
static const char commandKeyword30[]           PROGMEM = "SHOW"; /*show (internal) settings*/
static const char commandSyntax30[]            PROGMEM = "[key_word]";
static const char commandSyntaxAlternative30[] PROGMEM = "";
static const char commandShortDescription30[]  PROGMEM = "show (internal) settings";
static const uint8_t commandImplementation30   PROGMEM = TRUE;

// index: 31
#warning to put into one command OW?? with sub commands
static const char commandKeyword31[]           PROGMEM = "OWMR"; /*1-wire basics: match rom*/
static const char commandSyntax31[]            PROGMEM = "ID <pin_mask>";
static const char commandSyntaxAlternative31[] PROGMEM = "";
static const char commandShortDescription31[]  PROGMEM = "1-wire basics: match rom";
static const uint8_t commandImplementation31   PROGMEM = FALSE;

// index: 32
#warning to put into one command OW?? with sub commands
static const char commandKeyword32[]           PROGMEM = "OWPC"; /*1-wire basics: presence check*/
static const char commandSyntax32[]            PROGMEM = "[<pin_mask>]";
static const char commandSyntaxAlternative32[] PROGMEM = "";
static const char commandShortDescription32[]  PROGMEM = "1-wire basics: presence check";
static const uint8_t commandImplementation32   PROGMEM = FALSE;

// index: 33
#warning to put into one command OW?? with sub commands
static const char commandKeyword33[]           PROGMEM = "OWRb"; /*1-wire basics: receive bit, wait for it*/
static const char commandSyntax33[]            PROGMEM = "<pin_mask> <delay> <timeout: N (times delay)> ";
static const char commandSyntaxAlternative33[] PROGMEM = "";
static const char commandShortDescription33[]  PROGMEM = "1-wire basics: receive bit, wait for it";
static const uint8_t commandImplementation33   PROGMEM = FALSE;

// index: 34
#warning to put into one command OW?? with sub commands
static const char commandKeyword34[]           PROGMEM = "OWRB"; /*1-wire basics: receive byte*/
static const char commandSyntax34[]            PROGMEM = "[<pin_mask>]";
static const char commandSyntaxAlternative34[] PROGMEM = "";
static const char commandShortDescription34[]  PROGMEM = "1-wire basics: receive byte";
static const uint8_t commandImplementation34   PROGMEM = FALSE;

// index: 35
#warning to put into one command OW?? with sub commands
static const char commandKeyword35[]           PROGMEM = "OWSC"; /*1-wire basics: send command*/
static const char commandSyntax35[]            PROGMEM = "<command_key_word> [<pin_mask> [arguments ...]] ";
static const char commandSyntaxAlternative35[] PROGMEM = "";
static const char commandShortDescription35[]  PROGMEM = "1-wire basics: send command";
static const uint8_t commandImplementation35   PROGMEM = FALSE;

// index: 36
#warning to put into one command OW?? with sub commands
static const char commandKeyword36[]           PROGMEM = "OWSB"; /*1-wire basics: send byte*/
static const char commandSyntax36[]            PROGMEM = "<byte> [<pin_mask>]";
static const char commandSyntaxAlternative36[] PROGMEM = "";
static const char commandShortDescription36[]  PROGMEM = "1-wire basics: send byte";
static const uint8_t commandImplementation36   PROGMEM = FALSE;

// index: 37
#warning to put into one command OW?? with sub commands
static const char commandKeyword37[]           PROGMEM = "OWSA"; /*1-wire API settings: set/get 1-wire specific API settings*/
static const char commandSyntax37[]            PROGMEM = "<command_key_word> [arguments] ";
static const char commandSyntaxAlternative37[] PROGMEM = "";
static const char commandShortDescription37[]  PROGMEM = "1-wire API settings";
static const uint8_t commandImplementation37   PROGMEM = TRUE;

// index: 38
static const char commandKeyword38[]           PROGMEM = "TWIS"; /* I2C access */
static const char commandSyntax38[]            PROGMEM = "<0|1> <I2C address> <data length> <byte1 ... byte8>";
static const char commandSyntaxAlternative38[] PROGMEM = "";
static const char commandShortDescription38[]  PROGMEM = "I2C access";
static const uint8_t commandImplementation38   PROGMEM = TRUE;

// index: 39
static const char commandKeyword39[]           PROGMEM = "I2C"; /* I2C access */
static const char commandSyntax39[]            PROGMEM = "<0|1> <I2C address> <data length> <byte1 ... byte8>";
static const char commandSyntaxAlternative39[] PROGMEM = "";
static const char commandShortDescription39[]  PROGMEM = "I2C access";
static const uint8_t commandImplementation39   PROGMEM = TRUE;

// index: 40
static const char commandKeyword40[]           PROGMEM = "RLTH"; /* relay threshold */
static const char commandSyntax40[]            PROGMEM = "[command_key_word] <value>";
static const char commandSyntaxAlternative40[] PROGMEM = "";
static const char commandShortDescription40[]  PROGMEM = "relay threshold";
static const uint8_t commandImplementation40   PROGMEM = TRUE;

// index: 41
static const char commandKeyword41[]           PROGMEM = "CMD1"; /* command (dummy name) */
static const char commandSyntax41[]            PROGMEM = "[???]";
static const char commandSyntaxAlternative41[] PROGMEM = "";
static const char commandShortDescription41[]  PROGMEM = "";
static const uint8_t commandImplementation41   PROGMEM = FALSE;

// index: 42
static const char commandKeyword42[]           PROGMEM = "CMD2"; /* command (dummy name) */
static const char commandSyntax42[]            PROGMEM = "[???]";
static const char commandSyntaxAlternative42[] PROGMEM = "";
static const char commandShortDescription42[]  PROGMEM = "";
static const uint8_t commandImplementation42   PROGMEM = FALSE;

// index: 43
static const char commandKeyword43[]           PROGMEM = "CMD3"; /* command (dummy name) */
static const char commandSyntax43[]            PROGMEM = "[???]";
static const char commandSyntaxAlternative43[] PROGMEM = "";
static const char commandShortDescription43[]  PROGMEM = "";
static const uint8_t commandImplementation43   PROGMEM = FALSE;

// index: 44
static const char commandKeyword44[]           PROGMEM = "SPI"; /* SPI master */
static const char commandSyntax44[]            PROGMEM = "[data]";
static const char commandSyntaxAlternative44[] PROGMEM = "<cmd> <arguments>";
static const char commandShortDescription44[]  PROGMEM = "SPI master (slave)";
static const uint8_t commandImplementation44   PROGMEM = TRUE;

// index: 45
static const char commandKeyword45[]           PROGMEM = "GNWR"; /* send <address> <data> for waveform generator */
static const char commandSyntax45[]            PROGMEM = "<address> <data>";
static const char commandSyntaxAlternative45[] PROGMEM = "";
static const char commandShortDescription45[]  PROGMEM = "waveform generator write data";
static const uint8_t commandImplementation45   PROGMEM = TRUE;

// index: 46
static const char commandKeyword46[]           PROGMEM = "GNRE";
static const char commandSyntax46[]            PROGMEM = "<address>";
static const char commandSyntaxAlternative46[] PROGMEM = "";
static const char commandShortDescription46[]  PROGMEM = "waveform generator read data";
static const uint8_t commandImplementation46   PROGMEM = TRUE;

// index: 47
static const char commandKeyword47[]           PROGMEM = "OW8S";
static const char commandSyntax47[]            PROGMEM = "[ID [value]]";
static const char commandSyntaxAlternative47[] PROGMEM = "";
static const char commandShortDescription47[]  PROGMEM = "1-wire octal switches";
static const uint8_t commandImplementation47   PROGMEM = TRUE;

// index: 48
static const char commandKeyword48[]           PROGMEM = "WDOG"; /*set/get watch dog status*/
static const char commandSyntax48[]            PROGMEM = "[enable/disable/0|1]";
static const char commandSyntaxAlternative48[] PROGMEM = "[cmd] [arguments]";
static const char commandShortDescription48[]  PROGMEM = "set/get watch dog status";
static const uint8_t commandImplementation48   PROGMEM = FALSE;

// index: 49
static const char commandKeyword49[]           PROGMEM = "VERS";
static const char commandSyntax49[]            PROGMEM = "";
static const char commandSyntaxAlternative49[] PROGMEM = "";
static const char commandShortDescription49[]  PROGMEM = "code version";
static const uint8_t commandImplementation49   PROGMEM = TRUE;

// index: 50
static const char commandKeyword50[]           PROGMEM = "IDN"; /* command (dummy name) */
static const char commandSyntax50[]            PROGMEM = "";
static const char commandSyntaxAlternative50[] PROGMEM = "";
static const char commandShortDescription50[]  PROGMEM = "returns device IDN";
static const uint8_t commandImplementation50   PROGMEM = TRUE;

// index: 51
static const char commandKeyword51[]           PROGMEM = "APWI"; /* command (dummy name) */
static const char commandSyntax51[]            PROGMEM = "<command> <portID> <posID> <chipID> [<Arguments>]";
static const char commandSyntaxAlternative51[] PROGMEM = "";
static const char commandShortDescription51[]  PROGMEM = "APFEL ASIC command set";
static const uint8_t commandImplementation51   PROGMEM = TRUE;

// index: 52
static const char commandKeyword52[]           PROGMEM = "APFEL"; /* command (dummy name) */
static const char commandSyntax52[]            PROGMEM = "<command> <portID> <posID> <chipID> [<Arguments>]";
static const char commandSyntaxAlternative52[] PROGMEM = "";
static const char commandShortDescription52[]  PROGMEM = "APFEL ASIC command set";
static const uint8_t commandImplementation52   PROGMEM = TRUE;

// index: 53
static const char commandKeyword53[]           PROGMEM = "CMD8"; /* command (dummy name) */
static const char commandSyntax53[]            PROGMEM = "[???]";
static const char commandSyntaxAlternative53[] PROGMEM = "";
static const char commandShortDescription53[]  PROGMEM = "";
static const uint8_t commandImplementation53   PROGMEM = FALSE;

// index: 54
static const char commandKeyword54[]           PROGMEM = "CMD9"; /* command (dummy name) */
static const char commandSyntax54[]            PROGMEM = "[???]";
static const char commandSyntaxAlternative54[] PROGMEM = "";
static const char commandShortDescription54[]  PROGMEM = "";
static const uint8_t commandImplementation54   PROGMEM = FALSE;

/* this is the corresponding command key array, beware of the same order*/
const char* const commandKeywords[] PROGMEM = {
		commandKeyword00, commandKeyword01, commandKeyword02, commandKeyword03, commandKeyword04, commandKeyword05,
		commandKeyword06, commandKeyword07, commandKeyword08, commandKeyword09, commandKeyword10, commandKeyword11,
		commandKeyword12, commandKeyword13, commandKeyword14, commandKeyword15, commandKeyword16, commandKeyword17,
		commandKeyword18, commandKeyword19, commandKeyword20, commandKeyword21, commandKeyword22, commandKeyword23,
		commandKeyword24, commandKeyword25, commandKeyword26, commandKeyword27, commandKeyword28, commandKeyword29,
		commandKeyword30, commandKeyword31, commandKeyword32, commandKeyword33, commandKeyword34, commandKeyword35,
		commandKeyword36, commandKeyword37, commandKeyword38, commandKeyword39, commandKeyword40, commandKeyword41,
		commandKeyword42, commandKeyword43, commandKeyword44, commandKeyword45, commandKeyword46, commandKeyword47,
		commandKeyword48, commandKeyword49, commandKeyword50, commandKeyword51, commandKeyword52, commandKeyword53,
		commandKeyword54
 };
/* those are the command keywords of the API*/

/* this is the corresponding command key array, beware of the same order*/
const char* const commandSyntaxes[] PROGMEM = {
		commandSyntax00, commandSyntax01, commandSyntax02, commandSyntax03, commandSyntax04, commandSyntax05, commandSyntax06,
		commandSyntax07, commandSyntax08, commandSyntax09, commandSyntax10, commandSyntax11, commandSyntax12,
		commandSyntax13, commandSyntax14, commandSyntax15, commandSyntax16, commandSyntax17, commandSyntax18,
		commandSyntax19, commandSyntax20, commandSyntax21, commandSyntax22, commandSyntax23, commandSyntax24,
		commandSyntax25, commandSyntax26, commandSyntax27, commandSyntax28, commandSyntax29, commandSyntax30,
		commandSyntax31, commandSyntax32, commandSyntax33, commandSyntax34, commandSyntax35, commandSyntax36,
		commandSyntax37, commandSyntax38, commandSyntax39, commandSyntax40, commandSyntax41, commandSyntax42,
		commandSyntax43, commandSyntax44, commandSyntax45, commandSyntax46, commandSyntax47, commandSyntax48,
		commandSyntax49, commandSyntax50, commandSyntax51, commandSyntax52, commandSyntax53, commandSyntax54
 };

/* this is the corresponding command key array, beware of the same order*/
const char* const commandSyntaxAlternatives[] PROGMEM = {
		commandSyntaxAlternative00, commandSyntaxAlternative01, commandSyntaxAlternative02, commandSyntaxAlternative03,
		commandSyntaxAlternative04, commandSyntaxAlternative05, commandSyntaxAlternative06, commandSyntaxAlternative07,
		commandSyntaxAlternative08, commandSyntaxAlternative09, commandSyntaxAlternative10, commandSyntaxAlternative11,
		commandSyntaxAlternative12, commandSyntaxAlternative13, commandSyntaxAlternative14, commandSyntaxAlternative15,
		commandSyntaxAlternative16, commandSyntaxAlternative17, commandSyntaxAlternative18, commandSyntaxAlternative19,
		commandSyntaxAlternative20, commandSyntaxAlternative21, commandSyntaxAlternative22, commandSyntaxAlternative23,
		commandSyntaxAlternative24, commandSyntaxAlternative25, commandSyntaxAlternative26, commandSyntaxAlternative27,
		commandSyntaxAlternative28, commandSyntaxAlternative29, commandSyntaxAlternative30, commandSyntaxAlternative31,
		commandSyntaxAlternative32, commandSyntaxAlternative33, commandSyntaxAlternative34, commandSyntaxAlternative35,
		commandSyntaxAlternative36, commandSyntaxAlternative37, commandSyntaxAlternative38, commandSyntaxAlternative39,
		commandSyntaxAlternative40, commandSyntaxAlternative41, commandSyntaxAlternative42, commandSyntaxAlternative43,
		commandSyntaxAlternative44, commandSyntaxAlternative45, commandSyntaxAlternative46, commandSyntaxAlternative47,
		commandSyntaxAlternative48, commandSyntaxAlternative49, commandSyntaxAlternative50, commandSyntaxAlternative51,
		commandSyntaxAlternative52, commandSyntaxAlternative53, commandSyntaxAlternative54
 };

/* this is the corresponding command key array, beware of the same order*/
const char* const commandShortDescriptions[] PROGMEM= {
		commandShortDescription00, commandShortDescription01, commandShortDescription02, commandShortDescription03,
		commandShortDescription04, commandShortDescription05, commandShortDescription06, commandShortDescription07,
		commandShortDescription08, commandShortDescription09, commandShortDescription10, commandShortDescription11,
		commandShortDescription12, commandShortDescription13, commandShortDescription14, commandShortDescription15,
		commandShortDescription16, commandShortDescription17, commandShortDescription18, commandShortDescription19,
		commandShortDescription20, commandShortDescription21, commandShortDescription22, commandShortDescription23,
		commandShortDescription24, commandShortDescription25, commandShortDescription26, commandShortDescription27,
		commandShortDescription28, commandShortDescription29, commandShortDescription30, commandShortDescription31,
		commandShortDescription32, commandShortDescription33, commandShortDescription34, commandShortDescription35,
		commandShortDescription36, commandShortDescription37, commandShortDescription38, commandShortDescription39,
		commandShortDescription40, commandShortDescription41, commandShortDescription42, commandShortDescription43,
		commandShortDescription44, commandShortDescription45, commandShortDescription46, commandShortDescription47,
		commandShortDescription48, commandShortDescription49, commandShortDescription50, commandShortDescription51,
		commandShortDescription52, commandShortDescription53, commandShortDescription54
 };

const uint8_t* const commandImplementations[] PROGMEM= {
		&commandImplementation00, &commandImplementation01, &commandImplementation02, &commandImplementation03,
		&commandImplementation04, &commandImplementation05, &commandImplementation06, &commandImplementation07,
		&commandImplementation08, &commandImplementation09, &commandImplementation10, &commandImplementation11,
		&commandImplementation12, &commandImplementation13, &commandImplementation14, &commandImplementation15,
		&commandImplementation16, &commandImplementation17, &commandImplementation18, &commandImplementation19,
		&commandImplementation20, &commandImplementation21, &commandImplementation22, &commandImplementation23,
		&commandImplementation24, &commandImplementation25, &commandImplementation26, &commandImplementation27,
		&commandImplementation28, &commandImplementation29, &commandImplementation30, &commandImplementation31,
		&commandImplementation32, &commandImplementation33, &commandImplementation34, &commandImplementation35,
		&commandImplementation36, &commandImplementation37, &commandImplementation38, &commandImplementation39,
		&commandImplementation40, &commandImplementation41, &commandImplementation42, &commandImplementation43,
		&commandImplementation44, &commandImplementation45, &commandImplementation46, &commandImplementation47,
		&commandImplementation48, &commandImplementation49, &commandImplementation50, &commandImplementation51,
		&commandImplementation52, &commandImplementation53, &commandImplementation54
 };

/* pointer of array for defined serial error number*/
static const char se00[] PROGMEM = "no valid command name";                     // SERIAL_ERROR_no_valid_command_name = 0,
static const char se01[] PROGMEM = "command is too long";                       // SERIAL_ERROR_command_is_too_long,
static const char se02[] PROGMEM = "argument has invalid type";                 // SERIAL_ERROR_argument_has_invalid_type,
static const char se03[] PROGMEM = "undefined error type";                      // SERIAL_ERROR_undefined_error_type,
static const char se04[] PROGMEM = "argument(s) have invalid type";             // SERIAL_ERROR_arguments_have_invalid_type,
static const char se05[] PROGMEM = "argument(s) exceed(s) allowed boundary";    // SERIAL_ERROR_arguments_exceed_boundaries,
static const char se06[] PROGMEM = "too many arguments";                        // SERIAL_ERROR_too_many_arguments,
static const char se07[] PROGMEM = "too few arguments";                         // SERIAL_ERROR_too_few_arguments,
static const char se08[] PROGMEM = "invalid sub command name";                  // SERIAL_ERROR_invalid_sub_command_name,
static const char se09[] PROGMEM = "argument string too long";                  // SERIAL_ERROR_argument_string_too_long,

const char* const serial_error[] PROGMEM = { se00, se01, se02, se03, se04, se05, se06, se07, se08, se09};

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

const char* const general_error[] PROGMEM = { ge00, ge01, ge02, ge03, ge04, ge05, ge06, ge07, ge08, ge09, ge10 };

/* pointer of array for defined general error number*/
static const char errorType00[] PROGMEM = "ERRG"; /*general*/
static const char errorType01[] PROGMEM = "ERRA"; /*api*/
static const char errorType02[] PROGMEM = "ERRC"; /*can protocol*/
static const char errorType03[] PROGMEM = "ERRM"; /*can hardware*/
static const char errorType04[] PROGMEM = "ERRT"; /*TWI / I2C*/
static const char errorType05[] PROGMEM = "ERRU"; /*undefined*/

/* this is the corresponding error Type array, beware of the same order*/
const char* const errorTypes[] PROGMEM = {
		errorType00,
		errorType01,
		errorType02,
		errorType03,
		errorType04,
		errorType05
};

int8_t uart0_init = 0; /* return variable of UART0_Init function*/
int8_t can_init = 0; /* return variable of  canInit function*/
int8_t twim_init = 0; /* return variable of TWIM_Init function*/
int8_t owi_init = 0; /* return variable of OWI_Init function*/
int8_t timer0_init = 0; /* return variable of Timer0_Init function*/
int8_t timer0A_init = 0;/* return variable of Timer0A_Init function*/

uint8_t resetSource = resetSource_UNDEFINED_REASON;

/*----------------------------------------------------------------------------------------------------*/

/*
 *this function initializes the UART0 register needed to allow the at90can128
 *USART transmissions
 */
int8_t UART0_Init( void )
{
	uint8_t intstate1 = SREG;/*save global interrupt flag*/
	/*disable interrupt*/
	cli();

	//clear strings
	clearString(uart_message_string, BUFFER_SIZE);
	clearString(decrypt_uartString, BUFFER_SIZE);
	clearString(uartString, BUFFER_SIZE);
	clearString(message, BUFFER_SIZE);

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
	if ( true == uartInputBufferExceeded)
	{
		/* maximum length check of input */

		CommunicationError_p(ERRA, SERIAL_ERROR_command_is_too_long, FALSE, PSTR("max: %i"), MAX_LENGTH_COMMAND);

		clearString(uartString, BUFFER_SIZE);
		uartInputBufferExceeded = false;
		return;
	}

	/* copy string into (buffer) uartString*/
	strncpy(decrypt_uartString, uartString, BUFFER_SIZE - 1);
    /* clear uartString, avoiding memset*/
    clearString(uartString, BUFFER_SIZE);

    printDebug_p(debugLevelEventDebug, debugSystemUART, __LINE__, filename, PSTR("UART string received:'%s'"), decrypt_uartString);

    if (globalDebugLevel >= debugLevelEventDebugVerbose)
    {
    	clearString(message, BUFFER_SIZE);
    	char uartCurrentCharacter;
    	size_t index=0;
    	while( decrypt_uartString[index] )
    	{
    		uartCurrentCharacter = decrypt_uartString[index];
    		if (!iscntrl(uartCurrentCharacter))
    		{
    			snprintf_P(message, BUFFER_SIZE -1, PSTR("%s%c"), message, uartCurrentCharacter);
    		}
    		else
    		{
    			snprintf_P(message, BUFFER_SIZE -1, PSTR("%s^[%#o"), message, uartCurrentCharacter);
    		}
    		index++;
    	}
        printDebug_p(debugLevelEventDebugVerbose, debugSystemUART, __LINE__, filename, PSTR("UART string received:'%s'"), decrypt_uartString);
    }

	/* split uart string into its elements */

	int8_t number_of_elements = -1;
	number_of_elements = uartSplitUartString(decrypt_uartString);

 	printDebug_p(debugLevelEventDebugVerbose, debugSystemCommandKey, __LINE__, filename,
 			PSTR("number of string elements found: %i"), number_of_elements);

	if ( 0 < number_of_elements  )
	{
		/* set number of arguments */
		ptr_uartStruct->number_of_arguments = number_of_elements -1;

		/* Find matching command keyword */
		ptr_uartStruct->commandKeywordIndex = apiFindCommandKeywordIndex(setParameter[0], commandKeywords, commandKeyNumber_MAXIMUM_NUMBER);

 		printDebug_p(debugLevelEventDebug, debugSystemCommandKey, __LINE__, filename, PSTR("keywordIndex of %s is %i"), setParameter[0], ptr_uartStruct->commandKeywordIndex);

		/* no matching keyword ?*/
		if ( 0 > ptr_uartStruct->commandKeywordIndex )
		{
			Check_Error(ptr_uartStruct);
		}
		else /* ptr_uartStruct->commandKeywordIndex >= 0 */
		{
			switch(ptr_uartStruct->commandKeywordIndex)
			{
			/*use its own parameter parsing*/
			case commandKeyNumber_HELP:
			case commandKeyNumber_SHOW:
			case commandKeyNumber_OWSA:
			case commandKeyNumber_RLTH:
			case commandKeyNumber_TWIS:
			case commandKeyNumber_I2C:
			case commandKeyNumber_DEBG:
			case commandKeyNumber_SPI:
			case commandKeyNumber_APWI:
			case commandKeyNumber_APFEL:
				Choose_Function(ptr_uartStruct);
				break;
				/*use 1-wire parsing*/
			case commandKeyNumber_OWTP:
			case commandKeyNumber_OWAD:
			case commandKeyNumber_OWDS:
			case commandKeyNumber_OWSS:
			case commandKeyNumber_OW8S:
			case commandKeyNumber_OWLS:
			case commandKeyNumber_OWMR:
			case commandKeyNumber_OWPC:
			case commandKeyNumber_OWRb:
			case commandKeyNumber_OWRB:
			case commandKeyNumber_OWSC:
			case commandKeyNumber_OWSB:
			   if ( 0 == owiConvertUartDataToOwiStruct() )
			   {
			      Choose_Function(ptr_uartStruct);
			   }
			   break;
			case commandKeyNumber_SEND:
			case commandKeyNumber_SUBS:
			case commandKeyNumber_USUB:
			case commandKeyNumber_CANT:
			case commandKeyNumber_CANS:
			case commandKeyNumber_CANU:
			case commandKeyNumber_CANP:
			case commandKeyNumber_CAN:
				/* call the function for the conversion frame format into CAN format*/
				apiConvertUartDataToCanUartStruct(0);
				if ( TRUE == canCheckParameterCanFormat(ptr_uartStruct) )
				{
					Choose_Function(ptr_uartStruct);
				}
				else
				{
					Check_Error(ptr_uartStruct);
				}
			   break;
			case commandKeyNumber_RGWR:
			case commandKeyNumber_RGRE:
			case commandKeyNumber_RADC:
			case commandKeyNumber_DBGL:
			case commandKeyNumber_DBGM:
			case commandKeyNumber_JTAG:
			case commandKeyNumber_PARA:
			case commandKeyNumber_GNWR:
			case commandKeyNumber_GNRE:
			case commandKeyNumber_VERS:
			case commandKeyNumber_IDN:
			default:
				/* call the function for the conversion frame format into CAN format*/
				apiConvertUartDataToCanUartStruct(0);
				/*call the function Check_Parameter*/
				/* TODO: improve this for different command keywords than CAN based,
				 * i.e. think to do checks maybe later when already having chosen the
				 * right function*/
				Choose_Function(ptr_uartStruct);
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
 * 		 only copy the first MAX_PARAMETER elements
 * 		 if there are more, still count the elements
 * 		 and copy the rest into decrypt_uartString_remainder
 *
 * no direct input : use of global variable decrypt_uartString
 * no direct output: use of global variable setParameter
 * return value:
 *          number of found elements,
 *          -1 else
 */

int8_t uartSplitUartString( char inputUartString[] )
{
	uint8_t parameterIndex;

	/*
	 * disassemble the inputUartString into stringlets
	 * separated by delimiters UART_DELIMITER
	 * into elements of array setParameter
	 */

	char *save_ptr = NULL;
	char *result_ptr = NULL; /* pointer init */

	/* initial iteration */
	result_ptr = strtok_r(inputUartString, UART_DELIMITER, &save_ptr); /*search spaces in string */
	parameterIndex = 0; /* pointer of setParameter*/

	while ( result_ptr != NULL )
	{
		if (MAX_LENGTH_PARAMETER < strlen(result_ptr))
		{
			CommunicationError_p(ERRA, SERIAL_ERROR_argument_string_too_long, FALSE, NULL);
			return -1;
		}

		/* only copy the first MAX_PARAMETER elements
		 * if there are more, still count the elements
		 * and copy the rest into inputUartString_remainder*/
		if ( MAX_PARAMETER > parameterIndex )
		{
			strncpy(setParameter[parameterIndex], result_ptr, MAX_LENGTH_PARAMETER);
		}
		if (MAX_PARAMETER == parameterIndex)
		{
			clearString(decrypt_uartString_remainder, BUFFER_SIZE);
			strncpy(decrypt_uartString_remainder, result_ptr, BUFFER_SIZE -1 );
			strncat(decrypt_uartString_remainder, UART_DELIMITER, BUFFER_SIZE -1 );
			strncat(decrypt_uartString_remainder, save_ptr, BUFFER_SIZE -1 );
			printDebug_p(debugLevelEventDebug, debugSystemDecrypt, __LINE__, filename, PSTR("remainder '%s'"), decrypt_uartString_remainder);
		}

		result_ptr = strtok_r(NULL, UART_DELIMITER, &save_ptr);
		parameterIndex++;
	}

 	printDebug_p(debugLevelEventDebug, debugSystemDecrypt, __LINE__, filename, PSTR("found %i strings"), parameterIndex);

	return parameterIndex;

}//END of uartSplitUartString function


/*
 *this function assigns the various parameters of the string to the structure and reviewed the range of *individual  parameters.
 *the  function uses the
 *	global two dimensional array char setParameter[MAX_PARAMETER][MAX_LENGTH_PARAMETER] (as input)
 *		and the offset to start from
 *	and returns no parameter
 */

void apiConvertUartDataToCanUartStruct( uint8_t offset )
{
	/* convert char in hexadecimal*/

#warning TODO: strtoul and its tailptr correctly applied
#warning TODO: is setParameter still useful

	// http://www.gnu.org/software/libc/manual/html_node/Parsing-of-Integers.html
	//
	// The strtol (``string-to-long'') function converts the initial
	// part of string to a signed integer, which is returned as a
	// value of type long int.
	//
	//This function attempts to decompose string as follows:
	//
	//	- A (possibly empty) sequence of whitespace
	//	 characters. Which characters are whitespace is
	//	 determined by the isspace function . These are
	//	 discarded.
	//
	//	- An optional plus or minus sign (+ or -).
	//
	//	- A nonempty sequence of digits in the radix
	//	 specified by base.
	//
	//If base is zero, decimal radix is assumed unless the
	// series of digits begins with 0 (specifying octal radix), or
	// 0x or 0X (specifying hexadecimal radix); in other words,
	// the same syntax used for integer constants in C.
	//
	//Otherwise base must have a value between 2 and 36. If
	// base is 16, the digits may optionally be preceded by 0x
	// or 0X. If base has no legal value the value returned is 0l
	// and the global variable errno is set to EINVAL.
	//
	//	- Any remaining characters in the string. If tailptr is
	//	 not a null pointer, strtol stores a pointer to this tail in
	//	 *tailptr.
	//
	//If the string is empty, contains only whitespace, or does
	// not contain an initial substring that has the expected
	// syntax for an integer in the specified base, no
	// conversion is performed. In this case, strtol returns a
	// value of zero and the value stored in *tailptr is the value
	// of string.
	//
	//In a locale other than the standard "C" locale, this
	// function may recognize additional implementation-
	// dependent syntax.
	//
	//If the string has valid syntax for an integer but the value
	// is not representable because of overflow, strtol returns
	// either LONG_MAX or LONG_MIN , as appropriate for the
	// sign of the value. It also sets errno to ERANGE to
	// indicate there was overflow.
	//
	//You should not check for errors by examining the return
	// value of strtol, because the string might be a valid
	// representation of 0l, LONG_MAX, or LONG_MIN. Instead,
	// check whether tailptr points to what you expect after
	// the number (e.g. '\0' if the string should end after the
	// number). You also need to clear errno before the call
	// and check it afterward, in case there was overflow.
	//
	//Here is a function which parses a string as a sequence of integers and returns the sum of them
	//    int
	//    sum_ints_from_string (char *string)
	//    {
	//      int sum = 0;
	//
	//      while (1) {
	//        char *tail;
	//        int next;
	//
	//        /* Skip whitespace by hand, to detect the end.  */
	//        while (isspace (*string)) string++;
	//        if (*string == 0)
	//          break;
	//
	//        /* There is more nonwhitespace,  */
	//        /* so it ought to be another number.  */
	//        errno = 0;
	//        /* Parse it.  */
	//        next = strtol (string, &tail, 0);
	//        /* Add it in, if not overflow.  */
	//        if (errno)
	//          printf ("Overflow\n");
	//        else
	//          sum += next;
	//        /* Advance past it.  */
	//        string = tail;
	//      }
	//
	//      return sum;
	//    }

	//   ptr_uartStruct->Name = setParameter[0];
	if (1 + offset >= MAX_PARAMETER)
	{
		return;
	}
	ptr_uartStruct->Uart_Message_ID = (uint32_t) strtoul(setParameter [1 + offset], &ptr_setParameter[ 1 + offset], 16);
	if (2 + offset >= MAX_PARAMETER)
	{
		return;
	}
	ptr_uartStruct->Uart_Mask       = (uint32_t) strtoul(setParameter [2 + offset], &ptr_setParameter[ 2 + offset], 16);
	if (3 + offset >= MAX_PARAMETER)
	{
		return;
	}
	ptr_uartStruct->Uart_Rtr        = (uint8_t)  strtoul(setParameter [3 + offset], &ptr_setParameter[ 3 + offset], 16);
	if (4 + offset >= MAX_PARAMETER)
	{
		return;
	}
	ptr_uartStruct->Uart_Data[0]    = (uint16_t) strtoul(setParameter [5 + offset], &ptr_setParameter[ 4 + offset], 16);
	if (5 + offset >= MAX_PARAMETER)
	{
		return;
	}
	ptr_uartStruct->Uart_Length     = (uint8_t)  strtoul(setParameter [4 + offset], &ptr_setParameter[ 5 + offset], 16);
	if (6 + offset >= MAX_PARAMETER)
	{
		return;
	}
	ptr_uartStruct->Uart_Data[1]    = (uint16_t) strtoul(setParameter [6 + offset], &ptr_setParameter[ 6 + offset], 16);
	if (7 + offset >= MAX_PARAMETER)
	{
		return;
	}
	ptr_uartStruct->Uart_Data[2]    = (uint16_t) strtoul(setParameter [7 + offset], &ptr_setParameter[ 7 + offset], 16);
	if (8 + offset >= MAX_PARAMETER)
	{
		return;
	}
	ptr_uartStruct->Uart_Data[3]    = (uint16_t) strtoul(setParameter [8 + offset], &ptr_setParameter[ 8 + offset], 16);
	if (9 + offset >= MAX_PARAMETER)
	{
		return;
	}
	ptr_uartStruct->Uart_Data[4]    = (uint16_t) strtoul(setParameter [9 + offset], &ptr_setParameter[ 9 + offset], 16);
	if (10 + offset >= MAX_PARAMETER)
	{
		return;
	}
	ptr_uartStruct->Uart_Data[5]    = (uint16_t) strtoul(setParameter[10 + offset], &ptr_setParameter[10 + offset], 16);
	if (11 + offset >= MAX_PARAMETER)
	{
		return;
	}
	ptr_uartStruct->Uart_Data[6]    = (uint16_t) strtoul(setParameter[11 + offset], &ptr_setParameter[11 + offset], 16);
	if (12 + offset >= MAX_PARAMETER)
	{
		return;
	}
	ptr_uartStruct->Uart_Data[7]    = (uint16_t) strtoul(setParameter[12 + offset], &ptr_setParameter[12 + offset], 16);

} // END of function Convert_UartFormat_to_CanFormat

/*
 * int8_t apiFindCommandKeywordIndex(const char string[], PGM_P keywords[], size_t keywordMaximumIndex )
 *
 * this function parses for all valid command matching keyword
 * of the keyword array keywords
 * on the first MAX_LENGTH_PARAMETER characters of the string
 *
 * it has an char pointer as input
 * it returns
 *     the commandKeyIndex of the corresponding command key word
 *     -1 if not found
 *     -99 on error
 */

int8_t apiFindCommandKeywordIndex(const char string[], PGM_P const keywords[], size_t keywordMaximumIndex )
{
	if (NULL == string )          {return -99;} /* NULL pointer */
	if (NULL == keywords )        {return -99;} /* NULL pointer */
	if (STRING_END == string[0] ) {return -99;} /* empty string */

 	printDebug_p(debugLevelEventDebug, debugSystemCommandKey, __LINE__, filename, PSTR("get index of keyword: %s"), string);

	// find matching command keyword

 	size_t keywordIndex = 0;

 	/*first find max non control length*/
 	size_t length = 0;
 	for (length = 0; length < strlen(string) && length < MAX_LENGTH_PARAMETER; length++)
 	{
 		if (iscntrl(string[length])) break;
 	}
	printDebug_p(debugLevelEventDebug, debugSystemCommandKey, __LINE__, filename, PSTR("keyword length: %i, strlen: %i "), length, strlen(string));

 	while ( keywordIndex < keywordMaximumIndex )
	{
		if ( 0 == strncasecmp_P(string, (const char*) ( pgm_read_word( &(keywords[keywordIndex])) ), length) )
		{
 			printDebug_p(debugLevelEventDebug, debugSystemCommandKey, __LINE__, filename, PSTR("keyword '%s' matches, index %i "), string, keywordIndex);
			return keywordIndex;
		}
        else
        {
         	printDebug_p(debugLevelEventDebugVerbose, debugSystemCommandKey, __LINE__, filename, PSTR("keyword '%s' doesn't match '%S'"), string, (const char*) ( pgm_read_word( &(keywords[keywordIndex])) ));
			keywordIndex++;
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
	return ( 0 <= ptr_uartStruct->commandKeywordIndex && TRUE == canCheckParameterCanFormat(ptr_uartStruct) ) ? TRUE : FALSE;
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
 * the function has a pointer of the serial structure as input and returns no parameter
 */

int8_t Check_Error( struct uartStruct *ptr_uartStruct )
{
   static int8_t error = FALSE;

   error = FALSE;

   if ( 0 > ptr_uartStruct->commandKeywordIndex )
   {
      CommunicationError_p(ERRA, SERIAL_ERROR_no_valid_command_name, 0, NULL);
      error = TRUE;
   }
   else
   {
      for (uint8_t index = 1; index < MAX_PARAMETER; index ++)
      {
         if (0 != *ptr_setParameter[index])
         {
            CommunicationError_p(ERRA, SERIAL_ERROR_argument_has_invalid_type, 1, PSTR("%s"), setParameter[index]);
            error = TRUE;
         }
         if ( TRUE == error )
         {
            break;
         }
      } //end for
      if ( FALSE == error)
      {
    	  switch (ptr_uartStruct->commandKeywordIndex)
    	  {
    		  case commandKeyNumber_SEND: /*CAN*/
    		  case commandKeyNumber_CANT: /*CAN*/
    		  case commandKeyNumber_SUBS: /*CAN*/
    		  case commandKeyNumber_CANS: /*CAN*/
    		  case commandKeyNumber_USUB: /*CAN*/
    		  case commandKeyNumber_CANU: /*CAN*/
    		  case commandKeyNumber_CAN:  /*CAN*/
    		  case commandKeyNumber_CANP: /*CAN*/
    			  error = canCheckInputParameterError(ptr_uartStruct);
    			  break;
    		  case commandKeyNumber_I2C: /*I2C*/
    		  case commandKeyNumber_TWIS: /*I2C*/
    		  {
    			  error = twiMasterCheckError(ptr_uartStruct);
    		  }
    		  break;
    		  default: /*uncovered command keys*/
    		  {
    		  }
    		  break;
    	  }
      }
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
	case commandKeyNumber_CANT:
		/* command : SEND CAN-Message-ID ID-Range [RTR=0 <Number of data bytes = 0> Data0 ... Data7] */
		/* no RTR */
		/* command      : SEND CAN-Message-ID ID-Range 0 <Number of data bytes> Data0 ... Data7]
		 * response now : RECV SEND CAN-Message-ID "command will be carried out"*/
		/* RTR */
		/* command      : SEND CAN-Message-ID ID-Range 1 <Number of requested data bytes>]
		 * response  now: RECV CAN_Mob CAN-Message-ID CAN-Length [Data0 ... Data7] */
		/* response TODO: RECV SEND CAN-Message-ID CAN-Length [Data0 ... Data7] */
		canSendMessage(ptr_uartStruct); /* call function with name canSendMessage */
		printDebug_p(debugLevelEventDebugVerbose, debugSystemCAN, __LINE__, filename, PSTR("Choose_Function: call finished"));
		break;
	case commandKeyNumber_SUBS:
	case commandKeyNumber_CANS:
		/* command : SUBS CAN-Message-ID ID-Range
		 * response: */
		canSubscribeMessage(ptr_uartStruct);
		break;
	case commandKeyNumber_USUB:
	case commandKeyNumber_CANU:
		/* command : USUB CAN-Message-ID ID-Range
		 * response: */
		canUnsubscribeMessage(ptr_uartStruct);
		break;
	case commandKeyNumber_CANP:
	case commandKeyNumber_CAN:
		/*not yet implemented*/
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
		registerWriteRegister(ptr_uartStruct); /* call function with name registerWriteRegister  */
		break;
	case commandKeyNumber_RGRE:
		/* command      : RGRE Register*/
		/* response now : RECV the value %x has been written in Register */
		/* response TODO: RECV RGWR Register Value */
		registerReadRegister(ptr_uartStruct); /* call function with name  registerReadRegister */
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
	case commandKeyNumber_OWDS: /*set/get one wire dual switches*/
		owiDualSwitches(ptr_uartStruct);
		break;
	case commandKeyNumber_RSET:
        reset(ptr_uartStruct);
		break;
	case commandKeyNumber_INIT:
        init(ptr_uartStruct);
		break;
	case commandKeyNumber_OWSS:
		read_status_simpleSwitches(ptr_uartStruct); /* call function with name  registerReadRegister */
		break;
	case commandKeyNumber_OWLS:
       /* command : OWLS
        * response: RECV OWLS 1: bus mask: 0xXX ID <owi ID>
        *           ...
        * response: RECV OWLS N: bus mask: 0xXX ID <owi ID>*/
	   owiShowDevicesID(ptr_uartStruct);
		break;
	case commandKeyNumber_PING:
		keep_alive(ptr_uartStruct);
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
	case commandKeyNumber_PARA: /*parasitic devices*/
	   owiFindParasitePoweredDevices(TRUE);
		break;
	case commandKeyNumber_SHOW: /*show (internal) settings*/
		show(ptr_uartStruct);
		break;
	case commandKeyNumber_DEBG: /*set/get debug level*/
		/* command : DEBG [level [mask]]
		 * set response: ...
		 * get response: RECV DEBG level mask*/
		apiDebug(ptr_uartStruct);
		break;
	case commandKeyNumber_DBGL: /*set/get debug level*/
		/* command : DBGL [level]
		 * set response: ...
		 * get response: RECV DBGL level */
		apiDebug(ptr_uartStruct);
		break;
	case commandKeyNumber_DBGM: /*set/get only debug system mask*/
		/* command : DBGM [mask]
		 * set response: ...
		 * get response: RECV DBGM mask*/
		apiDebug(ptr_uartStruct);
		break;
	case commandKeyNumber_JTAG: /*toggle/set JTAG availability*/
	   modifyJTAG(ptr_uartStruct);
		break;
	case commandKeyNumber_HELP: /*output some help*/
		help(ptr_uartStruct);
		break;
	case commandKeyNumber_OWTR: /*trigger one-wire device(s) for action, if possible*/
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
    case commandKeyNumber_SPI: /* command (dummy name) */
    	spiApi(ptr_uartStruct);
    	break;
    case commandKeyNumber_APWI: /* command (dummy name) */
    case commandKeyNumber_APFEL: /* command (dummy name) */
    	apfelApi(ptr_uartStruct);
    	break;
    case commandKeyNumber_GNWR: /* generator write */
      waveformGeneratorWriteRegister(ptr_uartStruct);
       break;
    case commandKeyNumber_GNRE: /* generator read */
      waveformGeneratorReadRegister(ptr_uartStruct);
      break;
    case commandKeyNumber_OW8S: /* 1-wire 8-fold switch */
    	owiOctalSwitches(ptr_uartStruct);
    	break;
    case commandKeyNumber_TWIS: /* I2C interface (twi) */
    case commandKeyNumber_I2C:  /* I2C interface (twi) */
    	twiMaster(ptr_uartStruct);
       break;
    case commandKeyNumber_VERS: /* version */
    	version();
       break;
    case commandKeyNumber_IDN: /* version */
    	identification();
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
	if ( counter1 == 20 ) /* 20 overflow for timer1=1s */
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


/* interrupt for receiving data via serial communication with defined interrupt vector SIG_UART0_RECV */
ISR (SIG_UART0_RECV)
{
	unsigned char c = UDR0;

	if ( c == '\n' || c == '\r' ) /* the string is complete? */
	{
		uartString[nextCharPos] = '\0';
		uartReady = 1; /* mark, that we got an UART_interrupt, to be handled by main */
		nextCharPos = 0;
	}
	else if ( BUFFER_SIZE - 1 == nextCharPos ) /* string exceeds length, skip remainder, set flag */
	{
		uartInputBufferExceeded = true;
	}
	else /* add character and trailing '\0' */
	{
		uartString[nextCharPos] = c;
		nextCharPos++;
		uartString[nextCharPos] = '\0';
	}
}//END of ISR (SIG_UART0_RECV)

/*
 *USART0 must be initialized before using this function
 *for use this function, be sure that data is no longer then 8 bit
 *(single character) this function will send  8 bits from the at90can128
 */

#warning UART add timeout or transmit interrupt? Add check for USART0 init ?

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

#warning TODO: UART also change UART send activities into interrupt based one, q.v. https://www.mikrocontroller.net/articles/Interrupt
/*
 *this function sends a string to serial communication
 * the input parameter is a defined global variable outputString
 * the output parameter is an integer
 * 0 -> the message is sent
 */

int16_t UART0_Send_Message_String( char *outputString, uint16_t maxSize )
{
	/* this function sends the string pointed to by outputString to UART
	 * this happens char by char until STRING_END is found
	 * afterwards the message string is cleared up to the size maxSize
	 *
	 * if outputString is NULL
	 *    - outputString is set to the global variable uart_message_string
	 *    - and maxSize is set to BUFFER_SIZE
	 * else if maxSize is 0
	 *    - maxSize is assumed to be BUFFER_SIZE
	 *
	 * returns
	 *   - in case of errors: -1
	 *   - else number of transfered characters (max INT16_MAX )
	 *
	 * */

	if (FALSE != uart0_init)
	{

		/* if outputString is NULL, take as default uart_message_string and its size BUFFER_SIZE */
		if (NULL == outputString)
		{
			if (NULL != uart_message_string)
			{
				outputString = uart_message_string;
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

		for ( index = 0; STRING_END != outputString[index] && index < maxSize ; index++ )
		{
			UART0_Transmit_p(outputString[index]);
		}
		UART0_Transmit_p('\n');
		clearString(outputString, maxSize); /*clear outputString variable*/
		return index;
	}
	else
	{
		#warning TODO UART0: think of a method to react on a non initialized uart0
		return 0;
	}
}//END of UART0_Send_Message_String function


/*
 *this function sends a string to serial communication
 * the input parameter is a defined global variable outputString
 * the output parameter is an integer
 * 0 -> the message is sent
 * only use for the function owiFindFamilyDevicesAndAccessValues in one_wire.c
 */
int8_t UART0_Send_Message_String_woLF( char *outputString, uint32_t maxSize )
{
	for ( uint16_t j = 0 ; j < strlen((char *) outputString) && j < maxSize ; j++ )
	{
		UART0_Transmit_p(outputString[j]);
	}

	clearString(outputString, maxSize); /*clear outputString variable*/
	return 0;
}//END of UART0_Send_Message_String_woLF


uint8_t CommunicationError( uint8_t errorType, const int16_t errorIndex, const uint8_t flag_printCommand,
		PGM_P alternativeErrorMessage, ...)
{
/* This (new) Communication Error function
 * sends a ((partly) predefined) error message to UART
 *
 * output: ERRG/C/U/A/M <error number> <error message> [<alternative/extra Error>]
 * output: ERRA/C/U/A/M "<command key> <command arguments>" --- <error number> <error message>
 *
 * positive <error number> = ERRindex * 100 + errorIndex
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
     	   printDebug_p(debugLevelEventDebug, debugSystemApi, __LINE__, filename, PSTR("wrong error index ... returning"));
          return 1; /*shouldn't happen*/
          break;
    }

    /* check alternative pointer, return if empty*/
    if ( TRUE == flag_UseOnlyAlternatives && NULL == alternativeErrorMessage)
    {
     	printDebug_p(debugLevelEventDebug, debugSystemApiMisc, __LINE__, filename, PSTR("alternatives: NULL ... returning"));
    	return 1;
    }

    /* compose message
     * output: ERRG/C/U/A/M/T <error number> <error message> [<alternative/extra Error> <alternative/extra number>]
     * output: ERRA/C/U/A/M/T "<command key> <command arguments>" --- <error number> <error message>
     * */
    /* clear string*/
    clearString(uart_message_string, BUFFER_SIZE);

    /* cat Error Type */
    strncat_P(uart_message_string, (const char*) (pgm_read_word( &(errorTypes[errorType]))), BUFFER_SIZE - 1 );

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
             snprintf_P(uart_message_string, BUFFER_SIZE -1 , string_s_i_, uart_message_string, (errorType * 100) + errorIndex);
             strncat_P(uart_message_string, (const char*) (pgm_read_word( &(general_error[errorIndex]))), BUFFER_SIZE -1);
             break;
          case ERRC:
             snprintf_P(uart_message_string, BUFFER_SIZE -1 , string_s_i_, uart_message_string, (errorType * 100) + errorIndex);
             strncat_P(uart_message_string, (const char*) (pgm_read_word( &(can_error[errorIndex]))), BUFFER_SIZE -1);
             break;
          case ERRA:
             snprintf_P(uart_message_string, BUFFER_SIZE -1 , string_s_i_, uart_message_string, (errorType * 100) + errorIndex);
             strncat_P(uart_message_string, (const char*) (pgm_read_word( &(serial_error[errorIndex]))), BUFFER_SIZE -1);
             break;
          case ERRM:
             snprintf_P(uart_message_string, BUFFER_SIZE -1 , string_s_i_, uart_message_string, (errorType * 100) + errorIndex);
             strncat_P(uart_message_string, (const char*) (pgm_read_word( &(mob_error[errorIndex]))), BUFFER_SIZE -1);
             break;
          case ERRT:
              snprintf_P(uart_message_string, BUFFER_SIZE -1 , string_s_i_, uart_message_string, (errorType * 100) + errorIndex);
              strncat_P(uart_message_string, (const char*) (pgm_read_word( &(twi_error[errorIndex]))), BUFFER_SIZE -1);
              break;
          case ERRU:
              snprintf_P(uart_message_string, BUFFER_SIZE -1 , string_s_i_, uart_message_string, (errorType * 100) + errorIndex);
              break;
          default:
         	  printDebug_p(debugLevelEventDebug, debugSystemApiMisc, __LINE__, filename, PSTR("wrong error type %i... returning"), errorType);
        	  return 1;
             break;
       }
    }

    if ( NULL != alternativeErrorMessage )
    {
       if (TRUE == flag_UseOnlyAlternatives)
       {
          snprintf_P(uart_message_string,  BUFFER_SIZE - 1 , string_s_i_, uart_message_string, errorIndex);
       }
       else
       {
          strncat_P(uart_message_string, PSTR(" *** "), BUFFER_SIZE -1);
       }

       va_list argumentPointers;
       va_start (argumentPointers, alternativeErrorMessage);
       vsnprintf_P(message, BUFFER_SIZE - 1, alternativeErrorMessage, argumentPointers);
       va_end(argumentPointers);

       strncat(uart_message_string, "\"", BUFFER_SIZE -1);
       strncat(uart_message_string, message, BUFFER_SIZE -1);
       strncat(uart_message_string, "\"", BUFFER_SIZE -1);
    }

    UART0_Send_Message_String_p(NULL,0);

    return 0;
}//END of CommunicationError function

void printDebug( uint8_t debugLevel, uint32_t debugMaskIndex, int16_t line, PGM_P file, PGM_P format, ...)
{
	if ( debugLevel <= globalDebugLevel && ((globalDebugSystemMask >> debugMaskIndex) & 1))
	{
		clearString(message, BUFFER_SIZE);
		clearString(uart_message_string, BUFFER_SIZE);

		va_list argumentPointers;
		va_start (argumentPointers, format);
		vsnprintf_P(message, BUFFER_SIZE - 1, format, argumentPointers);
		va_end(argumentPointers);

		// header: "DEBUG (%4i, %s, %s ) %s"
		strncat_P(uart_message_string, PSTR("DEBUG"), BUFFER_SIZE -1);
		strncat_P(uart_message_string, PSTR(" "), BUFFER_SIZE -1);
		if ( 0 < line || file )
		{
			strncat_P(uart_message_string, PSTR("("), BUFFER_SIZE -1);
			if ( 0 < line )
			{
				snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s%4i"),uart_message_string, line);
			}
			// ,

			if ( 0 < line && file )
			{
				strncat_P(uart_message_string, PSTR(", "), BUFFER_SIZE -1);
			}

			if (file)
			{
				strncat_P(uart_message_string, file, BUFFER_SIZE -1);
			}
			strncat_P(uart_message_string, PSTR(") "), BUFFER_SIZE -1);
		}
		strncat(uart_message_string, message, BUFFER_SIZE -1);

		UART0_Send_Message_String_p(NULL,0);
	}
}

/*
 this function initializes all init functions again and activates the interrupt
 */
void Initialization( void )
{
#warning TODO find a generalized way to check for correct init and a modular possibility to fail and still run, e.g. relay

   uint16_t status = 0;

   /* disable interrupts*/
   cli();

   determineAndHandleResetSource();

   // if not watchdog reset
   if (resetSource_WATCHDOG != resetSource)
   {
	   watchdogIncarnationsCounter = 0;
   }

   /*
    * pointers and structures
    */
   ptr_uartStruct = &uartFrame; /* initialize pointer for CPU-structure */
   initUartStruct(ptr_uartStruct);/*initialize basic properties of uartStruct*/

   ptr_canStruct = &canFrame; /* initialize pointer for CAN-structure */

   ptr_owiStruct = &owiFrame; /* initialize pointer for 1-wire structure*/
   owiInitOwiStruct(ptr_owiStruct); /* initialize basic properties*/

   /* initialize flags */
#warning TODO: replace hardcoded values by named enum states
   nextCharPos = 0;
   uartReady = 0;
   canReady = canState_IDLE;
   canTimerOverrun = FALSE; /*variable for can timer overrun interrupt*/
   timer0Ready = 0;
   timer1Ready = 0;
   timer0AReady = 0;
   timer0ASchedulerReady = 0;
   ptr_subscribe = 2; /*start value of pointer*/
   ptr_buffer_in = NULL; /*initialize pointer for write in buffer_ring*/
   ptr_buffer_out = NULL; /*initialize pointer for read in  buffer_ring*/
   res0 = 0, res1 = 0, res2 = 0, res3 = 0;
   res4 = 0, res5 = 0, res6 = 0, res7 = 0;
   canCurrentMObStatus = 0;

   relayThresholdCurrentState = relayThresholdState_IDLE;

   owiBusMask = 0xFF;
   adcBusMask = 0x0F;

   uart0_init = UART0_Init();

   InitIOPorts();

   disableJTAG(FALSE);

   twim_init = Twim_Init (250000);

   if( FALSE == twim_init)
   {
	   twiErrorCode = CommunicationError_p(ERRT, TWI_ERROR_Error_in_initiating_TWI_interface, FALSE, NULL);
   }

   owi_init = OWI_Init(0x3f);

   if( FALSE == owi_init)
   {
#warning OWI INIT create realistic error message
   }

   can_init = canInit(CAN_DEFAULT_BAUD_RATE); /* initialize can-controller with a baudrate 250kps */

   if ( -1 == can_init )
   {
      canErrorCode = CAN_ERROR_CAN_was_not_successfully_initialized;
      CommunicationError_p(ERRC, canErrorCode, FALSE, NULL);
#warning exit must not be used - replace by "unsigned char status __attribute__ ((section (".noinit"))) / reset / retry / fallback "
      exit(0);
   }
   else
   {
      timer0_init = Timer0_Init(); /* initialize first timer */
   }

   if ( 1 != timer0_init )
   {
      generalErrorCode = CommunicationError_p(ERRG, GENERAL_ERROR_init_for_timer0_failed, FALSE, NULL);
#warning exit must not be used - replace by "unsigned char status __attribute__ ((section (".noinit"))) / reset / retry / fallback "
      exit(0);
   }
   else
   {
      timer0A_init = Timer0A_Init(); /* initialize second timer*/
   }
   if ( 1 != timer0A_init )
   {
      generalErrorCode = CommunicationError_p(ERRG, GENERAL_ERROR_init_for_timer0A_failed, FALSE, NULL);
#warning exit must not be used - replace by "unsigned char status __attribute__ ((section (".noinit"))) / reset / retry / fallback "
      exit(0);
   }


   clearString(currentResponseKeyword,MAX_LENGTH_KEYWORD);
   strncat_P(currentResponseKeyword, (const char*) ( pgm_read_word( &(responseKeywords[responseKeyNumber_RECV])) ), MAX_LENGTH_KEYWORD - 1);

   // start relay init
   if ( FALSE != relayThresholdEnable_flag)
   {
	   status = relayThresholdInit();

	   if (FALSE == status)
	   {
		   CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, TRUE, PSTR("Relay Init Failed") );

#warning TODO: missing action in case of failure: either exit or deactivate relay
	   }
   }

   // SPI
   spiInit();
   spiEnable(true);

#ifdef TESTING_ENABLE
   // Testing
   testingInit();
#endif

# warning remove it
//PORTG |= (1<<PG2);

   /* enable interrupts*/
   sei();

}//END of Initialization

/*This function initializes all input /output of the microcontroller
 *the  function has no input and output variable
 */

void InitIOPorts( void )
{
   uint8_t intstate = SREG; /* save global interrupt flag */
   cli();

#warning check right setting while merging, DDRA=0x00 could also be possible
   /* DDRX = 0xff =>  output ,  PORTX = 0x00 => disabled pullups*/
   DDRA  = 0xFF; //(1 << DDA0)| (1 << DDA1)| (1 << DDA2)| (1 << DDA3)| (1 << DDA4)| (1 << DDA5)| (1 << DDA6)| (1 << DDA7);
   PORTA = 0x00; //(0 << PA0) | (0 << PA1) | (0 << PA2) | (0 << PA3) | (0 << PA4) | (0 << PA5) | (0 << PA6) | (0 << PA7);
#warning probably n1 << needed by MDC Relay RLTH
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
   printDebug_p(debugLevelEventDebug, debugSystemApi, __LINE__, filename, PSTR("going to set ports for JTAG from: PING:%#x, PORTG:%#x"), PING&0xFF, PORTG&0xFF);

   DDRG  = (1 << DDG0)| (1 << DDG1)| (1 << DDG2)| (1 << DDG3) | (0 << DDG4);
   PORTG = (0 << PG0) | (0 << PG1) | (1 << PG2) | (1 << PG3)  | (1 << PG4);

   printDebug_p(debugLevelEventDebug, debugSystemApi, __LINE__, filename, PSTR("having changed ports for JTAG to: PING:%#x, PORTG:%#x"), PING&0xFF, PORTG&0xFF);
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
    UART0_Send_Message_String_p(NULL,0);*/

   SREG = intstate; /*restore global interrupt flag */

}//END of Init function

/*this function checks the functionality of the software*/
void keep_alive( struct uartStruct *ptr_uartStruct )
{
	flag_pingActive = ( 0 != ptr_uartStruct->Uart_Message_ID );
	strncat_P(uart_message_string, PSTR("RECV PING mechanism is "), BUFFER_SIZE - 1 );
	strncat_P(uart_message_string, ( flag_pingActive ) ? PSTR("enabled") : PSTR("disabled"), BUFFER_SIZE - 1 );
	UART0_Send_Message_String_p(NULL,0);
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
 *  output: uart_message_string
 */

#warning TODO: make uart_message_string optional buffer to write to

void createExtendedSubCommandReceiveResponseHeader(struct uartStruct * ptr_uartStruct,
                                                   int8_t commandKeywordIndex, int8_t subCommandKeywordIndex, PGM_P const commandKeywords[])
{
   /* make sure commandKeywordIndex is shown */
   int8_t keywordIndex = ptr_uartStruct->commandKeywordIndex;

   if (0 <= commandKeywordIndex) {ptr_uartStruct->commandKeywordIndex = commandKeywordIndex;}

   createReceiveHeader(ptr_uartStruct, uart_message_string, BUFFER_SIZE);

   /* reset commandKeywordIndex to original value */
   ptr_uartStruct->commandKeywordIndex = keywordIndex;

   if (-1 < subCommandKeywordIndex && NULL != commandKeywords)
   {
	   strncat_P(uart_message_string, (const char*) ( pgm_read_word( &(commandKeywords[subCommandKeywordIndex])) ), BUFFER_SIZE - 1);
	   strncat_P(uart_message_string, PSTR(" "), BUFFER_SIZE - 1);
   }
}

/*
 * getNumberOfHexDigits(const char *string, const uint16_t maxLength)
 *
 * returns
 *	 		- number of hex digits of string,
 *	 			if it is a number with hexadecimal digits
 *	 			(0x/0X of hex numbers are ignored) up to the maximum maxLength-1
 * 				NOTE:
 * 					if numeric value is not separated from next word
 * 					nor isn't followed by '\0',
 * 					it isn't a number
 * 		or
 * 			- "1"
 * 				in case of an case insensitive numerical constant name for 0/1 resulting in length 1:
 * 					using the definitions in isNumericalConstantOne() and isNumericalConstantZero()
 *		or
 *			- "0"
 *				else
 *
 * why not using strtoul(l)(string, NULL, 16) ?
 *		- numerical constants not supported
 *		- 64 bit not supported
 *		- string could represent a very long hex number > 16 digits
 */

uint16_t getNumberOfHexDigits(const char string[], const uint16_t maxLength)
{
	printDebug_p(debugLevelEventDebugVerbose, debugSystemApi, __LINE__, filename, PSTR("input: '%s' maxLength %i"), string, maxLength);

    uint16_t length = 0;
    bool prefixSet = false;

    /* first check for strings, which stand for 0/1 */
    if ( isNumericalConstantZero( string ) || isNumericalConstantOne( string ) )
    {
    	length = 1;
    }
    else
    {
    /* check if argument contains of digits */
    	if ( 0 == strncasecmp_P( string, PSTR("0x"),2) )
    	{
    		length = 2;
    		prefixSet = true;
    	}

    	/* calculate length of argument and check if all of them are hex numbers */
    	while (length < maxLength && isxdigit(string[length])) {length++;}

    	/* check if numeric value is not separated from next word nor isn't followed by '\0', so it isn't a number, */
    	if (length+1 < maxLength)
    	{
    		if   ( ! ( isspace(string[length+1]) || ('\0' == string[length+1] ) ) )
    		{
    			printDebug_p(debugLevelEventDebugVerbose, debugSystemApi, __LINE__, filename, PSTR("not a number - value followed by non-space character: %i, '%c'"), string[length+1], string[length+1]);
    			length = 0;
    		}
    	}
    }

	if (prefixSet)
	{
		length -= 2;
	}
	printDebug_p(debugLevelEventDebugVerbose, debugSystemApi, __LINE__, filename, PSTR("length of \"%s\" is %i"), string, length);
   	return length;
}

/*
 *	int8_t getUnsignedNumericValueFromParameterIndex(uint8_t parameterIndex, uint64_t *ptr_value)
 *
 * 	gets from string parameter array "setParameter" string at position "parameterIndex"
 * 		and converts it to an unsigned number stored in "ptr_value"
 *
 *	returns
 *		- -1
 *			in case of errors
 *		- 0
 *			else (success)
 */

int8_t getUnsignedNumericValueFromParameterIndex(uint8_t parameterIndex, uint64_t *ptr_value)
{
	if ( MAX_PARAMETER < parameterIndex || NULL == ptr_value)
	{
		CommunicationError_p(ERRG, dynamicMessage_ErrorIndex, TRUE, PSTR("getNumericValueFromParameter: wrong input parameters"));
		return -1;
	}

	if ( -1 == getUnsignedNumericValueFromParameterString(setParameter[parameterIndex], ptr_value) )
	{
		CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, TRUE, PSTR("command argument position %i (\"%s\") not a numeric value"), parameterIndex, setParameter[parameterIndex]);
		return -1;
	}

	return 0;
}

/*
 * bool isNumericArgument(const char string[])
 *
 * returns
 * 		true
 * 			in case of an case insensitive numerical constant name for 0/1 resulting in length 1:
 * 				using the definitions in isNumericalConstantOne() and isNumericalConstantZero()
 * 			or
 * 			string is a hex number
 * else
 * 		false
 * */

bool isNumericArgument(const char string[], const uint16_t maxLength)
{
	return (0 < getNumberOfHexDigits(string, maxLength)) ? true : false;
}

/*
 * bool isNumericalConstantOne(const char string[])
 *
 * returns true
 * 		if string matches case insensitively
 * 			TRUE, T not allowed due to ambiguity of 'F' with 0xF
 * 			H(IGH)
 * 			ON
 * else
 * 		false
 * */

bool isNumericalConstantOne(const char string[])
{
	if ( NULL == string )
	{
		CommunicationError_p(ERRG, dynamicMessage_ErrorIndex, TRUE, PSTR("isNumericalConstantOne: NULL input"));
		return false;
	}

	return (
    		//( 0 == strcasecmp_P(string, PSTR("T")   )) ||
    		( 0 == strcasecmp_P(string, PSTR("TRUE"))) ||
    		( 0 == strcasecmp_P(string, PSTR("ON")  )) ||
    		( 0 == strcasecmp_P(string, PSTR("H")   )) ||
    		( 0 == strcasecmp_P(string, PSTR("HIGH")))
    );
}

/*
 * bool isNumericalConstantZero(const char string[])
 *
 * returns true
 * 		if string matches case insensitively
 * 			FALSE , F not allowed due to ambiguity with 0xF
 *	 		L(LOW)
 *	 		OFF
 * else
 * 		false
 */

bool isNumericalConstantZero(const char string[])
{
	if ( NULL == string )
	{
		CommunicationError_p(ERRG, dynamicMessage_ErrorIndex, TRUE, PSTR("isNumericalConstantOne: NULL input"));
		return false;
	}
    return (
    		//( 0 == strcasecmp_P(string, PSTR("F")   )) ||
    		( 0 == strcasecmp_P(string, PSTR("FALSE"))) ||
    		( 0 == strcasecmp_P(string, PSTR("OFF")  )) ||
    		( 0 == strcasecmp_P(string, PSTR("L")   )) ||
    		( 0 == strcasecmp_P(string, PSTR("LOW")))
    );
}

/*
 *	int8_t getUnsignedNumericValueFromParameterString(uint8_t parameterIndex, uint64_t *ptr_value)
 *
 * 	gets from string parameter array "setParameter" string at position "parameterIndex"
 * 		and converts it to an unsigned number stored in "ptr_value"
 *
 *	returns
 *		- -1
 *			in case of errors
 *		- 0
 *			else (success)
 */

int8_t getUnsignedNumericValueFromParameterString(const char string[], uint64_t *ptr_value)
{
	if ( NULL == string || NULL == ptr_value)
	{
		CommunicationError_p(ERRG, dynamicMessage_ErrorIndex, TRUE, PSTR("getUnsignedNumericValueFromParameterString: NULL input parameters"));
		return -1;
	}

    if ( isNumericArgument(string, MAX_LENGTH_PARAMETER))
    {

    	#warning TODO: resolve ambiguity between shortcut "F" and Hex 0xF, by using varTypes?

    	/* ON,OFF,H(IGH),L(OW) : TRUE/FALSE */
    	if ( isNumericalConstantOne( string ) )
    	{
    		*ptr_value = TRUE;
    	}
    	else if ( isNumericalConstantZero( string ) )
    	{
    		*ptr_value = FALSE;
    	}
    	else
    	{
    		/* unsigned long */
    		if ( 8 >= getNumberOfHexDigits(string, MAX_LENGTH_PARAMETER) )
    		{
    			*ptr_value = strtoul(string, NULL, 16);
    		}
    		else
    		{
    			/* unsigned long long */
    			/* lower 8 digits, i.e. 32 bits */
    			*ptr_value = strtoul(&string[strlen(string) - 8], NULL, 16);

    			/* remaining higher digits*/

    			char string2[19] = "";
    			strncpy(string2, string, strlen(string) - 8 );
    			*ptr_value += (((uint64_t)strtoul(string2, NULL, 16)) << 32);
    		}
    		printDebug_p(debugLevelEventDebugVerbose, debugSystemApiMisc, __LINE__, (const char*)  ( filename ), PSTR("%#x"), *ptr_value);
    	}
    }
	else
	{
		CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, TRUE, PSTR("command argument position (\"%s\") not a numeric value"), string);
		return -1;
	}
	return 0;
}

void reset(struct uartStruct *ptr_uartStruct)
{
	for (uint8_t seconds = RESET_TIME_TO_WAIT_S; seconds > 0; --seconds)
	{
		wdt_enable(WDTO_500MS);
		createReceiveHeader(NULL, NULL, 0);
		snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s--- %i seconds to reset (via watchdog)"),uart_message_string, seconds );
		UART0_Send_Message_String_p(NULL,0);
		wdt_disable();
		_delay_ms(1000);
	}

    // watchdog
	wdt_enable(WDTO_15MS);
	while (1);
	wdt_disable();
}

void init(struct uartStruct *ptr_uartStruct)
{
	createReceiveHeader(NULL, NULL, 0);
	strncat_P(uart_message_string, PSTR("(re)init of system"), BUFFER_SIZE - 1 );
    UART0_Send_Message_String_p(NULL,0);

    Initialization();
}

uint8_t initUartStruct(struct uartStruct *ptr_myUartStruct)
{
   if (NULL == ptr_myUartStruct)
   {
      return 1;
   }

   ptr_myUartStruct->Uart_Message_ID = 0;
   ptr_myUartStruct->Uart_Mask = 0;
   ptr_myUartStruct->Uart_Rtr = 0;
   ptr_myUartStruct->Uart_Length = 0;
   for (unsigned int i = 0; i < 8; i++) { ptr_myUartStruct->Uart_Data[i]=0; }
   ptr_myUartStruct->commandKeywordIndex = commandKeyNumber_MAXIMUM_NUMBER;
   ptr_myUartStruct->number_of_arguments = 0;

   return 0;

}

//void backTrace(size_t level)
//{
//   /*
//        // prints out level steps of function calls
//        // before the function call (backtrace)
//        // Useful for debugging
//    */
//
//   void** array = (void*) calloc(level, sizeof(void));
//   int    size = 0;
//   //size = backtrace (array, level);
//
//   if(0 != size)
//   {
//      char **strings;
//      //strings = backtrace_symbols (array, size);
//      //message(stdout,NULL ,-1 ,"BACKTRACE",NULL, "Obtained %zd stack frames.\n", size);
//
//      int i=0;
//      for (i = 0; i < size; i++)
//      {
//         if (strings && strings[i])
//         {
//            //message(stdout,NULL ,-1 ,"BACKTRACE",NULL, " %s\n", strings[i]);
//         }
//      }
//      free(strings);
//   }
//   else
//   {
//      //message(stderr,__FILE__,__LINE__, "ERROR", "backTrack", "Could not retrieve backtrace information");
//   }
//   //safePArrayFree(array,level);
//}

void startMessage(void)
{
	clearString(uart_message_string, BUFFER_SIZE);
	strncat_P(uart_message_string, (const char*) ( pgm_read_word( &(responseKeywords[responseKeyNumber_SYST])) ), BUFFER_SIZE - 1);
	UART0_Send_Message_String_p(NULL,0);

	showResetSource(TRUE);

	version();
	showMem(ptr_uartStruct, commandShowKeyNumber_UNUSED_MEM_START);
	showMem(ptr_uartStruct, commandShowKeyNumber_UNUSED_MEM_NOW);
	owiApiFlag(ptr_uartStruct, owiApiCommandKeyNumber_COMMON_ADC_CONVERSION);
	owiApiFlag(ptr_uartStruct, owiApiCommandKeyNumber_COMMON_TEMPERATURE_CONVERSION);

	if ( debugLevelVerboseDebug <= globalDebugLevel && ( ( globalDebugSystemMask >> debugSystemMain ) & 0x1 ) )
	{
		owiFindParasitePoweredDevices(TRUE);
	}

	if (0 != watchdogIncarnationsCounter)
	{
		showWatchdogIncarnationsCounter(TRUE);
	}
}

size_t getMaximumStringArrayLength_P(PGM_P const array[], size_t maxIndex, size_t maxResult)
{
	size_t maxLength = 0;
	for (size_t index = 0; index < maxIndex; ++index)
	{
		if ( maxLength < strlen_P((const char*) (pgm_read_word( &(array[index])) )))
		{ maxLength = strlen_P((const char*) (pgm_read_word( &(array[index])))); }
	}
	maxLength = (maxLength < maxResult) ? maxLength : maxResult;
	return maxLength;
}

size_t getMaximumStringArrayLength(const char* array[], size_t maxIndex, size_t maxResult)
{
	size_t maxLength = 0;
	for (size_t index = 0; index < maxIndex; ++index)
	{
		if ( maxLength < strlen( array[index]))
		{ maxLength = strlen( array[index]); }
	}
	maxLength = (maxLength < maxResult) ? maxLength : maxResult;
	return maxLength;
}

void determineAndHandleResetSource()
{
	/*
	 * void handleResetSource(uint8_t startup_flag)
	 */
		// The MCU Status Register provides information on which reset source caused an MCU reset.
		//	    Bit 0 – PORF:  	Power-On Reset
		//	    Bit 1 – EXTRF: 	External Reset
		//	    Bit 2 – BORF:  	Brown-Out Reset
		//		Bit 3 – WDRF:  	Watchdog Reset
		//   	Bit 4 – JTRF:  	JTAG Reset Flag
		//  							This bit is set if a reset is being caused
		//                              by a logic one in the JTAG Reset Register selected by
		//                              the JTAG instruction AVR_RESET. This bit is reset by
		// 								a Power-on Reset, or by writing a logic zero to the flag.


		//evaluate mcusr

		if ( mcusr & (1 << JTRF) )
		{
			resetSource = resetSource_JTAG;
		}
		else if ( mcusr & (1 << WDRF) )
		{
			resetSource = resetSource_WATCHDOG;
		}
		else if ( mcusr & (1 << BORF) )
		{
			resetSource = resetSource_BROWN_OUT;
		}
		else if ( mcusr & (1 << EXTRF) )
		{
			resetSource = resetSource_EXTERNAL;
		}
	    else if ( mcusr & (1 << PORF) )
		{
			resetSource = resetSource_POWER_ON;
		}
		else
		{
			resetSource = resetSource_UNKNOWN_REASON;
		}

		//action
		switch(resetSource)
		{
		case resetSource_WATCHDOG:
			watchdogIncarnationsCounter++;
			break;
		case resetSource_JTAG:
		case resetSource_BROWN_OUT:
		case resetSource_EXTERNAL:
		case resetSource_POWER_ON:
		case resetSource_UNKNOWN_REASON:
		default:
			watchdogIncarnationsCounter = 0;
			break;
		}
}

uint8_t apiShowOrAssignParameterToValue(int16_t nArgumentArgs, uint8_t parameterIndex, void *value, uint8_t type, uint64_t min, uint64_t max, bool report, char message[])
{
	if ( NULL == message)
	{
		message = uart_message_string;
	}
	switch (nArgumentArgs)
	{
		case 0: /*print*/
			return apiShowValue(message, value, type );
			break;
		case 1: /*write*/
		default:
			/* take second parameter, i.e. first argument to sub command and fill it into value*/

			if ( apiCommandResult_FAILURE > apiAssignParameterToValue(2, value, type, min, max) )
			{
				if (report)
				{
					/* report by recursive call */
					apiShowValue(message, value, type );
					return apiCommandResult_SUCCESS_WITH_OUTPUT;
				}
			}

			break;
	}
	return apiCommandResult_SUCCESS_QUIET;
}

uint8_t apiAssignParameterToValue(uint8_t parameterIndex, void *value, uint8_t type, uint64_t min, uint64_t max)
{
#warning integrate type casting into getUnsignedNumericValueFromParameterIndex(parameterIndex, &inputValue)) ?
	uint64_t inputValue = 0;

	if ( 0 != getUnsignedNumericValueFromParameterIndex(parameterIndex, &inputValue))
	{
		return apiCommandResult_FAILURE_QUIET;
	}

	if ( (min > inputValue) || (max < inputValue) )
	{
		CommunicationError_p(ERRA, SERIAL_ERROR_arguments_exceed_boundaries, true, PSTR("[%ul,%ul] %ul"), min, max, inputValue);
		return apiCommandResult_FAILURE_QUIET;
	}

	/* set */
	switch( type )
	{
		case apiVarType_BOOL_OnOff:
		case apiVarType_BOOL_TrueFalse:
		case apiVarType_BOOL_HighLow:
		case apiVarType_BOOL:
			*((bool*)value) = (bool) (inputValue != 0);
			break;
		case apiVarType_UINT8:
			*((uint8_t*)value) = UINT8_MAX & inputValue;
			break;
		case apiVarType_UINT16:
			*((uint16_t*)value) = UINT16_MAX & inputValue;
			break;
		case apiVarType_UINT32:
			*((uint32_t*)value) = UINT32_MAX & inputValue;
			break;
		case apiVarType_UINT64:
			*((uint64_t*)value) = UINT64_MAX & inputValue;
			break;
		case apiVarType_UINTPTR:
			*((uintptr_t*)value) = UINTPTR_MAX & inputValue;
			break;
		case apiVarType_DOUBLE:
			*((double*)value) = inputValue;
			break;
		default:
			CommunicationError_p(ERRG, SERIAL_ERROR_arguments_have_invalid_type, 0, NULL);
			return apiCommandResult_FAILURE_QUIET;
			break;
	}

	return apiCommandResult_SUCCESS_QUIET;
}

uint8_t apiShowValue(char string[], void *value, uint8_t type )
{
	if (NULL == string)
	{
		string = uart_message_string;
	}
	switch( type )
	{
		case apiVarType_BOOL_OnOff:
			strncat_P(string, *((bool*)value)?PSTR("ON"):PSTR("OFF"), BUFFER_SIZE - 1);
			break;
		case apiVarType_BOOL_TrueFalse:
			strncat_P(string, *((bool*)value)?PSTR("TRUE"):PSTR("FALSE"), BUFFER_SIZE - 1);
			break;
		case apiVarType_BOOL_HighLow:
			strncat_P(string, *((bool*)value)?PSTR("HIGH"):PSTR("LOW"), BUFFER_SIZE - 1);
			break;
		case apiVarType_BOOL:
			snprintf_P(string, BUFFER_SIZE - 1, string_sX , string, *((bool*)value));
			break;
		case apiVarType_UINT8:
			snprintf_P(string, BUFFER_SIZE - 1, string_sX , string, *((uint8_t*)value));
			break;
		case apiVarType_UINT16:
			snprintf_P(string, BUFFER_SIZE - 1, string_sX , string, *((uint16_t*)value));
			break;
		case apiVarType_UINT32:
			snprintf_P(string, BUFFER_SIZE - 1, string_sX , string, *((uint32_t*)value));
			break;
		case apiVarType_UINT64:
			snprintf_P(string, BUFFER_SIZE - 1, string_sX , string, *((uint64_t*)value));
			break;
		case apiVarType_DOUBLE:
			snprintf_P(string, BUFFER_SIZE - 1, string_sX , string, *((double*)value));
			break;
		default:
			CommunicationError_p(ERRG, SERIAL_ERROR_arguments_have_invalid_type, 0, NULL);
			return apiCommandResult_FAILURE_QUIET;
			break;
	}
	return apiCommandResult_SUCCESS_WITH_OUTPUT;
}

void apiSubCommandsFooter( uint16_t result )
{
	switch (result)
	{
		case apiCommandResult_SUCCESS_WITH_OUTPUT:
			UART0_Send_Message_String_p(uart_message_string, BUFFER_SIZE - 1);
			break;
		case apiCommandResult_SUCCESS_WITH_OPTIONAL_OUTPUT__OK:
			/* verbose response to commands*/
			if (debugLevelVerboseDebug <= globalDebugLevel && ((globalDebugSystemMask >> debugSystemApi) & 1))
			{
				strncat_P(uart_message_string, PSTR("OK"), BUFFER_SIZE - 1);
				UART0_Send_Message_String_p(uart_message_string, BUFFER_SIZE - 1);
			}
			else
			{
				clearString(uart_message_string, BUFFER_SIZE);
			}
			break;
		case apiCommandResult_SUCCESS_WITH_OUTPUT__OK:
				strncat_P(uart_message_string, PSTR("OK"), BUFFER_SIZE - 1);
				UART0_Send_Message_String_p(uart_message_string, BUFFER_SIZE - 1);
			break;
		case apiCommandResult_SUCCESS_WITH_OUTPUT__DONE:
				strncat_P(uart_message_string, PSTR("DONE"), BUFFER_SIZE - 1);
				UART0_Send_Message_String_p(uart_message_string, BUFFER_SIZE - 1);
			break;
		case apiCommandResult_FAILURE_NOT_A_SUB_COMMAND:
			CommunicationError_p(ERRA, SERIAL_ERROR_no_valid_command_name, true, PSTR("not a sub command"));
			break;
		case apiCommandResult_SUCCESS_QUIET:
		case apiCommandResult_FAILURE_QUIET:
			clearString(uart_message_string, BUFFER_SIZE);
			/* printouts elsewhere generated */
			break;
		case apiCommandResult_FAILURE:
		default:
			CommunicationError_p(ERRA, dynamicMessage_ErrorIndex, true, PSTR("command failed"));
			break;
	}
}
