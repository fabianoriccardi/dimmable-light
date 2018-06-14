/**
 * Test to show how multiple triacs can be commanded with one timer.
 * This is a very basic example, so the dimming is constant per bulb.
 * 
 * NOTE: do not connect sync or gate with D3 - GPIO0,  D4 - GPIO2
 * they are needed to switch between the boot mode.
 */

#include <Ticker.h>

#include "dimmable_light.h"

Ticker dim;
int period=5;

DimmableLight l1(D5);
DimmableLight l2(D6);
DimmableLight l3(D4);

void doDim(void){
  static uint8_t step=0;
  Serial.println("Dimming...");
  switch(step){
    case 0:
    l1.setBrightness(1000);
    l2.setBrightness(3000);
    l3.setBrightness(5000);
    break;
    case 1:
    l1.setBrightness(3000);
    l2.setBrightness(5000);
    l3.setBrightness(7000);
    break;
    case 2:
    l1.setBrightness(5000);
    l2.setBrightness(7000);
    l3.setBrightness(9000);
    break;
    case 3:
    l1.setBrightness(7000);
    l2.setBrightness(9000);
    l3.setBrightness(9000);
    break;
    case 4:
    l1.setBrightness(9000);
    l2.setBrightness(9000);
    l3.setBrightness(9000);
    break;
    default: break;
  }

  if(step==4){
    step=0;
  }else{
    step++;
  }
}

void setup() {
  Serial.begin(115200);
  while(!Serial);
  Serial.println();
  Serial.println("Test HARDWARE timer for dimmer on ESP8266");
  
  Serial.print("Init the dimmable light class... ");
  DimmableLight::begin();
  Serial.println("Done!");

  dim.attach(period,doDim);
}

void loop() {}
