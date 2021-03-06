/*
   Licensed under the EUPL V.1.1, Lizenziert unter EUPL V.1.1 
*/
/*
 * api_help.h
 *
 *  Created on: Apr 23, 2010
 *      Author: P.Zumbruch@gsi.de
 */

#ifndef API_HELP_H_
#define API_HELP_H_
enum helpCommandTypes
{
	helpCommandTypes_NO_ARGUMENTS = 0,
	helpCommandTypes_MAXIMUM_NUMBER
};

enum helpMode
{
	helpMode_IMPLEMENTED = 0,
	helpMode_ALL,
	helpMode_TODO,
	helpMode_MAXIMUM_NUMBER
};

extern const char* const helpCommandKeywords[] PROGMEM;
enum helpCommandKeyNumber
{
	helpCommandKeyNumber_IMPLEMENTED = 0,
	helpCommandKeyNumber_ALL,
	helpCommandKeyNumber_TODO,
	helpCommandKeyNumber_MAXIMUM_NUMBER
};

void help(struct uartStruct *ptr_uartStruct);
void helpAll(uint8_t mode, char prefix[]);
void helpShowAvailableSubCommands(int maximumIndex, PGM_P const commandKeywords[]);
void helpShowCommandOrResponse(char* currentReceiveHeader, PGM_P modifier, PGM_P string, ...);
void (*helpShowCommandOrResponse_p)(char*, PGM_P,  PGM_P, ...);

enum helpShowTypes
{
	helpShowType_EMPTY = 0,
	helpShowType_COMMAND,
	helpShowType_RESPONSE,
	helpShowType_ELSE,
	helpShowType_MAXIMUM_NUMBER
};

#endif /* API_HELP_H_ */
