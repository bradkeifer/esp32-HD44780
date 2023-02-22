#pragma once

// flags for display on/off control instruction
#define LCD_DISPLAY_ON 0x04     /*!< Display control bitmask for Display On */
#define LCD_DISPLAY_OFF 0x00    /*!< Display control bitmask for Display Off */
#define LCD_CURSOR_ON 0x02      /*<! Display control bitmask for Cursor On */
#define LCD_CURSOR_OFF 0x00     /*!< Display control bitmask for Cursor Off */
#define LCD_BLINK_ON 0x01       /*!< Display control bitmask for Cursor Blink On */
#define LCD_BLINK_OFF 0x00      /*!< Display control bitmask for Cursor Blink Off */

// flags for cursor or display shift instruction
#define LCD_DISPLAY_MOVE 0x08   /*!< Cursor/Display Shift bitmask for Display Shift */
#define LCD_CURSOR_MOVE 0x00    /*!< Cursor/Display Shift bitmask for Cursor Move */
#define LCD_MOVE_RIGHT 0x04     /*!< Cursor/Display Shift bitmask for Shift Right */
#define LCD_MOVE_LEFT 0x00      /*!< Cursor/Display Shift bitmask for Shift Left */

// flags for function set instruction
#define LCD_8BIT_MODE 0x10      /*!< Function Set bitmask for 8 bit interface data length */
#define LCD_4BIT_MODE 0x00      /*!< Function Set bitmask for 4 bit interface data length */
#define LCD_2LINE 0x08          /*!< Function Set bitmask for multi-line display */
#define LCD_1LINE 0x00          /*!< Function Set bitmask for single line display */
#define LCD_5x10DOTS 0x04       /*!< Function Set bitmask for 5x10 dot matrix */
#define LCD_5x8DOTS 0x00        /*!< Function Set bitmask for 5x8 dot matrix */

// flags for backlight state
#define LCD_BACKLIGHT_ON 1      /*!< Backlight on state */
#define LCD_BACKLIGHT_OFF 0     /*!< Backlight off state */

// flags for display entry mode
// Refer Table 6 of datasheet for details
#define LCD_ENTRY_DECREMENT         0x00 /*!< Decrements DDRAM and shifts cursor left*/
#define LCD_ENTRY_INCREMENT         0x02 /*!< Increments DDRAM and shifts cursor right*/
#define LCD_ENTRY_DISPLAY_SHIFT     0x01 /*!< Shifts entire display. Right if decrement. Left if increment*/
#define LCD_ENTRY_DISPLAY_NO_SHIFT  0x00 /*!< Display does not shift*/


