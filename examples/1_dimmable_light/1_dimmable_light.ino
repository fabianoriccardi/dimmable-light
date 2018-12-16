#include "dimmable_light.h"

// Pin listening to AC zero cross signal
const int syncPin = D7;

// Delay between a brightness changement in millisecond
int period = 50;

DimmableLight light(D5);

void setup() {
  Serial.begin(115200);
  while(!Serial);
  Serial.println();
  Serial.println("Dimmer for ESP8266");
  
  Serial.print("Init the dimmable light class... ");
  DimmableLight::setSyncPin(syncPin);
  // VERY IMPORTANT: Call this method to start internal light routine
  DimmableLight::begin();
  Serial.println("Done!");
}

void loop() {
  for(int i=0;i<256;i++){
    light.setBrightness(i);
    delay(period);
  }
}
