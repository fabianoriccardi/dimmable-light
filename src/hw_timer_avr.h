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
 * Here there is specific AVR code. AVR is usually programmed at very low level
 * than respect ESP8266/ESP32, so this file is needed to provide a minimalistic
 * "HAL" to simplify timer usage.
 ***********************************************************************************/
#ifdef AVR

#ifndef HW_TIMER_ARDUINO_H
#define HW_TIMER_ARDUINO_H

#include <stdint.h>

/**
 * convert microsecond to tick, max micro is 32767, otherwize it returns 0.
 */
uint16_t microsecond2Tick(uint16_t micro);

/**
 * Configure the timer to be started by timerStart()
 */
void timerBegin();

/**
 * Set callback function on timer triggers
 */
void timerSetCallback(void (*f)());

/**
 * Let's start the timer: it triggers after x ticks,
 * then it stops.
 * tick length depends on MCU clock and prescaler, please use
 * microsecond2Tick(..) to feed timerStart(..).
 *
 * NOTE: 0 or 1 values are not accepted
 */
void timerStartAndTrigger(uint16_t tick);

void timerSetAlarm(uint16_t tick);

void timerStop();

#endif  // HW_TIMER_ARDUINO_H

#endif  // END AVR
