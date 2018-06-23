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