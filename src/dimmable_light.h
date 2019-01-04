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
#ifndef DIMMABLE_LIGHT_H
#define DIMMABLE_LIGHT_H

#include "Arduino.h"
#include "thyristor.h"

/**
 * This is the user-oriented DimmableLight class, 
 * a wrapper on Thyristor class.
 */
class DimmableLight{
  public:
  	DimmableLight(int pin)
                  :thyristor(pin),brightness(0){
      if(nLights<N){   
        nLights++;
      }else{
        Serial.println("Max lights number reached, the light is not created!");
        // return error or exception
      }
    }

  	/**
   	 * Set the brightness, 0 to turn off the lamp
   	 */
  	void setBrightness(uint8_t bri){
      brightness=bri;
      uint16_t newDelay=10000-(uint16_t)(((uint32_t)bri*10000)/255);
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
   * Set the pin dedicated to receive the AC zero cross signal
   */
  static void setSyncPin(uint8_t pin){
    Thyristor::setSyncPin(pin);
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
