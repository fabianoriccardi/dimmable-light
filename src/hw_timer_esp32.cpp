/***************************************************************************
 *   This file is part of Dimmable Light for Arduino, a library to         *
 *   control dimmers.                                                      *
 *                                                                         *
 *   Copyright (C) 2018-2022  Fabiano Riccardi                             *
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
#ifdef ESP32

#include "hw_timer_esp32.h"

#define TIMER_ID 0

static hw_timer_t* timer = nullptr;

void timerInit(void (*callback)()) {
  // Use 1st timer of 4 (counted from zero).
  // Set 80 divider for prescaler (see ESP32 Technical Reference Manual for more
  // info), count up. The counter starts to increase its value.
  timer = timerBegin(TIMER_ID, 80, true);
  if (timer == nullptr) { Serial.println("Timer error"); }
  // Attach onTimer function to our timer.
  timerAttachInterrupt(timer, callback, true);

  timerAlarmWrite(timer, 1000000, true);
  timerAlarmEnable(timer);
}

void IRAM_ATTR startTimerAndTrigger(uint32_t delay) {
  timerWrite(timer, 0);
  timerAlarmWrite(timer, delay, true);
  timerStart(timer);
}

void IRAM_ATTR stopTimer() {
  timerStop(timer);
}

#endif  // END ESP32
