#pragma once

#include <stdbool.h>
#include <stdio.h>
#include "esp_err.h"
#include "driver/i2c.h"
#include "sdkconfig.h"

#ifdef __cplusplus
extern "C" {
#endif

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


// Configuration Items
#define I2C_MASTER_SDA_IO CONFIG_SDA_GPIO /*!< GPIO for I2C SDA signal. Set with menuconfig. */
#define I2C_MASTER_SCL_IO CONFIG_SCL_GPIO /*!< GPIO for I2C SCL signal. Set with menuconfig. */
#define I2C_MASTER_NUM I2C_NUM_0  /*!< I2C port number, the number of I2C peripheral interfaces available will depend on the chip. Set with menuconfig. */
#ifdef CONFIG_HARDWARE_I2C_PORT1
#define I2C_MASTER_NUM I2C_NUM_1
#endif
#define I2C_MASTER_FREQ_HZ CONFIG_I2C_CLK_FREQ /*!< I2C master clock frequency. Set with menuconfig. */
#define LCD_ADDR CONFIG_LCD_ADDR /*!< Address of the display on the I2C bus. Set with menuconfig. */
#define LCD_ROWS CONFIG_LCD_ROWS /*!< Number of rows in the display. Set with menuconfig. */
#define LCD_COLUMNS CONFIG_LCD_COLUMNS /*!< Number of columns in the display. Set with menuconfig. */
#define LCD_BACKLIGHT LCD_BACKLIGHT_ON /*!< Initial state of the backlight. Set with menuconfig. */
#ifdef CONFIG_LCD_BACKLIGHT_OFF
#define LCD_BACKLIGHT LCD_BACKLIGHT_OFF
#endif


/**
 * @brief Macro to set default LCD configuration
 *
 * @details
 *          - i2c_port = I2C_MASTER_NUM
 *          - address = LCD_ADDR
 *          - columns = LCD_COLUMNS
 *          - rows = LCD_ROWS
 *          - display_function = LCD_4BIT_MODE | LCD_2LINE | LCD_5x8DOTS
 *          - display_control = LCD_DISPLAY_ON | LCD_CURSOR_OFF | LCD_BLINK_OFF
 *          - display_mode = LCD_ENTRY_INCREMENT | LCD_ENTRY_DISPLAY_NO_SHIFT
 *          - cursor_column = 0
 *          - cursor_row = 0
 *          - backlight = LCD_BACKLIGHT
 *          - initialized = false
*/
#define LCD_HANDLE_DEFAULT_CONFIG() {                                   \
    .i2c_port = I2C_MASTER_NUM,                                         \
    .address = LCD_ADDR,                                                \
    .columns = LCD_COLUMNS,                                             \
    .rows = LCD_ROWS,                                                   \
    .display_function = LCD_4BIT_MODE | LCD_2LINE | LCD_5x8DOTS,        \
    .display_control = LCD_DISPLAY_ON | LCD_CURSOR_OFF | LCD_BLINK_OFF, \
    .display_mode = LCD_ENTRY_INCREMENT | LCD_ENTRY_DISPLAY_NO_SHIFT,   \
    .cursor_column = 0,                                                 \
    .cursor_row = 0,                                                    \
    .backlight = LCD_BACKLIGHT,                                         \
    .initialized = false,                                               \
}

/**
 * @brief LCD handle
 *
 * @details This is required to be populated with LCD Display configuration
 *          data prior to calling lcd_init().
*/
typedef struct {
    i2c_port_t i2c_port;        /*!< I2C controller used. Must be populated prior to calling lcd_init(). */
    uint8_t address;            /*!< Address of the LCD on the I2C bus. Must be populated prior to calling lcd_init(). */
    uint8_t columns;            /*!< Number of columns. Must be populated prior to calling lcd_init(). */
    uint8_t rows;               /*!< Number of rows. Must be populated prior to calling lcd_init(). */
    uint8_t display_function;   /*!< Current state of display function flag. Must be populated prior to calling lcd_init(). */
    uint8_t display_control;    /*!< Current state of display control flag. Must be populated prior to calling lcd_init(). */
    uint8_t display_mode;       /*!< Current state of display mode flag. Must be populated prior to calling lcd_init(). */
    uint8_t cursor_column;      /*!< Current column position of cursor. First column is position 0. */
    uint8_t cursor_row;         /*!< Current row position of cursor. First row is position 0. */
    uint8_t backlight;          /*!< Current state of backlight. */
    bool initialized;           /*!< Private flag to reflect initialization state. */

} lcd_handle_t;

/**
 * @brief Initialise LCD panel and lcd_handle data structure
 *
 * @param[inout] lcd_handle Handle to be used for future interaction with the LCD panel
 *
 * @details I2C driver must be configured and installed prior to calling lcd_init().
 *
 * @return
 *          - ESP_OK                Success
 *          - ESP_ERR_INVALID_ARG   if parameter is invalid
 *          - ESP_ERR_INVALID_STATE I2C driver not installed or not in master mode
 *          - ESP_ERR_NOT_FOUND     if LCD not found at the io handle
*/
esp_err_t lcd_init(lcd_handle_t *lcd_handle);

/**
 * @brief Probe for existence of LCD at the specified address on the I2C bus
 *
 * @param[in] handle LCD handle
 *
 * @return
 *          - ESP_OK                Success
 *          - ESP_ERR_NOT_FOUND     LCD not found
 *          - ESP_ERR_INVALID_ARG   Parameter error
 *          - ESP_ERR_INVALID_STATE I2C driver not installed or not in master mode
 *          - ESP_ERR_TIMEOUT       Operation timeout because the bus is busy
*/
esp_err_t lcd_probe(lcd_handle_t *handle);

/**
 * @brief Move the cursor to the home position
 *
 * @details This is a relatively slow function, taking 1.52ms with a
 *          270kHz clock. Please use lcd_clear_screen() as a faster
 *          alternative if possible.
 *
 * @param[inout] handle LCD handle. Cursor position details are updated
 *
 * @return
 *          - ESP_OK     Success
 *          - ESP error code propagated from error source
*/
esp_err_t lcd_home(lcd_handle_t *handle);

/**
 * @brief Write a character to the LCD
 *
 * @param[inout] handle LCD handle. Cursor position details are updated
 * @param[in] c Character to be written to the LCD at the current cursor position
 *
 * @return
 *          - ESP_OK     Success
 *          - ESP_ERR_INVALID_SIZE Write would cause screen display overflow
 *          - ESP error code propagated from error source
*/
esp_err_t lcd_write_char(lcd_handle_t *handle, char c);

/**
 * @brief Write a character string to the LCD
 *
 * @details Characters in the string are written to the LCD one character at a time.
 *          If the character string is too long for the LCD, only the characters that
 *          will fit on the display will be written and ESP_ERR_INVALID_SIZE error will
 *          be returned.
 *          NOTE: This is not true - pls check the code Brad.
 *
 * @param[inout] handle LCD handle. Cursor position details are updated
 * @param[in] str Character string to be written to the LCD starting at the current
 *          cursor position
 *
 * @return
 *          - ESP_OK     Success
 *          - ESP_ERR_INVALID_SIZE Write would cause screen display overflow
 *          - ESP error code propagated from error source
*/
esp_err_t lcd_write_str(lcd_handle_t *handle, char *str);

/**
 * @brief Move the cursor to a specified row and column
 *
 * @details Column and row numbers commence at 0. Therefore, the home position is
 *          col=0, row=0.
 *
 * @param[inout] handle LCD handle. Cursor position details are updated.
 * @param[in] col The column number to move the cursor to.
 * @param[in] row The row number to move the cursor to.
 *
 * @return
 *          - ESP_OK     Success
 *          - ESP_ERR_INVALID_ARG   Invalid parameter
 *          - ESP error code propagated from error source
*/
esp_err_t lcd_set_cursor(lcd_handle_t *handle, uint8_t col, uint8_t row);

/**
 * @brief Clear the display. Cursor row and column reset to 0.
 *
 * @details This function is very fast to execute and should be used
 *          instead of it's sibling function lcd_home(), which is much
 *          slower. Refer Table 6 of HD44780U datasheet for details.
 *
 * @param[inout] handle LCD. Cursor position details are updated
 *
 * @return
 *          - ESP_OK    Success
 *          - ESP error code propagated from error source
*/
esp_err_t lcd_clear_screen(lcd_handle_t *handle);

/**
 * @brief Turn the display off
 *
 * @param[inout] handle LCD. Display control details are updated
 *
 * @return
 *          - ESP_OK    Success
 *          - ESP error code propagated from error source
*/
esp_err_t lcd_no_display(lcd_handle_t *handle);

/**
 * @brief Turn the display on
 *
 * @param[inout] handle LCD. Display control details are updated
 *
 * @return
 *          - ESP_OK    Success
 *          - ESP error code propagated from error source
*/
esp_err_t lcd_display(lcd_handle_t *handle);

/**
 * @brief Turn the cursor off
 *
 * @param[inout] handle LCD. Display control details are updated
 *
 * @return
 *          - ESP_OK    Success
 *          - ESP error code propagated from error source
*/
esp_err_t lcd_no_cursor(lcd_handle_t *handle);

/**
 * @brief Turn the cursor on
 *
 * @param[inout] handle LCD. Display control details are updated
 *
 * @return
 *          - ESP_OK    Success
 *          - ESP error code propagated from error source
*/
esp_err_t lcd_cursor(lcd_handle_t *handle);

/**
 * @brief Turn character blink at cursor position off
 *
 * @param[inout] handle LCD. Display control details are updated
 *
 * @return
 *          - ESP_OK    Success
 *          - ESP error code propagated from error source
*/
esp_err_t lcd_no_blink(lcd_handle_t *handle);

/**
 * @brief Turn character blink at cursor position on
 *
 * @param[inout] handle LCD. Display control details are updated
 *
 * @return
 *          - ESP_OK    Success
 *          - ESP error code propagated from error source
*/
esp_err_t lcd_blink(lcd_handle_t *handle);

/**
 * @brief Shifts the display to the left
 *
 * @details Shifts the display left without writing or reading display data.
 *          This function is used to correct or search the display.
 *          The address counter contents will not change.
 *
 * @param[in] handle LCD.
 *
 * @return
 *          - ESP_OK    Success
 *          - ESP error code propagated from error source
*/
esp_err_t lcd_display_shift_left(lcd_handle_t *handle);

/**
 * @brief Shifts the display to the right
 *
 * @details Shifts the display right without writing or reading display data.
 *          This function is used to correct or search the display.
 *          The address counter contents will not change.
 *
 * @param[in] handle LCD.
 *
 * @return
 *          - ESP_OK    Success
 *          - ESP error code propagated from error source
*/
esp_err_t lcd_display_shift_right(lcd_handle_t *handle);

/**
 * @brief Set text direction to be left to right
 *
 * @details Increments the DDRAM address by 1 when a character code is written
 *          into or read from DDRAM
 *
 * @param[in] handle LCD. Display control details are updated
 *
 * @return
 *          - ESP_OK    Success
 *          - ESP error code propagated from error source
*/
esp_err_t lcd_left_to_right(lcd_handle_t *handle);

/**
 * @brief Set text direction to be right to left
 *
 * @details Decrements the DDRAM address by 1 when a character code is written
 *          into or read from DDRAM
 *
 * @param[in] handle LCD. Display control details are updated
 *
 * @return
 *          - ESP_OK    Success
 *          - ESP error code propagated from error source
*/
esp_err_t lcd_right_to_left(lcd_handle_t *handle);

/**
 * @brief Enable autoscroll
 *
 * @details This gives the effect that the cursor will seem to not move but the
 *          display does. Enabling this function breaks the row and column tracking
 *          capability of the lcd_handle and hence is not yet supported by the LCD API.
 *
 * @param[inout] handle Display mode is updated.
 *
 * @return
 *          - ESP_ERR_NOT_SUPPORTED This function is not yet supported
*/
esp_err_t lcd_autoscroll(lcd_handle_t *handle);

/**
 * @brief Disable autoscroll
 *
 * @param[inout] handle Display mode is updated.
 *
 * @return
 *          - ESP_OK    Success
 *          - ESP error code propagated from error source
*/
esp_err_t lcd_no_autoscroll(lcd_handle_t *handle);

/*
void lcd_createChar(uint8_t location, uint8_t charmap[]);
*/

/**
 * @brief Enables backlight.
 *
 * @param[inout] handle Backlight element of the handle is updated.
 *
 * @return
 *          - ESP_OK    Success
 *          - ESP error code propagated from error source
*/
esp_err_t lcd_backlight(lcd_handle_t *handle);

/**
 * @brief Disables backlight.
 *
 * @param[inout] handle Backlight element of the handle is updated.
 *
 * @return
 *          - ESP_OK    Success
 *          - ESP error code propagated from error source
*/
esp_err_t lcd_no_backlight(lcd_handle_t *handle);

#ifdef __cplusplus
}
#endif
