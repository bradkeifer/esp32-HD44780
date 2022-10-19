| Supported Targets | ESP32 | ESP32-H2 | ESP32-C3 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- |

# LCD Tools Example

(See the README.md file in the upper level 'examples' directory for more information about examples.)

## Overview

LCD Tools is a simple but very useful tool for developing LCD related applications. IT is derived from the [I2C Tools](https://github.com/espressif/esp-idf/tree/master/examples/peripherals/i2c/i2c_tools) example in the esp-idf. As follows, this example supports twenty two command-line tools:

1. `lcd_detect`: It will scan the configured I2C bus for devices and output a table with the list of detected devices on the bus. They may or may not be LCD devices.
2. `lcd_config`: It will configure the I2C bus with specific GPIO number, port number, frequency, LCD address, rows and columns.
3. `lcd_handle`: It will display the contents of the lcd_handle data structure which is used internally by the lcd API.
4. `lcd_init`: It will initialize the HD44780-based LCD, placing it in 4-bit interface data mode and readying it for using the following commands.
5. `lcd_home`: Executes the `Return home` instruction.
6. `lcd_write_str`: Writes a string of characters to the display.
7. `lcd_set_cursor`: Positions the cursor at the specified row and column position.
8. `lcd_clear_creen`: Executes the `Clear display` instruction.
9. `lcd_no_display`: Sets entire display off.
10. `lcd_display`: Sets entire display on.
11. `lcd_no_cursor`: Sets cursor off.
12. `lcd_cursor`: Sets cursor on.
13. `lcd_no_blink`: Sets blinking of cursor position off.
14. `lcd_blink`: Sets blinking of cursor position on.
15. `lcd_no_autoscroll`: Sets display scroll mode off.
16. `lcd_autoscroll`: Sets display scroll mode on. This will give appearance that the cursor does not move, but the displayed characters do.
17. `lcd_no_backlight`: Sets the backlight (if there is one) off.
18. `lcd_backlight`: Sets the backlight on.
19. `lcd_shift_l`: Shifts the entire display one character to the left.
20. `lcd_shift_r`: Shifts the entire display one character to the right.
21. `lcd_l_to_r`: Sets text direction for future character writes to be from left to right.
22. `lcd_r_to_l`: Sets text direction for future character writes to be from right to left.

If you have some trouble in developing LCD related applications, or just want to test some functions of the LCD device, you can play with this example first.

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

- You can choose whether or not to save command history into flash in `Store command history in flash` option.
- Select which `I2C peripheral to use`.
- Select the `I2C clock frequency`.
- Select the `SDA GPIO number` for the I2C bus.
- Select the `SCL GPIO number` for the I2C bus.
- Select the `LCD Address` for the LCD display on I2C bus.
- Select the `LCD Rows` for the LCD display.
- Select the `LCD Columns` for the LCD display.
- Select the initial state of the LCD backlight.

### Build and Flash

Run `idf.py -p PORT flash monitor` to build and flash the project..

(To exit the serial monitor, type ``Ctrl-]``.)

See the [Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for full steps to configure and use ESP-IDF to build projects.

## Example Output

### Check all supported commands and their usages

```bash
 ==============================================================
 |             Steps to Use lcd-tools                         |
 |                                                            |
 |  1. Try 'help', check all supported commands               |
 |  2. Try 'lcd_detect' to scan devices on the I2C bus        |
 |  3. Try 'lcd_config' to configure your I2C bus             |
 |  4. Try 'lcd_handle' to output the LCD handle data         |
 |  5. Try 'lcd_init' to initialize LCD                       |
 |  6. Try 'lcd_home' to return the cursor to home and        |
 |     display to to its original status if it was shifted.   |
 |  7. Try 'lcd_write_str' to write a string to the LCD       |
 |  8. Try 'lcd_set_cursor' to position the cursor at a       |
 |     specified row and column position.                     |
 |  9. Try 'lcd_clear_screen' to clear the display.           |
 |  10. Try 'lcd_no_display' to turn the display off.         |
 |  11. Try 'lcd_display' to turn the display on.             |
 |  12. Try 'lcd_no_cursor' to turn the cursor off.           |
 |  13. Try 'lcd_cursor' to turn the cursor on.               |
 |  14. Try 'lcd_no_blink' to turn blinking of the cursor off.|
 |  15. Try 'lcd_blink' to turn blinking of the cursor on.    |
 |  16. Try 'lcd_no_autoscroll' to turn display scroll off.   |
 |  17. Try 'lcd_autoscroll' to turn display scroll on.       |
 |  18. Try 'lcd_no_backlight' to turn backlight off.         |
 |  19. Try 'lcd_backlight' to turn backlight on.             |
 |  20. Try 'lcd_shift_l' to shift the display left.          |
 |  21. Try 'lcd_shift_r' to shift the display right.         |
 |  22. Try 'lcd_l_to_r' set the text direction left to right.|
 |  23. Try 'lcd_r_to_l' set the text direction right to left.|
 |                                                            |
 ==============================================================

lcd-tools> help
help 
  Print the list of registered commands

lcd_config  --i2c_port=<0|1> --address=<0xaddr> --columns=<columns> --rows=<rows>
  Config LCD Parameters
  --i2c_port=<0|1>  Set the I2C bus port number
  --address=<0xaddr>  Set the address of the LCD on the I2C bus
  --columns=<columns>  Set the number of columns of the LCD
  --rows=<rows>  Set the number of rows of the LCD

lcd_init 
  Initialise the LCD panel

lcd_detect 
  Scan I2C bus for devices (may or may not be LCD\'s)

lcd_handle 
  Output the LCD handle data

lcd_home 
  Return home

lcd_write_str  --string=<string>
  Write a string of character to the LCD
  --string=<string>  Character string to write to LCD

lcd_set_cursor  -c <column> -r <row>
  Set the cursor position
  -c, --column=<column>  Set the column number to move to
  -r, --row=<row>  Set the row number to move to

lcd_clear_screen 
  Clear the display

lcd_no_display 
  Turn the display off

lcd_display 
  Turn the display on

lcd_no_cursor 
  Turn the cursor off

lcd_cursor 
  Turn the cursor on

lcd_no_blink 
  Turn the cursor blink off

lcd_blink 
  Turn the cursor blink on

lcd_no_autoscroll 
  Disable autoscroll

lcd_autoscroll 
  Enable autoscroll

lcd_no_backlight 
  Disable backlight

lcd_backlight 
  Enable backlight

lcd_shift_l 
  Shift display left

lcd_shift_r 
  Shift display right

lcd_l_to_r 
  Set text direction to be left to right

lcd_r_to_l 
  Set text direction to be right to left

free 
  Get the current size of free heap memory

heap 
  Get minimum size of free heap memory that was available during program execu
  tion

version 
  Get version of chip and SDK

restart 
  Software reset of the chip

tasks 
  Get information about running tasks

deep_sleep  [-t <t>] [--io=<n>] [--io_level=<0|1>]
  Enter deep sleep mode. Two wakeup modes are supported: timer and GPIO. If no
  wakeup option is specified, will sleep indefinitely.
  -t, --time=<t>  Wake up time, ms
      --io=<n>  If specified, wakeup using GPIO with given number
  --io_level=<0|1>  GPIO level to trigger wakeup

light_sleep  [-t <t>] [--io=<n>]... [--io_level=<0|1>]...
  Enter light sleep mode. Two wakeup modes are supported: timer and GPIO. Mult
  iple GPIO pins can be specified using pairs of 'io' and 'io_level' arguments
  . Will also wake up on UART input.
  -t, --time=<t>  Wake up time, ms
      --io=<n>  If specified, wakeup using GPIO with given number
  --io_level=<0|1>  GPIO level to trigger wakeup
```

### Configure the LCD

```bash
lcd-tools> lcd_config --i2c_port=0 --address=0x3f --columns=20 --rows=4
```

* `--i2c_port` option to specify the port of I2C, here we choose port 0 for test.
* `--address` option to specify the I2C address of the LCD, here we choose 0x3f.
* `--columns` option to specify the number of columns of the LCD, here we choose 20.
* `--rows` option to specify the number of rows of the LCD, here we choose 4.

### Check the LCD address (7 bits) on the I2C bus

```bash
lcd-tools> lcd_detect
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 3f 
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
70: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
```

* Here we found the address of the LCD is 0x3f.

### Initialize the LCD

```bash
lcd-tools> lcd_init
LCD successfully initialised
```

### Display some characters on the display

```bash
lcd-tools> lcd_write_str --string=Hello\ World
Success writing string: Hello World
```

* The string `Hello World` is displayed left to right on the LCD from the home cursor position.

## Troubleshooting

* I don’t find any available address when running `lcd_detect` command.
  * Make sure your wiring connection is right.
  * Make sure you have configured your project with correct I2C pin assignments.
  * Reset your LCD device, and then run `lcd_detect` again.
* Install and run the [I2C Tools](https://github.com/espressif/esp-idf/tree/master/examples/peripherals/i2c/i2c_tools) example from the [esp-idf](https://github.com/espressif/esp-idf/) to check that the I2C interface is working correctly.

(For any technical queries, please open an [issue](https://github.com/bradkeifer/esp32-HD44780/issues) on GitHub. I will get back to you as soon as possible.)
