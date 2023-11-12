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
#ifdef AVR

#include "hw_timer_avr.h"
#include <util/atomic.h>
#include <Arduino.h>

/**
 * This parameter controls the timer used by this library. Timer0 is used by Arduino core, so you
 * shouldn't use it. The remaining timers are 8-bits or 16-bits. From my experience with
 * incandescence bulbs, I have observed sharp steps in brightness when using 8-bits timers, so I
 * decided to set as default the first available 16-bit timer: the *1*.
 *
 * Free timers ID:
 * - [1;2] on Arduino Uno (ATmega328P)
 * - [1;5] on Arduino Mega (ATmega2560)
 */
#define TIMER_ID 1

#if TIMER_ID == 0 || TIMER_ID == 2
#define N_BIT_TIMER 8
#else
#define N_BIT_TIMER 16
#endif

// Some helpful macros to support different timers
#define _TCCRxA(X)             TCCR##X##A
#define TCCRxA(X)              _TCCRxA(X)
#define _TCCRxB(X)             TCCR##X##B
#define TCCRxB(X)              _TCCRxB(X)
#define _TIMSKx(X)             TIMSK##X
#define TIMSKx(X)              _TIMSKx(X)
#define _OCIExA(X)             OCIE##X##A
#define OCIExA(X)              _OCIExA(X)
#define _TCNTxL(X)             TCNT##X##L
#define TCNTxL(X)              _TCNTxL(X)
#define _TCNTxH(X)             TCNT##X##H
#define TCNTxH(X)              _TCNTxH(X)
#define _TCNTx(X)              TCNT##X
#define TCNTx(X)               _TCNTx(X)
#define _OCRxAH(X)             OCR##X##AH
#define OCRxAH(X)              _OCRxAH(X)
#define _OCRxAL(X)             OCR##X##AL
#define OCRxAL(X)              _OCRxAL(X)
#define _OCRxA(X)              OCR##X##A
#define OCRxA(X)               _OCRxA(X)

#define _TIMER_COMPA_VECTOR(X) TIMER##X##_COMPA_vect
#define TIMER_COMPA_VECTOR(X)  _TIMER_COMPA_VECTOR(X)

static void (*timer_callback)() = nullptr;

ISR(TIMER_COMPA_VECTOR(TIMER_ID)) {
  // Disable interrupt of Output Compare A
  TIMSKx(TIMER_ID) &= 0b11111101;

  if (timer_callback != nullptr) { timer_callback(); }
}

uint16_t microsecond2Tick(uint16_t micro) {
  // a frequency value to match the conversion in MICROSECONDS
  static const uint32_t freq = F_CPU / 1000000;
#if N_BIT_TIMER == 8
  static const uint16_t prescaler = 1024;
#elif N_BIT_TIMER == 16
  static const uint16_t prescaler = 8;
#endif
  static_assert((((uint32_t)1 << N_BIT_TIMER) - 1) / ((float)F_CPU / prescaler) * 1000000 > 10000,
                "the timer configuration has to allows to store a time value greater than 10000 "
                "(microseconds)");
  static const uint16_t shifterValue = F_CPU / prescaler < 1000000 ? prescaler / freq : freq / prescaler;

  if (micro >= 32768) { return 0; }

  // Optimized integer division(with rounding) and multiplication.
  uint16_t ticks;
  if (F_CPU / prescaler < 1000000) {
    ticks = micro / (shifterValue / 2);
    if (ticks & 0x0001) {
      // it must be ceiled
      return (ticks >> 1) + 1;
    } else {
      // it must be floored
      return ticks >> 1;
    }
  } else if (F_CPU / prescaler > 1000000) {
    return micro * shifterValue;
  } else {
    return micro;
  }
}

void timerBegin() {
  // clean control registers TCCRxA and TCC2B registers
  TCCRxA(TIMER_ID) = 0;
  // Set CTC mode
  TCCRxB(TIMER_ID) = 0x08;

  // Reset the counter
  // From the AVR datasheet: "To do a 16-bit write, the high byte must be written
  // before the low byte. For a 16-bit read, the low byte must be read
  // before the high byte".
#if N_BIT_TIMER == 8
  TCNTx(TIMER_ID) = 0;
#elif N_BIT_TIMER == 16
  TCNTxH(TIMER_ID) = 0;
  TCNTxL(TIMER_ID) = 0;
#endif
}

void timerSetCallback(void (*f)()) {
  timer_callback = f;
}

void timerStartAndTrigger(uint16_t tick) {
  timerStop();

#if N_BIT_TIMER == 8
  TCNTx(TIMER_ID) = 0;
#elif N_BIT_TIMER == 16
  TCNTxH(TIMER_ID) = 0;
  TCNTxL(TIMER_ID) = 0;
#endif

  tick--;
#if N_BIT_TIMER == 8
  OCRxA(TIMER_ID) = tick;
#elif N_BIT_TIMER == 16
  OCRxAH(TIMER_ID) = tick >> 8;
  OCRxAL(TIMER_ID) = tick;
#endif

#if N_BIT_TIMER == 8
  // 0x07: start counter with prescaler 1024
  TCCRxB(TIMER_ID) = 0x07;
#elif N_BIT_TIMER == 16
  // 0x02: start counter with prescaler 8
  TCCRxB(TIMER_ID) |= 0x02;
#endif

  // enable interrupt of Output Compare A
  TIMSKx(TIMER_ID) = 1 << OCIExA(TIMER_ID);
}

void timerSetAlarm(uint16_t tick) {
#if N_BIT_TIMER == 8
  OCRxA(TIMER_ID) = tick;
#elif N_BIT_TIMER == 16
  OCRxAH(TIMER_ID) = tick >> 8;
  OCRxAL(TIMER_ID) = tick;
#endif

  // enable interrupt of Output Compare A
  TIMSKx(TIMER_ID) = 1 << OCIExA(TIMER_ID);
}

void timerStop() {
  TCCRxB(TIMER_ID) &= 0b11111000;
}

#endif  // END AVR
