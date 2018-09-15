#include <SoftwareSerial.h>
#include <SerialCommand.h>
#include <Ticker.h>

#include "effect.h"

int pins[N_LIGHTS] = {D1,D2,D5,D6,D8,D0,D3,D4};

const int syncPin = D7;

SerialCommand serialCmd;

int effectSelected = -1;
void unrecognized(){
    Serial.println("Command not recognized"); 
}

void setup() {
  Serial.begin(115200);
  while(!Serial);
  Serial.println();
  Serial.println("Starting the example effect");
  
  Serial.print("Init the dimmable light class... ");

  lights=(DimmableLight**)malloc(sizeof(DimmableLight)*N_LIGHTS);
  for(int i=0;i<N_LIGHTS;i++){
    lights[i] = new DimmableLight(pins[i]);
  }
  
  DimmableLight::setSyncPin(syncPin);
  DimmableLight::begin();
  Serial.println("Done!");

  Serial.println(String("Number of instantiated lights: ") + DimmableLight::getLightNumber());

  int c = 0;
  serialCmd.addCommand("e1",[](){effectSelected = 0; doEqual();});
  serialCmd.addCommand("e2",[](){effectSelected = 1; doEqualOnOff();});
  serialCmd.addCommand("e3",[](){effectSelected = 2; doDimSpecificStep();});
  serialCmd.addCommand("e4",[](){effectSelected = 3; doRangeLimit();});
  serialCmd.addCommand("e5",[](){effectSelected = 4; doNearValues();});
  serialCmd.addCommand("e6",[](){effectSelected = 5; doDimMixed();});
  serialCmd.addCommand("e7",[](){effectSelected = 6; doDimSweepEqual();});
  serialCmd.addCommand("e8",[](){effectSelected = 7; doOnOffSweep();});
  serialCmd.addCommand("e9",[](){effectSelected = 8; doInvertedDim();});
  serialCmd.addCommand("e10",[](){effectSelected = 9; doCircularSwipe();});
  serialCmd.addCommand("e11",[](){effectSelected = 10; doRandomBri();});
  serialCmd.addCommand("e12",[](){effectSelected = 11; doRandomBriPeehole();});
  
  serialCmd.addDefaultHandler(unrecognized);

}

void loop(){
    serialCmd.readSerial();
    switch(effectSelected){
        case -1:
            Serial.println("Off state");
            delay(20);
            break;
        case 0:
            //doEqual();
            break;
        case 1:
            //doEqualOnOff();
            break;
        case 2:
            //doDimSpecificStep();
            break;
        case 3:
            //doRangeLimit();
            break;
        case 4:
            //doNearValues();
            break;
        case 5:
            //doDimMixed();
            break;
        case 6:
            //doDimSweepEqual();
            break;
        case 7:
            //doInvertedDim();
            break;
        case 8:
            //doCircularSwipe();
            break;
        case 9:
            //doCircularSwipe();
            break;
        case 10:
            //doCircularSwipe();
            break;
        default:
            Serial.println("Default switch case");
    }
    //delay(50);
}
