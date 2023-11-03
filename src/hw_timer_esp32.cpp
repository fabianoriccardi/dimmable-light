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
#ifdef ESP32

#include "hw_timer_esp32.h"

const static int TIMER_ID = 0;

static hw_timer_t* timer = nullptr;

void timerInit(void (*callback)()) {
  // Use 1st timer of 4 (counted from zero).
  // Set 80 divider for prescaler (see ESP32 Technical Reference Manual for more
  // info), count up. The counter starts to increase its value.
  timer = timerBegin(TIMER_ID, 80, true);
  timerStop(timer);
  timerWrite(timer, 0);

  timerAttachInterrupt(timer, callback, false);
}

void ARDUINO_ISR_ATTR startTimerAndTrigger(uint32_t delay) {
  timerWrite(timer, 0);
  timerAlarmWrite(timer, delay, false);
  timerAlarmEnable(timer);
  timerStart(timer);
}

void ARDUINO_ISR_ATTR setAlarm(uint32_t delay) {
  timerAlarmWrite(timer, delay, false);

  // On core v2.0.0-2.0.1, the timer alarm is automatically disabled after triggering,
  // so re-enable the alarm
  timerAlarmEnable(timer);
}

void ARDUINO_ISR_ATTR stopTimer() {
  timerStop(timer);
}

#endif  // END ESP32
