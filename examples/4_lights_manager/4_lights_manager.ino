/**
 * A simple example to show the usage of DimmableLightManager class.
 * This class helps you to give a name to each light, and using it
 * to control a light.
 */
#include <dimmable_light_manager.h>

const int N = 3;

#if defined(ARDUINO_ARCH_ESP8266)
const int syncPin = 13;
const int pins[N] = { 5, 4, 14 };
#elif defined(ARDUINO_ARCH_ESP32)
const int syncPin = 23;
const int pins[N] = { 4, 16, 17 };
#elif defined(ARDUINO_ARCH_AVR)
const int syncPin = 2;
const int pins[N] = { 3, 4, 5 };
#elif defined(ARDUINO_ARCH_SAMD)
const int syncPin = 2;
const int pins[N] = { 3, 4, 5 };
#elif (defined(ARDUINO_ARCH_RP2040) && !defined(ARDUINO_ARCH_MBED))
const int syncPin = 2;
const int pins[N] = { 3, 4, 5 };
#endif

DimmableLightManager dlm;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println();
  Serial.println("Dimmable Light for Arduino: fourth example");

  Serial.println("Initializing the dimmable light class... ");

  // Add all the lights you need
  for (int i = 0; i < N; i++) {
    if (dlm.add(String("light") + (i + 1), pins[i])) {
      Serial.println(String("   ") + (i + 1) + "-th light added correctly");
    } else {
      Serial.println("   Light isn't added, probably this name was already used...");
    }
  }

  DimmableLight::setSyncPin(syncPin);
  DimmableLightManager::begin();

  Serial.println("Done!");
}

void loop() {
  for (int b = 0; b < 255; b += 10) {
    for (int i = 0; i < dlm.getCount(); i++) {
      std::pair<String, DimmableLight*> p = dlm.get();
      String lightName = p.first;
      DimmableLight* dimLight = p.second;
      // Altervatively, you can require to the manager a specific light
      // DimmableLight* dimLight = dlm.get("light1");

      Serial.println(String("Setting --") + lightName + "-- to brightness: " + b);
      dimLight->setBrightness(b);
    }
    delay(500);
  }
  Serial.println();
}
