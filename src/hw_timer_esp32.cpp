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
#ifdef ESP32

#include "hw_timer_esp32.h"

#define TIMER_ID 0

static hw_timer_t* timer = nullptr;

void timerInit(void (*callback)()){
    // Use 1st timer of 4 (counted from zero).
    // Set 80 divider for prescaler (see ESP32 Technical Reference Manual for more
    // info), count up. The counter starts to increase its value.
    timer = timerBegin(TIMER_ID, 80, true);
    // Attach onTimer function to our timer.
    timerAttachInterrupt(timer, callback, true);
}

void IRAM_ATTR setCallback(void (*callback)()){
    // Third parameter stands for "edge" (true) and "level" (false)
    timerAttachInterrupt(timer, callback, true);
}

void IRAM_ATTR startTimerAndTrigger(uint32_t delay){
    //timeStop(timer);
    timer->dev->config.enable = 0;

    // timerWrite(timer, 0);
    timer->dev->load_high = 0;
    timer->dev->load_low = 0;
    timer->dev->reload = 1;

    // Set alarm to call onTimer function "delay" microsecond.
    // Repeat the alarm (third parameter)
    //timerAlarmWrite(timer, delay, true);
    timer->dev->alarm_high = 0;
    timer->dev->alarm_low = delay;
    // Reload the counter, but do not stop the counting
    timer->dev->config.autoreload = 0;

    // Start an alarm
    //timerAlarmEnable(timer);
    timer->dev->config.alarm_en = 1;

    // Start timer 
    //timerStart(timer);
    timer->dev->config.enable = 1;
}

void IRAM_ATTR stopTimer(){
    timer->dev->config.enable = 0;
}

#endif // END ESP32
