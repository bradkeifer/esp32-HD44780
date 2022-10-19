# LCD Example

(See the README.md file in the upper level 'examples' directory for more information about examples.)

This example demonstrates some of the features of the LCD API and provides an example application that could be used where an alphanumeric liquid crystal display (LCD) based on the HD44780 chipset is connected to the esp32 MCU via an I2C bus.

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

```
I (390) lcd_example: Running LCD Demo
I (390) lcd_example: Clear screen
I (390) lcd_example: Write string:<columns>x<rows> I2C LCD
I (1430) lcd_example: Clear screen
I (1430) lcd_example: Write string:Lets write some characters!
I (2520) lcd_example: Clear screen
I (2530) lcd_example: Set cursor on column 0, row 0
I (2560) lcd_example: Testing text direction right to left
I (3600) lcd_example: Reverting text direction to left to right
I (3670) lcd_example: Shift display right
I (3680) lcd_example: Finished row 0
I (4680) lcd_example: Set cursor on column 0, row 1
I (4710) lcd_example: Testing text direction right to left
I (5750) lcd_example: Reverting text direction to left to right
I (5820) lcd_example: Shift display left
I (5830) lcd_example: Finished row 1
I (6830) lcd_example: Set cursor on column 0, row 2
I (6860) lcd_example: Testing text direction right to left
I (7900) lcd_example: Reverting text direction to left to right
I (7970) lcd_example: Shift display right
I (7980) lcd_example: Finished row 2
I (8980) lcd_example: Set cursor on column 0, row 3
I (9010) lcd_example: Testing text direction right to left
I (10050) lcd_example: Reverting text direction to left to right
I (10120) lcd_example: Shift display left
I (10130) lcd_example: Finished row 3
I (11130) lcd_example: LCD Demo finished
```

For a 20x4 LCD display, the display will briefly show the following characters at the end of the LCD Demo cycle, and then the cycle will repeat indefinitely:
```
56789:;<=>?@ABCDEFGH
56789:;<=>?@ABCDEFGH
56789:;<=>?@ABCDEFGH
56789:;<=>?@ABCDEFGH
```

## Troubleshooting

* App generates errors and aborts.
  * Make sure your wiring connection is correct.
  * Make sure you have configured your project with correct I2C pin assignments and LCD configuration.
  * Rebuild your app, and then rerun.
* Install and run the [I2C Tools](https://github.com/espressif/esp-idf/tree/master/examples/peripherals/i2c/i2c_tools) example from the [esp-idf](https://github.com/espressif/esp-idf/) to check that the I2C interface is working correctly.

(For any technical queries, please open an [issue](https://github.com/bradkeifer/esp32-HD44780/issues) on GitHub. I will get back to you as soon as possible.)

## Example Breakdown

For a 20 column, 4 row LCD, the LCD Demo app loops through a cycle of:
* Checking for existence of LCD at the configured I2C address
* Clearing the LCD screen
* Writing the string ```20x4 I2C LCD```
* Writing the string ```Lets write some characters!```
* Clearing the LCD screen
* Setting the cursor on and to blink
* For each row of the display:
 * Move the cursor to the leftmost column of the row
 * Starts writing ascii characters left to right, starting with '!' and incrementing through the ascii character set.
 * When half way across the row, the text direction is changed to be right to left and further characters are written back to the leftmost column of the display.
 * The text direction is than changed back to left to right and incremented ascii characters are written across the entire row.
  * If the row is even numbered, the entire display is shifted right by one character space.
  * If the row is odd numbered, the entire display is shifted left by one character space.
