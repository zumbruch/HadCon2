#ifndef API_DEFINE__H
#define API_DEFINE__H
/*the define.h header file contents all define variable*/

#define CODE_VERSION "4.6"

#ifndef HADCON_VERSION
#define HADCON_VERSION 2
#endif

/*string ending sign: '0':48 "\0"==0? => yes, string ending=="\0"? => yes */
#define STRING_END 0

/* threshold for CommunicationError, below this value the global variable message
 * is used instead of the constand alternative text */

#define F_CPU 10000000UL

#define MAX_LENGTH_KEYWORD   4
/* maximum length of their respective parameters of the strings via UART */

/*  Formula for calculating UBBR0:
 *  UBBR0= ((F_CPU/(16*baud))<<U2X0)-1
 */

/*currently selected baud rate for the serial communication: baud rate = 115200 */
#define BAUD       10

/*size of array for the variable */
#define BUFFER_SIZE 140

/*maximum value for 11 bit identifier for CAN standard 2.0A*/
#define MAX_ELEVEN_BIT  2047

/*maximum input in the ring buffer*/
#define MAX_INPUT  30

/*start Pointer to receive  CAN Data via CAN-Bus*/
#define MAX_BYTE_CAN_DATA  8

/*maximum arguments of received CAN Data*/
#define MAX_CAN_DATA  11

/*maximum length of received CAN Data*/
#define MAX_LENGTH_CAN_DATA  41

/*maximum length of command with CAN standard 2.0A*/
#define MAX_LENGTH_COMMAND  48

/*maximum length of various error*/
#define MAX_LENGTH_ERROR  50

/*maximum number of parameters in string received via UART */
#define MAX_PARAMETER          13

/* maximum length of their respective parameters of the strings via UART */
#define MAX_LENGTH_PARAMETER   25

/*size of subscribe ID and Mask*/
#define MAX_LENGTH_SUBSCRIBE  15

/*undefined Mailbox */
#define NOMOB   0xFF

/*timeout for serial and CAN communication*/
#define TIMEOUT_UART 5000 /*Timeout for UART-communication*/
#define CAN_TIMEOUT_US 5000 /*Timeout for CAN-communication*/

/*Timeout error*/
#define TIME_S__ERROR "timeout for serial communication"

/*currently selected default baud rate for the CAN communication: baud rate = 250kps
 *and other selected baud rate for the CAN communication: baud rate = 100 ... 1000kps*/
#define ONETHOUSAND_KBPS          1000000UL
#define FIVEHUNDERT_KBPS           500000UL
#define TWOHUNDERTFIFTY_KBPS       250000UL
#define TWOHUNDERT_KBPS            200000UL
#define ONEHUNDERTTWENTYFIVE_KBPS  125000UL
#define ONEHUNDERT_KBPS            100000UL

/* confirmation of sending on the bus */
#define  READY     "command will be carried out"
#define  NOREADY   "command will not be carried out"
/*define specific for gas_temp*/
#define  THIGH    900
#define  TLOW     100

#warning HADCON devices are around, which exceed those 20 devices, formerly to be 60
#define OWI_MAX_NUM_DEVICES       20

#define TRUE    1
#define FALSE   0

#define RESULT_OK 0
#define RESULT_FAILURE 1

#define OWI_FAMILY_DS2450_ADC                 0x20
#define OWI_FAMILY_DS18B20_TEMP               0x28
#define OWI_FAMILY_DS18S20_TEMP               0x10
#define OWI_FAMILY_DS2405_SIMPLE_SWITCH       0x05
#define OWI_FAMILY_DS2413_DUAL_SWITCH         0x3A
#define OWI_FAMILY_DS2408_OCTAL_SWITCH        0x29

/*OWI_MAX_NUM_PIN_BUS indicates the number of active pins for 1-wire bus on the HadCon board */
#if ( HADCON_VERSION == 1)
#define OWI_MAX_NUM_PIN_BUS 8
#elif ( HADCON_VERSION == 2)
#define OWI_MAX_NUM_PIN_BUS 6
#else
#define OWI_MAX_NUM_PIN_BUS 0
#endif

#define BV(x)  (1<<x)
#define BIT(x)  (1<<x)

/* disabling JTAG allowed */
#define ALLOW_DISABLE_JTAG

#define OWI_ADC_DS2450_MAX_RESOLUTION 16

#define min(a,b) ( ((a)<(b))?(a):(b) )

#endif

