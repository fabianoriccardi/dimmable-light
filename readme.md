# Dimmable Light for Arduino
A simple library to manage thyristors (aka dimmer) easily in Arduino environment. 

## Features
1. Multiple and indipendent control over thyristors
2. Support to multiple platforms (ESP8266/ESP32/AVR/...)
3. Raise interrupts only if strictly necessary (i.e. when the applicance has to turn on, no useless periodic interrupts)

## Motivations
This library was born from the curiosity to learn how hardware timer works on ESP8266 (precision and flexibility) and to control old-fashioned incandescence lights.

Actually it was interesting (and sometime frustrating) to discover that a *simple* peripheral such as timer can really vary from architecture to architecture. In my opinion, the ESP8266 is the worst implementation among these 3 architectures. It could be barely considered a timer/counter: no Input Compare, 1 Output Compare channel, only-down counter and single interrupt on 0 matching. It should carry only 2 of them, and, usually, the first is dedicated to WiFi management. This can stuck application requiring multiple and simultaneous PWM. The unique pro is that they are 32 bits. ESP32 is way better then its predecessor: 4 64bits counters with plenty functionalities, but again no input capture and just 1 output compare channel. The most surpring implementation was provided by old but gold AVR family. The main drawback is that they are only 8 or 16 bits, but you should consider that they are very old (they were launched more that 20 years ago) and 8-bit born. However, they provide many functionalities such as Input Compare/Output Compare with multiple channel and pretty clear and uniform control register over the different family's models. Moreover, they are well supported by well-written C header files containing complete registers' specifications.

## Installation
You can install Dimmable Light for Arduino through Arduino Library Manager or cloning this repository.

### Requirements
You need Arduino IDE and the appropriate board packages. If you want to use the library on AVR boards such as Arduino/Genuino Uno, you also need [ArduinoSTL](https://github.com/mike-matera/ArduinoSTL) (available on Arduino Library Manager). If you want to compile the 6th example (the most complete), you also need [ArduinoSerialCommand](https://github.com/kroimon/Arduino-SerialCommand) library.

## Usage
The main APIs are accessible through DimmableLight class. First, you must instantiate one or more DimmableLight, specifying the corresponding pin. Second, you must set Zero Cross pin, calling the static method *setSyncPin(..)*. Finally you must call static method *begin()*: this activated interrupt on Zero Cross pin, hence activating thryristor whenever required. To set the delivered power level, call method *setBrightness(..)*: it accepts value from 0 to 255, fitting into 8 bit. 
More details and ready-to-compile code in *examples* folder.

## Examples
There are 5 examples, enumerated from the simplest to the most complete. The 5th shows a bunch of effect applied to 8 lights. [Here](https://youtu.be/DRJcCIZw_Mw) you can find a brief video showing the 9th and 11th effect. This code make use of [this boards](https://www.ebay.it/itm/8CH-AC-LED-BULB-DIMMER-SSR-RELAY-110V-220V-SMART-HOME-ARDUINO-RASPBERRY/122631760038), but you can easily replace it with others (cheaper).
In these images you can see the full hardware setting:
!["Lamps"](https://i.ibb.co/zVBRB9k/IMG-4045.jpg "Lamps")
8 incandescence bulbs

!["Boards"](https://i.ibb.co/YN2Fktn/IMG-4041.jpg "Boards")
Wemos D1 mini (v2.3.0) and a board equipped with 8 dimmers

## Limitations
On ESP8266 the library makes use of hardware timer (Timer 1), hence it can create conflicts with other functionalities such as PWM, Tone and so on.

## To-Do
Release 1.3:

- [ ] Enable/Disable zero cross interrupt method
- [ ] Main refactor and give a clear definition of timer behaviour for all platform (providing a clear schema)
- [ ] if all lights are set off by the user, autmatically disable zero cross iterrupt 
- [ ] support for 60Hz electrical network
