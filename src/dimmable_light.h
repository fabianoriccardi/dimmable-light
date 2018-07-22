#ifndef DIMMABLE_LIGHT_H
#define DIMMABLE_LIGHT_H

#include "Arduino.h"
#include "thyristor.h"

/**
 * This is the user-accessible DimmableLight class.
 * 
 * NOTE for programmers: the class should keep a static array with a brightness value
 * 					per object (that is a light).
 */
class DimmableLight{
  public:
  	DimmableLight(int pin)
                  :thyristor(pin),brightness(0){
      if(nLights<N-1){   
        nLights++;
      }else{
        // return error or exception
      }
    }

  	/**
   	 * Set the brightness, 0 to turn off the lamp
   	 */
  	void setBrightness(uint8_t bri){
      brightness=bri;
      uint16_t newDelay=10000-bri*10000/255;
      thyristor.setDelay(newDelay);
    };

  	/**
  	 * Return the current brightness
  	 */
  	uint8_t getBrightness(){
  		return brightness;
  	}

  	/**
   	 * Turn off the light
   	 */
  	void turnOff(){
		  setBrightness(0);
  	}

  	~DimmableLight(){
      nLights--;
    }

  /**
   * Set the timer and the interrupt routine
   * Actually this function doesn't do nothing the first light is created 
   */
  static void begin(){
    Thyristor::begin();
  }

  /**
   * Return the number of instantiated lights
   */
  static uint8_t getLightNumber(){
    return nLights;
  };

	private:
  static const uint8_t N = 8;
	static uint8_t nLights;

  Thyristor thyristor;
	
	/**
	 * Memorize the ms to wait until turn on the light
	 * 0-->255. That's is 1 unit is approx 40microseconds
	 */
	uint8_t brightness;
};

#endif
