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

#if defined(ARDUINO_ARCH_RP2040) && !defined(ARDUINO_ARCH_MBED)

#include "hw_timer_pico.h"
#include <Arduino.h>

static void (*timer_callback)() = nullptr;
static alarm_id_t alarm_id;
static alarm_pool_t *alarm_pool;

void timerBegin() {
  alarm_pool = alarm_pool_get_default();
}

void timerSetCallback(void (*callback)()) {
  timer_callback = callback;
}

void timerStart(uint64_t t) {
  if (alarm_id) {
    cancel_alarm(alarm_id);
    alarm_id = 0;
  }

  alarm_id = alarm_pool_add_alarm_in_us(
    alarm_pool, t,
    [](alarm_id_t, void *) -> int64_t {
      if (timer_callback != nullptr) { timer_callback(); }
      alarm_id = 0;
      return 0;  // Do not reschedule alarm
    },
    NULL, true);
}

#endif  // END ARDUINO_ARCH_RP2040
