/***************************************************************************
 *   Copyright (C) 2018 by Fabiano Riccardi                                *
 *                                                                         *
 *   This file is part of Dimmable Light for ESP8266.                      *
 *                                                                         *
 *   Dimmable Light for Arduino is free software; you can redistribute     *
 *   it and/or modify it under the terms of the GNU General Public         *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   As a special exception, if other files instantiate templates or use   *
 *   macros or inline functions from this file, or you compile this file   *
 *   and link it with other works to produce a work based on this file,    *
 *   this file does not by itself cause the resulting work to be covered   *
 *   by the GNU General Public License. However the source code for this   *
 *   file must still be made available in accordance with the GNU General  *
 *   Public License. This exception does not invalidate any other reasons  *
 *   why a work based on this file might be covered by the GNU General     *
 *   Public License.                                                       *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>   *
 ***************************************************************************/
#include "thyristor.h"
#if defined(ESP8266)
#include "hw_timer.h"
#elif defined(ESP32)
// no need to include libraries...
#else
#error "only ESP8266 and ESP32 for Arduino are supported"
#endif

// Activate this define to enable a check on the period between zero and the next one
//#define CHECK_INT_PERIOD
// Activate this macro to check if all the light were managed in a semi-period
//#define CHECK_MANAGED_THYR

#ifdef ESP32
static hw_timer_t* timer = NULL;
#endif

struct PinDelay{
  uint8_t pin;
  uint16_t delay;
};

/**
 * Temporary struct to provide the interrupt a memory concurrent-safe
 */
static struct PinDelay pinDelay[Thyristor::N];

/**
 * Number of thyristors already managed in the current semi-wave
 */
static uint8_t thyristorManaged=0;

/**
 * Timer routine to turn on one or more thyristors
 */
#ifdef ESP32
void IRAM_ATTR activateThyristors(){
#else
void activateThyristors(){
#endif
  // Alternative way to manage the pin, it should become low after the triac started
  //delayMicroseconds(10);
  //digitalWrite(AC_LOADS[phase],LOW);
  
  uint8_t firstToBeUpdated=thyristorManaged;
  // This condition means:
  // trigger immediately is there is not time to active the timer for the next light 
  // (i.e delay differene less than 20microseconds)
  // After some experiment, even 50 microseconrd are noticeble, so I decided 
  // to set the threshold lower that 20microsecond (wrt the resolution of the user class, 
  // that it about 39microsecond, this loop is used only for equal values)
  for(; (thyristorManaged<Thyristor::nThyristors-1 && pinDelay[thyristorManaged+1].delay-pinDelay[firstToBeUpdated].delay<20); thyristorManaged++){
    digitalWrite(pinDelay[thyristorManaged].pin, HIGH);
  }

  digitalWrite(pinDelay[thyristorManaged].pin, HIGH);

  thyristorManaged++;

  uint8_t pulseWidth = 15;
  delayMicroseconds(pulseWidth);

  for(int i=0;i<thyristorManaged;i++){
    digitalWrite(pinDelay[i].pin, LOW);
  }

  if(thyristorManaged<Thyristor::nThyristors){
    int delay = pinDelay[thyristorManaged].delay-pinDelay[thyristorManaged-1].delay-pulseWidth;
  #if defined(ESP8266)
    hw_timer_arm(delay);
  #elif defined(ESP32)
    // Reset timer
    timerWrite(timer, 0);
    // Set alarm to call onTimer function every second (value in microseconds).
    // Repeat the alarm (third parameter)
    timerAlarmWrite(timer, delay, false);
    // Start an alarm
    timerAlarmEnable(timer);
  #endif
  }
}

#ifdef CHECK_INT_PERIOD
bool first=true;
uint32_t lastTime;
#endif

/**
 * This function manage the triac/dimmer in a single semi-period (that is 10ms @50Hz)
 * This function will be called multiple times per semi-period (in case of multi 
 * lamps with different at least a different delay value).
 */
void zero_cross_int(){
  // This is kind of optimization software, but not electrical:
  // This avoid to wait 10micros in a interrupt or setting interrupt 
  // to turn off the PIN (this last solution could be evaluated...)
  for(int i=0;i<Thyristor::nThyristors;i++){
   digitalWrite(pinDelay[i].pin,LOW);
  }

#ifdef CHECK_INT_PERIOD
  if(first){
    lastTime=micros();
    first=false;
  }else{
    uint32_t now=micros();
    if(now-lastTime>10015||now-lastTime<9090){
      Serial.println(now-lastTime);
    }
    lastTime=now;
  }
#endif

#ifdef CHECK_MANAGED_THYR
  if(thyristorManaged!=Thyristor::nThyristors){
    Serial.print("E");
    Serial.println(thyristorManaged);
  }
#endif

  // Update the structures and set thresholds, if needed
  if(Thyristor::newDelayValues && !Thyristor::updatingStruct){
    Thyristor::newDelayValues=false;
    //Serial.println("UI");
    for(int i=0;i<Thyristor::nThyristors;i++){
      pinDelay[i].pin=Thyristor::thyristors[i]->pin;
      // Rounding delays to avoid error and unexpected behaviour due to 
      // non-ideal thyristors and not perfect sine wave 
      if(Thyristor::thyristors[i]->delay<=200){
        pinDelay[i].delay=200;
      }else if(Thyristor::thyristors[i]->delay>=9800){
        pinDelay[i].delay=9800;
      }else{
        pinDelay[i].delay=Thyristor::thyristors[i]->delay;
      }
    }
    // for(int i=0;i<Thyristor::nThyristors;i++){
    //   Serial.print(String("int: ") + pinDelay[i].pin);
    //   Serial.print(" ");
    //   Serial.println(pinDelay[i].delay);
    // }   
  }

  thyristorManaged = 0;

  // This block of code is inteded to manage the case near to the next semi-period:
  // In this case we should avoid to trigger the timer, because the effective semiperiod 
  // perceived by the esp8266 could be less than 10000microsecond. This could be due to 
  // the relative time (there is no possibily to set the timer to an absolute time)
  // Moreover, it is impossible to disable an interrupt once it is armed, neither 
  // change the callback function.
  // NOTE: don't know why, but the timer seem trigger even when it is not set...
  // so a provvisory solution if to set the relative callback to NULL!
  // NOTE 2: this improvement should be think eve for multiple lamp!
  if(thyristorManaged<Thyristor::nThyristors && pinDelay[thyristorManaged].delay<9950){
  #if defined(ESP8266)
    hw_timer_set_func(activateThyristors);
    hw_timer_arm(pinDelay[thyristorManaged].delay);
  #elif defined(ESP32)
    // Reset timer
    timerWrite(timer, 0);
    // Set alarm to call onTimer function every second (value in microseconds).
    // Repeat the alarm (third parameter)
    timerAlarmWrite(timer, pinDelay[thyristorManaged].delay, false);
    // Start an alarm
    timerAlarmEnable(timer);
  #endif
  }else{
  #if defined(ESP8266)
    hw_timer_set_func(NULL);
  #elif defined(ESP32)
    timerAlarmDisable(timer);
  #endif
  }
}

void Thyristor::begin(){
  pinMode(digitalPinToInterrupt(syncPin), INPUT);
  attachInterrupt(digitalPinToInterrupt(syncPin), zero_cross_int, RISING);

#if defined(ESP8266)
  // FRC1 is a low priority timer, it can't interrupt other ISR
  hw_timer_init(FRC1_SOURCE, 0);
#elif defined(ESP32)
  // Use 1st timer of 4 (counted from zero).
  // Set 80 divider for prescaler (see ESP32 Technical Reference Manual for more
  // info).
  timer = timerBegin(0, 80, true);
  // Attach onTimer function to our timer.
  timerAttachInterrupt(timer, &activateThyristors, true);
#endif
}

void Thyristor::setDelay(uint16_t newDelay){
  if(verbosity>2){
    for(int i=0;i<Thyristor::nThyristors;i++){
      Serial.print(String("setB: ") + "posIntoArray:" + thyristors[i]->posIntoArray + " pin:" + thyristors[i]->pin);
      Serial.print(" ");
      Serial.println(thyristors[i]->delay);
    }
  }

  // Reorder the array to speed up the interrupt.
  // This mini-algorithm works on a different memory area wrt the interrupt,
  // so it is concurrent-safe code

  updatingStruct=true;
  // Array example, it is always ordered, higher values means lower delay
  // [45,678,5000,7500,9000]
  if(newDelay>delay){
    if(verbosity>2) Serial.println("\tlowering the light..");
    bool done=false;
    /////////////////////////////////////////////////////////////////
    // Let's find the new position
    int i=posIntoArray+1;
    while(i<nThyristors && !done){
      if(newDelay<=thyristors[i]->delay){
        done=true;
      }else{
        i++;
      }
    }
    // This could be due to 2 factor:
    // 1) the light is already the lowest delay
    // 2) the delay is not changed to overpass the neightbour
    if(posIntoArray+1==i){
      if(verbosity>2) Serial.println("No need to shift..");
    }else{
      int target;
      // Means that we have reached the end, the target i the last element
      if(i==nThyristors){
        target=nThyristors-1;
      }else{
        target=i-1;
      }
  
      // Let's shift
      for(int i=posIntoArray;i<target;i++){
        thyristors[i]=thyristors[i+1];
        thyristors[i]->posIntoArray=i;
      }
      thyristors[target]=this;
      this->posIntoArray=target;
    }
  }else if(newDelay<delay){
    if(verbosity>2) Serial.println("\traising the light..");
    bool done=false;
    int i=posIntoArray-1;
    while(i>=0 && !done){
      if(newDelay>=thyristors[i]->delay){
        done=true;
      }else{
        i--;
      }
    }
    if(posIntoArray-1==i){
      if(verbosity>2) Serial.println("No need to shift..");
    }else{
      int target;
      // Means that we have reached the start, the target is the first element
      if(!done){
        target=0;
      }else{
        target=i+1;
      }
  
      // Let's shift
      for(int i=posIntoArray;i>target;i--){
        thyristors[i]=thyristors[i-1];
        thyristors[i]->posIntoArray=i;
      }
      thyristors[target]=this;
      this->posIntoArray=target;
    }
  }else{
    if(verbosity>2) Serial.println("No need to perform the exchange, the delay is the same!");
  }
  delay=newDelay;
  newDelayValues=true;
  updatingStruct=false;
  
  if(verbosity>2){
    for(int i=0;i<Thyristor::nThyristors;i++){
      Serial.print(String("\tsetB: ") + "posIntoArray:" + thyristors[i]->posIntoArray + " pin:" + thyristors[i]->pin);
      Serial.print(" ");
      Serial.println(thyristors[i]->delay);
    }
  }

  //Serial.println(String("Brightness (in ms to wait): ") + delay);
}

/**
 * @brief      No reorder, init all the light at the begin of your sketch
 *
 * @param[in]  pin   The pin
 */
Thyristor::Thyristor(int pin)
                :pin(pin),delay(10000){
  if(nThyristors<N){
    pinMode(pin,OUTPUT);
    
    updatingStruct=true;
    
    posIntoArray=nThyristors;
    nThyristors++;
    thyristors[posIntoArray]=this;
    
    //Full reorder of the array
    for(int i=0;i<nThyristors;i++){
      for(int j=i+1;j<nThyristors-1;j++){
        if(thyristors[i]->delay>thyristors[j]->delay){
          Thyristor *temp=thyristors[i];
          thyristors[i]=thyristors[j];
          thyristors[j]=temp;
        }
      }
    }
    // Set the posIntoArray with a "brutal" assignement to each Thyristor
    for(int i=0;i<nThyristors;i++){
      thyristors[i]->posIntoArray=i;
    }

    newDelayValues=true;
    
    // NO because this struct is updated by the routine!
//    pinDelay[posIntoArray].pin;
//    pinDelay[posIntoArray].delay=10000;
    
    updatingStruct=false;
  }else{
    // return error or exception
  }
}

Thyristor::~Thyristor(){
  // Recompact the array
  updatingStruct=true;
  nThyristors--;
  //remove the light from the static pinDelay array
  Serial.println("I should implement the array shrinking");
  updatingStruct=false;
}

uint8_t Thyristor::nThyristors = 0;
Thyristor* Thyristor::thyristors[Thyristor::N] = {nullptr};
bool Thyristor::newDelayValues = false;
bool Thyristor::updatingStruct = false;
uint8_t Thyristor::syncPin = 255;
