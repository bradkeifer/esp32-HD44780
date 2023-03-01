#pragma once

#include <esp_err.h>

#include "fwd.h"

#ifdef __cplusplus
extern "C" {
#endif


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
esp_err_t lcd_probe(const lcd_handle_t *handle);

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


/**
 * @brief Write custom character to CGRAM at specified location.
 *
 * @details Writes character given in byte array[8] to CGRAM and resets cursor address.
 *
 * @param[inout] handle LCD handle. Character written to CGRAM. Cursor position set to 0.
 * @param[in] location The location in CGRAM.
 * @param[in] charmap The character bitmap in form of byte array[8].
 *
 * @return
 *          - ESP_OK     Success
 *          - ESP_ERR_INVALID_ARG   Invalid parameter
 *          - ESP error code propagated from error source
*/
esp_err_t lcd_write_cgram(lcd_handle_t *handle, uint8_t location, uint8_t *charmap);


#ifdef __cplusplus
}
#endif
