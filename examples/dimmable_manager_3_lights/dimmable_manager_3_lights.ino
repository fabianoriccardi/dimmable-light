#include <Ticker.h>

#include "dimmable_light_manager.h"

Ticker dim;
float period = 0.05;

DimmableLightManager dlm;

void doRaise(void){
  static uint8_t brightnessStep=0;
  Serial.println(String("Dimming at: ") + brightnessStep + "/255");
  dlm.get("light1")->setBrightness(brightnessStep);
  dlm.get("light2")->setBrightness(brightnessStep);
  dlm.get("light3")->setBrightness(brightnessStep);
  
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
  dlm.get("light1")->setBrightness(brightnessStep);
  dlm.get("light2")->setBrightness(brightnessStep);
  dlm.get("light3")->setBrightness(brightnessStep);
  
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
  
  // Add all the lights you need
  if(dlm.add("light1",D1)){
    Serial.println("First light added correctly");
  }else{
    Serial.println("Light isn't added, probably this was is already used...");
  }
  if(dlm.add("light2",D2)){
    Serial.println("First light added correctly");
  }else{
    Serial.println("Light isn't added, probably this was is already used...");
  }
  if(dlm.add("light3",D5)){
    Serial.println("First light added correctly");
  }else{
    Serial.println("Light isn't added, probably this was is already used...");
  }

  // Call thi method to start the light routine
  DimmableLightManager::begin();
  
  Serial.println("Done!");

  doRaise();
  dim.attach(period,doRaise);

  delay(500);
  
  for(int i=0;i<dlm.getCount();i++){
    std::pair<String,DimmableLight*> p=dlm.get();
    Serial.println(String(p.first) + " " + p.second->getBrightness());
  }
}

void loop() {
  
}
