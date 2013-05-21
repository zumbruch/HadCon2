/*
 * api_help.h
 *
 *  Created on: Apr 23, 2010
 *      Author: P.Zumbruch@gsi.de
 */

#ifndef API_HELP_H_
#define API_HELP_H_
void help(struct uartStruct *ptr_uartStruct);
void helpAll(uint8_t mode, char prefix[]);

enum helpMode
{
	helpMode_IMPLEMENTED = 0,
	helpMode_ALL,
	helpMode_TODO,
	helpMode_MAXIMUM_NUMBER
};

#endif /* API_HELP_H_ */
