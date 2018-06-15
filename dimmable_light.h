#ifndef DIMMABLE_LIGHT_H
#define DIMMABLE_LIGHT_H

#include "Arduino.h"

/**
 * This is the user-accessible DimmableLight class.
 * 
 * NOTE for programmers: the class should keep a static array with a brightness value
 * 					per object (that is a light).
 */
class DimmableLight{
  public:
	DimmableLight(int pin);

  	/**
   	 * Set the brightness, 0 to turn off the lamp
   	 */
  	void setBrightness(uint8_t b);

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

  	~DimmableLight();

  	/**
  	 * Set the timer and the interrupt routine
  	 * Actually this function doesn't do nothing the first light is created 
  	 */
  	static void begin();

  static const uint8_t N = 8;
	private:
	static uint8_t nLights;
	static DimmableLight* lights[];

  /**
   * Variable to tell the interrupt routine to update its internal structures
   */
	static bool newBrightnessValues;
	static bool updatingStruct;

	static uint8_t syncPin;

	uint8_t pin;
	
	/**
	 * Position into the static array, this is used to speed up the research 
	 * operation while setting the new brightness value.
	 */
	uint8_t posIntoArray;
	
	/**
	 * Memorize the ms to wait until turn on the light
	 * 0->10000 mapped to 0-->255. That's is 1 unit is approx 40microseconds
	 */
	uint16_t brightness;

	friend void activateLights();
	friend void zero_cross_int();
};

#endif
