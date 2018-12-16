/***************************************************************************
 *   Copyright (C) 2018 by Fabiano Riccardi                                *
 *                                                                         *
 *   This file is part of Dimmable Light for ESP8266.                      *
 *                                                                         *
 *   Dimmable Light for Arduino is free software; you can redistribute     *
 *   it and/or modify it under the terms of the GNU General Public         *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   As a special exception, if other files instantiate templates or use   *
 *   macros or inline functions from this file, or you compile this file   *
 *   and link it with other works to produce a work based on this file,    *
 *   this file does not by itself cause the resulting work to be covered   *
 *   by the GNU General Public License. However the source code for this   *
 *   file must still be made available in accordance with the GNU General  *
 *   Public License. This exception does not invalidate any other reasons  *
 *   why a work based on this file might be covered by the GNU General     *
 *   Public License.                                                       *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>   *
 ***************************************************************************/
#ifndef THYRISTOR_H
#define THYRISTOR_H

#include "Arduino.h"

/**
 * This is the developer-accessible Thryristor class.
 * 
 * NOTE for programmers: the class should keep a static array with a delay value
 *          per object (that is a thyristor).
 */
class Thyristor{
  public:
    Thyristor(int pin);
    Thyristor(Thyristor const &) = delete; 
    void operator=(Thyristor const &t) = delete;

    /**
     * Set the delay, 10000 (ms, with 50Hz voltage) to turn off the thyristor
     */
    void setDelay(uint16_t d);

    /**
     * Return the current delay
     */
    uint16_t getDelay(){
      return delay;
    }

    /**
     * Turn off the thyristor
     */
    void turnOff(){
      setDelay(0);
    }

    ~Thyristor();

  /**
   * Set the timer and the interrupt routine
   * Actually this function doesn't do nothing the first thryristor is created 
   */
  static void begin();

  /**
   * Return the number of instantiated lights
   */
  static uint8_t getThyristorNumber(){
    return nThyristors;
  };

  /**
   * Set the pin dedicated to receive the AC zero cross signal
   */
  static void setSyncPin(uint8_t pin){
    syncPin = pin;
  }

  static const uint8_t N = 8;
  private:
  static uint8_t nThyristors;
  static Thyristor* thyristors[];

  /**
   * Variable to tell the interrupt routine to update its internal structures
   */
  static bool newDelayValues;
  static bool updatingStruct;

  static uint8_t syncPin;

  static const uint8_t verbosity = 2;

  uint8_t pin;
  
  /**
   * Position into the static array, this is used to speed up the research 
   * operation while setting the new brightness value.
   */
  uint8_t posIntoArray;
  
  /**
   * delay to wait before turn on the thryristor
   */
  uint16_t delay;

  friend void activateThyristors();
  friend void zero_cross_int();
};

#endif // END THYRISTOR_H