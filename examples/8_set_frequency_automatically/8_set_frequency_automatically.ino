/**
 * This examples shows how to dinamically set the network frequency,
 * so your device can adapt to 50Hz or 60Hz without changing firmware.
 *
 * You just need to activate the library, then wait a while to sample
 * the actual frequency and then set the correct frequency. Since the
 * detected value may be imprecise due to noise, it is up to you
 * to implement the logic to chose the proper frequency. The frequency
 * is calculated with a moving average (by default on the last 5 values)
 * and it is continuosly updated.
 *
 * NOTE: you have to select NETWORK_FREQ_RUNTIME and MONITOR_FREQUENCY
 *       #defines in thyristor.h
 */
#include <dimmable_light.h>

const int syncPin = 13;
const int thyristorPin = 14;

DimmableLight light(thyristorPin);

// Delay between brightness increments, in milliseconds
const int period = 50;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println();
  Serial.println("Dimmable Light for Arduino: setting network frequency automatically");

  Serial.print("Initializing DimmableLight library... ");
  DimmableLight::setSyncPin(syncPin);
  // VERY IMPORTANT: Call this method to activate the library
  DimmableLight::begin();

  // A small delay to get some samples
  delay(50);

  float f = DimmableLight::getDetectedFrequency();
  int frequency = round(f);
  if (frequency == 50) {
    Serial.print("Setting frequency to 50Hz... ");
    DimmableLight::setFrequency(50);
    Serial.println("Done!");
  } else if (frequency == 60) {
    Serial.print("Setting frequency to 60Hz... ");
    DimmableLight::setFrequency(60);
    Serial.println("Done!");
  } else {
    Serial.println(String("Unknown frequency detected: ") + f);
    while (1)
      ;
  }

  Serial.println("Light dimming...");
}

void loop() {
  for (int i = 0; i < 256; i++) {
    light.setBrightness(i);
    delay(period);
  }

  // Remember that the frequency is continuously updated
  Serial.println(String("Updated frequency: ") + DimmableLight::getDetectedFrequency());
}
