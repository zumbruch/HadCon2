/*
 * relay.h
 *
 *  Created on: Jul 16, 2010
 *      Author: epics
 */

#ifndef RELAY_H_
#define RELAY_H_

extern volatile uint8_t relayThresholdEnable_flag;
extern volatile uint8_t relayThresholdCurrentState;
extern volatile uint8_t relayThresholdNewState;

extern volatile  int8_t relayThresholdExternHighChannel;
extern volatile  int8_t relayThresholdExternLowChannel;
extern volatile uint8_t relayThresholdInputChannelMask; /*mask of channels of atmel ADC to get values from*/
extern volatile uint8_t relayThresholdExtThresholdsMask; /*mask of ext channels of atmel ADC to get threshold values from*/
extern volatile uint8_t relayThresholdOutputPin; /*pin on PORTA to switch value for relay*/
extern volatile uint8_t relayThresholdLedPin;
extern volatile uint8_t relayThresholdReportCurrentStatusEnable_flag;

extern volatile uint8_t relayThresholds_valid[16];
extern volatile uint8_t relayThresholdAllThresholdsValid;
extern volatile uint8_t relayThresholdRelayValue;
extern volatile uint8_t relayThresholdRelaySetValue;
extern volatile uint8_t relayThresholdOutOfBoundsLock_flag;
extern volatile uint8_t relayThresholdsInBoundsPolarity;
extern volatile uint8_t relayThresholdsOutOfBoundsPolarity;
extern volatile uint8_t relayThresholdsUndefinedIndifferentPolarity;
extern volatile uint8_t relayThresholdsExternalPolarityPinPos;
extern volatile uint8_t relayThresholdsExternalPolarityPort;
extern volatile uint8_t relayThresholdsExternalPolarity;
extern volatile uint8_t relayThresholdInvertPolarity_flag;
extern volatile uint8_t relayThresholdOutOfBoundsStateLock_flag;
extern volatile uint8_t relayThresholdUseIndividualThresholds;
extern volatile uint16_t relayThresholdHigh[8]; /*the maximum gas pressure*/
extern volatile uint16_t relayThresholdLow[8];  /*the minimum gas pressure*/


enum relayThresholdEnum
{
      relayThreshold_LOW=0,
      relayThreshold_HIGH,
      relayThreshold_MAXIMUM_INDEX
};

enum relayThresholdInputSourceEnum
{
	relayThresholdInputSource_RADC=0,
	relayThresholdInputSource_OWI_MDC_PRESSURE_1,
	relayThresholdInputSource_OWI_MDC_PRESSURE_2,
	relayThresholdInputSource_MAXIMUM_INDEX
};

extern const char* const relayThresholdStateStrings[] PROGMEM;
enum relayThresholdStates
{
   relayThresholdState_INIT,
   relayThresholdState_IDLE,
   relayThresholdState_INBETWEEN_BOUNDS,
   relayThresholdState_OUT_OF_BOUNDS,
   relayThresholdState_UNDEFINED,
   relayThresholdState_MAXIMUM_NUMBER
};

extern const char* const relayThresholdCommandKeywords[] PROGMEM;
enum relayThresholdCommandKeyNumber
{
      relayThresholdCommandKeyNumber_CURRENT_STATE = 0,
      relayThresholdCommandKeyNumber_CURRENT_VALUES,
      relayThresholdCommandKeyNumber_THR_HIGH,
      relayThresholdCommandKeyNumber_THR_LOW,
      relayThresholdCommandKeyNumber_ENABLE,
      relayThresholdCommandKeyNumber_INPUT_CHANNEL_MASK,
      relayThresholdCommandKeyNumber_OUTPUT_PIN,
      relayThresholdCommandKeyNumber_LED_PIN,
      relayThresholdCommandKeyNumber_REPORT_STATE_ENABLE,
      relayThresholdCommandKeyNumber_RELAY_VALUE,
      relayThresholdCommandKeyNumber_THRESHOLDS,
      relayThresholdCommandKeyNumber_STATUS,
      relayThresholdCommandKeyNumber_OUT_OF_BOUNDS_LOCK,
      relayThresholdCommandKeyNumber_IN_BOUNDS_POLARITY,
      relayThresholdCommandKeyNumber_OUT_OF_BOUNDS_POLARITY,
      relayThresholdCommandKeyNumber_INVERT_POLARITY_LOGIC,
      relayThresholdCommandKeyNumber_EXTERNAL_IN_BOUNDS_POLARITY,
      relayThresholdCommandKeyNumber_EXTERNAL_IN_BOUNDS_PIN_POS,
      relayThresholdCommandKeyNumber_EXTERNAL_IN_BOUNDS_PORT,
      relayThresholdCommandKeyNumber_POLARITY,
      relayThresholdCommandKeyNumber_POLARITY_REINIT,
      relayThresholdCommandKeyNumber_REINIT,
      relayThresholdCommandKeyNumber_VALIDATE_THRESHOLDS,
      relayThresholdCommandKeyNumber_INPUT_CHANNEL_MASK_ADD,
      relayThresholdCommandKeyNumber_INPUT_CHANNEL_MASK_DEL,
      relayThresholdCommandKeyNumber_EXTERN_THR_HIGH_CHANNEL,
      relayThresholdCommandKeyNumber_EXTERN_THR_LOW_CHANNEL,
      relayThresholdCommandKeyNumber_USE_INDIVIDUAL_THRESHOLDS,
      relayThresholdCommandKeyNumber_EXTERN_THRESHOLD_CHANNEL_MASK,
      relayThresholdCommandKeyNumber_MAXIMUM_NUMBER

};

void relayThreshold(struct uartStruct *ptr_uartStruct);
void relayThresholdMiscSubCommands(struct uartStruct *ptr_uartStruct, int16_t index );

uint8_t relayThresholdInit(void);
uint8_t relayThresholdInBoundPolarityInit(void);
void relayThresholdDetermineStateAndTriggerRelay(uint8_t index);
uint8_t relayThresholdDetermineState( uint8_t currentState, uint32_t currentValue, uint32_t high, uint32_t low );
uint8_t relayThresholdDetermineNewRelaySetValue( uint8_t newState );
void relayThresholdsReportCurrentStatus(void);

uint32_t relayThresholdsGetCurrentValue(uint8_t channel);
uint8_t relayThresholdGetExternHighChannel(void);
uint8_t relayThresholdSetExternHighChannel(int8_t value);
uint8_t relayThresholdGetExternHighChannel(void);
uint8_t relayThresholdSetExternHighChannel(int8_t);
uint8_t relayThresholdGetExternLowChannel(void);
uint8_t relayThresholdSetExternLowChannel(int8_t value);
uint8_t relayThresholdGetUseIndividualThresholds(void);
uint8_t relayThresholdSetUseIndividualThresholds(uint8_t value);
uint8_t relayThresholdSetExternThresholdChannel(int8_t channel, volatile int8_t *threshold );

uint8_t relayThresholdsSetLowThreshold(uint16_t value, int8_t channel);
uint8_t relayThresholdsSetHighThreshold(uint16_t value, int8_t channel);
uint8_t relayThresholdsSetEnableFlag(uint8_t value);
uint8_t relayThresholdsSetInputChannelMask(uint8_t value);
uint8_t relayThresholdsAddChannelToInputChannelMask(uint8_t value);
uint8_t relayThresholdsDeleteChannelFromInputChannelMask(uint8_t value);
uint8_t relayThresholdsSetOutputPin(uint8_t value);
uint8_t relayThresholdsSetLedPin(uint8_t value);
uint8_t relayThresholdsSetReportCurrentStatusEnableFlag(uint8_t value);
uint8_t relayThresholdsSetRelayValue(uint8_t value);
uint8_t relayThresholdsSetRelaySetValue(uint8_t value);
uint8_t relayThresholdsSetOutOfBoundsLockFlag(uint8_t value);

uint8_t relayThresholdsSetExternalPolarityPinPos(uint8_t value);
uint8_t relayThresholdsSetInvertPolarityFlag(uint8_t value);
uint8_t relayThresholdsSetExternalPolarityPort(uint8_t value);

uint8_t relayThresholdsSetAllRelevantThresholdsValid(uint8_t value);
uint8_t relayThresholdsCheckAllRelevantThresholdsValid(void);
uint8_t relayThresholdsGetAllThresholdsValid(void);

uint32_t relayThresholdsGetHighThreshold(uint8_t channel);
uint32_t relayThresholdsGetLowThreshold(uint8_t channel);
uint32_t relayThresholdsGetChannelValue(uint8_t channel, volatile uint16_t threshold[]);
uint8_t relayThresholdGetCurrentChannelThreshold(uint8_t channel, uint16_t *low, uint16_t *high);

uint8_t relayThresholdsGetEnableFlag(void);
uint8_t relayThresholdsGetInputChannelMask(void);
uint8_t relayThresholdsGetOutputPin(void);
uint8_t relayThresholdsGetLedPin(void);
uint8_t relayThresholdsGetCurrentState(void);
uint8_t relayThresholdsGetReportStateEnableFlag(void);
uint8_t relayThresholdsGetRelayValue(void);
uint8_t relayThresholdsGetRelaySetValue(void);
uint8_t relayThresholdsGetOutOfBoundsLockFlag(void);
uint8_t relayThresholdsGetExtThresholdsMask(void);

uint8_t relayThresholdsGetInBoundsPolarity(void);
uint8_t relayThresholdsGetOutOfBoundsPolarity(void);
uint8_t relayThresholdsGetExternalPolarityPinPos(void);
uint8_t relayThresholdsGetExternalPolarityPort(void);
uint8_t relayThresholdsGetExternalPolarity(void);
uint8_t relayThresholdsGetInvertPolarityFlag(void);

uint8_t relayThresholdsSetChannelThreshold(uint16_t value, int8_t channel, volatile uint16_t threshold[], uint8_t offset);

extern void (*relayThresholdDetermineStateAndTriggerRelay_p)(uint8_t);

#endif /* RELAY_H_ */
