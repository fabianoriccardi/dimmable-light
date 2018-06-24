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

DimmableLight l1(D1);
DimmableLight l2(D2);
DimmableLight l3(D5);

void doEqual(){
  const float period = 3;
  static int briLevels[]={0,1,2,50,100,150,254,255};
  static uint8_t brightnessStep=0;
  Serial.println(String("Dimming at: ") + briLevels[brightnessStep] + "/255");
  l1.setBrightness(briLevels[brightnessStep]);
  l2.setBrightness(briLevels[brightnessStep]);
  l3.setBrightness(briLevels[brightnessStep]);
  brightnessStep++;
  if(brightnessStep==sizeof(briLevels)/sizeof(briLevels[0])){
    brightnessStep=0;
  }
  dim.once(period,doEqual);
}

void doEqualOnOff(){
  const float period = 3;
  static int briLevels[]={0,255};
  static uint8_t brightnessStep=0;
  Serial.println(String("Dimming at: ") + briLevels[brightnessStep] + "/255");
  l1.setBrightness(briLevels[brightnessStep]);
  l2.setBrightness(briLevels[brightnessStep]);
  l3.setBrightness(briLevels[brightnessStep]);
  brightnessStep++;
  if(brightnessStep==sizeof(briLevels)/sizeof(briLevels[0])){
    brightnessStep=0;
  }
  dim.once(period,doEqualOnOff);
}

/**
 * Test ordering algorithm
 */
void doDimSpecificStep(void){
  const float period = 3;
  static int briLevels1[]={40,200};
  static int briLevels2[]={60,160};
  static int briLevels3[]={80,150};
  static uint8_t brightnessStep=0;
  Serial.println(String("Dimming at: ") + briLevels1[brightnessStep] + " and " + briLevels2[brightnessStep] + " and " + briLevels3[brightnessStep] +" /255");
  l1.setBrightness(briLevels1[brightnessStep]);
  l2.setBrightness(briLevels2[brightnessStep]);
  l3.setBrightness(briLevels3[brightnessStep]);

  brightnessStep++;
  if(brightnessStep==sizeof(briLevels1)/sizeof(briLevels1[0])){
    brightnessStep=0;
  }
  dim.once(period, doDimSpecificStep);
}

/**
 * Test the limit on off
 */
void doRangeLimit(void){
  const float period = 5;
  static int briLevels1[]={0,255};
  static int briLevels2[]={255,0};
  static int briLevels3[]={100,100};
  static uint8_t brightnessStep=0;
  Serial.println(String("Dimming at: ") + briLevels1[brightnessStep] + " and " + briLevels2[brightnessStep] + " and " + briLevels3[brightnessStep] +" /255");
  l1.setBrightness(briLevels1[brightnessStep]);
  l2.setBrightness(briLevels2[brightnessStep]);
  l3.setBrightness(briLevels3[brightnessStep]);

  brightnessStep++;
  if(brightnessStep==sizeof(briLevels1)/sizeof(briLevels1[0])){
    brightnessStep=0;
  }
  dim.once(period, doRangeLimit);
}

/**
 * Test the limit on off
 */
void doNearValues(void){
  const float period = 4;
  static int briLevels1[]={70,70};
  static int briLevels2[]={71,71};
  static int briLevels3[]={72,73};
  static uint8_t brightnessStep=0;
  Serial.println(String("Dimming at: ") + briLevels1[brightnessStep] + " and " + briLevels2[brightnessStep] + " and " + briLevels3[brightnessStep] +" /255");
  l1.setBrightness(briLevels1[brightnessStep]);
  l2.setBrightness(briLevels2[brightnessStep]);
  l3.setBrightness(briLevels3[brightnessStep]);

  brightnessStep++;
  if(brightnessStep==sizeof(briLevels1)/sizeof(briLevels1[0])){
    brightnessStep=0;
  }
  dim.once(period, doNearValues);
}

/**
 * A generic behaviour: 
 */
void doDimMixed(void){
  const float period = 0.05;
  static uint8_t brightnessStep=1;
  static bool up = true;
  l1.setBrightness(brightnessStep);
  int b2 = 105;
  l2.setBrightness(b2);
  int b3 = -((int)brightnessStep-255);
  l3.setBrightness(b3);
  Serial.println(String("Dimming at: ") + brightnessStep + " " + b2 + " " + b3 + "/255");
  
  if(brightnessStep==255 && up){
    up=false;
  }else if(brightnessStep==0 && !up){
    up=true;
  }else{
    if(up){
      brightnessStep++;
    }else{
      brightnessStep--;
    }
    //Jump some steps...
//    if(brightnessStep==6){
//      brightnessStep=249;
//    }
  }
  dim.once(period,doDimMixed);
}

/**
 * A generic behaviour: 
 */
void doDimSweepEqual(void){
  const float period = 0.05;
  static uint8_t brightnessStep=1;
  static bool up = true;
  l1.setBrightness(brightnessStep);
  l2.setBrightness(brightnessStep);
  l3.setBrightness(brightnessStep);
  Serial.println(String("Dimming at: ") + brightnessStep + "/255");
  
  if(brightnessStep==255 && up){
    up=false;
  }else if(brightnessStep==0 && !up){
    up=true;
  }else{
    if(up){
      brightnessStep++;
    }else{
      brightnessStep--;
    }
  }
  dim.once(period,doDimSweepEqual);
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

  //doDimSpecificStep();
  //doEqual();
  //doEqualOnOff();
  doDimMixed();
  //doNearValues();
  //doRangeLimit();
  //doDimSweepEqual();
}

void loop() {}
