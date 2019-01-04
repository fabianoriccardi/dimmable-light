/** 
 * The main parameters to configure this sketch accordingly to your hardware setup are:
 *  - syncPin, that is the pin listening to AC zero cross signal
 *  - light, the pin which is connected to the thyristor
 */ 

#include "dimmable_light.h"

const int syncPin = D7;
const int thyristorPin = D5;

DimmableLight light(thyristorPin);

// Delay between a brightness changement in millisecond
int period = 50;

void setup() {
  Serial.begin(115200);
  while(!Serial);
  Serial.println();
  Serial.println("Dimmable Light for Arduino: first example");
  
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
