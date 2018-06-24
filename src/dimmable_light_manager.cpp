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