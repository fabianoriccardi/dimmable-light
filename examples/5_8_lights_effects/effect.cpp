#include "effect.h"
#include "Ticker.h"

Ticker dim;

DimmableLight lights[N_LIGHTS] = {{D1}, {D2}, {D5}, {D6}, {D8}, {D0}, {D3}, {D4}};

/**
 * Do some specific step, all the lights follow the same pattern
 */
void doEqual(){
  const float period = 3;
  static int briLevels[]={0,1,2,50,100,150,254,255};
  static uint8_t brightnessStep=0;
  Serial.println(String("Dimming at: ") + briLevels[brightnessStep] + "/255");
  for(int i=0;i<N_LIGHTS;i++){
    lights[i].setBrightness(briLevels[brightnessStep]);
  }

  brightnessStep++;
  if(brightnessStep==sizeof(briLevels)/sizeof(briLevels[0])){
    brightnessStep=0;
  }
  dim.once(period,doEqual);
}

/**
 * Turn on/off all the bulbs simultaneously
 */
void doEqualOnOff(){
  const float period = 3;
  static int briLevels[]={0,255};
  static uint8_t brightnessStep=0;
  Serial.println(String("Dimming at: ") + briLevels[brightnessStep] + "/255");
  for(int i=0;i<N_LIGHTS;i++){
    lights[i].setBrightness(briLevels[brightnessStep]);
  }
  
  brightnessStep++;
  if(brightnessStep==sizeof(briLevels)/sizeof(briLevels[0])){
    brightnessStep=0;
  }
  dim.once(period,doEqualOnOff);
}

/**
 * Switch lights between specific steps
 */
void doDimSpecificStep(void){
  const float period = 3;
  static int briLevels1[]={40,200};
  static int briLevels2[]={60,160};
  static int briLevels3[]={80,150};
  static uint8_t brightnessStep=0;
  Serial.println(String("Dimming at: ") + briLevels1[brightnessStep] + " and " + briLevels2[brightnessStep] + " and " + briLevels3[brightnessStep] +" /255");
  lights[1].setBrightness(briLevels1[brightnessStep]);
  lights[2].setBrightness(briLevels2[brightnessStep]);
  lights[3].setBrightness(briLevels3[brightnessStep]);

  brightnessStep++;
  if(brightnessStep==sizeof(briLevels1)/sizeof(briLevels1[0])){
    brightnessStep=0;
  }
  dim.once(period, doDimSpecificStep);
}

/**
 * Test a mixture between on-off-middle
 */
void doRangeLimit(void){
  const float period = 5;
  static int briLevels1[]={0,255};
  static int briLevels2[]={255,0};
  static int briLevels3[]={100,100};
  static uint8_t brightnessStep=0;
  Serial.println(String("Dimming at: ") + briLevels1[brightnessStep] + " and " + briLevels2[brightnessStep] + " and " + briLevels3[brightnessStep] +" /255");
  lights[1].setBrightness(briLevels1[brightnessStep]);
  lights[2].setBrightness(briLevels2[brightnessStep]);
  lights[3].setBrightness(briLevels3[brightnessStep]);

  brightnessStep++;
  if(brightnessStep==sizeof(briLevels1)/sizeof(briLevels1[0])){
    brightnessStep=0;
  }
  dim.once(period, doRangeLimit);
}

/**
 * Test the eyes limit switching between near values
 */
void doNearValues(void){
  const float period = 4;
  static int briLevels1[]={73,70};
  static int briLevels2[]={71,71};
  static int briLevels3[]={72,73};
  static uint8_t brightnessStep=0;
  Serial.println(String("Dimming at: ") + briLevels1[brightnessStep] + " and " + briLevels2[brightnessStep] + " and " + briLevels3[brightnessStep] +" /255");
  lights[1].setBrightness(briLevels1[brightnessStep]);
  lights[2].setBrightness(briLevels2[brightnessStep]);
  lights[3].setBrightness(briLevels3[brightnessStep]);

  brightnessStep++;
  if(brightnessStep==sizeof(briLevels1)/sizeof(briLevels1[0])){
    brightnessStep=0;
  }
  dim.once(period, doNearValues);
}

/**
 * The 1st the 5th are turned off; the 3rd is fixed to half; and the 2nd and 4th dim
 * on the contrary than respect each other.
 */
void doDimMixed(void){
  const float period = 0.05;
  static uint8_t brightnessStep=1;
  static bool up = true;
  
  lights[1].setBrightness(brightnessStep);
  int b2 = 105;
  lights[2].setBrightness(b2);
  int b3 = -((int)brightnessStep-255);
  lights[3].setBrightness(b3);
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
 * All the lights dim simultaneously in the same way.
 */
void doDimSweepEqual(void){
  const float period = 0.05;
  static uint8_t brightnessStep=1;
  static bool up = true;
  for(int i=0;i<N_LIGHTS;i++){
    lights[i].setBrightness(brightnessStep);
  }
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

void doOnOffSweep(){
  const float period = 0.7;
  static int16_t step = 0;
  
  for(int i=0;i<N_LIGHTS;i++){
    if(step==i){
      lights[i].setBrightness(255);
    }else{
      lights[i].setBrightness(0);
    }
  }
  
  step++;
  if(step==N_LIGHTS){
    step=0;
  }
  dim.once(period,doOnOffSweep);
}

/**
 * The group formed by even bulbs dim on the contrary to the odd group.
 */
void doInvertedDim(void){
  const float period = 0.05;
  static uint8_t brightnessStep=1;
  static bool up = true;
  int oppositeBrightness = -((int)brightnessStep-255);

  for(int i=0;i<N_LIGHTS;i++){
    if(i%2==0){
      lights[i].setBrightness(brightnessStep);
    }else{
      lights[i].setBrightness(oppositeBrightness);
    }
  }
  Serial.println(String("Dimming at: ") + brightnessStep + " " + oppositeBrightness + "/255");
  
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
  dim.once(period,doInvertedDim);
}

/**
 * This retun the module of a number (optimized)
 * Min 0 (included), max is escluded)
 */
unsigned int tap(unsigned int value,unsigned int max){
  if(value<max){
    return value;
  }
  return value%max;
}

/**
 * Input a number between 0 and 512, return a triangular function [0;255]
 */
uint8_t conversion(uint16_t value){
  int simmetricValue=0;
  if(value>=0 && value<=255){
    simmetricValue = value;
  }
  if(value>=256 && value<=511){
    simmetricValue = -value+511;
  }
  return simmetricValue;
}

uint8_t conversionPow(uint16_t value){
  int simmetricValue=0;
  if(value>=256 && value<=511){
    simmetricValue = -value+511;
  }
  if(value>=0 && value<=255){
    simmetricValue=value;
  }
  
  if(simmetricValue<150){
    return 0;
  }  
  int y = pow(simmetricValue-150,1.2);
  if(y>255){
    return 255;
  }
  return y;
}

/**
 * Turn on the light with (255/nLights) steps offset between consecutive lights
 */
void doCircularSwipe(void){
  const float period = 0.01;

  static uint16_t brightnessStep = 255;

  // Alternatively, you can use the function conversionPow(..) instead conversion(..)
  for(int i=0;i<N_LIGHTS;i++){
    lights[i].setBrightness(conversion(tap(brightnessStep+32*i,512)));
  }

  brightnessStep++;
  if(brightnessStep==512){
    brightnessStep=0;
  }
  dim.once(period,doCircularSwipe);
}

void doRandomBri(){
  const float period = 0.7;

  for(int i=0;i<N_LIGHTS;i++){
    int bri = random(0,256);
    lights[i].setBrightness(bri);
  }
  dim.once(period,doRandomBri);
}

/**
 * The variance of random number is restricted around the mean value step after step
 */
void doRandomBriPeehole(){
  const float period = 0.7;
  const uint16_t briStep = 10;
  const uint16_t totStep = 16;
  
  static uint16_t iteration = 0;
  
  for(int i=0; i<N_LIGHTS; i++){
    int bri;
    // The last 2 step are set to the same brightness
    if(iteration>=totStep-3){
      bri = 127;
    }else{
      bri = random(0 + briStep*iteration, 256-briStep*iteration);
    }
    Serial.print(String(bri) + " ");
    lights[i].setBrightness(bri);
  }
  Serial.println();
  
  iteration++;
  if(iteration==totStep){
    iteration=0;
  }
  dim.once(period, doRandomBriPeehole);
}

/**
 * The variance of random number is restricted around the mean value step after step
 */
void doRandomPushExtremeValues(){
  const float period = 1;
  const uint16_t briStep = 10;
    
  for(int i=0; i<N_LIGHTS; i++){
    int bri = random(0, briStep*2+1);
    if(bri<briStep){
      bri = bri;
    }else{
      bri = 256 - (briStep*2 - bri);
    }
    Serial.print(String(bri) + " ");
    lights[i].setBrightness(bri);
  }
  Serial.println();
  
  dim.once(period, doRandomPushExtremeValues);
}
