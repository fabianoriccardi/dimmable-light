/******************************************************************************
 *  This file is part of Dimmable Light for Arduino, a library to control     *
 *  dimmers.                                                                  *
 *                                                                            *
 *  Copyright (C) 2018-2023  Fabiano Riccardi                                 *
 *                                                                            *
 *  Dimmable Light for Arduino is free software; you can redistribute         *
 *  it and/or modify it under the terms of the GNU Lesser General Public      *
 *  License as published by the Free Software Foundation; either              *
 *  version 2.1 of the License, or (at your option) any later version.        *
 *                                                                            *
 *  This library is distributed in the hope that it will be useful,           *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU          *
 *  Lesser General Public License for more details.                           *
 *                                                                            *
 *  You should have received a copy of the GNU Lesser General Public License  *
 *  along with this library; if not, see <http://www.gnu.org/licenses/>.      *
 ******************************************************************************/
#include "thyristor.h"
#include "circular_queue.h"
#include <Arduino.h>

#if defined(ARDUINO_ARCH_ESP8266)
#include "hw_timer_esp8266.h"
#elif defined(ARDUINO_ARCH_ESP32)
#include "hw_timer_esp32.h"
#elif defined(ARDUINO_ARCH_AVR)
#include "hw_timer_avr.h"
#elif defined(ARDUINO_ARCH_SAMD)
#include "hw_timer_samd.h"
#elif defined(ARDUINO_ARCH_RP2040) && !defined(ARDUINO_ARCH_MBED)
#include "hw_timer_pico.h"
#else
#error "only ESP8266, ESP32, AVR, SAMD & RP2040 (non-mbed) architectures are supported"
#endif

// Ignore zero-cross interrupts when they occurs too early w.r.t semi-period ideal length.
// The constant *semiPeriodShrinkMargin* defines the "too early" margin.
// This filter affects the MONITOR_FREQUENCY measurement.
//#define FILTER_INT_PERIOD

// FOR DEBUG PURPOSE ONLY. This option requires FILTER_INT_PERIOD enabled.
// Print on serial port the time passed from the previous zero cross interrupt when the semi-period
// length is exceed the interval defined by *semiPeriodShrinkMargin* and *semiPeriodExpandMargin*.
//#define PRINT_INT_PERIOD

// FOR DEBUG PURPOSE ONLY.
// Prints a char on the serial port if not all thyristors are managed in a semi-period.
//#define CHECK_MANAGED_THYR

// Force the signal length of thyristor's gate. If not enabled, the signal to gate
// is turned off through an interrupt just before the end of the period.
// Look at gateTurnOffTime constant for more info.
//#define PREDEFINED_PULSE_LENGTH

// In microseconds
#ifdef NETWORK_FREQ_FIXED_50HZ
static const uint16_t semiPeriodLength = 10000;
#endif
#ifdef NETWORK_FREQ_FIXED_60HZ
static const uint16_t semiPeriodLength = 8333;
#endif
#ifdef NETWORK_FREQ_RUNTIME
static uint16_t semiPeriodLength = 0;
#endif

// These margins are precautions against noise, electrical spikes and frequency skew errors.
// Activation delays lower than *startMargin* turn the thyristor fully ON.
// Activation delays higher than *endMargin* turn the thyristor fully OFF.
// Tune this parameters accordingly to your setup (electrical network, MCU, and ZC circuitry).
// Values are expressed in microseconds.
static const uint16_t startMargin = 200;
static const uint16_t endMargin = 500;

// This parameter represents the time span in which 2 (or more) very near delays are merged (the
// higher ones are merged in the smaller one). This could be necessary for 2 main reasons:
// 1) Efficiency, in fact in some applications you will never seem differences between
//    near delays, hence raising many interrupts is useless.
// 2) MCU inability to satisfy very tight "timer start".
// After some experiments on incandescence light bulbs, I noted that even 50 microseconds
// are not negligible, so I decided to set threshold lower than 20microsecond. Before lowering this
// value, check the documentation of the specific MCU since some have limitations. For example,
// ESP8266 API documentation suggests to set timer dealy higher than >10us. If you use 8-bit timers
// on AVR, you should set a bigger mergePeriod (e.g. 100us).
static const uint16_t mergePeriod = 20;

// Period in microseconds before the end of the semiperiod when an interrupt is triggered to
// turn off all gate signals. This parameter doesn't have any effect if you enable
// PREDEFINED_PULSE_LENGTH.
static const uint16_t gateTurnOffTime = 300;

static_assert(endMargin - gateTurnOffTime > mergePeriod, "endMargin must be greater than "
                                                         "(gateTurnOffTime + mergePeriod)");

#ifdef PREDEFINED_PULSE_LENGTH
// Length of pulse on thyristor's gate pin. This parameter is not applied if thyristor is fully on
// or off. This option is suitable only for very short pulses, since it blocks the ISR for the
// specified amount of time.
static uint8_t pulseWidth = 15;
#endif

struct PinDelay {
  uint8_t pin;
  uint16_t delay;
};

enum class INT_TYPE { ACTIVATE_THYRISTORS, TURN_OFF_GATES };

static INT_TYPE nextISR = INT_TYPE::ACTIVATE_THYRISTORS;

/**
 * Temporary struct manipulated by the ISR storing the timing information about each dimmer.
 */
static struct PinDelay pinDelay[Thyristor::N];

/**
 * Summary of thyristors' state used by ISR (concurrent-safe).
 */
static bool _allThyristorsOnOff = true;

/**
 * Tell if zero-cross interrupt is enabled.
 */
static bool interruptEnabled = false;

/**
 * Number of thyristors already managed in the current semi-period.
 */
static uint8_t thyristorManaged = 0;

/**
 * Number of thyristors FULLY on. The remaining ones must be turned
 * off by turn_off_gates_int at the end of the semi-period.
 */
static uint8_t alwaysOnCounter = 0;
static uint8_t alwaysOffCounter = 0;

#if defined(ARDUINO_ARCH_ESP8266)
void HW_TIMER_IRAM_ATTR turn_off_gates_int() {
#elif defined(ARDUINO_ARCH_ESP32)
void ARDUINO_ISR_ATTR turn_off_gates_int() {
#else
void turn_off_gates_int() {
#endif
  for (int i = alwaysOnCounter; i < Thyristor::nThyristors; i++) {
    digitalWrite(pinDelay[i].pin, LOW);
  }

#if defined(ARDUINO_ARCH_AVR)
  timerStop();
#endif
}

/**
 * Timer routine to turn on one or more thyristors. This function may be be called multiple times
 * per semi-period depending on the current thyristors configuration.
 */
#if defined(ARDUINO_ARCH_ESP8266)
void HW_TIMER_IRAM_ATTR activate_thyristors() {
#elif defined(ARDUINO_ARCH_ESP32)
void ARDUINO_ISR_ATTR activate_thyristors() {
#else
void activate_thyristors() {
#endif

  const uint8_t firstToBeUpdated = thyristorManaged;

  for (;
       // The last thyristor is managed outside the loop
       thyristorManaged < Thyristor::nThyristors - 1 &&
       // Consider the "near" thyristors
       pinDelay[thyristorManaged + 1].delay - pinDelay[firstToBeUpdated].delay < mergePeriod &&
       // Exclude the one who must remain totally off
       pinDelay[thyristorManaged].delay <= semiPeriodLength - endMargin;
       thyristorManaged++) {
    digitalWrite(pinDelay[thyristorManaged].pin, HIGH);
  }
  digitalWrite(pinDelay[thyristorManaged].pin, HIGH);
  thyristorManaged++;

  // This while is dedicated to all those thyristors with delay == semiPeriodLength-margin; those
  // are the ones who shouldn't turn on, hence they can be skipped
  while (thyristorManaged < Thyristor::nThyristors && pinDelay[thyristorManaged].delay == semiPeriodLength) {
    thyristorManaged++;
  }

#ifdef PREDEFINED_PULSE_LENGTH
  delayMicroseconds(pulseWidth);

  for (int i = firstToBeUpdated; i < thyristorManaged; i++) { digitalWrite(pinDelay[i].pin, LOW); }
#endif

  if (thyristorManaged < Thyristor::nThyristors) {
    int delayAbsolute = pinDelay[thyristorManaged].delay;

#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAMD) || (defined(ARDUINO_ARCH_RP2040) && !defined(ARDUINO_ARCH_MBED))
    int delayRelative = delayAbsolute - pinDelay[firstToBeUpdated].delay;
#endif

#if defined(ARDUINO_ARCH_ESP8266)
    timer1_write(US_TO_RTC_TIMER_TICKS(delayRelative));
#elif defined(ARDUINO_ARCH_ESP32)
    setAlarm(delayAbsolute);
#elif defined(ARDUINO_ARCH_AVR)
    timerSetAlarm(microsecond2Tick(delayRelative));
#elif defined(ARDUINO_ARCH_SAMD)
  timerStart(microsecond2Tick(delayRelative));
#elif defined(ARDUINO_ARCH_RP2040) && !defined(ARDUINO_ARCH_MBED)
  timerStart(delayRelative);
#else
  #error "Not implemented"
#endif
  } else {

#ifdef PREDEFINED_PULSE_LENGTH
    // If there are not more thyristor to serve, I can stop timer. Energy saving?
#if defined(ARDUINO_ARCH_ESP8266)
    // Given the Arduino HAL and esp8266 technical reference manual,
    // when timer triggers, the counter stops because it has reach zero
    // and no-autorealod was set (this timer can only down-count).
#elif defined(ARDUINO_ARCH_ESP32)
    stopTimer();
#elif defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAMD)
    // Given actual HAL, AVR and SAMD counter automatically stops on interrupt
#elif defined(ARDUINO_ARCH_RP2040) && !defined(ARDUINO_ARCH_MBED)
    // Timer callback is not rescheduled
#endif
#else
    // If there are not more thyristors to serve, set timer to turn off gates' signal
    uint16_t delayAbsolute = semiPeriodLength - gateTurnOffTime;

#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAMD) || (defined(ARDUINO_ARCH_RP2040) && !defined(ARDUINO_ARCH_MBED))
    uint16_t delayRelative = delayAbsolute - pinDelay[firstToBeUpdated].delay;
#endif

#if defined(ARDUINO_ARCH_ESP8266)
    timer1_attachInterrupt(turn_off_gates_int);
    timer1_write(US_TO_RTC_TIMER_TICKS(delayRelative));
#elif defined(ARDUINO_ARCH_ESP32)
    nextISR = INT_TYPE::TURN_OFF_GATES;
    setAlarm(delayAbsolute);
#elif defined(ARDUINO_ARCH_AVR)
    timerSetCallback(turn_off_gates_int);
    timerSetAlarm(microsecond2Tick(delayRelative));
#elif defined(ARDUINO_ARCH_SAMD)
    timerSetCallback(turn_off_gates_int);
    timerStart(microsecond2Tick(delayRelative));
#elif defined(ARDUINO_ARCH_RP2040) && !defined(ARDUINO_ARCH_MBED)
    timerSetCallback(turn_off_gates_int);
    timerStart(delayRelative);
#else
    #error "Not implemented"
#endif
#endif
  }
}

#ifdef FILTER_INT_PERIOD
// In microsecond
const static int semiPeriodShrinkMargin = 50;
const static int semiPeriodExpandMargin = 50;
#endif

#if defined(FILTER_INT_PERIOD) || defined(MONITOR_FREQUENCY)
static uint32_t lastTime = 0;
#endif

#ifdef MONITOR_FREQUENCY
// Circular queue to compute the moving average
static CircularQueue<uint32_t, 5> queue;
static uint32_t total = 0;
#endif

#if defined(ARDUINO_ARCH_ESP8266)
void HW_TIMER_IRAM_ATTR zero_cross_int() {
#elif defined(ARDUINO_ARCH_ESP32)
void ARDUINO_ISR_ATTR zero_cross_int() {
#else
void zero_cross_int() {
#endif

#if defined(FILTER_INT_PERIOD) || defined(MONITOR_FREQUENCY)
  if (!lastTime) {
    lastTime = micros();
  } else {
    uint32_t now = micros();

    // "diff" is correct even when timer rolls back, because these values are unsigned
    uint32_t diff = now - lastTime;

#ifdef PRINT_INT_PERIOD
    if (diff < semiPeriodLength - semiPeriodShrinkMargin) {
#ifdef ARDUINO_ARCH_ESP32
      ets_printf("B%d\n", diff);
#else
      Serial.println(String('B') + diff);
#endif
    }
    if (diff > semiPeriodLength + semiPeriodExpandMargin) {
#ifdef ARDUINO_ARCH_ESP32
      ets_printf("A%d\n", diff);
#else
      Serial.println(String('A') + diff);
#endif
    }
#endif

#ifdef FILTER_INT_PERIOD
    // Filters out spurious interrupts. The effectiveness of this simple
    // filter could vary depending on noise on electrical networ.
    if (diff < semiPeriodLength - semiPeriodShrinkMargin) { return; }
#endif

#endif

#if defined(ARDUINO_ARCH_AVR)
    // Early timer start, only for avr. This is necessary since the instructions executed in this
    // ISR take much time (more than 30us with only 4 dimmers). Before the end of this ISR, either
    // the timer is stop or the alarm time is properly set.
    timerStartAndTrigger(microsecond2Tick(15000));
#endif

#if defined(FILTER_INT_PERIOD) || defined(MONITOR_FREQUENCY)
#ifdef MONITOR_FREQUENCY
    // if diff is very very greater than the theoretical value, the electrical signal
    // can be considered as lost for a while and I must reset my moving average.
    // I decided to use "16" because is a power of 2, very fast to be computed.
    if (semiPeriodLength && diff > semiPeriodLength * 16) {
      queue.reset();
      total = 0;
    } else {
      // If filtering has passed, I can update the moving average
      uint32_t valueToRemove = queue.insert(diff);
      total += diff;
      total -= valueToRemove;
    }
#endif

    lastTime = now;
  }
#endif

  // Turn OFF all the thyristors, even if always ON.
  // This is to speed up transitions between ON to OFF state:
  // If I don't turn OFF all those thyristors, I must wait
  // a semiperiod to turn off those one.
  for (int i = 0; i < Thyristor::nThyristors; i++) { digitalWrite(pinDelay[i].pin, LOW); }

#ifdef CHECK_MANAGED_THYR
  if (thyristorManaged != Thyristor::nThyristors) {
#ifdef ARDUINO_ARCH_ESP32
    ets_printf("E%d\n", thyristorManaged);
#else
    Serial.print("E");
    Serial.println(thyristorManaged);
#endif
  }
#endif

  // Update the structures and set thresholds, if needed
  if (Thyristor::newDelayValues && !Thyristor::updatingStruct) {
    Thyristor::newDelayValues = false;
    alwaysOffCounter = 0;
    alwaysOnCounter = 0;
    for (int i = 0; i < Thyristor::nThyristors; i++) {
      pinDelay[i].pin = Thyristor::thyristors[i]->pin;
      // Rounding delays to avoid error and unexpected behavior due to
      // non-ideal thyristors and not perfect sine wave
      if (Thyristor::thyristors[i]->delay == 0) {
        alwaysOnCounter++;
        pinDelay[i].delay = 0;
      } else if (Thyristor::thyristors[i]->delay < startMargin) {
        alwaysOnCounter++;
        pinDelay[i].delay = 0;
      } else if (Thyristor::thyristors[i]->delay == semiPeriodLength) {
        alwaysOffCounter++;
        pinDelay[i].delay = semiPeriodLength;
      } else if (Thyristor::thyristors[i]->delay > semiPeriodLength - endMargin) {
        alwaysOffCounter++;
        pinDelay[i].delay = semiPeriodLength;
      } else {
        pinDelay[i].delay = Thyristor::thyristors[i]->delay;
      }
    }
    _allThyristorsOnOff = Thyristor::allThyristorsOnOff;
  }

  thyristorManaged = 0;

  // if all are on and off, I can disable the zero cross interrupt
  if (_allThyristorsOnOff) {
    for (int i = 0; i < Thyristor::nThyristors; i++) {
      if (pinDelay[i].delay == semiPeriodLength) {
        digitalWrite(pinDelay[i].pin, LOW);
      } else {
        digitalWrite(pinDelay[i].pin, HIGH);
      }
      thyristorManaged++;
    }

#if defined(MONITOR_FREQUENCY)
    if (!Thyristor::frequencyMonitorAlwaysEnabled) {
      interruptEnabled = false;
      detachInterrupt(digitalPinToInterrupt(Thyristor::syncPin));

      queue.reset();
      total = 0;

      lastTime = 0;
    }
#elif defined(FILTER_INT_MONITOR)
    lastTime = 0;
    interruptEnabled = false;
    detachInterrupt(digitalPinToInterrupt(Thyristor::syncPin));
#else
    interruptEnabled = false;
    detachInterrupt(digitalPinToInterrupt(Thyristor::syncPin));
#endif

    return;
  }

  // Turn on thyristors with 0 delay (always on)
  while (thyristorManaged < Thyristor::nThyristors && pinDelay[thyristorManaged].delay == 0) {
    digitalWrite(pinDelay[thyristorManaged].pin, HIGH);
    thyristorManaged++;
  }

  // This block of code is inteded to manage the case near to the next semi-period:
  // In this case we should avoid to trigger the timer, because the effective semiperiod
  // perceived by the esp8266 could be less than 10000microsecond. This could be due to
  // the relative time (there is no possibily to set the timer to an absolute time)
  // Moreover, it is impossible to disable an interrupt once it is armed, neither
  // change the callback function.
  // NOTE: don't know why, but the timer seem trigger even when it is not set...
  // so a provvisory solution if to set the relative callback to NULL!
  // NOTE 2: this improvement should be think even for multiple lamp!
  if (thyristorManaged < Thyristor::nThyristors && pinDelay[thyristorManaged].delay < semiPeriodLength) {
    uint16_t delayAbsolute = pinDelay[thyristorManaged].delay;
#if defined(ARDUINO_ARCH_ESP8266)
    timer1_attachInterrupt(activate_thyristors);
    timer1_write(US_TO_RTC_TIMER_TICKS(delayAbsolute));
#elif defined(ARDUINO_ARCH_ESP32)
    // setCallback(activate_thyristors);
    nextISR = INT_TYPE::ACTIVATE_THYRISTORS;
    startTimerAndTrigger(delayAbsolute);
#elif defined(ARDUINO_ARCH_AVR)
    timerSetCallback(activate_thyristors);
    timerSetAlarm(microsecond2Tick(delayAbsolute));
#elif defined(ARDUINO_ARCH_SAMD)
  timerSetCallback(activate_thyristors);
  timerStart(microsecond2Tick(delayAbsolute));
#elif defined(ARDUINO_ARCH_RP2040) && !defined(ARDUINO_ARCH_MBED)
  timerSetCallback(activate_thyristors);
  timerStart(pinDelay[thyristorManaged].delay);
#else
  # error "Not implemented"
#endif
  } else {

    // This while is dedicated to all those thyristor wih delay == semiPeriodLength-margin; those
    // are the ones who shouldn't turn on, hence they can be skipped
    while (thyristorManaged < Thyristor::nThyristors && pinDelay[thyristorManaged].delay == semiPeriodLength) {
      thyristorManaged++;
    }

#if defined(ARDUINO_ARCH_ESP8266)
    // Given the Arduino HAL and esp8266 technical reference manual,
    // when timer triggers, the counter stops because it has reached zero
    // and no-autorealod was set (this timer can only down-count).
#elif defined(ARDUINO_ARCH_ESP32)
    stopTimer();
#elif defined(ARDUINO_ARCH_AVR)
    timerStop();
#elif defined(ARDUINO_ARCH_SAMD)
  // Given actual HAL, and SAMD counter automatically stops on interrupt
#elif defined(ARDUINO_ARCH_RP2040) && !defined(ARDUINO_ARCH_MBED)
    // Timer callback is not rescheduled
#endif
  }
}

#if defined(ARDUINO_ARCH_ESP8266)
void HW_TIMER_IRAM_ATTR isr_selector() {
#elif defined(ARDUINO_ARCH_ESP32)
void ARDUINO_ISR_ATTR isr_selector() {
#else
void isr_selector() {
#endif
  if (nextISR == INT_TYPE::ACTIVATE_THYRISTORS) {
    activate_thyristors();
  } else if (nextISR == INT_TYPE::TURN_OFF_GATES) {
    turn_off_gates_int();
  }
}

void Thyristor::setDelay(uint16_t newDelay) {
  if (verbosity > 2) {
    for (int i = 0; i < Thyristor::nThyristors; i++) {
      Serial.print(String("setB: ") + "posIntoArray:" + thyristors[i]->posIntoArray
                   + " pin:" + thyristors[i]->pin);
      Serial.print(" ");
      Serial.println(thyristors[i]->delay);
    }
  }

  if (newDelay > semiPeriodLength) { newDelay = semiPeriodLength; }

  // Reorder the array to speed up the interrupt.
  // This mini-algorithm works on a different memory area w.r.t. the ISR,
  // so it is concurrent-safe

  updatingStruct = true;
  // Array example, it is always ordered, higher values means lower brightness levels
  // [45,678,5000,7500,9000]
  if (newDelay > delay) {
    if (verbosity > 2) Serial.println("\tlowering the light..");
    bool done = false;
    /////////////////////////////////////////////////////////////////
    // Let's find the new position
    int i = posIntoArray + 1;
    while (i < nThyristors && !done) {
      if (newDelay <= thyristors[i]->delay) {
        done = true;
      } else {
        i++;
      }
    }
    // This could be due to 2 reasons:
    // 1) the light is already the lowest delay (i.e. turned off)
    // 2) the delay is not changed to overpass the neightbour
    if (posIntoArray + 1 == i) {
      if (verbosity > 2) Serial.println("No need to shift..");
    } else {
      int target;
      // Means that we have reached the end, the target i the last element
      if (i == nThyristors) {
        target = nThyristors - 1;
      } else {
        target = i - 1;
      }

      // Let's shift
      for (int i = posIntoArray; i < target; i++) {
        thyristors[i] = thyristors[i + 1];
        thyristors[i]->posIntoArray = i;
      }
      thyristors[target] = this;
      this->posIntoArray = target;
    }
  } else if (newDelay < delay) {
    if (verbosity > 2) Serial.println("\traising the light..");
    bool done = false;
    int i = posIntoArray - 1;
    while (i >= 0 && !done) {
      if (newDelay >= thyristors[i]->delay) {
        done = true;
      } else {
        i--;
      }
    }
    if (posIntoArray - 1 == i) {
      if (verbosity > 2) Serial.println("No need to shift..");
    } else {
      int target;
      // Means that we have reached the start, the target is the first element
      if (!done) {
        target = 0;
      } else {
        target = i + 1;
      }

      // Let's shift
      for (int i = posIntoArray; i > target; i--) {
        thyristors[i] = thyristors[i - 1];
        thyristors[i]->posIntoArray = i;
      }
      thyristors[target] = this;
      this->posIntoArray = target;
    }
  } else {
    if (verbosity > 2)
      Serial.println("Warning: you are setting the same delay as the previous one!");
    updatingStruct = false;
    return;
  }

  delay = newDelay;
  bool enableInt = mustInterruptBeReEnabled(newDelay);
  newDelayValues = true;
  updatingStruct = false;
  if (enableInt) {
    if (verbosity > 2) Serial.println("Re-enabling interrupt");
    interruptEnabled = true;
    attachInterrupt(digitalPinToInterrupt(syncPin), zero_cross_int, syncDir);
  }

  if (verbosity > 2) {
    for (int i = 0; i < Thyristor::nThyristors; i++) {
      Serial.print(String("\tsetB: ") + "posIntoArray:" + thyristors[i]->posIntoArray
                   + " pin:" + thyristors[i]->pin);
      Serial.print(" ");
      Serial.println(thyristors[i]->delay);
    }
  }
}

void Thyristor::turnOn() {
  setDelay(semiPeriodLength);
}

void Thyristor::begin() {
  pinMode(syncPin, syncPullup ? INPUT_PULLUP : INPUT);

#if defined(ARDUINO_ARCH_ESP8266)
  timer1_attachInterrupt(activate_thyristors);
  // These 2 registers assignments are the "unrolling" of:
  // timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE);
  T1C = (1 << TCTE) | ((TIM_DIV16 & 3) << TCPD) | ((TIM_EDGE & 1) << TCIT) | ((TIM_SINGLE & 1) << TCAR);
  T1I = 0;
#elif defined(ARDUINO_ARCH_ESP32)
  timerInit(isr_selector);
#elif defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAMD) || (defined(ARDUINO_ARCH_RP2040) && !defined(ARDUINO_ARCH_MBED))
  timerSetCallback(activate_thyristors);
  timerBegin();
#else
  #error "Not implemented"
#endif

#ifdef MONITOR_FREQUENCY
  // Starts immediately to sense the eletricity grid

  interruptEnabled = true;
  attachInterrupt(digitalPinToInterrupt(syncPin), zero_cross_int, syncDir);
#endif
}

float Thyristor::getFrequency() {
  if (semiPeriodLength == 0) { return 0; }
  return 1000000 / 2 / (float)(semiPeriodLength);
}

uint16_t Thyristor::getSemiPeriod() {
  return semiPeriodLength;
}

#ifdef NETWORK_FREQ_RUNTIME
void Thyristor::setFrequency(float frequency) {
  if (frequency < 0) { return; }

  if (frequency == 0) {
    semiPeriodLength = 0;
    return;
  }

  semiPeriodLength = 1000000 / 2 / frequency;
}
#endif

#ifdef MONITOR_FREQUENCY
float Thyristor::getDetectedFrequency() {
  int c;
  uint32_t tot;
  {
    // Stop interrupt to freeze variables modified or accessed in the interrupt
    noInterrupts();

    // "diff" is correct even when rolling back, because all of them are unsigned
    uint32_t diff = micros() - lastTime;

    // if diff is very very greater than the theoretical value, the electrical signal
    // can be considered as lost for a while.
    // I decided to use "16" because is a power of 2, very fast to be computed.
    if (semiPeriodLength && diff > semiPeriodLength * 16) {
      queue.reset();
      total = 0;
    }

    c = queue.getCount();
    tot = total;
    interrupts();
  }

  // We need at least a sample to return a value differnt from 0
  if (tot > 0) {
    // *1000000: us
    // /2: from semiperiod to full period
    float result = c * 1000000 / 2 / ((float)(tot));
    return result;
  }
  return 0;
}

void Thyristor::frequencyMonitorAlwaysOn(bool enable) {
  {
    // Stop interrupt to freeze variables modified or accessed in the interrupt
    noInterrupts();

    if (enable && !interruptEnabled) {
      interruptEnabled = true;
      attachInterrupt(digitalPinToInterrupt(syncPin), zero_cross_int, syncDir);
    }
    frequencyMonitorAlwaysEnabled = enable;

    interrupts();
  }
}
#endif

Thyristor::Thyristor(int pin) : pin(pin), delay(semiPeriodLength) {
  if (nThyristors < N) {
    pinMode(pin, OUTPUT);

    updatingStruct = true;

    posIntoArray = nThyristors;
    nThyristors++;
    thyristors[posIntoArray] = this;

    // Full reorder of the array
    for (int i = 0; i < nThyristors; i++) {
      for (int j = i + 1; j < nThyristors - 1; j++) {
        if (thyristors[i]->delay > thyristors[j]->delay) {
          Thyristor* temp = thyristors[i];
          thyristors[i] = thyristors[j];
          thyristors[j] = temp;
        }
      }
    }
    // Set the posIntoArray with a "brutal" assignement to each Thyristor
    for (int i = 0; i < nThyristors; i++) { thyristors[i]->posIntoArray = i; }

    newDelayValues = true;
    updatingStruct = false;
  } else {
    // TODO return error or exception
  }
}

Thyristor::~Thyristor() {
  // Recompact the array
  updatingStruct = true;
  nThyristors--;
  // TODO remove light from the static pinDelay array, and shrink the array
  updatingStruct = false;
}

bool Thyristor::areThyristorsOnOff() {
  bool allOnOff = true;
  int i = 0;
  while (i < nThyristors && allOnOff) {
    if (thyristors[i]->getDelay() != 0 && thyristors[i]->getDelay() != semiPeriodLength) {
      allOnOff = false;
    } else {
      i++;
    }
  }
  return allOnOff;
}

bool Thyristor::mustInterruptBeReEnabled(uint16_t newDelay) {
  bool interruptMustBeEnabled = true;

  // Temp values those are "commited" at the end of this method
  bool newAllThyristorsOnOff = allThyristorsOnOff;

  if (newDelay == semiPeriodLength || newDelay == 0) {
    newAllThyristorsOnOff = areThyristorsOnOff();
  } else {
    // if newDelay is not optimizable i.e. a value between (0; semiPeriodLength)
    newAllThyristorsOnOff = false;
  }

  allThyristorsOnOff = newAllThyristorsOnOff;
  if (verbosity > 1) Serial.println(String("allThyristorsOnOff: ") + allThyristorsOnOff);
  return !interruptEnabled && interruptMustBeEnabled;
}

uint8_t Thyristor::nThyristors = 0;
Thyristor* Thyristor::thyristors[Thyristor::N] = { nullptr };
bool Thyristor::newDelayValues = false;
bool Thyristor::updatingStruct = false;
bool Thyristor::allThyristorsOnOff = true;
uint8_t Thyristor::syncPin = 255;
decltype(RISING) Thyristor::syncDir = RISING;
bool Thyristor::syncPullup = false;
bool Thyristor::frequencyMonitorAlwaysEnabled = true;
