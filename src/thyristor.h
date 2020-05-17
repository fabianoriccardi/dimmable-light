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

// Set how the network frequency is selected.
// The first 2 options fix the frequency to the common values (respectively to
// 50 and 60Hz) at compile time.
// The third option allows you change network frequency at runtime. This method
// enables the setFrequency() method. The main drawback is that it is a bit more 
// inefficient w.r.t the "fixed frequency" alternatives.
// Select one and ONLY one among the following alternatives:
#define NETWORK_FREQ_FIXED_50HZ
//#define NETWORK_FREQ_FIXED_60HZ
//#define NETWORK_FREQ_RUNTIME

// If enabled, you can monitor the actual frequency of the electrical network.
//#define MONITOR_FREQUENCY


/**
 * This is the core class of this library, to be used to get the finest control
 * on thyristors. 
 * 
 * NOTE: Design Principle for this library: The concept of Thyristor is agnostic 
 * to controlled appliance, hence measurement unit to control them should be also
 * appliance-agnostic. However, this will lead to more abstract code,
 * an undesiderable effect especially in contextes such as Arduino,
 * oriented to be as user-friendly as possible.
 * 
 * For these reason, I decided to separate the library in 2 main level of classes: 
 * a low level one (appliance-agnostic), and a higher level one for final 
 * user (a nicer appliance-dependent wrapper, e.g. Dimmable Light).
 * 
 * About this class, the "core" of the library, the name of the method 
 * to control a dimmer is setDelay(..) and not, for example, setPower(..),
 * setBrightness(..),... This gives a precise idea of what's
 * happening in electrical world, that is controlling the activation time
 * of the thyristor.
 * Secondly, the measurement unit is expressed in microsecond, 
 * allowing the finest and feasible control reachable with old and cheap
 * MCUs such as Arduino Uno (often still exceeding the real need).
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

  /**
   * Get the semiperiod.
   */
  static uint16_t getSemiPeriod();

#ifdef NETWORK_FREQ_RUNTIME
  /**
   * Set target frequency. Negative values are ignored;
   * zero set the semi-period to 0.
   */
  static void setFrequency(float frequency);
#endif

#ifdef MONITOR_FREQUENCY
  /**
   * Get the detected frequency on the electrical network, constantly updated.
   * Return 0 if there is no signal or while sampling the first periods.
   * 
   * NOTE: when (re)starting, it will take a while before returning a value different from 0.
   */
  static float getDetectedFrequency();

  /**
   * Check if frequency monitor is always enabled.
   */
  static bool isFrequencyMonitorAlwaysOn(){
    return frequencyMonitorAlwaysEnabled;
  }

  /**
   * Control if the monitoring can be automatically stopped when 
   * all lights are on and off. True to force the constant monitoring,
   * false to allow automatic stop. By default the monitoring is always active.
   *
   */
  static void frequencyMonitorAlwaysOn(bool enable);
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
   * In particular, this variable is used to prevent the copy of the memory used by
   * the array of struct during reordering (interrupt can continue because it
   * keeps its own copy of the array).
   * A condition variable does not make sense because interrupt routine cannot be 
   * stopped.
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
   * True means the is always listeing, false means
   * auto-stop when all lights are on/off.
   */
  static bool frequencyMonitorAlwaysEnabled;

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
