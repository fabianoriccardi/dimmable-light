/**
 * This examples provides some effects to test and demonstrate the potentiality of DimmableLight.
 * Once you can uploaded this sketch, you can select one effect through the serial port. Just
 * type the code's effect (values among ["e0"-"e12"]) and "stop" to stop the current effect.
 * Remember to select CRLF line ending in Arduino IDE serial console.
 *
 * NOTE: install https://github.com/kroimon/Arduino-SerialCommand
 */
#include "effect.h"

#include <SerialCommand.h>

SerialCommand serialCmd;

int effectSelected = -1;
void unrecognized(const char* message) {
  Serial.println(String(message) + ": command not recognized");
  serialCmd.clearBuffer();
}

void stopEffect() {
  offAllLights();

  effectSelected = -1;
  effect = nullptr;
}

void selectEffect(unsigned char effectId) {
  if (effectSelected != effectId) {
    effectSelected = effectId;

    switch (effectSelected) {
      case 0:
        Serial.println("##New Effect Selected## Equal");
        doEqual();
        break;
      case 1:
        Serial.println("##New Effect Selected## Equal On Off");
        doEqualOnOff();
        break;
      case 2:
        Serial.println("##New Effect Selected## Dim Specific Step");
        doDimSpecificStep();
        break;
      case 3:
        Serial.println("##New Effect Selected## Range Limit");
        doRangeLimit();
        break;
      case 4:
        Serial.println("##New Effect Selected## Near Values");
        doNearValues();
        break;
      case 5:
        Serial.println("##New Effect Selected## Dim Mixed");
        doDimMixed();
        break;
      case 6:
        Serial.println("##New Effect Selected## Dim Sweep Equal");
        doDimSweepEqual();
        break;
      case 7:
        Serial.println("##New Effect Selected## On Off Sweep");
        doOnOffSweep();
        break;
      case 8:
        Serial.println("##New Effect Selected## Inverted Dim");
        doInvertedDim();
        break;
      case 9:
        Serial.println("##New Effect Selected## Circular Swipe");
        doCircularSwipe();
        break;
      case 10:
        Serial.println("##New Effect Selected## Random Bri");
        doRandomBri();
        break;
      case 11:
        Serial.println("##New Effect Selected## Random Bri Peephole");
        doRandomBriPeephole();
        break;
      case 12:
        Serial.println("##New Effect Selected## Random Push Extreme Values");
        doRandomPushExtremeValues();
        break;
      case 13:
        Serial.println("##New Effect Selected## Circular Swipe Regular");
        doCircularSwipeRegular();
        break;
      default: Serial.println("Effect ID not implemented");
    }
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println();
  Serial.println("Dimmable Light for Arduino: sixth example");

  initLights();

  serialCmd.addCommand("stop", []() {
    stopEffect();
  });
  serialCmd.addCommand("e0", []() {
    selectEffect(0);
  });
  serialCmd.addCommand("e1", []() {
    selectEffect(1);
  });
  serialCmd.addCommand("e2", []() {
    selectEffect(2);
  });
  serialCmd.addCommand("e3", []() {
    selectEffect(3);
  });
  serialCmd.addCommand("e4", []() {
    selectEffect(4);
  });
  serialCmd.addCommand("e5", []() {
    selectEffect(5);
  });
  serialCmd.addCommand("e6", []() {
    selectEffect(6);
  });
  serialCmd.addCommand("e7", []() {
    selectEffect(7);
  });
  serialCmd.addCommand("e8", []() {
    selectEffect(8);
  });
  serialCmd.addCommand("e9", []() {
    selectEffect(9);
  });
  serialCmd.addCommand("e10", []() {
    selectEffect(10);
  });
  serialCmd.addCommand("e11", []() {
    selectEffect(11);
  });
  serialCmd.addCommand("e12", []() {
    selectEffect(12);
  });
  serialCmd.addCommand("e13", []() {
    selectEffect(13);
  });

  serialCmd.setDefaultHandler(unrecognized);

  Serial.println("Select one effect ranging between e0 and e13");
}

void loop() {
  serialCmd.readSerial();

  uint32_t now = millis();
  if (effectSelected == -1) {
    delay(20);
  } else if (now - lastCall > period) {
    if (effect != nullptr) effect();
  }
}
