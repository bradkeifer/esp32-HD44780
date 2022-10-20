# ESP32 Liquid Crystal Display (LCD) Driver

## General Information

This repository hosts  a ESP32 series Soc compatible driver for I2C-based alphanumeric Liquid Crystal Displays (LCD's) that use the Hitachi HD44780 controller.

Inspired by and derived from the Arduino [LiquidCrystal_I2C](https://github.com/johnrickman/LiquidCrystal_I2C) driver and the subsequent basic port [ESP-IDF HD44780 (I2C) Driver](https://github.com/maxsydney/ESP32-HD44780).

## Installation Instructions


### Using esp-idf

- Clone or download and extract the repository to the components folder of your ESP-IDF project
- Configure I2C bus and LCD peripheral(s) using `menuconfig`
- Include `lcd.h` in your code

## Examples

### Initialisation

```c
#include "lcd.h"

/**
 * Work in progress
*/
```
 