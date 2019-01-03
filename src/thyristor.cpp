/***************************************************************************
 *   Copyright (C) 2018, 2019 by Fabiano Riccardi                          *
 *                                                                         *
 *   This file is part of Dimmable Light for Arduino                       *
 *                                                                         *
 *   Dimmable Light for Arduino is free software; you can redistribute     *
 *   it and/or modify it under the terms of the GNU Lesser General Public  *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>   *
 ***************************************************************************/
#include "thyristor.h"
#if defined(ESP8266)
#include "hw_timer_esp8266.h"
#elif defined(ESP32)
// no need to include libraries...
#elif defined(AVR)
#include "hw_timer_avr.h"
#else
#error "only ESP8266 and ESP32 and AVR for Arduino are supported"
#endif

// Activate this define to enable a check on the period between zero and the next one
//#define CHECK_INT_PERIOD
// Activate this macro to check if all the light were managed in a semi-period
//#define CHECK_MANAGED_THYR

#ifdef ESP32
static hw_timer_t* timer = nullptr;

typedef struct {
    union {
        struct {
            uint32_t reserved0:   10;
            uint32_t alarm_en:     1;             /*When set  alarm is enabled*/
            uint32_t level_int_en: 1;             /*When set  level type interrupt will be generated during alarm*/
            uint32_t edge_int_en:  1;             /*When set  edge type interrupt will be generated during alarm*/
            uint32_t divider:     16;             /*Timer clock (T0/1_clk) pre-scale value.*/
            uint32_t autoreload:   1;             /*When set  timer 0/1 auto-reload at alarming is enabled*/
            uint32_t increase:     1;             /*When set  timer 0/1 time-base counter increment. When cleared timer 0 time-base counter decrement.*/
            uint32_t enable:       1;             /*When set  timer 0/1 time-base counter is enabled*/
        };
        uint32_t val;
    } config;
    uint32_t cnt_low;                             /*Register to store timer 0/1 time-base counter current value lower 32 bits.*/
    uint32_t cnt_high;                            /*Register to store timer 0 time-base counter current value higher 32 bits.*/
    uint32_t update;                              /*Write any value will trigger a timer 0 time-base counter value update (timer 0 current value will be stored in registers above)*/
    uint32_t alarm_low;                           /*Timer 0 time-base counter value lower 32 bits that will trigger the alarm*/
    uint32_t alarm_high;                          /*Timer 0 time-base counter value higher 32 bits that will trigger the alarm*/
    uint32_t load_low;                            /*Lower 32 bits of the value that will load into timer 0 time-base counter*/
    uint32_t load_high;                           /*higher 32 bits of the value that will load into timer 0 time-base counter*/
    uint32_t reload;                              /*Write any value will trigger timer 0 time-base counter reload*/
} hw_timer_reg_t;

typedef struct hw_timer_s {
    hw_timer_reg_t * dev;
    uint8_t num;
    uint8_t group;
    uint8_t timer;
    portMUX_TYPE lock;
} hw_timer_t;

#endif

// In microseconds
static const uint16_t semiPeriodLength = 10000;
// The margins are precautions against noise, electrical spikes and frequency skew errors
// delays after endMargin are always off (hence no useless interrupts). 
// You could tune this parameter accordingly to your settings (electrical network and MCU).
static const uint16_t startMargin = 200;
static const uint16_t endMargin = 500;

// This parameter represents the time span in which 2 (or more) very near delays are merged:
// This could be necessary for 2 main reasons:
// 1) Efficiency, in fact in some applications you will never seem differences between
//    near delays, hence raising many interrupts is useless.
// 2) MCU inability to satisfy very tight "timer start".
// After some experiments on incandescence light bulbs, I noted that even 50 microseconds 
// are not negligible, so I decided to set threshold lower that 20microsecond (note that 
// ESP8266 API documentation suggests to set timer on >10us). If you would to use 8bit 
// timers on AVR, you should set a bigger mergePeriod (e.g. 100us).
static const  unsigned int mergePeriod = 20;

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
#if defined(ESP8266)
void ICACHE_RAM_ATTR activateThyristors(){
#elif defined(ESP32)
void IRAM_ATTR activateThyristors(){
#else
void activateThyristors(){
#endif
  // Alternative way to manage the pin, it should become low after the triac started
  //delayMicroseconds(10);
  //digitalWrite(AC_LOADS[phase],LOW);
  uint8_t firstToBeUpdated=thyristorManaged;

  for(; (thyristorManaged<Thyristor::nThyristors-1 && pinDelay[thyristorManaged+1].delay-pinDelay[firstToBeUpdated].delay<mergePeriod) && (pinDelay[thyristorManaged].delay<semiPeriodLength-endMargin); thyristorManaged++){
    digitalWrite(pinDelay[thyristorManaged].pin, HIGH);
  }
  digitalWrite(pinDelay[thyristorManaged].pin, HIGH);
  thyristorManaged++;

  // This while is dedicated to all those thyristor wih delay >= semiPeriodLength-margin; those are
  // the ones who shouldn't turn on, hence they can be skipped
  while(thyristorManaged<Thyristor::nThyristors && pinDelay[thyristorManaged].delay>=semiPeriodLength-endMargin){
    thyristorManaged++;
  }

  uint8_t pulseWidth = 15;
  delayMicroseconds(pulseWidth);

  for(int i=0;i<thyristorManaged;i++){
    digitalWrite(pinDelay[i].pin, LOW);
  }

  if(thyristorManaged<Thyristor::nThyristors){
    int delay = pinDelay[thyristorManaged].delay-pinDelay[firstToBeUpdated].delay-pulseWidth;
  #if defined(ESP8266)
    timer1_write(US_TO_RTC_TIMER_TICKS(delay));
  #elif defined(ESP32)
    // Reset timer
    //timerWrite(timer, 0);
    timer->dev->load_high = 0;
    timer->dev->load_low = 0;
    timer->dev->reload = 1;

    // Set alarm to call onTimer function every second (value in microseconds).
    // Repeat the alarm (third parameter)
    //timerAlarmWrite(timer, delay, false);
    timer->dev->alarm_low = (uint32_t) delay;
    timer->dev->alarm_high = 0;
    timer->dev->config.autoreload = 0;

    // Start an alarm
    //timerAlarmEnable(timer);
    timer->dev->config.alarm_en = 1;
  #elif defined(AVR)
    if(!timerStartAndTrigger(microsecond2Tick(delay))){
      Serial.println("activateThyristors() error timer");
    }
  #endif
  }else{
    // If there are not more thyristor to serve, I can stop timer. Energy saving?
  #if defined(ESP8266)
    // Given the Arduino HAL and esp8266 technical reference manual,
    // when timer triggers, the counter stops because it has reach zero
    // and no-autorealod was set (this timer can only down-count).
  #elif defined(ESP32)
    timer->dev->config.enable = 0;
  #elif defined(AVR)
    // Given actual HAL, AVR counter automatically stops on interrupt
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
#if defined(ESP8266)
void ICACHE_RAM_ATTR zero_cross_int(){
#elif defined(ESP32)
void IRAM_ATTR zero_cross_int(){
#else
void zero_cross_int(){
#endif
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
      if(Thyristor::thyristors[i]->delay<=startMargin){
        pinDelay[i].delay=startMargin;
      }else if(Thyristor::thyristors[i]->delay>=semiPeriodLength-endMargin){
        pinDelay[i].delay=semiPeriodLength-endMargin;
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
  // NOTE 2: this improvement should be think even for multiple lamp!
  if(thyristorManaged<Thyristor::nThyristors && pinDelay[thyristorManaged].delay<9950){
  #if defined(ESP8266)
    timer1_write(US_TO_RTC_TIMER_TICKS(pinDelay[thyristorManaged].delay));
  #elif defined(ESP32)
    // Reset timer
    //timerWrite(timer, 0);
    timer->dev->load_high = 0;
    timer->dev->load_low = 0;
    timer->dev->reload = 1;
    timer->dev->config.enable = 1;

    // Set alarm to call onTimer function every second (value in microseconds).
    // Repeat the alarm (third parameter)
    //timerAlarmWrite(timer, pinDelay[thyristorManaged].delay, false);
    timer->dev->alarm_low = (uint32_t) pinDelay[thyristorManaged].delay;
    timer->dev->alarm_high = 0;
    timer->dev->config.autoreload = 0;

    // Start an alarm
    //timerAlarmEnable(timer);
    timer->dev->config.alarm_en = 1;
  #elif defined(AVR)
    if(!timerStartAndTrigger(microsecond2Tick(pinDelay[thyristorManaged].delay))){
      Serial.println("zero_cross_int() error timer");
    }
  #endif
  }else{
  #if defined(ESP8266)
    // Given the Arduino HAL and esp8266 technical reference manual,
    // when timer triggers, the counter stops because it has reach zero
    // and no-autorealod was set (this timer can only down-count).
  #elif defined(ESP32)
    timer->dev->config.enable = 0;
  #elif defined(AVR)
    // Given actual HAL, AVR counter automatically stops on interrupt
  #endif
  }
}

void Thyristor::begin(){
  pinMode(digitalPinToInterrupt(syncPin), INPUT);
  attachInterrupt(digitalPinToInterrupt(syncPin), zero_cross_int, RISING);

#if defined(ESP8266)
  timer1_attachInterrupt(activateThyristors);
  // These 2 registers assignements are the "unrolling" of:
  // timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE);
  T1C = (1 << TCTE) | ((TIM_DIV16 & 3) << TCPD) | ((TIM_EDGE & 1) << TCIT) | ((TIM_SINGLE & 1) << TCAR);
  T1I = 0;
#elif defined(ESP32)
  // Use 1st timer of 4 (counted from zero).
  // Set 80 divider for prescaler (see ESP32 Technical Reference Manual for more
  // info), count up. The counter starts to increase its value.
  timer = timerBegin(0, 80, true);
  // Attach onTimer function to our timer.
  timerAttachInterrupt(timer, &activateThyristors, true);
#elif defined(AVR)
  timerSetCallback(activateThyristors);
  timerBegin();
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
