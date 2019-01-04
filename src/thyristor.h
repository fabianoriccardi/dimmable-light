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
#ifndef THYRISTOR_H
#define THYRISTOR_H

#include "Arduino.h"

/**
 * This is the developer-oriented Thyristor class. 
 * 
 * NOTE Class Design Principle: The concept of Thyristor is agnostic 
 * with respect to appliance controlled, hence measurement unit 
 * to regulate the power should be also appliance-agnostic. Moreover,
 * I decided to separate the code in 2 level of classes: 
 * a low level one (appliance-agnostic), and a higher level one for final 
 * user (a nice appliance-dependent Wrapper, e.g. Dimmable Light). Hence, 
 * about this class:
 * 1) the control method is called setDelay(..) and not, for example, setPower(..), 
 *      setBrightness(..),... giving a precise idea of what's happening in 
 *      electrical world.
 * 2) time in microsecond (allowing a fine control, often exceeding the real need),
 *      to match the notion of time given by setDelay()
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