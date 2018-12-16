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
#include "dimmable_light_manager.h"

bool DimmableLightManager::add(String lightName, uint8_t pin){
  const char* temp=lightName.c_str();
  std::unordered_map<std::string, DimmableLight*>::const_iterator it=dla.find(temp);
  if(it==dla.end()){
    DimmableLight* pDimLight=new DimmableLight(pin);
    dla.insert({lightName.c_str(),pDimLight});
    return true;
  }else{
    return false;
  }
}

DimmableLight* DimmableLightManager::get(String lightName){
  const char* temp=lightName.c_str();
  std::unordered_map<std::string,DimmableLight*>::const_iterator it=dla.find(temp);
  if(it!=dla.cend()){
    return (it->second);
  }else{
    return nullptr;
   }
}

std::pair<String, DimmableLight*> DimmableLightManager::get(){
  static std::unordered_map<std::string,DimmableLight*>::const_iterator it=dla.begin();
  String name=it->first.c_str();
  std::pair<String, DimmableLight*> res(name,it->second);

  it++;
  if(it==dla.cend()){
    it=dla.begin();
  }
  return res;
}