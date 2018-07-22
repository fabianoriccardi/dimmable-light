#include "thyristor.h"
#include "hw_timer.h"

// Activate this define to enable a check on the period between zero and the next one
//#define CHECK_INT_PERIOD
// Activate this macro to check if all the light were managed in a semi-period
//#define CHECK_MANAGED_THYR

struct PinDelay{
  uint8_t pin;
  uint16_t delay;
};

/**
 * Temporary struct to provide the interrupt a memory concurrent-safe
 */
static struct PinDelay pinDelay[Thyristor::N];

/**
 * Number of thyristors already managed in the current semi-wave
 */
static uint8_t thyristorManaged=0;

/**
 * Timer routine to turn on one or more thyristors
 */
void activateThyristors(){
  // Alternative way to manage the pin, it should become low after the triac started
  //delayMicroseconds(10);
  //digitalWrite(AC_LOADS[phase],LOW);
  
  uint8_t firstToBeUpdated=thyristorManaged;
  // This condition means:
  // trigger immediately is there is not time to active the timer for the next light 
  // (i.e delay differene less than 20microseconds)
  // After some experiment, even 50 microseconrd are noticeble, so I decided 
  // to set the threshold lower that 20microsecond (wrt the resolution of the user class, 
  // that it about 39microsecond, this loop is used only for equal values)
  for(; (thyristorManaged<Thyristor::nThyristors-1 && pinDelay[thyristorManaged+1].delay-pinDelay[firstToBeUpdated].delay<20); thyristorManaged++){
    digitalWrite(pinDelay[thyristorManaged].pin, HIGH);
  }

  digitalWrite(pinDelay[thyristorManaged].pin, HIGH);

  thyristorManaged++;

  uint8_t pulseWidth = 15;
  delayMicroseconds(pulseWidth);

  for(int i=0;i<thyristorManaged;i++){
    digitalWrite(pinDelay[i].pin, LOW);
  }

  if(thyristorManaged<Thyristor::nThyristors){
    hw_timer_arm(pinDelay[thyristorManaged].delay-pinDelay[thyristorManaged-1].delay-pulseWidth);
  }
}

#ifdef CHECK_INT_PERIOD
bool first=true;
uint32_t lastTime;
#endif

/**
 * This function manage the triac/dimmer in a single semi-period (that is 10ms @50Hz)
 * This function will be called multiple times per semi-period (in case of multi 
 * lamps with different at least a different delay value).
 */
void zero_cross_int(){
  // This is kind of optimization software, but not electrical:
  // This avoid to wait 10micros in a interrupt or setting interrupt 
  // to turn off the PIN (this last solution could be evaluated...)
  for(int i=0;i<Thyristor::nThyristors;i++){
   digitalWrite(pinDelay[i].pin,LOW);
  }

#ifdef CHECK_INT_PERIOD
  if(first){
    lastTime=micros();
    first=false;
  }else{
    uint32_t now=micros();
    if(now-lastTime>10015||now-lastTime<9090){
      Serial.println(now-lastTime);
    }
    lastTime=now;
  }
#endif

#ifdef CHECK_MANAGED_THYR
  if(thyristorManaged!=Thyristor::nThyristors){
    Serial.print("E");
    Serial.println(thyristorManaged);
  }
#endif

  // Update the structures and set thresholds, if needed
  if(Thyristor::newDelayValues && !Thyristor::updatingStruct){
    Thyristor::newDelayValues=false;
    //Serial.println("UI");
    for(int i=0;i<Thyristor::nThyristors;i++){
      pinDelay[i].pin=Thyristor::thyristors[i]->pin;
      if(Thyristor::thyristors[i]->delay<=120){
        pinDelay[i].delay=120;
      }else if(Thyristor::thyristors[i]->delay>=9880){
        pinDelay[i].delay=9880;
      }else{
        pinDelay[i].delay=Thyristor::thyristors[i]->delay;
      }
    }
    // for(int i=0;i<Thyristor::nThyristors;i++){
    //   Serial.print(String("int: ") + pinDelay[i].pin);
    //   Serial.print(" ");
    //   Serial.println(pinDelay[i].delay);
    // }   
  }

  thyristorManaged = 0;

  // This block of code is inteded to manage the case near to the next semi-period:
  // In this case we should avoid to trigger the timer, because the effective semiperiod 
  // perceived by the esp8266 could be less than 10000microsecond. This could be due to 
  // the relative time (there is no possibily to set the timer to an absolute time)
  // Moreover, it is impossible to disable an interrupt once it is armed, neither 
  // change the callback function.
  // NOTE: don't know why, but the timer seem trigger even when it is not set...
  // so a provvisory solution if to set the relative callback to NULL!
  // NOTE 2: this improvement should be think eve for multiple lamp!
  if(thyristorManaged<Thyristor::nThyristors && pinDelay[thyristorManaged].delay<9950){
    hw_timer_set_func(activateThyristors);
    hw_timer_arm(pinDelay[thyristorManaged].delay);
  }else{
    hw_timer_set_func(NULL);
  }
}

void Thyristor::begin(){
  pinMode(digitalPinToInterrupt(syncPin), INPUT);
  attachInterrupt(digitalPinToInterrupt(syncPin), zero_cross_int, RISING);

  // FRC1 is a low priority timer, it can't interrupt other ISR
  hw_timer_init(FRC1_SOURCE, 0);
}

void Thyristor::setDelay(uint16_t newDelay){
  if(verbosity>2){
    for(int i=0;i<Thyristor::nThyristors;i++){
      Serial.print(String("setB: ") + "posIntoArray:" + thyristors[i]->posIntoArray + " pin:" + thyristors[i]->pin);
      Serial.print(" ");
      Serial.println(thyristors[i]->delay);
    }
  }

  // Reorder the array to speed up the interrupt.
  // This mini-algorithm works on a different memory area wrt the interrupt,
  // so it is concurrent-safe code

  updatingStruct=true;
  // Array example, it is always ordered, higher values means lower delay
  // [45,678,5000,7500,9000]
  if(newDelay>delay){
    if(verbosity>2) Serial.println("\tlowering the light..");
    bool done=false;
    /////////////////////////////////////////////////////////////////
    // Let's find the new position
    int i=posIntoArray+1;
    while(i<nThyristors && !done){
      if(newDelay<=thyristors[i]->delay){
        done=true;
      }else{
        i++;
      }
    }
    // This could be due to 2 factor:
    // 1) the light is already the lowest delay
    // 2) the delay is not changed to overpass the neightbour
    if(posIntoArray+1==i){
      if(verbosity>2) Serial.println("No need to shift..");
    }else{
      int target;
      // Means that we have reached the end, the target i the last element
      if(i==nThyristors){
        target=nThyristors-1;
      }else{
        target=i-1;
      }
  
      // Let's shift
      for(int i=posIntoArray;i<target;i++){
        thyristors[i]=thyristors[i+1];
        thyristors[i]->posIntoArray=i;
      }
      thyristors[target]=this;
      this->posIntoArray=target;
    }
  }else if(newDelay<delay){
    if(verbosity>2) Serial.println("\traising the light..");
    bool done=false;
    int i=posIntoArray-1;
    while(i>=0 && !done){
      if(newDelay>=thyristors[i]->delay){
        done=true;
      }else{
        i--;
      }
    }
    if(posIntoArray-1==i){
      if(verbosity>2) Serial.println("No need to shift..");
    }else{
      int target;
      // Means that we have reached the start, the target is the first element
      if(!done){
        target=0;
      }else{
        target=i+1;
      }
  
      // Let's shift
      for(int i=posIntoArray;i>target;i--){
        thyristors[i]=thyristors[i-1];
        thyristors[i]->posIntoArray=i;
      }
      thyristors[target]=this;
      this->posIntoArray=target;
    }
  }else{
    if(verbosity>2) Serial.println("No need to perform the exchange, the delay is the same!");
  }
  delay=newDelay;
  newDelayValues=true;
  updatingStruct=false;
  
  if(verbosity>2){
    for(int i=0;i<Thyristor::nThyristors;i++){
      Serial.print(String("\tsetB: ") + "posIntoArray:" + thyristors[i]->posIntoArray + " pin:" + thyristors[i]->pin);
      Serial.print(" ");
      Serial.println(thyristors[i]->delay);
    }
  }

  //Serial.println(String("Brightness (in ms to wait): ") + delay);
}

/**
 * @brief      No reorder, init all the light at the begin of your sketch
 *
 * @param[in]  pin   The pin
 */
Thyristor::Thyristor(int pin)
                :pin(pin),delay(10000){
  if(nThyristors<N-1){
    pinMode(pin,OUTPUT);
    
    updatingStruct=true;
    
    posIntoArray=nThyristors;
    nThyristors++;
    thyristors[posIntoArray]=this;
    
    //Full reorder of the array
    for(int i=0;i<nThyristors;i++){
      for(int j=i+1;j<nThyristors-1;j++){
        if(thyristors[i]->delay>thyristors[j]->delay){
          Thyristor *temp=thyristors[i];
          thyristors[i]=thyristors[j];
          thyristors[j]=temp;
        }
      }
    }
    // Set the posIntoArray with a "brutal" assignement to each Thyristor
    for(int i=0;i<nThyristors;i++){
      thyristors[i]->posIntoArray=i;
    }

    newDelayValues=true;
    
    // NO because this struct is updated by the routine!
//    pinDelay[posIntoArray].pin;
//    pinDelay[posIntoArray].delay=10000;
    
    updatingStruct=false;
  }else{
    // return error or exception
  }
}

Thyristor::~Thyristor(){
  // Recompact the array
  updatingStruct=true;
  nThyristors--;
  //remove the light from the static pinDelay array
  Serial.println("I should implement the array shrinking");
  updatingStruct=false;
}

uint8_t Thyristor::nThyristors = 0;
Thyristor* Thyristor::thyristors[Thyristor::N] = {nullptr};
bool Thyristor::newDelayValues = false;
bool Thyristor::updatingStruct = false;
uint8_t Thyristor::syncPin = 255;
