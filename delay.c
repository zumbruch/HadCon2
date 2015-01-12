/*
   Licensed under the EUPL V.1.1, Lizenziert unter EUPL V.1.1 
*/
/*******************************************************
 File:	Delay.c
 Latest version generated:	02.06.2007

 Delay functions usage (include Delay.h):

 	Delay_ms (10)		// Delay 10 ms
 	Delay_us (100)		// Delay 100 us

 *******************************************************/
#include "delay.h"
/* 
   Precise Delay Functions 
   V 0.5, Martin Thomas, 9/2004
   
   In the original Code from Peter Dannegger a timer-interrupt
   driven "timebase" has been used for precise One-Wire-Delays.
   My loop-approach is less elegant but may be more usable 
   as library-function. Since it's not "timer-dependent"
   See also delay.h.
   
   Inspired by the avr-libc's loop-code
*/
void Delayloop32(uint32_t loops) 
{
  __asm__ volatile ( "cp  %A0,__zero_reg__ \n\t"  \
                     "cpc %B0,__zero_reg__ \n\t"  \
                     "cpc %C0,__zero_reg__ \n\t"  \
                     "cpc %D0,__zero_reg__ \n\t"  \
                     "breq L_Exit_%=       \n\t"  \
                     "L_LOOP_%=:           \n\t"  \
                     "subi %A0,1           \n\t"  \
                     "sbci %B0,0           \n\t"  \
                     "sbci %C0,0           \n\t"  \
                     "sbci %D0,0           \n\t"  \
                     "brne L_LOOP_%=            \n\t"  \
                     "L_Exit_%=:           \n\t"  \
                     : "=w" (loops)              \
					 : "0"  (loops)              \
                   );                             \
    
	return;
}

