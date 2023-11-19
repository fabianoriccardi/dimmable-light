/**
 * An extension of the first example to demonstrate how easy is controlling multiple lights.
 */
#include <dimmable_light.h>

const int syncPin = 13;

DimmableLight light1(14);
DimmableLight light2(12);

// Delay between brightness changes, in milliseconds
const int period = 1000;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println();
  Serial.println("Dimmable Light for Arduino: second example");
  Serial.println();

  Serial.print("Initializing the dimmable light class... ");
  DimmableLight::setSyncPin(syncPin);
  DimmableLight::begin();
  Serial.println("Done!");
}

void loop() {
  light1.setBrightness(0);
  light2.setBrightness(255);
  delay(period);

  light1.setBrightness(128);
  light2.setBrightness(128);
  delay(period);

  light1.setBrightness(255);
  light2.setBrightness(0);
  delay(period);
}
