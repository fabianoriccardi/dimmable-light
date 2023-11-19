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

#ifndef HW_TIMER_SAMD_H
#define HW_TIMER_SAMD_H

#include <stdint.h>

/**
 * Convert from microsecond to tick.
 * Max microseconds value is 10000, for higher values it returns 0.
 */
uint16_t microsecond2Tick(uint16_t micro);

/**
 * Initialize the timer.
 */
void timerBegin();

/**
 * Set callback function on timer triggers
 */
void timerSetCallback(void (*callback)());

/**
 * Start the timer to trigger after the specified number of ticks.
 *
 * NOTE: 0 or 1 values are not accepted
 */
void timerStart(uint16_t tick);

#endif  // HW_TIMER_SAMD_H

#endif  // ARDUINO_ARCH_SAMD
