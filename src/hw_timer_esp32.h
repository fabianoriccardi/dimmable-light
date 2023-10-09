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
#ifndef HW_TIMER_ESP32_H
#define HW_TIMER_ESP32_H

#include <Arduino.h>

void timerInit(void (*callback)());

void setCallback(void (*callback)());

void startTimerAndTrigger(uint32_t delay);

void stopTimer();

#endif  // END HW_TIMER_ESP32_H