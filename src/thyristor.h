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

#include <stdint.h>

/**
 * These defines affect the declaration of this class and the relative wrappers.
 */

// Set the frequecy selection way. The first 2 are fixed at compile time,
// while the third method allows you set whichever value at runtime.
// If FIXED, setFrequency() is not available.
#define NETWORK_FREQ_FIXED_50HZ
//#define NETWORK_FREQ_FIXED_60HZ
//#define NETWORK_FREQ_RUNTIME

// If enabled, a constant monitoring of the electrical network frequency is performed.
// If not enabled, getDetectedFrequency() is not available.
//#define MONITOR_FREQUENCY


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
  void setDelay(uint16_t delay);

  /**
   * Return the current delay.
   */
  uint16_t getDelay() const{
    return delay;
  }

  /**
   * Turn on the thyristor at full power.
   */
  void turnOn();

  /**
   * Turn off the thyristor.
   */
  void turnOff(){
    setDelay(0);
  }

  ~Thyristor();

  /**
   * Setup timer and interrupt routine.
   */
  static void begin();

  /**
   * Return the number of instantiated thyristors.
   */
  static uint8_t getThyristorNumber(){
    return nThyristors;
  };

  /**
   * Set the pin dedicated to receive the AC zero cross signal.
   */
  static void setSyncPin(uint8_t pin){
    syncPin = pin;
  }

  /**
   * Get frequency.
   */
  static float getFrequency();

#ifdef NETWORK_FREQ_RUNTIME
  /**
   * Set target frequency.
   */
  static void setFrequency(float frequency);
#endif

#ifdef MONITOR_FREQUENCY
  /**
   * Get the detected frequency on the electrical network, constantly updated.
   */
  static float getDetectedFrequency();
#endif

  static const uint8_t N = 8;

private:
  /**
   * Tell if interrupt must be re-enabled. This metohd affect allMixedOnOff variable.
   * This methods must be called every time a thyristor's delay is updated.
   *
   * NewDelay the new delay just set of this thyristor.
   * Return true if interrupt for zero cross detection should be re-enabled,
   * false do nothing.
   */
  bool mustInterruptBeReEnabled(uint16_t newDelay);

  /**
   * Search if all the values are only on and off.
   * Return true if all are on/off, false otherwise.
   */
  bool areThyristorsOnOff();
  
  /**
   * Number of instantiated thyristors.
   */
  static uint8_t nThyristors;

  /**
   * Vector of instatiated thyristors.
   */
  static Thyristor* thyristors[];

  /**
   * Variable to tell to interrupt routine to update its internal structures
   */
  static bool newDelayValues;

  /**
   * Variable to avoid concurrency problem between interrupt and threads.
   * Condition variable is not used because interrupt cannot be stopped.
   */
  static bool updatingStruct;
  
  /**
   * This variable tells if the thyristors are completely ON and OFF,
   * mixed configuration are included. If one thyristor has a value between
   * (0; semiPeriodLength), this variable is false. If true, this implies that
   * zero cross interrupt must be enabled to manage the thyristor activation.
   */
  static bool allThyristorsOnOff;

  /**
   * Pin receiving the external Zero Cross signal.
   */
  static uint8_t syncPin;

  /**
   * 0) no messages
   * 1) error messages
   * 2) debug messages
   * 3) info messages
   */
  static const uint8_t verbosity = 1;

  /**
   * Pin used to control thyristor's gate.
   */ 
  uint8_t pin;
  
  /**
   * Position into the static array, this is used to speed up the research 
   * operation while setting the new brightness value.
   */
  uint8_t posIntoArray;
  
  /**
   * Time to wait before turning on the thryristor.
   */
  uint16_t delay;

  friend void activate_thyristors();
  friend void zero_cross_int();
  friend void turn_off_gates_int();
};

#endif // END THYRISTOR_H
