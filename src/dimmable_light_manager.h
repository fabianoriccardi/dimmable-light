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
#ifndef DIMMABLE_LIGHT_MANAGER_H
#define DIMMABLE_LIGHT_MANAGER_H

#include "dimmable_light.h"

#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_RP2040)
// Unfortunately Arduino defines max/min macros, those create conflicts with the one
// defined by C++/STL environment
#undef max
#undef min
#include <unordered_map>
#elif defined(AVR)
#include <ArduinoSTL.h>
#include <map>
#endif

#include <string>

/**
 * Class to store the mapping between a DimmableLight object and
 * a (friendly) name. This could be useful when developing APIs.
 */
class DimmableLightManager {
public:
  /**
   * Create a new light with a given name
   */
  bool add(String lightName, uint8_t pin);

  /**
   * Get a light with a specific name, if any
   */
  DimmableLight* get(String lightName);

  /**
   * Get a light from from the contaniner.
   *
   * This method is "circular", that means once you get the last element
   * the nect call return the first one.
   */
  std::pair<String, DimmableLight*> get();

  int getCount() {
    return dla.size();
  }

  static void begin() {
    DimmableLight::begin();
  }

private:
#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_RP2040)
  std::unordered_map<std::string, DimmableLight*> dla;
#elif defined(AVR)
  std::map<std::string, DimmableLight*> dla;
#endif
};

#endif
