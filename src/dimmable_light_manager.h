#ifndef DIMMABLE_LIGHT_MANAGER_H
#define DIMMABLE_LIGHT_MANAGER_H

#include <unordered_map>
#include <dimmable_light.h>

class DimmableLightManager{
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

  int getCount(){
    return dla.size();
  }

  static void begin(){
    DimmableLight::begin();
  }
private:
  std::unordered_map<std::string, DimmableLight*> dla;
};

#endif