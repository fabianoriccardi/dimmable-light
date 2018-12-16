#include <Arduino.h>
#include "dimmable_light.h"

#define N_LIGHTS 8

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
