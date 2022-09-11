/**
 * This advanced example shows how to use the DimmableLightManager
 * combined with Ticker class.
 *
 * In this case the friendly names are "light1", "light2" and so on...
 * Once your setup is ready (light, thyristor, wemos flashed with this sketch),
 * you should see a very simple effect: all the light will fade from
 * dark to maximum brightness simultaneously.
 *
 * NOTE: compiles only for ESP8266 and ESP32 because the Ticker.h dependency
 */

#include <Ticker.h>
#include <dimmable_light_manager.h>

#define N 5

const int syncPin = 13;
int pins[N] = { 5, 4, 14, 12, 15 };

// This param modifies the effect speed. The value is the period between a
// brightness value and the next one, in seconds
const float period = 0.05;

Ticker dim;

DimmableLightManager dlm;

void doRaise(void) {
  static uint8_t brightnessStep = 0;

  for (int i = 0; i < dlm.getCount(); i++) {
    DimmableLight* dimLight = dlm.get().second;
    dimLight->setBrightness(brightnessStep);
  }

  if (brightnessStep == 255) {
    brightnessStep = 0;
    dim.attach(period, doLower);
  } else {
    brightnessStep++;
  }
}

void doLower(void) {
  static uint8_t brightnessStep = 255;

  for (int i = 0; i < dlm.getCount(); i++) {
    DimmableLight* dimLight = dlm.get().second;
    dimLight->setBrightness(brightnessStep);
  }

  if (brightnessStep == 0) {
    brightnessStep = 255;
    dim.attach(period, doRaise);
  } else {
    brightnessStep--;
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println();
  Serial.println("Dimmable Light for Arduino: fifth example");

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

  // This line starts the effect. In the meanwhile,
  // you can continue to execution your own code (thanks to Ticker library)
  dim.attach(period, doRaise);
}

void loop() {
  // Print the light name and its actual brightness
  for (int i = 0; i < dlm.getCount(); i++) {
    std::pair<String, DimmableLight*> p = dlm.get();
    DimmableLight* dimLight = p.second;
    // Altervatively, you can require to the manager a specific light
    // DimmableLight* dimLight = dlm.get("light1");

    Serial.println(String(p.first) + " brightness:" + dimLight->getBrightness());
  }
  Serial.println();

  delay(500);
}
