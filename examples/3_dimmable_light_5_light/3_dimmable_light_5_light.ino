/**
 * In this example you can see a number of effects on indipendent lights.
 * To switch among the available effects, (un)comment the proper line in the setup() function.
 *
 * NOTE: compiles only for ESP8266 and ESP32 because the Ticker.h dependency.
 */

#include <Ticker.h>
#include <dimmable_light.h>

const int syncPin = 13;
DimmableLight l1(5);
DimmableLight l2(4);
DimmableLight l3(14);
DimmableLight l4(12);
DimmableLight l5(15);

Ticker dim;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println();
  Serial.println("Dimmable Light for Arduino: third example");
  Serial.println();

  Serial.print("Initializing the dimmable light class... ");
  DimmableLight::setSyncPin(syncPin);
  DimmableLight::begin();
  Serial.println("Done!");

  Serial.println(String("Number of instantiated lights: ") + DimmableLight::getLightNumber());

  // Uncomment one and only one among the following lines to see an effect
  // doEqual();
  // doEqualOnOff();
  // doDimSpecificStep();
  // doRangeLimit();
  // doNearValues();
  // doDimMixed();
  // doDimSweepEqual();
  // doInvertedDim();
  doCircularSwipe();
}

/**
 * Set particular values of brightness to every light.
 */
void doEqual() {
  const float period = 3;
  static int briLevels[] = { 0, 1, 2, 50, 100, 150, 254, 255 };
  static uint8_t brightnessStep = 0;
  Serial.println(String("Dimming at: ") + briLevels[brightnessStep] + "/255");
  l1.setBrightness(briLevels[brightnessStep]);
  l2.setBrightness(briLevels[brightnessStep]);
  l3.setBrightness(briLevels[brightnessStep]);
  l4.setBrightness(briLevels[brightnessStep]);
  l5.setBrightness(briLevels[brightnessStep]);

  brightnessStep++;
  if (brightnessStep == sizeof(briLevels) / sizeof(briLevels[0])) { brightnessStep = 0; }
  dim.once(period, doEqual);
}

/**
 * Turn on and off simultaneously all the bulbs.
 */
void doEqualOnOff() {
  const float period = 3;
  static int briLevels[] = { 0, 255 };
  static uint8_t brightnessStep = 0;
  Serial.println(String("Dimming at: ") + briLevels[brightnessStep] + "/255");
  l1.setBrightness(briLevels[brightnessStep]);
  l2.setBrightness(briLevels[brightnessStep]);
  l3.setBrightness(briLevels[brightnessStep]);
  l4.setBrightness(briLevels[brightnessStep]);
  l5.setBrightness(briLevels[brightnessStep]);

  brightnessStep++;
  if (brightnessStep == sizeof(briLevels) / sizeof(briLevels[0])) { brightnessStep = 0; }
  dim.once(period, doEqualOnOff);
}

/**
 * Set brightness to specific values.
 */
void doDimSpecificStep(void) {
  const float period = 3;
  static int briLevels1[] = { 40, 200 };
  static int briLevels2[] = { 60, 160 };
  static int briLevels3[] = { 80, 150 };
  static uint8_t brightnessStep = 0;
  Serial.println(String("Dimming at: ") + briLevels1[brightnessStep] + " and "
                 + briLevels2[brightnessStep] + " and " + briLevels3[brightnessStep] + " /255");
  l1.setBrightness(briLevels1[brightnessStep]);
  l2.setBrightness(briLevels2[brightnessStep]);
  l3.setBrightness(briLevels3[brightnessStep]);

  brightnessStep++;
  if (brightnessStep == sizeof(briLevels1) / sizeof(briLevels1[0])) { brightnessStep = 0; }
  dim.once(period, doDimSpecificStep);
}

/**
 * Test a mixture between on, off and middle brightness.
 */
void doRangeLimit(void) {
  const float period = 5;
  static int briLevels1[] = { 0, 255 };
  static int briLevels2[] = { 255, 0 };
  static int briLevels3[] = { 100, 100 };
  static uint8_t brightnessStep = 0;
  Serial.println(String("Dimming at: ") + briLevels1[brightnessStep] + " and "
                 + briLevels2[brightnessStep] + " and " + briLevels3[brightnessStep] + " /255");
  l1.setBrightness(briLevels1[brightnessStep]);
  l2.setBrightness(briLevels2[brightnessStep]);
  l3.setBrightness(briLevels3[brightnessStep]);

  brightnessStep++;
  if (brightnessStep == sizeof(briLevels1) / sizeof(briLevels1[0])) { brightnessStep = 0; }
  dim.once(period, doRangeLimit);
}

/**
 * Test your eyes sensitivity by switching between near values. Will you see any difference?
 */
void doNearValues(void) {
  const float period = 4;
  static int briLevels1[] = { 70, 70 };
  static int briLevels2[] = { 71, 71 };
  static int briLevels3[] = { 72, 73 };
  static uint8_t brightnessStep = 0;
  Serial.println(String("Dimming at: ") + briLevels1[brightnessStep] + " and "
                 + briLevels2[brightnessStep] + " and " + briLevels3[brightnessStep] + " /255");
  l1.setBrightness(briLevels1[brightnessStep]);
  l2.setBrightness(briLevels2[brightnessStep]);
  l3.setBrightness(briLevels3[brightnessStep]);

  brightnessStep++;
  if (brightnessStep == sizeof(briLevels1) / sizeof(briLevels1[0])) { brightnessStep = 0; }
  dim.once(period, doNearValues);
}

/**
 * The 1st the 5th are turned off, the 3rd is fixed to half brightness, and the 2nd and 4th sweep
 * in the opposite direction w.r.t. each other.
 */
void doDimMixed(void) {
  const float period = 0.05;
  static uint8_t brightnessStep = 1;
  static bool up = true;
  l1.setBrightness(brightnessStep);
  int b2 = 105;
  l2.setBrightness(b2);
  int b3 = -((int)brightnessStep - 255);
  l3.setBrightness(b3);
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
  dim.once(period, doDimMixed);
}

/**
 * All the lights simultaneously fade in and out.
 */
void doDimSweepEqual(void) {
  const float period = 0.05;
  static uint8_t brightnessStep = 1;
  static bool up = true;
  l1.setBrightness(brightnessStep);
  l2.setBrightness(brightnessStep);
  l3.setBrightness(brightnessStep);
  l4.setBrightness(brightnessStep);
  l5.setBrightness(brightnessStep);
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
  dim.once(period, doDimSweepEqual);
}

/**
 * The group formed by even bulbs sweep in the opposite direction w.r.t. the odd group.
 */
void doInvertedDim(void) {
  const float period = 0.05;
  static uint8_t brightnessStep = 1;
  static bool up = true;
  int oppositeBrightness = -((int)brightnessStep - 255);

  l1.setBrightness(brightnessStep);
  l2.setBrightness(oppositeBrightness);
  l3.setBrightness(brightnessStep);
  l4.setBrightness(oppositeBrightness);
  l5.setBrightness(brightnessStep);
  Serial.println(String("Dimming at: ") + brightnessStep + " " + oppositeBrightness + "/255");

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
  dim.once(period, doInvertedDim);
}

/**
 * Turn on the light with (255/nLights) steps offset between consecutive lights.
 */
void doCircularSwipe(void) {
  const float period = 0.01;

  static uint16_t brightnessStep = 255;

  // Alternatively, you can use the function conversionPow(..) instead conversion(..)
  l1.setBrightness(triangularFunction(module(brightnessStep + 0, 512)));
  l2.setBrightness(triangularFunction(module(brightnessStep + 51 * 1, 512)));
  l3.setBrightness(triangularFunction(module(brightnessStep + 51 * 2, 512)));
  l4.setBrightness(triangularFunction(module(brightnessStep + 51 * 3, 512)));
  l5.setBrightness(triangularFunction(module(brightnessStep + 51 * 4, 512)));

  brightnessStep++;
  if (brightnessStep == 512) { brightnessStep = 0; }
  dim.once(period, doCircularSwipe);
}

/**
 * Return the module of a non-negative number (optimized).
 */
unsigned int module(unsigned int value, unsigned int max) {
  if (value < max) { return value; }
  return value % max;
}

/**
 * Given a number in range [0; 512), return a triangular function [0;255].
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

void loop() {}
