/***********************************************************************************
 * Here there is specific AVR code. AVR is usually programmed at very low level
 * than respect ESP8266/ESP32, so this file is needed to provide a minimum "HAL"
 * to simply timer usage
 ***********************************************************************************/
#ifdef AVR

#ifndef HW_TIMER_ARDUINO_H
#define HW_TIMER_ARDUINO_H

/**
 * convert microsecond to tick, max micro is 32767, otherwize it returns 0.
 */
uint16_t microsecond2Tick(uint16_t micro);

/**
 * Configure the timer to be ready to be started
 */
void timerBegin();

/**
 * Set the call staret when timer triggers
 */
void timerSetCallback(void (*f)());

/**
 * Let's start the timer: it triggers after x ticks.
 * 0 and 1 are not's accepted.
 */
bool timerStart(uint16_t tick);

#endif // HW_TIMER_ARDUINO_H

#endif // END AVR
