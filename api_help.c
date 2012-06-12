/*
 * api_help.c
 *
 *  Created on: Apr 23, 2010
 *  Modified  : Jun 10, 2011
 *      Author: p.zumbruch@gsi.de
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/iocan128.h>
#include <avr/iocanxx.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h>

#include "api.h"
#include "api_define.h"
#include "api_global.h"

#include "api_debug.h"
#include "api_show.h"
#include "api_help.h"
#include "one_wire.h"
#include "one_wire_adc.h"
#include "one_wire_dualSwitch.h"
#include "one_wire_octalSwitch.h"
#include "one_wire_simpleSwitch.h"
#include "one_wire_temperature.h"
#include "one_wire_api_settings.h"
#include "read_write_register.h"
#include "relay.h"
#include "help_twis.h"

void help(struct uartStruct *ptr_uartStruct)
{
    /* list all available commands with short descriptions
     * or
     * command specific
     *
     * command: HELP [CMND]
     * response: RECV HELP --- CMD1 "bla1"
     *               ...
     *           RECV HELP --- CMDx "blaX"
     * response: RECV HELP *** CMND "bla"
     *               ...
     * response: RECV HELP *** CMND "bla"
     */
#define LENGTH 50
    int32_t index = -1;
    int32_t indexCopy = -1;
    char header[LENGTH];
    char currentReceiveHeader[LENGTH];

    clearString(header, LENGTH);
    createReceiveHeader(NULL, header, LENGTH);
    switch (ptr_uartStruct->number_of_arguments)
    {
    case 0:
        snprintf_P(message, BUFFER_SIZE, PSTR("%s---"), header);
        snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s available commands are:"), message);
        UART0_Send_Message_String(NULL,0);
        for (index = 0; index < commandKeyNumber_MAXIMUM_NUMBER; index++)
        {
            /*exclude list*/
            if ( commandKeyNumber_OWON == index) { continue; }

            /* compose message */
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s "), message);
            strncat_P(uart_message_string, (const char*) (pgm_read_word( &(commandKeywords[index]))), BUFFER_SIZE -1 );
            strncat_P(uart_message_string, PSTR(" : "), BUFFER_SIZE - 1 );
            strncat_P(uart_message_string, (const char*) (pgm_read_word( &(commandKeywords[index]))), BUFFER_SIZE -1 );
            strncat_P(uart_message_string, PSTR(" "), BUFFER_SIZE - 1 );
            strncat_P(uart_message_string, (const char*) (pgm_read_word( &(commandShortDescriptions[index]))), LENGTH);
            while (strlen(uart_message_string) < (MAX_LENGTH_KEYWORD + 1 + LENGTH))
            {
                strncat_P(uart_message_string, PSTR(" "), 1);
            }
            strncat_P(uart_message_string, (const char*) (pgm_read_word( &(commandImplementations[index]))), MAX_LENGTH_COMMAND);
            UART0_Send_Message_String(NULL,0);
        }
        break;
    case 1:
        index = Parse_Keyword(&setParameter[1][0]);
        if ( -1 < index && index < commandKeyNumber_MAXIMUM_NUMBER)
        {
            /* get current Command keyword */
            clearString(currentCommandKeyword,MAX_LENGTH_KEYWORD);
            strncat_P(currentCommandKeyword, (const char*) (pgm_read_word( &(commandKeywords[index]))), MAX_LENGTH_KEYWORD);

            /* extend already prepared header by current command keyword */
            /* compose message string */
            clearString(message, BUFFER_SIZE);
            snprintf_P(message, BUFFER_SIZE, PSTR("%s%s ***"), header, currentCommandKeyword);

            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s %s "), message, currentCommandKeyword);
            strncat_P(uart_message_string, (const char*) (pgm_read_word( &(commandShortDescriptions[index]))), BUFFER_SIZE - 1 );
            UART0_Send_Message_String(NULL,0);

            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s "), message);
            strncat_P(uart_message_string, (const char*) (pgm_read_word( &(commandImplementations[index]))), BUFFER_SIZE - 1);
            UART0_Send_Message_String(NULL,0);

            /*create keyword specific receiveHeader, e.g. "RECV CMDP"*/
            indexCopy = ptr_uartStruct->commandKeywordIndex;
            ptr_uartStruct->commandKeywordIndex = index;
            clearString(currentReceiveHeader, LENGTH);
            createReceiveHeader(NULL, currentReceiveHeader, LENGTH);
            ptr_uartStruct->commandKeywordIndex = indexCopy;
        }
        switch (index)
        {
        case commandKeyNumber_SEND:
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s send CAN messages "), message );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s       command      : %s CAN-Message-ID ID-Range [RTR <Number of data bytes> Data0 ... Data7] "), message, currentCommandKeyword );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s    case RTR 0"), message );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s       command      : %s CAN-Message-ID ID-Range 0 <Number of data bytes> Data0 ... Data7]"), message, currentCommandKeyword );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s       response now : %s CAN-Message-ID \"command will be carried out\""), message, currentReceiveHeader );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s       response TODO: <nothing> "), message );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s       TODO: is this obsolete, debug level?"), message );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s    case RTR 1"), message );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s       command      : %s CAN-Message-ID ID-Range 0 <Number of data bytes> Data0 ... Data7]"), message, currentCommandKeyword );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s       response  now: %s CAN_Mob CAN-Message-ID CAN-Length [Data0 ... Data7] "), message, currentResponseKeyword );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s       response TODO: %s CAN-Message-ID CAN-Length [Data0 ... Data7] "), message, currentReceiveHeader );
            UART0_Send_Message_String(NULL,0);
            //                        Receive_Message(ptr_uartStruct); /* call function with name Receive_message */
            //                     }
            break;
        case commandKeyNumber_SUBS:
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s subscribe to CAN message ID "), message );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s CAN-Message-ID ID-Range"), message, currentCommandKeyword );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: "), message );
            UART0_Send_Message_String(NULL,0);
            //                     Subscribe_Message(ptr_uartStruct); /* call function with name Subscribe_Message */
            break;
        case commandKeyNumber_USUB:
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s unsubscribe from CAN message ID "), message );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s CAN-Message-ID ID-Range"), message, currentCommandKeyword );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: "), message );
            UART0_Send_Message_String(NULL,0);
            //                     Unsubscribe_Message(ptr_uartStruct); /* call function with name Unsubscribe_Message */
            break;
        case commandKeyNumber_STAT:
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s ??? "), message );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command  : %s [ID]"), message, currentCommandKeyword );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response : %s ID1 description1 status1"), message, currentReceiveHeader );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s            ..."), message );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s            %s IDX descriptionX statusX "), message, currentReceiveHeader );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s  originally ment to read the pressure in the bus"), message );
            UART0_Send_Message_String(NULL,0);
            //         //atmelControlLoopReadStatus(ptr_uartStruct);
            break;
        case commandKeyNumber_OWTP:
           /* command : OWTP [ID / keyword [keyword arguments]]
               * [ID] response: RECV OWTP ID1 value1\r
               *                ...
               *                RECV OWTP IDx valueX\n
               * keyword response: RECV keyword STATUS
               * command : OWTP
               */
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s one wire temperature "), message );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s [ID [flag_conv] | <command_keyword> [arguments] ] "), message, currentCommandKeyword );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response [ID]: %s ID1 value1"), message, currentReceiveHeader );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s                ..."), message );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s                %s IDx valueX "), message, currentReceiveHeader );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response <keyword>: %s command_key [<corresponding answer/acknowledge>]"), message, currentReceiveHeader );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s %s available command_keys"), message, currentCommandKeyword );
            UART0_Send_Message_String(NULL,0);
            for (int i=0; i < owiTemperatureCommandKeyNumber_MAXIMUM_NUMBER; i++)
            {
                snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s    "), message );
                strncat_P(uart_message_string, (const char*) (pgm_read_word( &(owiTemperatureCommandKeywords[i]))), MAX_LENGTH_PARAMETER) ;

                UART0_Send_Message_String(NULL,0);
            }
            break;
        case commandKeyNumber_RGWR:
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s register write "), message );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command      : %s Register [Value] "), message, currentCommandKeyword );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response now : %s the value %x has been written in Register "), message, currentResponseKeyword );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response TODO: %s Register Value (OldValue) "), message, currentReceiveHeader );
            UART0_Send_Message_String(NULL,0);
            //         writeRegister(ptr_uartStruct); /* call function with name writeRegister  */
            break;
        case commandKeyNumber_RGRE:
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s register write "), message );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command      : %s Register"), message, currentCommandKeyword );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response now : %s the value %x has been written in Register "), message, currentResponseKeyword );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response TODO: %s Register Value "), message, currentReceiveHeader );
            UART0_Send_Message_String(NULL,0);
            //         readRegister(ptr_uartStruct); /* call function with name  readRegister */
            break;
        case commandKeyNumber_RADC: /* read AVR's ADCs */
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s read AVR's ADCs "), message );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command      : %s [<ADC Channel>] "), message, currentCommandKeyword );
            UART0_Send_Message_String(NULL,0);
            //         atmelReadADCs(ptr_uartStruct);
            break;
        case commandKeyNumber_OWAD: /* one-wire adc */
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s one wire adc "), message );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s [ID [flag_conv [flag_init]]]"), message, currentCommandKeyword );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s ID1 value1.1 ... value1.n"), message, currentReceiveHeader );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s           ..."), message );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s           %s IDx valueX.1 ... valueX.n "), message, currentReceiveHeader );
            UART0_Send_Message_String(NULL,0);
            //         owiReadADCs(ptr_uartStruct);
            //         /* similar to OWTP*/
            break;
        case commandKeyNumber_OWDS:
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s one wire dual switches"), message );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command read : %s [ID]"), message, currentCommandKeyword );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command write: %s [ID] value"), message, currentCommandKeyword );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command keyword: %s [ID] on/off/toggle"), message, currentCommandKeyword );
            UART0_Send_Message_String(NULL,0);
            break;
        case commandKeyNumber_RSET:
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s reset micro controller using watchdog mechanism"), message );
            UART0_Send_Message_String(NULL,0);
            //         Initialization(ptr_uartStruct); /* call function with name  readRegister */
            break;
        case commandKeyNumber_OWSS:
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s one wire single switches"), message );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s [ID]"), message, currentCommandKeyword );
            UART0_Send_Message_String(NULL,0);
            //         read_status_simpleSwitches(ptr_uartStruct); /* call function with name  readRegister */
            break;
        case commandKeyNumber_OWLS:
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s one wire list devices [of type <family code>]"), message );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s [<Family Code>]"), message, currentCommandKeyword );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s bus mask: 0x<bus mask> ID: <owi ID>"), message, currentReceiveHeader );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s           ..."), message );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s bus mask: 0x<bus mask> ID :<owi ID>"), message, currentReceiveHeader );
            UART0_Send_Message_String(NULL,0);
            //         list_all_devices(ptr_uartStruct); /* call function with name  readRegister */
            break;
        case commandKeyNumber_PING:
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s receive an ALIV(e) periodically"), message );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s "), message, currentCommandKeyword );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: ALIV "), message );
            UART0_Send_Message_String(NULL,0);
            //         keep_alive(ptr_uartStruct); /* call function with name  readRegister */
            break;
        case commandKeyNumber_OWSP: /*one-wire set active pins/bus mask*/
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s one wire set/get pin/bus mask"), message );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s <bus mask>"), message, currentCommandKeyword );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: ... "), message );
            UART0_Send_Message_String(NULL,0);
            //         setOneWireBusMask(ptr_uartStruct);
            break;
        case commandKeyNumber_OWRP: /*one-wire read active pins/bus mask*/
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s one wire get pin/bus mask"), message );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : OWRP"), message );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s value "), message, currentReceiveHeader );
            UART0_Send_Message_String(NULL,0);
            //         getOneWireBusMask(ptr_uartStruct);
            break;
        case commandKeyNumber_ADSP: /*AVR's adcs set active pins/bus mask*/
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s AVR's ADCs set/get pin/bus mask"), message );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s <bus mask>"), message, currentCommandKeyword );
            UART0_Send_Message_String(NULL,0);
            break;
        case commandKeyNumber_RLSL: /*relay set low  level*/
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s relay set low  level"), message );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s <bus> [val]"), message, currentCommandKeyword );
            UART0_Send_Message_String(NULL,0);
            break;
        case commandKeyNumber_RLSH: /*relay set high level*/
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s relay set high  level"), message );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s <bus> [val]"), message, currentCommandKeyword );
            UART0_Send_Message_String(NULL,0);
            break;
        case commandKeyNumber_RLSI: /*relay set ADC pin(s) to monitor "in" */
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s relay set input pin"), message );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s <bus> [pins]"), message, currentCommandKeyword );
            UART0_Send_Message_String(NULL,0);
            break;
        case commandKeyNumber_RLSO: /*relay set output pin(s) to switch "out" */
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s relay set output pin"), message );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s <bus> [pins]"), message, currentCommandKeyword );
            UART0_Send_Message_String(NULL,0);
            break;
        case commandKeyNumber_DEBG: /*set/get debug level*/
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s set/get debug level and mask"), message );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s [level [mask]]"), message, currentCommandKeyword );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s set response: ..."), message );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s get response: %s level mask"), message, currentReceiveHeader );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s    "), message );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s available debug levels are:"), message );
            UART0_Send_Message_String(NULL,0);
            for (int i=0; i < levelsDebug_MAXIMUM_INDEX; i++)
            {
                snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s    "), message );
                strncat_P(uart_message_string, (const char*) (pgm_read_word( &(debugLevelNames[i]))), BUFFER_SIZE -1) ;
                snprintf(uart_message_string, BUFFER_SIZE -1, "%s: 0x%X", uart_message_string, i);
                UART0_Send_Message_String(NULL,0);
            }
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s    "), message );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s available masks are:"), message );
            UART0_Send_Message_String(NULL,0);
            for (int i=0; i < debugSystems_MAXIMUM_INDEX; i++)
            {
                snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s    "), message );
                strncat_P(uart_message_string, (const char*) (pgm_read_word( &(debugSystemNames[i]))), BUFFER_SIZE -1) ;
                snprintf(uart_message_string, BUFFER_SIZE -1, "%s: 0x%08lX", uart_message_string, ((int32_t) 0x1) << i);
                UART0_Send_Message_String(NULL,0);
            }
            break;
        case commandKeyNumber_DBGL: /*set/get debug level*/
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s set/get debug level"), message );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s [level]"), message, currentCommandKeyword );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s set response: ..."), message );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s get response: %s level "), message, currentReceiveHeader );
            UART0_Send_Message_String(NULL,0);
            //         readModifyDebugLevel(ptr_uartStruct);
            break;
        case commandKeyNumber_DBGM: /*set/get only debug system mask*/
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s set/get debug mask"), message );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s [mask]"), message, currentCommandKeyword );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s set response: ..."), message );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s get response: %s mask"), message, currentReceiveHeader );
            UART0_Send_Message_String(NULL,0);
            //         readModifyDebugMask(ptr_uartStruct);
            break;
        case commandKeyNumber_PARA: /*set/get debug level*/
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s check for parasiticly connected devices"), message );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s [???]]"), message, currentCommandKeyword );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s %s pins 0xXX have/has parasitic devices"), message, currentReceiveHeader, currentCommandKeyword );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s %s pins 0xXX have been pulled HIGH within XX ms"), message, currentReceiveHeader, currentCommandKeyword );
            UART0_Send_Message_String(NULL,0);
            break;
//        case commandKeyNumber_WDOG: /*set/get debug level*/
//            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s set/get watch dog status"), message );
//            UART0_Send_Message_String(NULL,0);
//            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s [???]]"), message, currentCommandKeyword );
//            UART0_Send_Message_String(NULL,0);
//            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s set response: ..."), message );
//            UART0_Send_Message_String(NULL,0);
//            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s get response: %s ???"), message, currentReceiveHeader );
//            UART0_Send_Message_String(NULL,0);
//            //         readModifyDebugLevelAndMask(ptr_uartStruct);
//            break;
        case commandKeyNumber_JTAG: /*toggle/set JTAG availability*/
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s switch of %s and enable 4 more ADC channels"), message, currentCommandKeyword );
            UART0_Send_Message_String(NULL,0);
            break;
        case commandKeyNumber_OWTR: /*trigger one-wire device(s) for action, if possible*/
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s one wire ???"), message );
            UART0_Send_Message_String(NULL,0);
            break;
        case commandKeyNumber_HELP: /*output some help*/
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s get all commands or specific HELP"), message );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s [CMD]"), message, currentCommandKeyword );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s all response: %s --- ... "), message, currentReceiveHeader );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s cmd response: %s *** ... "), message, currentReceiveHeader );
            UART0_Send_Message_String(NULL,0);
            break;
        case commandKeyNumber_SHOW: /*output some help*/
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s show (internal) settings"), message );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s [command_key]"), message, currentCommandKeyword );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s all response: %s command_key1 ... "), message, currentReceiveHeader );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s               ...               ... "), message );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s               %s command_keyN ... "), message, currentReceiveHeader );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s cmd response: %s command_key"), message, currentReceiveHeader );
            UART0_Send_Message_String(NULL,0);
            snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s %s available command_keys"), message, currentCommandKeyword );
            UART0_Send_Message_String(NULL,0);
            for (int i=0; i < commandShowKeyNumber_MAXIMUM_NUMBER; i++)
            {
                snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s    "), message );
                strncat_P(uart_message_string, (const char*) (pgm_read_word( &(commandShowKeywords[i]))), MAX_LENGTH_PARAMETER) ;

                UART0_Send_Message_String(NULL,0);
            }
            break;
        case commandKeyNumber_OWMR: /*one wire basics: match rom*/
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s one wire basics: match rom"), message );
           UART0_Send_Message_String(NULL,0);
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s ID <pin_mask>"), message, currentCommandKeyword );
           UART0_Send_Message_String(NULL,0);
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s <acknowledge> "), message, currentReceiveHeader );
           UART0_Send_Message_String(NULL,0);
           break;
        case commandKeyNumber_OWPC: /*one wire basics: presence check*/
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s one wire basics: presence check"), message );
           UART0_Send_Message_String(NULL,0);
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s [<pin_mask>]"), message, currentCommandKeyword );
           UART0_Send_Message_String(NULL,0);
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s <bit mask of used channels> "), message, currentReceiveHeader );
           UART0_Send_Message_String(NULL,0);
           break;
        case commandKeyNumber_OWRb: /*one wire basics: receive bit, wait for it*/
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s one wire basics: receive bit, wait for it"), message );
           UART0_Send_Message_String(NULL,0);
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s <pin_mask> <delay> <timeout: N (times delay)> "), message, currentCommandKeyword );
           UART0_Send_Message_String(NULL,0);
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s <count of delays, timeout: c<=0> "), message, currentReceiveHeader );
           UART0_Send_Message_String(NULL,0);
           break;
        case commandKeyNumber_OWRB: /*one wire basics: receive byte*/
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s one wire basics: receive byte"), message );
           UART0_Send_Message_String(NULL,0);
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s [<pin_mask>]"), message, currentCommandKeyword );
           UART0_Send_Message_String(NULL,0);
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s <value> "), message, currentReceiveHeader );
           UART0_Send_Message_String(NULL,0);
           break;
        case commandKeyNumber_OWSC: /*one wire basics: send command*/
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s one wire basics: send command"), message );
           UART0_Send_Message_String(NULL,0);
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s <command_key_word> [<pin_mask> [arguments ...]] "), message, currentCommandKeyword );
           UART0_Send_Message_String(NULL,0);
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s command_key [<corresponding answer/acknowledge>]"), message, currentReceiveHeader );
           UART0_Send_Message_String(NULL,0);
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s %s available command_keys"), message, currentCommandKeyword );
           UART0_Send_Message_String(NULL,0);
           for (int i=0; i < 0 /*owiSendCommandKeyNumber_MAXIMUM_NUMBER*/; i++)
           {
               snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s    "), message );
               //strncat_P(uart_message_string, (const char*) (pgm_read_word( &(owiSendCommandKeywords[i]))), MAX_LENGTH_PARAMETER) ;

               UART0_Send_Message_String(NULL,0);
           }
           break;
        case commandKeyNumber_OWSB: /*one wire basics: send byte*/
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s one wire basics: send byte"), message );
           UART0_Send_Message_String(NULL,0);
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s <byte> [<pin_mask>]"), message, currentCommandKeyword );
           UART0_Send_Message_String(NULL,0);
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s <byte> <acknowledge> "), message, currentReceiveHeader );
           UART0_Send_Message_String(NULL,0);
           break;
        case commandKeyNumber_OWSA: /*one wire API settings: set/get 1-wire specific API settings*/
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s set/get 1-wire specific API settings"), message );
           UART0_Send_Message_String(NULL,0);
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s <command_key_word> [arguments] "), message, currentCommandKeyword );
           UART0_Send_Message_String(NULL,0);
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s command_key [<corresponding answer/acknowledge>]"), message, currentReceiveHeader );
           UART0_Send_Message_String(NULL,0);
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s %s available command_keys"), message, currentCommandKeyword );
           UART0_Send_Message_String(NULL,0);
           for (int i=0; i < owiApiCommandKeyNumber_MAXIMUM_NUMBER; i++)
           {
               snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s    "), message );
               strncat_P(uart_message_string, (const char*) (pgm_read_word( &(owiApiCommandKeywords[i]))), MAX_LENGTH_PARAMETER) ;
               UART0_Send_Message_String(NULL,0);
           }
           break;
        case commandKeyNumber_WDOG: /*set/get watch dog status*/
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s set/get watch dog status"), message );
           UART0_Send_Message_String(NULL,0);
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s [???]"), message, currentCommandKeyword );
           UART0_Send_Message_String(NULL,0);
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s [???] "), message, currentReceiveHeader );
           break;
           //        case commandKeyNumber_EXIT: /*exit*/
           //           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s exit"), message );
           //           UART0_Send_Message_String(NULL,0);
           //           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s [???]"), message, currentCommandKeyword );
           //           UART0_Send_Message_String(NULL,0);
           //           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s [???] "), message, currentReceiveHeader );
           //           UART0_Send_Message_String(NULL,0);
           //           break;
        case commandKeyNumber_RLTH: /* relay threshold */
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s manage threshold relay "), message );
           UART0_Send_Message_String(NULL,0);
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s [command_key_word]<"), message, currentCommandKeyword );
           UART0_Send_Message_String(NULL,0);
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s [command_key_word] <value> "), message, currentReceiveHeader );
           UART0_Send_Message_String(NULL,0);
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s              [...]              ... "), message );
           UART0_Send_Message_String(NULL,0);
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s [command_key_word] <value> "), message, currentReceiveHeader );
           UART0_Send_Message_String(NULL,0);
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s %s available command_keys"), message, currentCommandKeyword );
           UART0_Send_Message_String(NULL,0);
           for (int i=0; i < relayThresholdCommandKeyNumber_MAXIMUM_NUMBER; i++)
           {
               snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s    "), message );
               strncat_P(uart_message_string, (const char*) (pgm_read_word( &(relayThresholdCommandKeywords[i]))), MAX_LENGTH_PARAMETER) ;

               UART0_Send_Message_String(NULL,0);
           }
          break;
        case commandKeyNumber_VERS: /* print code version */
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s print code version "), message );
           UART0_Send_Message_String(NULL,0);
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s"), message, currentCommandKeyword );
           UART0_Send_Message_String(NULL,0);
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s <Version> "), message, currentReceiveHeader );
           UART0_Send_Message_String(NULL,0);
           break;
        case commandKeyNumber_TWIS: /* I2C / TWI */
           help_twis(currentReceiveHeader, currentCommandKeyword);
           break;
        case commandKeyNumber_CMD1: /* command (dummy name) */
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s  command (dummy name) "), message );
           UART0_Send_Message_String(NULL,0);
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s [???]"), message, currentCommandKeyword );
           UART0_Send_Message_String(NULL,0);
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s [???] "), message, currentReceiveHeader );
           UART0_Send_Message_String(NULL,0);
           break;
        case commandKeyNumber_CMD2: /* command (dummy name) */
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s  command (dummy name) "), message );
           UART0_Send_Message_String(NULL,0);
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s [???]"), message, currentCommandKeyword );
           UART0_Send_Message_String(NULL,0);
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s [???] "), message, currentReceiveHeader );
           UART0_Send_Message_String(NULL,0);
           break;
        case commandKeyNumber_CMD3: /* command (dummy name) */
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s  command (dummy name) "), message );
           UART0_Send_Message_String(NULL,0);
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s [???]"), message, currentCommandKeyword );
           UART0_Send_Message_String(NULL,0);
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s [???] "), message, currentReceiveHeader );
           UART0_Send_Message_String(NULL,0);
           break;
        case commandKeyNumber_CMD4: /* command (dummy name) */
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s  command (dummy name) "), message );
           UART0_Send_Message_String(NULL,0);
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s [???]"), message, currentCommandKeyword );
           UART0_Send_Message_String(NULL,0);
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s [???] "), message, currentReceiveHeader );
           UART0_Send_Message_String(NULL,0);
           break;
        case commandKeyNumber_CMD5: /* command (dummy name) */
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s  command (dummy name) "), message );
           UART0_Send_Message_String(NULL,0);
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s [???]"), message, currentCommandKeyword );
           UART0_Send_Message_String(NULL,0);
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s [???] "), message, currentReceiveHeader );
           UART0_Send_Message_String(NULL,0);
           break;
        case commandKeyNumber_CMD6: /* command (dummy name) */
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s  command (dummy name) "), message );
           UART0_Send_Message_String(NULL,0);
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s [???]"), message, currentCommandKeyword );
           UART0_Send_Message_String(NULL,0);
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s [???] "), message, currentReceiveHeader );
           UART0_Send_Message_String(NULL,0);
           break;
        case commandKeyNumber_OW8S: /* command (dummy name) */
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s one wire octal switches "), message );
           UART0_Send_Message_String(NULL,0);
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s - reads all switches and shows the IDs"), message, currentCommandKeyword );
           UART0_Send_Message_String(NULL,0);
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s <ID> - reads single switch"), message, currentCommandKeyword );
           UART0_Send_Message_String(NULL,0);
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s <value> - writes <value> to all switches"), message, currentCommandKeyword );
           UART0_Send_Message_String(NULL,0);
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s command : %s <ID> <value> - writes <value> to single switch"), message, currentCommandKeyword );
           UART0_Send_Message_String(NULL,0);
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s response: %s <current state of switch>"), message, currentReceiveHeader );
           UART0_Send_Message_String(NULL,0);
           break;
        default:
           snprintf_P(uart_message_string, BUFFER_SIZE - 1, PSTR("%s %s --- unknown command"), message, header );
           UART0_Send_Message_String(NULL,0);
           break;
        }
        break;
           default:
              ptr_uartStruct->number_of_arguments--;
              help(ptr_uartStruct);
              break;
    }
}

#if 0
strncat_P(     desc[commandKeyNumber_SEND],PSTR("send CAN messages "),DESC_LENGTH -1);
strncat_P(arguments[commandKeyNumber_SEND],PSTR("CAN-Message-ID ID-Range [RTR <Number of data bytes> Data0 ... Data7]"),ARG_LENGTH -1);
strncat_P(     desc[commandKeyNumber_SUBS],PSTR(" subscribe to CAN message ID "),DESC_LENGTH -1);
strncat_P(arguments[commandKeyNumber_SUBS],PSTR("CAN-Message-ID ID-Range"),ARG_LENGTH -1);
strncat_P(     desc[commandKeyNumber_USUB],PSTR("unsubscribe from CAN message ID"),DESC_LENGTH -1);
strncat_P(arguments[commandKeyNumber_USUB],PSTR("CAN-Message-ID ID-Range"),ARG_LENGTH -1);
strncat_P(     desc[commandKeyNumber_STAT],PSTR("???"),DESC_LENGTH -1);
strncat_P(arguments[commandKeyNumber_STAT],PSTR("[ID]"),ARG_LENGTH -1);
strncat_P(     desc[commandKeyNumber_OWTP],PSTR("1-wire temperature"),DESC_LENGTH -1);
strncat_P(arguments[commandKeyNumber_OWTP],PSTR("[ID [flag_conv [flag_init]]]"),ARG_LENGTH -1);
strncat_P(     desc[commandKeyNumber_RGWR],PSTR("register write "),DESC_LENGTH -1);
strncat_P(arguments[commandKeyNumber_RGWR],PSTR("Register [Value]"),ARG_LENGTH -1);
strncat_P(     desc[commandKeyNumber_RGRE],PSTR("register read "),DESC_LENGTH -1);
strncat_P(arguments[commandKeyNumber_RGRE],PSTR("Register"),ARG_LENGTH -1);
strncat_P(     desc[commandKeyNumber_RADC],PSTR("read AVR's ADCs "),DESC_LENGTH -1);
strncat_P(arguments[commandKeyNumber_RADC],PSTR("[<ADC Channel>]"),ARG_LENGTH -1);
strncat_P(     desc[commandKeyNumber_OWAD],PSTR("one-wire adc "),DESC_LENGTH -1);
strncat_P(arguments[commandKeyNumber_OWAD],PSTR("[ID [flag_conv [flag_init]]]"),ARG_LENGTH -1);
strncat_P(     desc[commandKeyNumber_OWON],PSTR("one wire set dual switches on "),DESC_LENGTH -1);
strncat_P(arguments[commandKeyNumber_OWON],PSTR("[ID]"),ARG_LENGTH -1);
strncat_P(     desc[commandKeyNumber_OWDS],PSTR(" set/get one wire dual switches  "),DESC_LENGTH -1);
strncat_P(arguments[commandKeyNumber_OWDS],PSTR("[ID]"),ARG_LENGTH -1);
strncat_P(     desc[commandKeyNumber_RSET],PSTR("reset micro controller"),DESC_LENGTH -1);
strncat_P(arguments[commandKeyNumber_RSET],PSTR(""),ARG_LENGTH -1);
strncat_P(     desc[commandKeyNumber_OWSS],PSTR("set/get one wire single switches  "),DESC_LENGTH -1);
strncat_P(arguments[commandKeyNumber_OWSS],PSTR("[ID]"),ARG_LENGTH -1);
strncat_P(     desc[commandKeyNumber_OWLS],PSTR("one wire list devices [of type <family code>]"),DESC_LENGTH -1);
strncat_P(arguments[commandKeyNumber_OWLS],PSTR("[<Family Code>]"),ARG_LENGTH -1);
strncat_P(     desc[commandKeyNumber_PING],PSTR("receive an ALIV(e) periodically"),DESC_LENGTH -1);
strncat_P(arguments[commandKeyNumber_PING],PSTR(""),ARG_LENGTH -1);
strncat_P(     desc[commandKeyNumber_OWSP],PSTR("one-wire set/get active pins/bus mask"),DESC_LENGTH -1);
strncat_P(arguments[commandKeyNumber_OWSP],PSTR("<bus mask>"),ARG_LENGTH -1);
strncat_P(     desc[commandKeyNumber_OWRP],PSTR("one-wire get pins' bus mask"),DESC_LENGTH -1);
strncat_P(arguments[commandKeyNumber_OWRP],PSTR(""),ARG_LENGTH -1);
strncat_P(     desc[commandKeyNumber_ADSP],PSTR("AVR's adcs set active pins/bus mask"),DESC_LENGTH -1);
strncat_P(arguments[commandKeyNumber_ADSP],PSTR("<bus mask>"),ARG_LENGTH -1);
strncat_P(     desc[commandKeyNumber_RLSL],PSTR("relay set low  level"),DESC_LENGTH -1);
strncat_P(arguments[commandKeyNumber_RLSL],PSTR("<bus> [val]"),ARG_LENGTH -1);
strncat_P(     desc[commandKeyNumber_RLSH],PSTR("relay set high level"),DESC_LENGTH -1);
strncat_P(arguments[commandKeyNumber_RLSH],PSTR("<bus> [val]"),ARG_LENGTH -1);
strncat_P(     desc[commandKeyNumber_RLSI],PSTR("relay set ADC pin(s) to monitor "in" "),DESC_LENGTH -1);
strncat_P(arguments[commandKeyNumber_RLSI],PSTR("<bus> [pins]"),ARG_LENGTH -1);
strncat_P(     desc[commandKeyNumber_RLSO],PSTR("relay set/get output pin mask "),DESC_LENGTH -1);
strncat_P(arguments[commandKeyNumber_RLSO],PSTR("<bus> [pins]"),ARG_LENGTH -1);
strncat_P(     desc[commandKeyNumber_DEBG],PSTR("set/get debug level and mask"),DESC_LENGTH -1);
strncat_P(arguments[commandKeyNumber_DEBG],PSTR("[level [mask]]"),ARG_LENGTH -1);
strncat_P(     desc[commandKeyNumber_DBGL],PSTR("set/get debug level"),DESC_LENGTH -1);
strncat_P(arguments[commandKeyNumber_DBGL],PSTR("[level]"),ARG_LENGTH -1);
strncat_P(     desc[commandKeyNumber_DBGM],PSTR("set/get debug mask"),DESC_LENGTH -1);
strncat_P(arguments[commandKeyNumber_DBGM],PSTR("[mask]"),ARG_LENGTH -1);
strncat_P(     desc[commandKeyNumber_WDOG],PSTR("set/get watch/dog status"),DESC_LENGTH -1);
strncat_P(arguments[commandKeyNumber_WDOG],PSTR("[???]"),ARG_LENGTH -1);
strncat_P(     desc[commandKeyNumber_JTAG],PSTR("toggle/set JTAG availability"),DESC_LENGTH -1);
strncat_P(arguments[commandKeyNumber_JTAG],PSTR("[???]"),ARG_LENGTH -1);
strncat_P(     desc[commandKeyNumber_OWTR],PSTR("trigger one-wire device(s) for action, if possible"),DESC_LENGTH -1);
strncat_P(arguments[commandKeyNumber_OWTR],PSTR("[???]"),ARG_LENGTH -1);
strncat_P(     desc[commandKeyNumber_HELP],PSTR("get all commands or specific HELP"),DESC_LENGTH -1);
strncat_P(arguments[commandKeyNumber_HELP],PSTR("[CMD]"),ARG_LENGTH -1);
strncat_P(     desc[commandKeyNumber_SHOW],PSTR("show (internal) settings"),DESC_LENGTH -1);
strncat_P(arguments[commandKeyNumber_SHOW],PSTR("[command_key]"),ARG_LENGTH -1);
strncat_P(     desc[commandKeyNumber_OWMR],PSTR("one wire basics: match rom"),DESC_LENGTH -1);
strncat_P(arguments[commandKeyNumber_OWMR],PSTR("ID <pin_mask>"),ARG_LENGTH -1);
strncat_P(     desc[commandKeyNumber_OWPC],PSTR("one wire basics: presence check"),DESC_LENGTH -1);
strncat_P(arguments[commandKeyNumber_OWPC],PSTR("[<pin_mask>]"),ARG_LENGTH -1);
strncat_P(     desc[commandKeyNumber_OWRb],PSTR("one wire basics: receive bit, wait for it"),DESC_LENGTH -1);
strncat_P(arguments[commandKeyNumber_OWRb],PSTR("<pin_mask> <delay> <timeout: N (times delay)>"),ARG_LENGTH -1);
strncat_P(     desc[commandKeyNumber_OWRB],PSTR("one wire basics: receive byte"),DESC_LENGTH -1);
strncat_P(arguments[commandKeyNumber_OWRB],PSTR("[<pin_mask>]"),ARG_LENGTH -1);
strncat_P(     desc[commandKeyNumber_OWSC],PSTR("one wire basics: send command"),DESC_LENGTH -1);
strncat_P(arguments[commandKeyNumber_OWSC],PSTR("<command_key_word> [<pin_mask> [arguments ...]]"),ARG_LENGTH -1);
strncat_P(     desc[commandKeyNumber_OWSB],PSTR("one wire basics: send byte"),DESC_LENGTH -1);
strncat_P(arguments[commandKeyNumber_OWSB],PSTR("<byte> [<pin_mask>]"),ARG_LENGTH -1);
strncat_P(     desc[commandKeyNumber_OWSA],PSTR("one wire API settings: set/get 1-wire specific API settings"),DESC_LENGTH -1);
strncat_P(arguments[commandKeyNumber_OWSA],PSTR("<command_key_word> [arguments]"),ARG_LENGTH -1);
#endif
