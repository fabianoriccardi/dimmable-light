/******************************************************************************
 *  This file is part of Dimmable Light for Arduino, a library to control     *
 *  dimmers.                                                                  *
 *                                                                            *
 *  Copyright (C) 2023  Adam Hoese                                            *
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

#ifndef HW_TIMER_PICO_H
#define HW_TIMER_PICO_H

#include <stdint.h>

/**
 * Initialize the timer.
 */
void timerBegin();

/**
 * Set callback function on timer triggers
 */
void timerSetCallback(void (*callback)());

/**
 * Start the timer to trigger after the specified number of microseconds.
 */
void timerStart(uint64_t t);

#endif  // HW_TIMER_PICO_H

#endif  // ARDUINO_ARCH_RP2040
