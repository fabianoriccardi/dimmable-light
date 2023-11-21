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
#include "dimmable_light_manager.h"

bool DimmableLightManager::add(String lightName, uint8_t pin) {
  const char* temp = lightName.c_str();
#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_RP2040)
  std::unordered_map<std::string, DimmableLight*>::const_iterator it = dla.find(temp);
#elif defined(AVR)
  std::map<std::string, DimmableLight*>::const_iterator it = dla.find(temp);
#endif
  if (it == dla.end()) {
    DimmableLight* pDimLight = new DimmableLight(pin);
    dla.insert({ lightName.c_str(), pDimLight });
    return true;
  } else {
    return false;
  }
}

DimmableLight* DimmableLightManager::get(String lightName) {
  const char* temp = lightName.c_str();
#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_RP2040)
  std::unordered_map<std::string, DimmableLight*>::const_iterator it = dla.find(temp);
#elif defined(AVR)
  std::map<std::string, DimmableLight*>::const_iterator it = dla.find(temp);
#endif
  if (it != dla.end()) {
    return (it->second);
  } else {
    return nullptr;
  }
}

std::pair<String, DimmableLight*> DimmableLightManager::get() {
#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_RP2040)
  static std::unordered_map<std::string, DimmableLight*>::const_iterator it = dla.begin();
#elif defined(AVR)
  static std::map<std::string, DimmableLight*>::const_iterator it = dla.begin();
#endif
  String name = it->first.c_str();
  std::pair<String, DimmableLight*> res(name, it->second);

  it++;
  if (it == dla.end()) { it = dla.begin(); }
  return res;
}
