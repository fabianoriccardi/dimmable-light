#include <Arduino.h>
#include "dimmable_light.h"

const int N_LIGHTS = 8;

#if defined(ESP8266)
const int syncPin = D7;
#elif defined(ESP32)
const int syncPin = 23;
#elif defined(AVR)
const int syncPin = 2;
#endif

extern void (*effect)();
// The period between a call and the next one in millisecond
extern uint16_t period;
extern uint32_t lastCall;

extern DimmableLight lights[];

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
void doRandomBriPeehole();
void doRandomPushExtremeValues();
