#ifdef AVR

#include <util/atomic.h>
#include <Arduino.h>
#include "hw_timer_avr.h"

void (*timer_callback)() = nullptr;

ISR(TIMER2_COMPA_vect){
    if(timer_callback != nullptr){
        timer_callback();
    }
}

/**
 * convert microsecond to tick, max micro is 32767, otherwize it returns 0.
 */
uint16_t microsecond2Tick(uint16_t micro){
  // a frequency value to match the conversion in MICROSECONDS
  static const uint32_t freq = F_CPU/1000000; 
  static const uint16_t prescaler = 1024;
  static const uint16_t shifterValue = prescaler/freq;
  if(micro>=32768){
    return 0;
  }
  uint16_t ticks = micro / (shifterValue/2);
  if(ticks & 0x0001){
    // it must be ceiled
    return (ticks>>1) + 1;
  }else{
    // it must be floored
    return ticks>>1;
  }
}

/**
 * Configure the timer to be ready to be started
 */
void timerBegin(){
  // clean control registers TCCRxA and TCC2B registers
  TCCR2A = 0;
  TCCR2B = 0;

  // Reset the counter
  TCNT2 = 0;
  // enable interrupt of Output Compare A
  TIMSK2 = 0x02;
}

/**
 * Set the call started when timer triggers
 */
void timerSetCallback(void (*f)()){
    timer_callback = f;
}


/**
 * Let's start the timer: it triggers after x ticks.
 * 0 is not's accepted.
 */
bool timerStart(uint16_t tick){
    if(tick<=1){
        return false;
    }
    //Serial.println(tick);
    TCNT2 = 0;

    tick--;
    OCR2A = tick;
    // 0x07: start with prescaler 1024
    TCCR2B = 0x07;
    return true;
}

#endif // END AVR
