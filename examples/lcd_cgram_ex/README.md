# LCD Example

(See the README.md file in the upper level 'examples' directory for more information about examples.)

This example demonstrates the use of CGRAM for creating custom characters and provides an example application that could be used with an LCD based on the HD44780 chipset.

## How to use example

### Hardware Required

To run this example, you should have any ESP32, ESP32-S and ESP32-C based development board and a HD44780-based LCD display with an I2C interface.

#### Pin Assignment:

**Note:** The following pin assignments are used by default, you can change them when you configure the project.

|                     | SDA    | SCL    |
| ------------------- | ------ | ------ |
| ESP32 I2C Master    | GPIO18 | GPIO19 |
| ESP32-S2 I2C Master | GPIO18 | GPIO19 |
| ESP32-S3 I2C Master | GPIO48 | GPIO2  |
| ESP32-C3 I2C Master | GPIO8  | GPIO6  |
| ESP32-H2 I2C Master | GPIO8  | GPIO6  |
| LCD Display         | SDA    | SCL    |

**Note: ** There’s no need to add an external pull-up resistors for SDA/SCL pin, because the driver will enable the internal pull-up resistors itself.

### Configure the project

Open the project configuration menu (`idf.py menuconfig`). Then go into `Example Configuration` menu.

- Select which `I2C peripheral to use`.
- Select the `I2C clock frequency`.
- Select the `SDA GPIO number` for the I2C bus.
- Select the `SCL GPIO number` for the I2C bus.
- Select the `LCD Address` for the LCD display on I2C bus.
- Select the `LCD Rows` for the LCD display.
- Select the `LCD Columns` for the LCD display.
- Select the initial state of the LCD backlight.

### Build and Flash

Build the project and flash it to the board, then run monitor tool to view serial output:

```
idf.py -p PORT flash monitor
```

(Replace PORT with the name of the serial port to use.)

(To exit the serial monitor, type ``Ctrl-]``.)

See the Getting Started Guide for full steps to configure and use ESP-IDF to build projects.

## Example Output


The display should be lit. Eight custom characters are being displayed twice, for a total of 16, with a delay of 100ms each.
The backlight is turned off, and after a short delay, turned back on & the display is cleared.

## Troubleshooting

* App generates errors and aborts.
  * Make sure your wiring connection is correct.
  * Make sure you have configured your project with correct I2C pin assignments and LCD configuration.
  * Rebuild your app, and then rerun.
* Install and run the [I2C Tools](https://github.com/espressif/esp-idf/tree/master/examples/peripherals/i2c/i2c_tools) example from the [esp-idf](https://github.com/espressif/esp-idf/) to check that the I2C interface is working correctly.

(For any technical queries, please open an [issue](https://github.com/bradkeifer/esp32-HD44780/issues) on GitHub. I will get back to you as soon as possible.)
