#ifndef LED__H
#define LED__H
char LED_print( char digit, char ASCII_data );
void LED_update( void );

struct LEDtype
{
	char digit1;
	char digit2;
} LED; // Global variables for printing to display

#endif
