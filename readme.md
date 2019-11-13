# Dimmable Light for Arduino
A library to manage thyristors (aka dimmer or triac) and phase-fired control (aka phase-cutting control) in Arduino environment. 

## Features
The main features of this library:

1. Control indipendently many thyristors at the same time
2. Support to multiple platforms (ESP8266/ESP32/AVR/SAMD)
3. Raise interrupts only if strictly necessary (i.e. when the applicance has to turn on, no useless periodic interrupts)
4. Control effective delivered power to appliances, not just thyristor's activation time

Here a complete comparison among the most similar libraries:

|                                   	| Dimmable Light for Arduino                           	| [RobotDynOfficial/RDBDimmer](https://github.com/RobotDynOfficial/RBDDimmer)                                           	| [circuitar/Dimmer](https://github.com/circuitar/Dimmer)                         	|
|-----------------------------------	|---------------------------------------------	|-----------------------------------------------------	|----------------------------------	|
| Multiple dimmers                  	| yes                                         	| yes                                                 	| yes                              	|
| Supported Frequencies                   	| 50/60Hz                                	| 50Hz                                           	| 50/60Hz                        	|
| Supported architecture            	| AVR, SAMD, ESP8266, ESP32                   	| AVR, SAMD, ESP8266, ESP32, STM32F1, STM32F4, SAM 	| AVR                              	|
| Control *effective* delivered power 	| yes, dynamic calculation                    	| no                                                  	| yes, static lookup table 	|
| Embedded automations          	| no                                          	| yes, automatic fade to new value                   	| yes, swipe effect                	|
| Optional zero-crossing mode | no                                          	| no                                                  	| yes                              	|
| Resolution                        	| up to 1us                                   	| 1/100 of semi-period energy                           	| 1/100 of semi-period length            	|
| Smart Interrupt Management        	| yes, automatically activated only if needed 	| no                                                  	| no                               	|
| Number of interrupts per semi-period (1)        	| number of instantiated dimmers + 1 	| 100                                                  	| 100                               	|

(1) In the worst case, with default settings 

## Motivations
This library was born from the curiosity to learn how hardware timer works on ESP8266 (precision and flexibility) and to control old-fashioned incandescence lights.

Actually it was interesting (and sometime frustrating) to discover that a *simple* peripheral such as timer can really vary from architecture to architecture. In my opinion, the ESP8266 is the worst implementation among these 3 architectures. It could be barely considered a timer/counter: no Input Compare, 1 Output Compare channel, only-down counter and single interrupt on 0 matching. It should carry only 2 of them, and, usually, the first is dedicated to WiFi management. This can stuck application requiring multiple and simultaneous PWM. The unique pro is that they are 32 bits. ESP32 is way better then its predecessor: 4 64bits counters with plenty functionalities, but again no input capture and just 1 output compare channel. The most surpring implementation was provided by old but gold AVR family. The main drawback is that they are only 8 or 16 bits, but you should consider that they are very old (they were launched more that 20 years ago) and 8-bit born. However, they provide many functionalities such as Input Compare/Output Compare with multiple channel and pretty clear and uniform control register over the different family's models. Moreover, they are well supported by well-written C header files containing complete registers' specifications.

## Installation
You can install Dimmable Light for Arduino through Arduino Library Manager or cloning this repository.

### Requirements
You need Arduino IDE and the appropriate board packages. If you want to use the library on AVR boards such as Arduino/Genuino Uno, you also need [ArduinoSTL](https://github.com/mike-matera/ArduinoSTL) (available on Arduino Library Manager). If you want to compile the 6th example (the most complete), you also need [ArduinoSerialCommand](https://github.com/kroimon/Arduino-SerialCommand) library.

## Usage
The main APIs are accessible through DimmableLight class. First, you must instantiate one or more DimmableLight, specifying the corresponding pin. Second, you must set Zero Cross pin, calling the static method *setSyncPin(..)*. Finally you must call static method *begin()*: it enables an interrupt on Zero Cross Detection, checking if thyristor(s) must be activated. To set the delivered power level, call method *setBrightness(..)*: it accepts value from 0 to 255, fitting into 8 bit. More details and ready-to-use code in *examples* folder.

If you encounter flickering problem due to noise on eletrical network, you can try to enable (uncommenting) *FILTER_INT_PERIOD* define at the begin of thyristor.cpp file.

If you have strong memory constrain, you can drop the functionalities provided by *dimmable_light_manager.h/cpp* (i.e. you can delete those files).

For more details check the [Wiki](https://github.com/fabiuz7/Dimmable-Light-Arduino/wiki).

## Examples
Along with the library there are 7 examples. If you are a beginner you should start from the first one. Note that examples 3 and 5 work only for esp8266 and esp32 mcu because their dependency on Ticker library. Example number 7 shows how to linearly control dimmer setting the effective energy delivered instead activation time.

The 6th is the most interesting because it provide a good set of effects, selectable from serial port. This example requires 8 dimmers, each one to control a bulb. [Here](https://youtu.be/DRJcCIZw_Mw) you can find a brief video showing the 9th and 11th effect. I had used [this board](https://www.ebay.it/itm/8CH-AC-LED-BULB-DIMMER-SSR-RELAY-110V-220V-SMART-HOME-ARDUINO-RASPBERRY/122631760038), but you can easily replace it with equivalent one.
In these images you can see the full hardware setting:

!["Lamps"](https://i.ibb.co/zVBRB9k/IMG-4045.jpg "Lamps")
8 incandescence bulbs.

!["Boards"](https://i.ibb.co/YN2Fktn/IMG-4041.jpg "Boards")
Wemos D1 mini (v2.3.0) and a board equipped with 8 dimmers.
