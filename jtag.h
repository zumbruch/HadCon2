/*
 * jtag.h
 *
 *  Created on: Jul 7, 2011
 *      Author: Peter Zumbruch, GSI, P.Zumbruch@gsi.de
 */

#ifndef API_JTAG_H_
#define API_JTAG_H_

#ifndef ALLOW_DISABLE_JTAG
   /* we need the JTAG interface, thus do not measure
    *the upper four ADC channels (used for JTAG, too)
    */
extern const uint8_t disableJTAG_flag;
#else
extern uint8_t disableJTAG_flag;
#endif

void disableJTAG(uint8_t disable);
void modifyJTAG(struct uartStruct *ptr_uartStruct);

#endif /* API_JTAG_H_ */
