/**
 * Test to show how multiple triacs can be commanded with one timer.
 * This is a very basic example, so the dimming is constant per bulb.
 * 
 * NOTE: do not connect sync or gate with D3 - GPIO0,  D4 - GPIO2
 * they are needed to switch between the boot mode.
 */

#include <Ticker.h>

#include "dimmable_light.h"
#include "thyristor.h"

Ticker dim;
float period = 0.05;

DimmableLight l1(D5);

void doDim(void){
  static int briLevels[]={0,1,2,3,4,127,252,253,254,255};
  static uint8_t brightnessStep=0;
  Serial.println(String("Dimming at: ") + briLevels[brightnessStep] + "/255");
  l1.setBrightness(briLevels[brightnessStep]);

  brightnessStep++;
  if(brightnessStep==sizeof(briLevels)/sizeof(briLevels[0])){
    brightnessStep=0;
  }
}

void doRaise(void){
  static uint8_t brightnessStep=0;
  Serial.println(String("Dimming at: ") + brightnessStep + "/255");
  l1.setBrightness(brightnessStep);
  if(l1.getBrightness()!=brightnessStep){
    Serial.println("Error!");
  }
  
  if(brightnessStep==255){
    brightnessStep=0;
    dim.attach(period,doLower);
  }else{
    brightnessStep++;
  }
}

void doLower(void){
  static uint8_t brightnessStep=255;
  Serial.println(String("Dimming at: ") + brightnessStep + "/255");
  l1.setBrightness(brightnessStep);
  if(l1.getBrightness()!=brightnessStep){
    Serial.println("Error!");
  }
  
  if(brightnessStep==0){
    brightnessStep=255;
    dim.attach(period,doRaise);
  }else{
    brightnessStep--;
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

  dim.attach(period,doRaise);
}

void loop() {}
