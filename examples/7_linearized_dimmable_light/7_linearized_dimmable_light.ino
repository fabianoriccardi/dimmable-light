/**
 * The main parameters to configure this sketch accordingly to your hardware setup are:
 *  - syncPin, that is the pin listening to AC zero cross signal
 *  - light, the pin which is connected to the thyristor
 */

#include <dimmable_light_linearized.h>

const int syncPin = 13;
const int thyristorPin = 14;

DimmableLightLinearized light(thyristorPin);

// Delay between a brightness changement in millisecond
const int period = 50;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println();
  Serial.println("Dimmable Light for Arduino: first example");

  Serial.print("Initializing the dimmable light class... ");
  DimmableLightLinearized::setSyncPin(syncPin);
  // VERY IMPORTANT: Call this method to start internal light routine
  DimmableLightLinearized::begin();
  Serial.println("Done!");
}

void loop() {
  for (int i = 0; i < 256; i++) {
    light.setBrightness(i);
    delay(period);
  }

  delay(2000);
}
