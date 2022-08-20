# Dimmable Light for Arduino

[![arduino-library-badge](https://www.ardu-badge.com/badge/Dimmable%20Light%20for%20Arduino.svg?)](https://www.ardu-badge.com/Dimmable%20Light%20for%20Arduino) ![Compile Library Examples](https://github.com/fabianoriccardi/dimmable-light/actions/workflows/LibraryBuild.yml/badge.svg)

A library to manage thyristors (aka dimmer or triac) and phase-fired control (aka phase-cutting control) in Arduino environment.

## Motivations

At the very beginning, this library was born from the curiosity to experiment the performance and capabilities of hardware timer on ESP8266 and to control old-fashioned incandescence lights.
In the second instance, I needed to move thyristor controller to other microcontrollers, and the library had to adapt to the underlying hardware timers, which often vary for specifications and expose a very different interface from architecture to architecture. Moreover, at the time there weren't multi-platform libraries to control thyristor, so I decided to extend and maintain this library.

### About the timers

Actually, it was interesting (and sometime frustrating) to discover that a *simple* peripheral such as timer can really vary from architecture to architecture. ESP8266 is equipped with 2 timers, but one is dedicated to Wi-Fi management. This can complicate the development of applications that require multiple and simultaneous use of timer. Moreover, it hasn't "advanced" capabilities such as input compare, multiple output compare channels, and bidirectional counter. At least, these 2 timers are 32-bit. ESP32 is way better than its predecessor: it has 4 64-bit counters with up and down counters, but again no input capture and just 1 output compare channel per timer. Finally, I tried the ATmega328 of AVR family. The main difference and drawback is that they have only 8-bit or 16-bit timers, but this is reasonable if you consider that they were launched in the late 1996, when 8-bit microcontrollers were the standard. However, they provide more functionalities such as input compare and output compare with multiple channels and uniformed control registers over the different family's models. Moreover, they are well-supported by C header files containing complete registers' specifications.
For sure, among these MCUs, the most complete implementation is provided by the old but gold AVR family.
This brief overview gives a glimpse of the variety of timers embedded in microcontrollers, and it highlights the importance of building an abstraction layer that hides these differences and expose the essential functionalities needed to control thyristors: one-shot trigger and stop the timer.

## Features

1. Control multiple thyristors at the same time
2. Compatible with multiple platforms (ESP8266/ESP32/AVR/SAMD)
3. Interrupt optimization (trigger interrupts only if necessary, no periodic interrupt)
4. Control the load via gate activation time or relative power

Here a comparison against 2 similar and popular libraries:

|                                          | Dimmable Light for Arduino                  | [RobotDynOfficial/RDBDimmer](https://github.com/RobotDynOfficial/RBDDimmer) | [circuitar/Dimmer](https://github.com/circuitar/Dimmer) |
|------------------------------------------|---------------------------------------------|-----------------------------------------------------------------------------|---------------------------------------------------------|
| Multiple dimmers                         | yes                                         | yes                                                                         | yes                                                     |
| Supported Frequencies                    | 50/60Hz                                     | 50Hz                                                                        | 50/60Hz                                                 |
| Supported architectures                  | AVR, SAMD, ESP8266, ESP32                   | AVR, SAMD, ESP8266, ESP32, STM32F1, STM32F4, SAM                            | AVR                                                     |
| Control *effective* delivered power      | yes, dynamic calculation                    | no                                                                          | yes, static lookup table                                |
| Predefined effects                       | no                                          | yes, automatic fade to new value                                            | yes, swipe effect                                       |
| Optional zero-crossing mode              | no                                          | no                                                                          | yes                                                     |
| Time resolution                          | 1us (2)                                     | 1/100 of semi-period energy                                                 | 1/100 of semi-period length                             |
| Smart Interrupt Management               | yes, automatically activated only if needed | no                                                                          | no                                                      |
| Number of interrupts per semi-period (1) | number of instantiated dimmers + 1          | 100                                                                         | 100                                                     |                                              |

(1) In the worst case, with default settings
(2) If the hardware timer allows it, otherwise it will be lower

### Requirements

You need Arduino IDE and the appropriate board packages. On AVR boards such as Arduino/Genuino Uno, you also need [ArduinoSTL](https://github.com/mike-matera/ArduinoSTL) (available on Arduino Library Manager).
If you want to compile the 6th example (the most complete), you also need [ArduinoSerialCommand](https://github.com/kroimon/Arduino-SerialCommand) library.

## Installation

The latest version of Dimmable Light for Arduino is available on Arduino Library Manager and on [PlatformIO registry](https://registry.platformio.org/libraries/fabianoriccardi/Dimmable%20Light%20for%20Arduino).

## Usage

The main APIs are accessible through DimmableLight class. Instantiate one or more DimmableLight, specifying the corresponding activation pin.

    DimmableLight dimmer(3);

Set the Zero Cross pin, calling the static method `setSyncPin`:

    DimmableLight::setSyncPin(2);

Then call the static method `begin`:

    DimmableLight::begin();

it enables the interrupt on Zero Cross Detection that checks if any thyristor must be activated. To set the activation time, call the method `setBrightness`:

    dimmer.setBrightness(150);

the given value is the relative activation time w.r.t. the semi-period length. The method accepts values in range [0; 255].

If you encounter flickering problem due to noise on eletrical network, you can try to enable (uncomment) `#define FILTER_INT_PERIOD` at the begin of `thyristor.cpp` file.

If you have strict memory constrain, you can drop the functionalities provided by `dimmable_light_manager.h/cpp` (i.e. you can delete those files).

For ready-to-use code look in `examples` folder. For more details check the header files and the [Wiki](https://github.com/fabianoriccardi/dimmable-light/wiki).

## Examples

Along with the library, there are 8 examples. If you are a beginner, you should start from the first one. Note that examples 3 and 5 work only for ESP8266 and ESP32 because of their dependency on Ticker library. Example 7 shows how to control linearly the energy delivered to the load instead of controlling directly the gate activation time.

The example 6 demonstrates various fascinating luminous effects and requires 8 dimmers, each one to control a light. [Here](https://youtu.be/DRJcCIZw_Mw) you can find a brief video showing the 9th and 11th effect. I had used [this board](https://www.ebay.it/itm/124269741187), but you can find an equivalent one.
In these images, you can see the full hardware setting:

!["Lamps"](https://i.ibb.co/zVBRB9k/IMG-4045.jpg "Lamps")
8 incandescence bulbs.

!["Boards"](https://i.ibb.co/YN2Fktn/IMG-4041.jpg "Boards")
Wemos D1 mini (v2.3.0) and a board equipped with 8 dimmers.
