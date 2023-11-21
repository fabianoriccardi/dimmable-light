// Select ONLY ONE between these 2 options
#define RAW_VALUES
//#define LINEARIZED_VALUES

#if defined(RAW_VALUES)
#include <dimmable_light.h>
#elif defined(LINEARIZED_VALUES)
#include <dimmable_light_linearized.h>
#endif

#include <stdint.h>

const int N_LIGHTS = 8;

#if defined(ESP8266)
const int syncPin = 13;
#elif defined(ESP32)
const int syncPin = 23;
#elif defined(AVR)
const int syncPin = 2;
#elif defined(ARDUINO_ARCH_SAMD)
const int syncPin = 2;
#elif (defined(ARDUINO_ARCH_RP2040) && !defined(ARDUINO_ARCH_MBED))
const int syncPin = 2;
#endif

#if defined(RAW_VALUES)
extern DimmableLight lights[];
#elif defined(LINEARIZED_VALUES)
extern DimmableLightLinearized lights[];
#endif

extern void (*effect)();
// The period between a call and the next one in millisecond
extern uint16_t period;
extern uint32_t lastCall;

void doEqual();
void doEqualOnOff();
void doDimSpecificStep();
void doRangeLimit();
void doNearValues();
void doDimMixed();
void doDimSweepEqual();
void doOnOffSweep();
void doInvertedDim();
void doCircularSwipe();
void doRandomBri();
void doRandomBriPeephole();
void doRandomPushExtremeValues();
void doCircularSwipeRegular();

void offAllLights();

void initLights();
