# AC Infinity ESP32 Mods

[![License: CC BY-SA 4.0](https://img.shields.io/badge/License-CC%20BY--SA%204.0-lightgrey.svg)](https://creativecommons.org/licenses/by-sa/4.0/)

This repository contains code and instructions to modify the AC Infinity products with an ESP32 microcontroller. The ESP32 is a powerful microcontroller that can be used to control the fan speed, monitor temperature and humidity, and connect to the internet. The modifications described in this repository will allow you to control your fan remotely, set up schedules, and monitor environmental conditions in your grow room or other indoor space.


## Airtap T-Series ESP32 Mods

Code and instructions to modify the AC Infinity Airtap T-Series inline duct fans with an ESP32 microcontroller. 

**Official Tindie Store:** [AC Infinity Airtap T-Series PCB](https://plantcare.li/airtap-t4?utm_source=github&utm_medium=link&utm_campaign=esp32-mods)

**Youtube Video:** [Shorts](https://www.youtube.com/shorts/T40RKEEfJKI)


### How to identify your Generation

3 buttons is Gen 1, 4 buttons is Gen 2.

![Vent Comparison](Airtap-Tx/compare-vent.png "Vent Comparison")

![PCB Comparison](Airtap-Tx/compare-pcb.png "PCB Comparison")

### Gen 1 Rev 1

This revision of Gen 1 has an aligment issue. When installed you should only use two left screws to secure the PCB. The right screws will not align with the holes. Also includes individual fan control with two PWM outputs.

[ESPHome Configuration](Airtap-Tx/Gen-1/esphome-config-rev1.md)

#### Features
 - 2x PWM Output
 - 1x Temperature Sensor
 - 1x Display
 - 3x Button

### Gen 1 Rev 2

This is the second revision of Gen 1. It has the same layout with fixed hole aligment. The second PWM output was removed and both fans are controlled from a single PWM output.

[ESPHome Configuration](Airtap-Tx/Gen-1/esphome-config-rev2.md)

#### Features
 - 1x PWM Output
 - 1x Temperature Sensor
 - 1x Display
 - 3x Button

### Gen 2 Rev 1

This is the second Generation of Airtap series. AC Infinity introduced a larger display, 4 buttons, bluetooth and a new PCB layout. This revision has a single PWM output and a temperature sensor.

[ESPHome Configuration](Airtap-Tx/Gen-2/esphome-config-rev1.md)

#### Features
 - 1x PWM Output
 - 1x Temperature Sensor
 - 1x Display
 - 4x Button