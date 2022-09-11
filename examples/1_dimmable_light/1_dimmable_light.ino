/**
 * Before uploading this sketch, check and modify the following variables
 * accordingly to your hardware setup:
 *  - syncPin, the pin listening for AC zero cross signal
 *  - thyristorPin, the pin connected to the thyristor
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
  Serial.println("Dimmable Light for Arduino: first example");

  Serial.print("Initializing DimmableLight library... ");
  DimmableLight::setSyncPin(syncPin);
  // VERY IMPORTANT: Call this method to activate the library
  DimmableLight::begin();
  Serial.println("Done!");
}

void loop() {
  for (int i = 0; i < 256; i++) {
    light.setBrightness(i);
    delay(period);
  }
}
