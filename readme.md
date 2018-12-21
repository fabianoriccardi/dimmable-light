# Dimmable Light for ESP
A simple library to manage up to 8 thyristors (also knows as dimmer) on ESP8266 and ESP32 in Arduino environment. 

## Motivations
This library was born from the curiosity to learn how hardware timer works on esp8266 (precision and flexibility) and to control the old fashioned incandescence lights. I can confirm the timer is pretty poor if compared with other SoC, and a single timer (there is also Timer 0, but it is mandatory for WiFi operations) is not enough for some applications (i.e. play a tone with buzzer and simultaneously control dimmers).

## Installation
You can install Dimmable Light for ESP through Arduino Library Manager or cloning this repository.

### Requirements
You need only Arduino IDE and ESP8266 and/or ESP32 Board Package. If you want to compile the 5th example, you also need [ArduinoSerialCommand](https://github.com/kroimon/Arduino-SerialCommand) library.

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
It makes use of hardware timer (Timer 1), hence it can create conflict with other functionalities like PWM, Tone and so on.
