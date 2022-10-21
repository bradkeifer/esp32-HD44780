Liquid Crystal Display (LCD)
============================

Overview
--------

The lcd library provides support for configuring and displaying text to alphanumeric dot-matrix liquid crystal displays that are based on the Hitachi HD74480 controller and connected to the ESP32 via I2C.

Application Examples
--------------------

Application examples demonstrating the functionality of the LCD library are provided in the examples directory of the repository. There are two example apps provided, :example:`lcd_tools` and :example:`lcd_example`.

:example:`lcd_tools` was derived from `i2c_tools <https://github.com/espressif/esp-idf/tree/v4.1/examples/peripherals/i2c/i2c_tools>`_ and provides a tool for testing the functionality of the lcd API and also to test the capability of an attached LCD display.

:example:`lcd_example` demonstrates some of the features of the LCD API and provides an example application that could be used where an alphanumeric liquid crystal display (LCD) based on the HD44780 chipset is connected to the esp32 MCU via an I2C bus.

.. ---------------------------- API Reference ----------------------------------

API Reference
-------------

.. include-build-file:: inc/lcd.inc
