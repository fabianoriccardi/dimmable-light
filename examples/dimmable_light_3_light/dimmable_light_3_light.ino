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
float period = 3;

DimmableLight l1(D1);
DimmableLight l2(D2);
DimmableLight l3(D6);

void doDim(void){
//  static int briLevels1[]={40,90};
//  static int briLevels2[]={210,180};
  static int briLevels1[]={40,230};
  static int briLevels2[]={90,180};
  static int briLevels3[]={110,190};
  static uint8_t brightnessStep=0;
  Serial.println(String("Dimming at: ") + briLevels1[brightnessStep] + " and " + briLevels2[brightnessStep] + " and " + briLevels3[brightnessStep] +" /255");
  l1.setBrightness(briLevels1[brightnessStep]);
  l2.setBrightness(briLevels2[brightnessStep]);
  l3.setBrightness(briLevels3[brightnessStep]);

  brightnessStep++;
  if(brightnessStep==sizeof(briLevels1)/sizeof(briLevels1[0])){
    brightnessStep=0;
  }
}

void doDimLinear(void){
  static uint8_t brightnessStep=0;
  Serial.println(String("Dimming at: ") + brightnessStep + "/255");
  l1.setBrightness(brightnessStep);
  int b2=-(brightnessStep-255);
  l2.setBrightness(b2);
  int b3 = 105;
  l3.setBrightness(b3);
  
  if(l1.getBrightness()!=brightnessStep || l2.getBrightness()!=b2 || l3.getBrightness()!=b3){
    Serial.println("Error!");
  }
  
  if(brightnessStep==255){
    brightnessStep=0;
  }else{
    brightnessStep++;
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

  Serial.println(String("Number of instantiated lights: ") + DimmableLight::getLightNumber());
  
  dim.attach(period,doDim);
  //l2.setBrightness(200);
}

void loop() {}
