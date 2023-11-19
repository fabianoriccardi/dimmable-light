#include "effect.h"

void (*effect)() = nullptr;
// The period between a call and the next one in millisecond
uint16_t period = 0;

uint32_t lastCall = 0;

// A complicated way to define objects to allow the testing of different
// classes on different microcontrollers
#if defined(RAW_VALUES)
extern DimmableLight
#elif defined(LINEARIZED_VALUES)
extern DimmableLightLinearized
#endif

#if defined(ESP8266)
  // Remember that GPIO0 (D3) and GPIO2 (D4) are "critical" since they control the boot phase.
  // I have to disconnect them to make it boot when using Krida's dimmers. If you want to
  // use those pins without disconnecting and connecting the wires, you need additional circuitry to
  // "protect" them.
    lights[N_LIGHTS] = { { 5 }, { 4 }, { 14 }, { 12 }, { 15 }, { 16 }, { 0 }, { 2 } };
#elif defined(ESP32)
  lights[N_LIGHTS] = { { 4 }, { 16 }, { 17 }, { 5 }, { 18 }, { 19 }, { 21 }, { 22 } };
#elif defined(AVR)  // Arduino
lights[N_LIGHTS] = { { 3 }, { 4 }, { 5 }, { 6 }, { 7 }, { 8 }, { 9 }, { 10 } };
#elif defined(ARDUINO_ARCH_SAMD)
lights[N_LIGHTS] = { { 3 }, { 4 }, { 5 }, { 6 }, { 7 }, { 8 }, { 9 }, { 10 } };
#elif (defined(ARDUINO_ARCH_RP2040) && !defined(ARDUINO_ARCH_MBED))
lights[N_LIGHTS] = { { 3 }, { 4 }, { 5 }, { 6 }, { 7 }, { 8 }, { 9 }, { 10 } };
#endif

/**
 * Set particular values of brightness to every light.
 */
void doEqual() {
  const unsigned int period = 3000;
  static const uint8_t briLevels[] = { 0, 1, 2, 50, 100, 150, 254, 255 };
  static uint8_t brightnessStep = 0;
  Serial.println(String("Dimming at: ") + briLevels[brightnessStep] + "/255");
  for (int i = 0; i < N_LIGHTS; i++) { lights[i].setBrightness(briLevels[brightnessStep]); }

  brightnessStep++;
  if (brightnessStep == sizeof(briLevels) / sizeof(briLevels[0])) { brightnessStep = 0; }

  ::period = period;
  lastCall = millis();
  effect = doEqual;
}

/**
 * Turn on and off simultaneously all the bulbs.
 */
void doEqualOnOff() {
  const unsigned int period = 3000;
  static int briLevels[] = { 0, 255 };
  static uint8_t brightnessStep = 0;
  Serial.println(String("Dimming at: ") + briLevels[brightnessStep] + "/255");
  for (int i = 0; i < N_LIGHTS; i++) { lights[i].setBrightness(briLevels[brightnessStep]); }

  brightnessStep++;
  if (brightnessStep == sizeof(briLevels) / sizeof(briLevels[0])) { brightnessStep = 0; }

  ::period = period;
  lastCall = millis();
  effect = doEqualOnOff;
}

/**
 * Set brightness to specific values.
 */
void doDimSpecificStep(void) {
  const unsigned int period = 3000;
  static const uint8_t briLevels1[] = { 40, 200 };
  static const uint8_t briLevels2[] = { 60, 160 };
  static const uint8_t briLevels3[] = { 80, 130 };
  static uint8_t brightnessStep = 0;
  Serial.println(String("Dimming at: ") + briLevels1[brightnessStep] + " and "
                 + briLevels2[brightnessStep] + " and " + briLevels3[brightnessStep] + " /255");
  lights[1].setBrightness(briLevels1[brightnessStep]);
  lights[2].setBrightness(briLevels2[brightnessStep]);
  lights[3].setBrightness(briLevels3[brightnessStep]);

  brightnessStep++;
  if (brightnessStep == sizeof(briLevels1) / sizeof(briLevels1[0])) { brightnessStep = 0; }

  ::period = period;
  lastCall = millis();
  effect = doDimSpecificStep;
}

/**
 * Test a mixture between on, off and middle brightness.
 */
void doRangeLimit(void) {
  const unsigned int period = 5000;
  static const uint8_t briLevels1[] = { 0, 255 };
  static const uint8_t briLevels2[] = { 255, 0 };
  static const uint8_t briLevels3[] = { 100, 100 };
  static uint8_t brightnessStep = 0;
  Serial.println(String("Dimming at: ") + briLevels1[brightnessStep] + " and "
                 + briLevels2[brightnessStep] + " and " + briLevels3[brightnessStep] + " /255");
  lights[1].setBrightness(briLevels1[brightnessStep]);
  lights[2].setBrightness(briLevels2[brightnessStep]);
  lights[3].setBrightness(briLevels3[brightnessStep]);

  brightnessStep++;
  if (brightnessStep == sizeof(briLevels1) / sizeof(briLevels1[0])) { brightnessStep = 0; }

  ::period = period;
  lastCall = millis();
  effect = doRangeLimit;
}

/**
 * Test your eyes sensitivity by switching between near values. Will you see any difference?
 */
void doNearValues(void) {
  const unsigned int period = 3000;
  static const uint8_t avg = 80;
  static const uint8_t diff = 2;
  static const uint8_t briLevelsLamp1[] = { avg - diff, avg };
  static const uint8_t briLevelsLamp2[] = { avg, avg + diff };
  static const uint8_t briLevelsLamp3[] = { avg + diff, avg - diff };
  static uint8_t brightnessStep = 0;
  Serial.println(String("Dimming at: ") + briLevelsLamp1[brightnessStep] + " and "
                 + briLevelsLamp2[brightnessStep] + " and " + briLevelsLamp3[brightnessStep] + " /255");
  lights[1].setBrightness(briLevelsLamp1[brightnessStep]);
  lights[2].setBrightness(briLevelsLamp2[brightnessStep]);
  lights[3].setBrightness(briLevelsLamp3[brightnessStep]);

  brightnessStep++;
  if (brightnessStep == sizeof(briLevelsLamp1) / sizeof(briLevelsLamp1[0])) { brightnessStep = 0; }

  ::period = period;
  lastCall = millis();
  effect = doNearValues;
}

/**
 * The 1st the 5th are turned off, the 3rd is fixed to half brightness, and the 2nd and 4th sweep
 * in the opposite direction w.r.t. each other.
 */
void doDimMixed(void) {
  const unsigned int period = 50;
  static uint8_t brightnessStep = 1;
  static bool up = true;

  lights[1].setBrightness(brightnessStep);
  int b2 = 105;
  lights[2].setBrightness(b2);
  int b3 = -((int)brightnessStep - 255);
  lights[3].setBrightness(b3);
  Serial.println(String("Dimming at: ") + brightnessStep + " " + b2 + " " + b3 + "/255");

  if (brightnessStep == 255 && up) {
    up = false;
  } else if (brightnessStep == 0 && !up) {
    up = true;
  } else {
    if (up) {
      brightnessStep++;
    } else {
      brightnessStep--;
    }
    // Jump some steps...
    //    if(brightnessStep==6){
    //      brightnessStep=249;
    //    }
  }

  ::period = period;
  lastCall = millis();
  effect = doDimMixed;
}

/**
 * All the lights simultaneously fade in and out.
 */
void doDimSweepEqual(void) {
  const unsigned int period = 50;
  static uint8_t brightnessStep = 1;
  static bool up = true;
  for (int i = 0; i < N_LIGHTS; i++) { lights[i].setBrightness(brightnessStep); }
  Serial.println(String("Dimming at: ") + brightnessStep + "/255");

  if (brightnessStep == 255 && up) {
    up = false;
  } else if (brightnessStep == 0 && !up) {
    up = true;
  } else {
    if (up) {
      brightnessStep++;
    } else {
      brightnessStep--;
    }
  }

  ::period = period;
  lastCall = millis();
  effect = doDimSweepEqual;
}

void doOnOffSweep() {
  const unsigned int period = 700;
  static int16_t step = 0;

  for (int i = 0; i < N_LIGHTS; i++) {
    if (step == i) {
      lights[i].setBrightness(255);
    } else {
      lights[i].setBrightness(0);
    }
  }

  step++;
  if (step == N_LIGHTS) { step = 0; }

  ::period = period;
  lastCall = millis();
  effect = doOnOffSweep;
}

/**
 * The group formed by even bulbs sweep in the opposite direction w.r.t. the odd group.
 */
void doInvertedDim(void) {
  const unsigned int period = 50;
  static uint8_t brightnessStep = 1;
  static bool up = true;
  int oppositeBrightness = -((int)brightnessStep - 255);

  Serial.println(String("Dimming at: ") + brightnessStep + " " + oppositeBrightness + "/255");
  for (int i = 0; i < N_LIGHTS; i++) {
    if (i % 2 == 0) {
      lights[i].setBrightness(brightnessStep);
    } else {
      lights[i].setBrightness(oppositeBrightness);
    }
  }

  if (brightnessStep == 255 && up) {
    up = false;
  } else if (brightnessStep == 0 && !up) {
    up = true;
  } else {
    if (up) {
      brightnessStep++;
    } else {
      brightnessStep--;
    }
  }

  ::period = period;
  lastCall = millis();
  effect = doInvertedDim;
}

/**
 * Return the module of a non-negative number (optimized).
 */
unsigned int module(unsigned int value, unsigned int max) {
  if (value < max) { return value; }
  return value % max;
}

/**
 * Given a number in range [0; 512), return a triangular function [0;255], if value is not in this
 * range, return 0.
 */
uint8_t triangularFunction(uint16_t value) {
  int simmetricValue = 0;
  if (value <= 255) { simmetricValue = value; }
  if (value >= 256 && value <= 511) { simmetricValue = -value + 511; }
  return simmetricValue;
}

/**
 * Given a number in range [0; 512), return a "pow-ed" triangular function [0;255].
 */
uint8_t conversionPow(uint16_t value) {
  int simmetricValue = 0;
  if (value >= 256 && value <= 511) { simmetricValue = -value + 511; }
  if (value <= 255) { simmetricValue = value; }

  if (simmetricValue < 150) { return 0; }
  int y = pow(simmetricValue - 150, 1.2);
  if (y > 255) { return 255; }
  return y;
}

/**
 * Turn on the light with (255/nLights) steps offset between consecutive lights.
 */
void doCircularSwipe(void) {
  const unsigned int period = 50;

  static uint16_t brightnessStep = 255;

  // Alternatively, you can use the function conversionPow(..) instead conversion(..)
  for (int i = 0; i < N_LIGHTS; i++) {
    int brightness = triangularFunction(module(brightnessStep + 32 * i, 512));
    lights[i].setBrightness(brightness);
  }

  brightnessStep++;
  if (brightnessStep == 512) { brightnessStep = 0; }

  ::period = period;
  lastCall = millis();
  effect = doCircularSwipe;
}

void doRandomBri() {
  const unsigned int period = 700;

  for (int i = 0; i < N_LIGHTS; i++) {
    int bri = random(0, 256);
    lights[i].setBrightness(bri);
  }

  ::period = period;
  lastCall = millis();
  effect = doRandomBri;
}

/**
 * The variance of random number is restricted around the mean value step after step
 */
void doRandomBriPeephole() {
  const unsigned int period = 700;
  const uint16_t briStep = 10;
  const uint16_t totStep = 16;

  static uint16_t iteration = 0;

  for (int i = 0; i < N_LIGHTS; i++) {
    int bri;
    // The last 2 step are set to the same brightness
    if (iteration >= totStep - 3) {
      bri = 127;
    } else {
      bri = random(0 + briStep * iteration, 256 - briStep * iteration);
    }
    Serial.print(String(bri) + " ");
    lights[i].setBrightness(bri);
  }
  Serial.println();

  iteration++;
  if (iteration == totStep) { iteration = 0; }

  ::period = period;
  lastCall = millis();
  effect = doRandomBriPeephole;
}

/**
 * The variance of random number is restricted around the mean value step after step
 */
void doRandomPushExtremeValues() {
  const unsigned int period = 1000;
  const uint16_t briStep = 10;

  for (int i = 0; i < N_LIGHTS; i++) {
    int bri = random(0, briStep * 2 + 1);
    if (bri < briStep) {
      bri = bri;
    } else {
      bri = 255 - (briStep * 2 - bri);
    }
    Serial.print(String(bri) + " ");
    lights[i].setBrightness(bri);
  }
  Serial.println();

  ::period = period;
  lastCall = millis();
  effect = doRandomPushExtremeValues;
}

/**
 * Perform a brightness sweep all over the lights with a delay between each light of DELAY steps.
 * This effect is perfectly smooth and simmetric w.r.t. doCircularSwipe().
 */
void doCircularSwipeRegular(void) {
  const unsigned int period = 40;
  const int HALF_PERIOD = 255;
  const int DELAY = 96;

  static int brightnessStep = 0;

  for (int i = 0; i < N_LIGHTS; i++) {
    unsigned int x = brightnessStep - DELAY * i < 0 ? 0 : brightnessStep - DELAY * i;
    unsigned int brightness = triangularFunction(module(x, DELAY * N_LIGHTS));
    lights[i].setBrightness(brightness);
  }

  brightnessStep++;

  // Avoid any long-term overflow
  if (brightnessStep >= HALF_PERIOD + 2 * DELAY * N_LIGHTS) {
    brightnessStep = HALF_PERIOD + DELAY * N_LIGHTS;
  }

  ::period = period;
  lastCall = millis();
  effect = doCircularSwipeRegular;
}

void offAllLights() {
  for (int i = 0; i < N_LIGHTS; i++) { lights[i].setBrightness(0); }
}

void initLights() {
  Serial.print("Initializing the dimmable light class... ");

#if defined(RAW_VALUES)
  DimmableLight::setSyncPin(syncPin);
  DimmableLight::begin();
#elif defined(LINEARIZED_VALUES)
  DimmableLightLinearized::setSyncPin(syncPin);
  DimmableLightLinearized::begin();
#endif
  Serial.println("Done!");

#if defined(RAW_VALUES)
  Serial.println(String("Number of instantiated lights: ") + DimmableLight::getLightNumber());
#elif defined(LINEARIZED_VALUES)
  Serial.println(String("Number of instantiated lights: ") + DimmableLightLinearized::getLightNumber());
#endif
}
