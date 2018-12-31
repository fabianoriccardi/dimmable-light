/***************************************************************************
 *   Copyright (C) 2018 by Fabiano Riccardi                                *
 *                                                                         *
 *   This file is part of Dimmable Light for ESP.                          *
 *                                                                         *
 *   Dimmable Light for ESP is free software; you can redistribute         *
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

/***********************************************************************************
 * Here there is specific AVR code. AVR is usually programmed at very low level
 * than respect ESP8266/ESP32, so this file is needed to provide a minimum "HAL"
 * to simply timer usage
 ***********************************************************************************/
#ifdef AVR

#ifndef HW_TIMER_ARDUINO_H
#define HW_TIMER_ARDUINO_H

/**
 * convert microsecond to tick, max micro is 32767, otherwize it returns 0.
 */
uint16_t microsecond2Tick(uint16_t micro);

/**
 * Configure the timer to be ready to be started
 */
void timerBegin();

/**
 * Set the call staret when timer triggers
 */
void timerSetCallback(void (*f)());

/**
 * Let's start the timer: it triggers after x ticks.
 * 0 and 1 are not's accepted.
 */
bool timerStart(uint16_t tick);

#endif // HW_TIMER_ARDUINO_H

#endif // END AVR