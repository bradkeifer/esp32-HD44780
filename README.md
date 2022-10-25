# ESP32 Liquid Crystal Display (LCD) Driver

## General Information

This repository hosts a ESP32 series Soc compatible driver for I2C-based alphanumeric Liquid Crystal Displays (LCD's) that use the Hitachi HD44780 controller. Additionally it provides some examples to assist in understanding the usage of the driver and to test its functionality.

Inspired by and derived from the Arduino [LiquidCrystal_I2C](https://github.com/johnrickman/LiquidCrystal_I2C) driver and the subsequent basic port [ESP-IDF HD44780 (I2C) Driver](https://github.com/maxsydney/ESP32-HD44780).

## API Documentation

Documentation of the API can be found [here](https://lcd.keifer1.duckdns.org).

## Installation Instructions


### Using esp-idf

- Clone or download and extract the repository to the components folder of your ESP-IDF project
- Configure I2C bus and LCD peripheral(s) using `menuconfig`
- Include `lcd.h` in your code

### Connecting the LCD Display

Chances are that the LCD Display will require 5V power supply. The ESP32 is a 3V3 device and the I2C SDA and SCL lines will almost certainly require the use of external pull-up resistors. 

I found that the example apps worked fine without using external pull-up resistors, but when I incorporated the LCD component into a more complex app that used wi-fi, I experienced strange access point connectivity issues. I eventually found that the solution to this was to apply 4k7 ohm pull-up resistors to a 3V3 power rail for both the SDA and SCL lines. A more complete solution would be to implement level shifting techniques on the I2C bus, as per NXP Semiconductors application note [AN10441](https://cdn-shop.adafruit.com/datasheets/AN10441.pdf).

## Examples

Two example apps are provided in the examples directory:

- lcd_example: This is a simple example and demonstrates initialization and some of the features of the LCD display.

- lcd_tools: This is a tool to test connectivity and all features of your LCD display.
