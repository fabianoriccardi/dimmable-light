#include "dimmable_light.h"

#include "hw_timer.h"

struct PinBri{
  uint8_t pin;
  uint16_t delay;
};

/**
 * Temporary struct to provide the interrupt a memory concurrent-safe
 */
static struct PinBri pinBri[DimmableLight::N];

/**
 * Number of lights already managed in the current semi-wave
 */
static uint8_t lightManaged=0;

void activateLights();

/**
 * This function manage the triac/dimmer in a single semi-period (that is 10ms @50Hz)
 * This function will be called multiple times per semi-period (in case of multi 
 * lamps with different at least a different brightness value).
 */
void zero_cross_int(){
  // This is kind of optimization software, but not electrical:
  // This avoid to wait 10micros in a interrupt or setting interrupt 
  // to turn off the PIN (this last solution could be evaluated...)
  for(int i=0;i<DimmableLight::nLights;i++){
    digitalWrite(pinBri[i].pin,LOW);
  }

  // Update the structures, if needed
  if(DimmableLight::newBrightnessValues && !DimmableLight::updatingStruct){
  	DimmableLight::newBrightnessValues=false;
    //Serial.println("UI");
  	for(int i=0;i<DimmableLight::nLights;i++){
  		pinBri[i].pin=DimmableLight::lights[i]->pin;
  		pinBri[i].delay=DimmableLight::lights[i]->brightness;
  	}
//    for(int i=0;i<DimmableLight::nLights;i++){
//      Serial.print(pinBri[i].pin);
//      Serial.print(" ");
//      Serial.println(pinBri[i].delay);
//    }
   
  }

  lightManaged = 0;
  
  // This for is intended for full brightness
  for(int lightManaged=0;pinBri[lightManaged].delay<30;lightManaged++){
  	//Serial.println("NL");
  	digitalWrite(pinBri[lightManaged].pin,HIGH);
  }

  // This block opof code is inteded to manage the case near to the next semi-period:
  // In this case we should avoid to trigger the timer, because the effective semiperiod 
  // perceived by the esp8266 could be less than 10000microsecond. This could be due to 
  // the relative time (there is no possibily to set the timer to an absolute time)
  // Moreover, it is impossible to disable an interrupt once it is armed, neither 
  // change the callback function.
  // NOTE: don't know why, but the timer seem trigger even when it is not set...
  // so a provvisory solution if to set the relative callback to NULL!
  // NOTE 2: this improvement should be think eve for multiple lamp!
  if(lightManaged<DimmableLight::nLights && pinBri[lightManaged].delay<9950){
    hw_timer_set_func(activateLights);
    hw_timer_arm(pinBri[lightManaged].delay);
  }else{
    hw_timer_set_func(NULL);
  }
}

/**
 * Timer routine to turn on one or more lights
 */
void activateLights(){
	// Alternative way to manage the pin, it should become low after the triac started
	//delayMicroseconds(10);
	//digitalWrite(AC_LOADS[phase],LOW);
  
	// This condition means:
	// trigger immediately is there is not time to active the timer for the next light (i.e delay differene less than 150microseconds)
	for(; (lightManaged<DimmableLight::nLights-1 && pinBri[lightManaged].delay<pinBri[lightManaged+1].delay+150); lightManaged++){
		digitalWrite(pinBri[lightManaged].pin, HIGH);
		// Turn on the following light, it this really similar to the previous... 
		//Some lights could be turned on 2 times, but of course it hasn't any effect
		//digitalWrite(pinBri[lightManaged+1].pin, HIGH);
	}

	digitalWrite(pinBri[lightManaged].pin, HIGH);
	lightManaged++;

	if(lightManaged<DimmableLight::nLights){
	  hw_timer_arm(pinBri[lightManaged].delay-pinBri[lightManaged-1].delay);
	}
}

void DimmableLight::begin(){
	pinMode(digitalPinToInterrupt(syncPin), INPUT);
	attachInterrupt(digitalPinToInterrupt(syncPin), zero_cross_int, RISING);

	// FRC1 is a low priority timer, it can't interrupt other ISR
	hw_timer_init(FRC1_SOURCE, 0);
	
}

void DimmableLight::setBrightness(uint8_t newBri){
  uint16_t newBrightness=10000-(int)(newBri)*10000/255;
  Serial.println(String("Brightness in between [0,10000]: ") + newBrightness);
	// reorder the array to speed up the interrupt.
	// Thi mini-algorithm works on a different memory area wrt the interrupt,
	// so it is concurrent-safe code

  	updatingStruct=true;
    // Array example, it is always ordered, higher values means lower brightness
    // [45,678,5000,7500,9000]
    if(newBrightness>brightness){
    	bool done=false;
    	for(int i=posIntoArray+1;i<nLights && !done;i++){
    		if(newBri<lights[i]->brightness){
    			// perform the exchange between posIntoArray
    			DimmableLight *temp=lights[posIntoArray];
    			lights[posIntoArray]=lights[i];
    			lights[i]=temp;
    			// update posinto array
    			lights[posIntoArray]->posIntoArray=i;
    			// this->posIntoArray=posIntoArray
    			lights[i]->posIntoArray=posIntoArray;
    			done=true;
    		}
    	}
    }else if(newBrightness<brightness){
    	bool done=false;
    	for(int i=posIntoArray-1;i>0  && !done;i--){
    		if(newBri>lights[i]->brightness){
    			// perform the exchange between posIntoArray
    			DimmableLight *temp=lights[posIntoArray];
    			lights[posIntoArray]=lights[i];
    			lights[i]=temp;
    			// update posinto array
    			lights[posIntoArray]->posIntoArray=i;
    			// this->posIntoArray=posIntoArray
    			lights[i]->posIntoArray=posIntoArray;
    			done=true;
    		}
    	}
    }else{
    	Serial.println("No need to perform the exchange, the brightness is the same!");
    }
    brightness=newBrightness;
    newBrightnessValues=true;
    updatingStruct=false;
    
	Serial.println(String("Brightness (in ms to wait): ") + brightness);
}

DimmableLight::DimmableLight(int pin)
								:pin(pin),brightness(0){
	if(nLights<N-1){
		pinMode(pin,OUTPUT);
    
		updatingStruct=true;
		
		posIntoArray=nLights;
		nLights++;
		lights[posIntoArray]=this;
		
		pinBri[posIntoArray].pin;
		pinBri[posIntoArray].delay=10000;
		
		updatingStruct=false;
	}else{
		// return error or exception
	}
}

DimmableLight::~DimmableLight(){
	// Recompact the array
	updatingStruct=true;
	nLights--;
	//remove the light from the static pinBri array
	Serial.println("I should implement the array shrinking");
	updatingStruct=false;
}

uint8_t DimmableLight::nLights = 0;
DimmableLight* DimmableLight::lights[DimmableLight::N] = {nullptr};
bool DimmableLight::newBrightnessValues = false;
bool DimmableLight::updatingStruct = false;
uint8_t DimmableLight::syncPin = D7;
