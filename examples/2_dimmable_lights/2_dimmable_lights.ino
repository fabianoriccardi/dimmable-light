#include "dimmable_light.h"

// Delay between a brightness changement in millisecond
int period = 1000;

DimmableLight light1(D5);
DimmableLight light2(D6);

void setup() {
  Serial.begin(115200);
  while(!Serial);
  Serial.println();
  Serial.println("Test HARDWARE timer for dimmer on ESP8266");
  
  Serial.print("Init the dimmable light class... ");
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
