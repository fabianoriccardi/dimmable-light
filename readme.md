# Dimmable Light for Arduino

[![arduino-library-badge](https://www.ardu-badge.com/badge/Dimmable%20Light%20for%20Arduino.svg?)](https://www.ardu-badge.com/Dimmable%20Light%20for%20Arduino) ![Compile Library Examples](https://github.com/fabianoriccardi/dimmable-light/actions/workflows/LibraryBuild.yml/badge.svg)

A library to manage thyristors (aka dimmer or triac) and phase-fired control (aka phase-cutting control) in Arduino environment.

## Motivations

At the very beginning, this library was born from the curiosity to experiment the performance and capabilities of hardware timer on ESP8266 and to control old-fashioned incandescence lights.
In the second instance, I wanted to port the original piece of software to ESP32, and so I started to conceive a flexible software architecture that better adapts to different hardware platforms. Moreover, at the time there weren't multi-platform libraries to control thyristors, so I decided to publish, extend, and maintain this library over time.

### About the timers

Actually, it was interesting (and sometime frustrating) to discover that a *simple* peripheral such as timer can heavily vary among different platforms.
For example, the ESP8266 is equipped with 2 timers, but only one is usable by the user since the other is reserved for Wi-Fi management. This can lead immediately to a complicate development if the user application needs the timer for multi purposes. For this reason, [ESP8266TimerInterrupt](https://github.com/khoih-prog/ESP8266TimerInterrupt) was born. Moreover, that timer hasn't "advanced" capabilities such as input compare, multiple output compare channels, a bidirectional counter, and it is only 23-bit. Another example is the ESP32, that is way better than its predecessor: it has 4 64-bit timers with up and down counters, but still no input capture and just 1 output compare channel per timer. Finally, I cannot avoid mentioning the AVR ATmega's timers: they have multiple full-featured 8-bit or 16-bit timers running at lower clock frequency than modern MCUs, which may reduce the overall resolution of dimmer control or lead to more complicated ISRs to handle multiple rollovers. At least, AVR MCUs, compared to ESP8266 and ESP32, are well-supported by C header files containing complete registers' specifications.
This brief overview gives a glimpse of the variety of properties to consider while working with timers embedded in microcontrollers, and it highlights the importance of building an abstraction layer that hides all these differences and exposes the 2 primitives needed to control thyristors: one-shot timer activation and stop counting.

## Features

1. Control multiple thyristors using a single hardware timer
2. Compatible with multiple platforms (ESP8266/ESP32/AVR/SAMD/RP2040)
3. Interrupt optimization (trigger interrupts only if necessary, no periodic interrupt)
4. Control the load by 2 measurement unit: gate activation time or linearized relative power
5. Documented parameters to finely tune the library on your hardware and requirements

Here the comparison against 3 similar and popular libraries:

|                                    | Dimmable Light for Arduino                            | [RobotDynOfficial/<br>RDBDimmer](https://github.com/RobotDynOfficial/RBDDimmer)                                            | [circuitar/Dimmer](https://github.com/circuitar/Dimmer)                          | [AJMansfield/<br>TriacDimmer](https://github.com/AJMansfield/TriacDimmer) |
|----------------------------------- |--------------------------------------------- |----------------------------------------------------- |---------------------------------- |---------------------------------- |
| Multiple dimmers                   | yes                                          | yes                                                  | yes                               | 2 |
| Supported frequencies                    | 50/60Hz                                 | 50Hz                                            | 50/60Hz                         | 50/60Hz |
| Supported architectures             | AVR, SAMD, ESP8266, ESP32, RP2040                  | AVR, SAMD, ESP8266, ESP32, STM32F1, STM32F4, SAM  | AVR                               | AVR |
| Control *effective* delivered power  | yes, dynamic calculation                     | no                                                   | yes, static lookup table  | no |
| Predefined effects           | no                                           | yes, automatic fade to new value                    | yes, swipe effect                 | no |
| Optional zero-crossing mode | no                                           | no                                                   | yes                               | no |
| Time resolution                         | 1us (2)                                    | 1/100 of semi-period energy                            | 1/100 of semi-period length             | 0.5us |
| Smart interrupt management         | yes, automatically activated only if needed  | no                                                   | no                                | no |
| Number of interrupts per semi-period (1)         | number of instantiated dimmers + 1  | 100                                                   | 100                                | 3 |

(1) In the worst case, with default settings\
(2) If the hardware timer allows it, otherwise it will be lower

## Installation

The latest version of Dimmable Light for Arduino is available on Arduino Library Manager and on [PlatformIO registry](https://registry.platformio.org/libraries/fabianoriccardi/Dimmable%20Light%20for%20Arduino).

On AVR boards such as Arduino/Genuino Uno, you also need [ArduinoSTL](https://github.com/mike-matera/ArduinoSTL) (available on Arduino Library Manager).
If you want to compile the 6th example (the most complete), you also need [ArduinoSerialCommand](https://github.com/kroimon/Arduino-SerialCommand) library.

> 📝 *for AVR core*: use AVR Core v1.8.2 or lower. This is because an incompatibility between ArduinoSTL and new versions of AVR core.

> 📝 *for PlatformIO users*: in `platformio.ini` file it is recommeded to add in `env` section the setting `lib_compat_mode = strict` to avoid conflicts with the default STL included in all environments (but not in Arduino-AVR core) and ArduinoSTL.

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
