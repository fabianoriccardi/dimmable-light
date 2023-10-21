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

/***********************************************************************************
 * Here there is specific SAMD code. SAMD21 is usually programmed at register level
 * so this file is needed to provide a minimalistic "HAL" to simplify timer usage.
 ***********************************************************************************/

#ifdef ARDUINO_ARCH_SAMD

#include "hw_timer_samd.h"
#include <Arduino.h>

// Supported timer: 3,4,5,... (NOTE: the one named as TC and not TCC).
// TC and TCC share the enumeration, where TCCs start from 0 and TCs
// follow up
#define TIMER_ID 3

#if TIMER_ID <= 2
#error "TIMER_ID must be between [3;7]"
#endif

// Some helpful macros to support different timers
#define _TCx(X)         TC##X
#define TCx(X)          _TCx(X)
#define _TCx_Handler(X) TC##X##_Handler
#define TCx_Handler(X)  _TCx_Handler(X)
#define _TCx_IRQn(X)    TC##X##_IRQn
#define TCx_IRQn(X)     _TCx_IRQn(X)
#define _TCx_(X)        TC##X##_
#define TCx_(X)         _TCx_(X)

#if TIMER_ID == 3
#define GCLK_CLKCTRL_ID_x GCLK_CLKCTRL_ID_TCC2_TC3
#elif TIMER_ID == 4 || TIMER_ID == 5
#define GCLK_CLKCTRL_ID_x GCLK_CLKCTRL_ID_TC4_TC5
#elif TIMER_ID == 6 || TIMER_ID == 7
#define GCLK_CLKCTRL_ID_x GCLK_CLKCTRL_ID_TC6_TC7
#endif

static void (*timer_callback)() = nullptr;

void TCx_Handler(TIMER_ID)() {
  TCx(TIMER_ID)->COUNT16.CTRLA.bit.ENABLE = 0;
  // Wait until TC3 is enabled
  while (TCx(TIMER_ID)->COUNT16.STATUS.bit.SYNCBUSY == 1)
    ;

  TCx(TIMER_ID)->COUNT16.INTFLAG.bit.MC0 = 1;

  timer_callback();
}

uint16_t microsecond2Tick(uint16_t micro) {
  // Source clock / prescaler (NOTE that multiple prescaler can be chained)
  static const uint32_t OSC8M_FREQ = 8000000;
  static const uint32_t baseFreq = OSC8M_FREQ / 2;
  static const uint16_t baseFreqForMicro = baseFreq / 1000000;
  if (micro > 10000) { return 0; }
  return baseFreqForMicro * micro;
}

void timerBegin() {
  // enable 8Mhz clock, prescaler to 0
  SYSCTRL->OSC8M.bit.PRESC = 0;
  SYSCTRL->OSC8M.reg |= SYSCTRL_OSC8M_ENABLE;

  // Configure Generic Clock Controller
  // Configure asynchronous clock source
  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_x;        // select TCx peripheral channel
  GCLK->CLKCTRL.reg |= GCLK_CLKCTRL_GEN_GCLK7;  // select source GCLK_GEN[0]
  GCLK->CLKCTRL.bit.CLKEN = 1;                  // enable TCx generic clock

  GCLK->GENCTRL.bit.SRC = GCLK_GENCTRL_SRC_OSC8M_Val;  // 0x06 OSC8M oscillator output, High
                                                       // accuracy 8Mhz clock
  GCLK->GENCTRL.bit.ID = 0x07;                         // select GCLK_GEN[7]
  GCLK->GENCTRL.bit.GENEN = 1;                         // enable generator

  GCLK->GENDIV.bit.ID = 0x07;  // select GCLK_GEN[7]
  GCLK->GENDIV.bit.DIV = 0;    // write no prescaler

  // Power Manager, usually peripheral are disabled on power reset!
  PM->APBCSEL.bit.APBCDIV = 0;          // no prescaler
  PM->APBCMASK.bit.TCx_(TIMER_ID) = 1;  // enable TCx interface

  TCx(TIMER_ID)->COUNT16.CTRLA.bit.MODE = 0;  // Configure Count Mode (16-bit)
  TCx(TIMER_ID)->COUNT16.CTRLA.bit.PRESCALER = TC_CTRLA_PRESCALER_DIV2_Val;  // Configure Prescaler
                                                                             // for divide by 2
  TCx(TIMER_ID)->COUNT16.CTRLBCLR.bit.DIR = 1;

  TCx(TIMER_ID)->COUNT16.CTRLC.bit.CPTEN0 = 0;
  TCx(TIMER_ID)->COUNT16.INTENSET.bit.MC0 = 1;  // Enable TCx compare mode interrupt generation //
                                                // Enable match interrupts on compare channel 0
  TCx(TIMER_ID)->COUNT16.CC[0].reg = 0;         // Initialize the compare register

  NVIC_EnableIRQ(TCx_IRQn(TIMER_ID));  // Enable TCx NVIC Interrupt Line
}

void timerSetCallback(void (*callback)()) {
  timer_callback = callback;
}

void timerStart(uint16_t t) {
  if (t <= 1) { return; }

  TCx(TIMER_ID)->COUNT16.COUNT.reg = 0;
  while (TCx(TIMER_ID)->COUNT16.STATUS.bit.SYNCBUSY == 1)
    ;

  TCx(TIMER_ID)->COUNT16.CC[0].reg = t;
  while (TCx(TIMER_ID)->COUNT16.STATUS.bit.SYNCBUSY == 1)
    ;

  TCx(TIMER_ID)->COUNT16.CTRLA.bit.ENABLE = 1;
  // Wait until Timer is enabled
  while (TCx(TIMER_ID)->COUNT16.STATUS.bit.SYNCBUSY == 1)
    ;
}

#endif  // END ARDUINO_ARCH_SAMD