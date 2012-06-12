
/*
 *
 * Author: Michael Traxler based on Giacomo Ortona's hadtempsens
 *
 * This program is running on the ATMEL AT90CAN128 and does the following:
 * - initialize the USART0 register allowing tx and rx of data - depending
 * on the input data received by the USART0 one can set the threshold of
 * the ADC.
 *
 * - If the ADC input voltage is larger than the value set, a digital
 * output is set from 3.3V to 0V
 */


/*
 * $Id: gas_monitor.c,v 1.1 2010-06-30 12:04:28 hadaq Exp $ $Source:
 * /home/giacomo/cvsroot/hadtempsens/tempsens2.c,v $ $Header:
 * /home/giacomo/cvsroot/hadtempsens/tempsens2.c,v 1.33 2006/09/22
 * 10:40:08 giacomo Exp $
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <util/delay.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/iocan128.h>


#include "OWIDeviceSpecific.h"
#include "OWIHighLevelFunctions.h"
#include "OWIBitFunctions.h"
#include <util/delay.h>

#define BV(x)  (1<<x)
#define BIT(x)  (1<<x)

uint8_t BUSES[5] = { OWI_PIN_2 };

#define DS1820_START_CONVERSION         0x44
#define DS1820_READ_SCRATCHPAD          0xbe
#define DS1820_WRITE_SCRATCHPAD         0x4E
#define DS1820_CONFIG_REGISTER          0x7f
#define DS1820_READ_POWER               0xb4

#define NUM_DEVICES       40


#define BAUD               7

// the number of ADC conversions done to get an average voltage value
#define ADC_CYCLES 3

// wait ADC_TIMEOUT clock cycles for a single ADC conversion
#define ADC_TIMEOUT 1000

volatile uint16_t thigh = 900;
volatile uint16_t tlow = 100;

int
break_point (int error_code, char *s)
{
  uint8_t intstate = SREG;
  int i;
  char *str;

  cli ();

  i = error_code;
  str = s;
  _delay_ms (10);

  SREG = intstate;
  return 1;
}


/*
 *USART0 must be initialized before using this function
 *for use this function, be sure that data is no longer then 8 bit
 *(single character) this function will send  8 bits from the at90can128
 */
void
USART0_Transmit (uint8_t data)
{
  /*
   * wait for empty transmit buffer
   */
  while (!(UCSR0A & (1 << UDRE0)))
    {
    }
  /*
   * put the data into the buffer
   */
  UDR0 = data;
}



uint16_t ReadTemperature(unsigned char bus, unsigned char * id)
{
    uint16_t temperature;

    // Reset, presence.

    if (OWI_DetectPresence(bus)==0)
    {
    return -999; // Error
    }

    // Match id founded earlier
    OWI_MatchRom(id, bus);
    // Send READ SCRATCHPAD command.
    OWI_SendByte(DS1820_READ_SCRATCHPAD, bus);
    // Read only two first bytes (temperature low, temperature high)
    // and place them in the 16 bit temperature variable.

    temperature = OWI_ReceiveByte(bus);
    temperature |= (OWI_ReceiveByte(bus) << 8);
    return temperature;
}


/*
 *  this function will read the IDÂŽs and temperatures of a certain number of
 * devices limited between 1 and NUM-DEVICES and will send the data through
 *  USART register
 * the function will return the number of devices on the bus
 * t is an array used for the storage of the temperatures values
 * IDS is a 2D array used for storage the sensors IDÂŽs
*/
int TempStored (uint8_t *pins) {
    uint16_t t;
    char PD;
    uint8_t IDS[NUM_DEVICES][8];
    int countDEV = 0;//number of devices
    int countDEVbus=0;//number of devices on the current bus
/*on onetemp are written the data to be sent away*/
    char onetemp[30];//27 should be enough

    PORTA |= 0x8;

/*clean one IDS array*/
    for(int a=0; a<7;a++){
        IDS[NUM_DEVICES-1][a]=0;
    }
    for(int b=0;b<1;b++) {
/*initialize the bus*/
    OWI_Init(pins[b]);
    uint8_t res=1;
    countDEV = countDEV + countDEVbus;
    countDEVbus=0;//reset number of devices on the bus

    //break_point(PD, "presence");
    if((PD=OWI_DetectPresence(pins[b]))!=0) {
//this will search the devices IdÂŽs till the max number of devices is reached or
// all the devices have been read out
        //break_point(PD, "presence");
        //break_point(countDEVbus, "Cound DEV bus");
        while(countDEVbus<NUM_DEVICES&&res!=OWI_ROM_SEARCH_FINISHED){
        if(countDEVbus==0){
            res=0;
        }
        OWI_DetectPresence(pins[b]);
        res=OWI_SearchRom(IDS[NUM_DEVICES-1], res, pins[b]);
        //break_point(res, "search ROM");
        for(int a=0;a<8;a++){
            IDS[countDEVbus][a]=IDS[NUM_DEVICES-1][a];
        }
        countDEVbus++;
        }

/*order to all the devices on the bus to perform a temp conversion*/
        OWI_DetectPresence(pins[b]);
        OWI_SendByte(OWI_ROM_SKIP, pins[b]);
        OWI_SendByte(DS1820_START_CONVERSION, pins[b]);
        while (OWI_ReadBit(pins[b])==0)
        {
        _delay_us(100);
        }

//this loop will record the IDS and the temperatures on regtemp
        for(uint8_t q=0;q<countDEVbus;q++){
        t = ReadTemperature(pins[b],IDS[q]);

        sprintf(onetemp,"%d: %02X%02X%02X%02X%02X%02X%02X%02X %.4X ",q+countDEV,IDS[q][0],IDS[q][1],IDS[q][2],IDS[q][3],IDS[q][4],IDS[q][5],IDS[q][6],IDS[q][7],t);
/*send the data*/
        //break_point(444, onetemp);
        for(int j=0;j<strlen(onetemp);j++) {
            if(onetemp[j]!=('\n'||'\0')) {
/*only non terminating characters have to be sent*/
            //break_point(443, onetemp);
            USART0_Transmit(onetemp[j]);
            }
        }
        }
    }
    }


    //break_point(1, onetemp);
    USART0_Transmit ('\n');

    return countDEV;
}



/*
 *this function initialize the USART 0 register needed to allow the at90can128
 *USART transmissions
 */
void
USART0_Init (unsigned int baud)
{
  // no tx are possible when the inizialization is going on.
  // if is possible, use TXC0 flag.

  // interruption should be disabled (cli())
  uint8_t intstate = SREG;
  cli ();
  /*
   * the TXC flag bit is set 1 when the entire frame in the Transmit
   * Shift Register has been shifted out and there are no new data
   * currently present in the transmit buffer
   */
  while (0 << TXC0)
    {
    }
  /*
   * set baud rate
   */
  UBRR0H = (uint8_t) (baud >> 8);
  UBRR0L = (uint8_t) (baud);
  /*
   * set frame format: I will set it now as 8 bits, no parity 1 stop bit
   */
  UCSR0C = (0 << UMSEL0) | (0 << UPM0) | (0 << USBS0) | (3 << UCSZ0);
  /*
   * enable transmitter and interrupt at receive
   */
  UCSR0B = (1 << TXEN0) | (1 << RXEN0) | (1 << RXCIE0);
  SREG = intstate;
}


void
Init (void)
{

  uint8_t intstate = SREG;
  cli ();

  // use port A as output
  DDRA = 0xFF;
  PORTA = 0x00;


  // use port B as input with activated pullups
  DDRB = 0x00;
  PORTB = 0xFF;

  // use port C as input with deactivated pullups
  DDRC = 0x03;
  PORTC = 0x00;

  // use port E as input with activated pullups
  DDRE = 0x00;
  PORTE = 0xFF;

  // use port F as input with ctivated pullups
  DDRF = 0x00;
  PORTF = 0xFF;

  // enable ADC and set clock prescale factor to 64 (p.280)
  ADCSRA = BIT (ADEN) | 0x06;
  // 0x80 => ADEN => enable ADC

  //break_point(BIT(ADEN) | 0x06, "hhallo");

  // low speed and manually triggered ADC conversion mode
  ADCSRB = 0;

  // deactivate digital input buffer for used ADC pins
#ifdef DISABLE_JTAG
  DIDR0 = 0xFF;
  //MCUCR |= 0x80;   // deactivate whole JTAG interface
#else
  DIDR0 = 0x0F;
#endif

  // enable only compare match interrupt on timer 1, channel A
  TIMSK1 = 0x02;

  // normal counter mode (no wave and/or PWM stuff) for timer 1, channel A
  TCCR1A = 0;
  TCCR1C = 0;

  SREG = intstate;

}               // End of Init




/*
 * USART0 must be initialized befor using this function, if data is longer
 * then 8bit, properly working is not assured
 */
uint8_t
USART0_Receive (void)
{
    uint16_t count = 0;
  /*
   * Wait for data to be received
   */
  while ( !(UCSR0A & (1 << RXC0))  && count < 5000 ) {
      count++;
      _delay_ms(1);
  }

  if(count>1000) {
      break_point (count, "count too large");
  }

  /*
   * Get and return received data from buffer
   */
  return UDR0;
}



int
USART_send_string (char *str)
{
  uint16_t j;

  for (j = 0; j < strlen ((char *) str) && j < 80; j++)
    {
      USART0_Transmit (str[j]);
    }
  USART0_Transmit ('\n');

  return 0;
}

void
toggle_pin (unsigned char pin_number)
{
  // USART_send_string("toggle pin called");

  long int i, c, d;
  _delay_ms (10);
  for (i = 0; i <= 50; i++)
    {
      _delay_ms (10);
      d = d + i * c;
      c = d + i;
    }

  // toggle FrontLED
  PINA = 0x3;


}


enum
{
  eNoError = 0,
  eWrongCANAddress = 1,     // bogus address set on hex-switches
  eCANDataSize = 2,     // bogus amount of bytes arrived
  eI2CTimeout = 4,      // IÂ²C problem
  eI2CFatalError = 8,       // IÂ²C problem
  eI2CRetryFailed = 16,     // IÂ²C problem
  eADCTimeout = 32,     // ADC conversion failed
  eCPLDAcknowledgeTimeout = 64  // CPLD communication problem
} ErrorCodes;

uint16_t Voltages[8];       // current supply voltages of the board

int
GetVoltage (unsigned int channel_nr)
{
  uint16_t Timeout;
  uint16_t Value;
  uint16_t iStep;
  int16_t TotalValue;

#ifndef DISABLE_JTAG
  // we need the JTAG interface, thus do not measure
  // the upper four ADC channels (used for JTAG, too)
  if (channel_nr > 3)
    {
      Voltages[channel_nr] = 0;
      return eNoError;
    }
#endif

  // use 3.3V supply voltage as reference voltage
  // and select the voltage to be measured
  ADMUX = 0x40 | (unsigned char) channel_nr;
  //ADMUX = 0x40 | 0x1B;

  TotalValue = 0;
  for (iStep = 0; iStep < ADC_CYCLES; iStep++)
    {
      // start a new single conversion
      ADCSRA |= BIT (ADSC);

      // wait until conversion is complete
      Timeout = ADC_TIMEOUT;
      while (!(ADCSRA & BIT (ADIF)) && (--Timeout > 0));

      // clear interrupt flag from former conversion
      ADCSRA |= BIT (ADIF);

      if (Timeout == 0)
    {
      return eADCTimeout;
    }

      // one must read ADCL before ADCH!
      Value = ADCL;
      Value |= (unsigned int) (ADCH & 0x03) << 8;

      //break_point(Value, "value");

      TotalValue += Value;
    }

  TotalValue = TotalValue / 3;

  // save results
  Voltages[channel_nr] = (uint16_t) TotalValue;

  return eNoError;
}


enum states {idle, below_low_thres, in_between , above_high_thres };
uint8_t cur_state;

int
main (void)
{
  /*
   * connected brate is the serial transmission baud rate regtemp is a
   * usercommand is the command that defines what action to perform
   */
  //
  // the definitions
  // of signed int


  uint8_t i;
  uint16_t res;
  char str[80];

  cur_state = idle;


  USART0_Init (BAUD);

  Init ();

  _delay_ms (10);

  USART_send_string ("please give command");

  _delay_ms (10);

  cli();

//  break_point (1, "before enable interrupt");  high

  // enable interrupts
  sei();

//  break_point (1, "after enable interrupt");

  res = GetVoltage (0);

  PORTA = PORTA | (0x2); // turn on lamp

  if(Voltages[0] < tlow) {
      cur_state = below_low_thres;
      //PORTA = PORTA | (0x1); // turn on relais
      PORTA = PORTA & (~0x1); // turn off relais
  }
  else if(Voltages[0] > thigh) {
      cur_state = above_high_thres;
      //PORTA = PORTA | (0x1); // turn on relais
      PORTA = PORTA & (~0x1); // turn off relais
  }
  else {
      cur_state = in_between;
      PORTA = PORTA | (0x1); // turn on relais
  }

  //sprintf(str, "cur_state: %d", cur_state);
  //break_point (res, str);

  while (1) {
      res = GetVoltage (0);

      /*
      if(res != eNoError) {
      Voltages[0]=1222;
      }
      */

      switch (cur_state) {
      case in_between:
      {
          if(Voltages[0] > thigh) {
          // turn off relais
          //break_point (res, "above threshold");
          PORTA = PORTA & (~0x1);
          cur_state = above_high_thres;
          }
          if(Voltages[0] < tlow) {
          // turn off relais
          //break_point (res, "above threshold");
          PORTA = PORTA & (~0x1);
          cur_state = below_low_thres;
          }
      }
      break;
      case below_low_thres:
      {
          _delay_ms(100);
          _delay_ms(100);
          PINA = 0x2; // toggle LED

          if(Voltages[0] > tlow +2) {
          // turn on relais
          PORTA = PORTA | (0x3);
          cur_state = in_between;
          }
      }
      break;
      case above_high_thres:
      {

          _delay_ms(100);
          _delay_ms(100);
          PINA = 0x2; // toggle LED

          //break_point (res, "above threshold");
          if(Voltages[0] < thigh -2) {
          // turn on relais and lamp
          PORTA = PORTA | (0x3);
          cur_state = in_between;
          }
      }
      break;
      default:
      {
          cur_state = below_low_thres;

      }
      }

      i=i*2;
  }// end of while-loop

} // end of main




ISR(USART0_RX_vect) {

  uint8_t i;
  char tmp_str[100];
  uint8_t res = 0, res2 = 0;
  uint8_t usercommand;


  cli();


  //USART_send_string ("entered USART0_RX_vect");

  strcpy(tmp_str, "no command received from ETRAX.");

  usercommand = USART0_Receive();


  switch (usercommand)
  {
      case 'm':
      {
          //sprintf(tmp_str, "command received : %c", usercommand);
          //USART_send_string (tmp_str);


      for(i=0; i<=1;i++) {
          _delay_ms (10);
      }
      res = GetVoltage (0);
      res2 = GetVoltage (1);
      if (res != 0 || res2 != 0)
        {
        sprintf (tmp_str, "error in GetVoltage: res: %d", res);
        USART_send_string (tmp_str);
          }
      sprintf (tmp_str, "voltage measurement: %d", Voltages[0]);
      USART_send_string (tmp_str);

      sprintf (tmp_str, "current thresholds set in ATMEL: high: %d, low: %d", thigh, tlow);
      USART_send_string (tmp_str);

      sprintf (tmp_str, "state of relais: %d", (PORTA & 0x1));
      USART_send_string (tmp_str);

      TempStored(BUSES);

      _delay_ms(100);
      }
      break;
      case '0':
      {
          //sprintf(tmp_str, "command received : %c", usercommand);
          //USART_send_string (tmp_str);


      for(i=0; i<=1;i++) {
          _delay_ms (10);
      }
      res = GetVoltage (0);
      //res2 = GetVoltage (1);
      if (res != 0 || res2 != 0)
        {
        sprintf (tmp_str, "error in GetVoltage: res: %d", res);
        USART_send_string (tmp_str);
          }
      sprintf (tmp_str, "voltage measurement: %d", Voltages[0]);
      USART_send_string (tmp_str);

      _delay_ms(100);
      }
      break;
      case '1':
      {
          //sprintf(tmp_str, "command received : %c", usercommand);
          //USART_send_string (tmp_str);


      for(i=0; i<=1;i++) {
          _delay_ms (10);
      }
      res = GetVoltage (0);
      res2 = GetVoltage (1);
      if (res != 0 || res2 != 0)
        {
        sprintf (tmp_str, "error in GetVoltage: res: %d", res);
        USART_send_string (tmp_str);
          }
      sprintf (tmp_str, "voltage measurement: %d", Voltages[1]);
      USART_send_string (tmp_str);

      _delay_ms(100);
      }
      break;
      case 'b':
      {
      /*
       * set thresholds
       */

      char str1[10];
      char str2[10];

      //sprintf(tmp_str, "command received : %c", usercommand);
      //USART_send_string (tmp_str);


      for (i = 0; i < 4; i++)
      {
          str1[i] = USART0_Receive ();
      }
      str1[i] = 0;
      thigh = atoi(str1);

      for (i = 0; i < 4; i++)
      {
          str2[i] = USART0_Receive ();
      }
      str2[i] = 0;
      tlow = atoi(str2);

      thigh = atoi(str1);

      sprintf (tmp_str, "received thresholds: high: %d, low: %d, str1: %s, str2: %s , i: %d", thigh, tlow, str1, str2, i);
      USART_send_string (tmp_str);


      sprintf (tmp_str, "new thresholds set in ATMEL: high: %d, low: %d", thigh, tlow);
      USART_send_string (tmp_str);


      }
      break;        // end of set limits

      case 'c':
      {
      sprintf(tmp_str, "Error, unknown command received : %c", usercommand);
      USART_send_string (tmp_str);
      }
      break;

      case 's':          //status
      {
      char tmp_str[120];
      char *tmp_str2;
      switch (cur_state) {
          case in_between:
          {
          tmp_str2 = "Pressure is OK";
          }
          break;
          default:
          {
          tmp_str2 = "Pressure not OK!" ;
          }
      }

      sprintf(tmp_str, "Status: %s", tmp_str2);
      USART_send_string (tmp_str);
      }
      break;
      case 't': //temp
      {
      TempStored(BUSES);
      }
      break;
      default:
      {
      sprintf(tmp_str, "Error, unknown command received : %c", usercommand);
      USART_send_string (tmp_str);
      }

  }         // end of switch




  // break_point (1, "end of main");

  sei();

}  // end of ISR




