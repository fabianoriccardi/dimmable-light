#include "dimmable_light.h"

// Delay between a brightness changement in millisecond
int period = 50;

DimmableLight light(D5);

void setup() {
  Serial.begin(115200);
  while(!Serial);
  Serial.println();
  Serial.println("Test HARDWARE timer for dimmer on ESP8266");
  
  Serial.print("Init the dimmable light class... ");
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
