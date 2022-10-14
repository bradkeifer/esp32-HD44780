#pragma once

#include <stdbool.h>
#include <stdio.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// flags for display on/off control
#define LCD_DISPLAY_ON 0x04
#define LCD_DISPLAY_OFF 0x00
#define LCD_CURSOR_ON 0x02
#define LCD_CURSOR_OFF 0x00
#define LCD_BLINK_ON 0x01
#define LCD_BLINK_OFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAY_MOVE 0x08
#define LCD_CURSOR_MOVE 0x00
#define LCD_MOVE_RIGHT 0x04
#define LCD_MOVE_LEFT 0x00

// flags for function set
#define LCD_8BIT_MODE 0x10
#define LCD_4BIT_MODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// flags for backlight control
#define LCD_BACKLIGHT 0x08
#define LCD_NO_BACKLIGHT 0x00

// flags for display entry mode
#define LCD_ENTRY_RIGHT 0x00
#define LCD_ENTRY_LEFT 0x02
#define LCD_ENTRY_SHIFT_INCREMENT 0x01
#define LCD_ENTRY_SHIFT_DECREMENT 0x00

/**
 * @brief LCD handle structure
*/
typedef struct {
    i2c_port_t i2c_port;        /*!< I2C Controller used */
    uint8_t address;            /*!< Address of the LCD on the I2C bus */
    uint8_t columns;            /*!< Number of columns */
    uint8_t rows;               /*!< Number of rows */
    uint8_t display_function;   /*!< Current state of display function */
    uint8_t display_control;    /*!< Current state of display control */
    uint8_t display_mode;       /*!< Current state of display mode */
    uint8_t cursor_column;      /*!< Current column position of cursor */
    uint8_t cursor_row;         /*!< Current row position of cursor */
    uint8_t backlight;          /*!< Current state of backlight */

} lcd_handle_t;

/**
 * @brief Initialise LCD panel and lcd_handle data structure
 *
 * @param[inout] lcd_handle Handle to be used for future interaction with the LCD panel
 *
 * @return
 *          - ESP_ERR_INVALID_ARG   if parameter is invalid
 *          - ESP_ERR_NOT_FOUND     if LCD not found at the io handle
*           - ESP_OK                on success
*/
esp_err_t lcd_init(lcd_handle_t *lcd_handle);

/**
 * @brief Probe for existence of LCD at the specified address on the I2C bus
 *
 * @param[in] handle LCD handle
 *
 * @return  - ESP_OK                Success
 *          - ESP_ERR_NOT_FOUND     LCD not found
 *          - ESP_ERR_INVALID_ARG   Parameter error
 *          - ESP_ERR_INVALID_STATE I2C driver not installed or not in master mode
 *          - ESP_ERR_TIMEOUT       Operation timeout because the bus is busy
*/
esp_err_t lcd_probe(lcd_handle_t *handle);

/**
 * @brief Move the cursor to the home position
 *
 * @param[inout] handle LCD handle. Cursor position details are updated
 *
 * @return TBD - needs to use ESP_ERR in the long run
*/
void lcd_home(lcd_handle_t *handle);

void lcd_writeChar(char c);
void lcd_writeStr(char* str);
void lcd_setCursor(uint8_t col, uint8_t row);
void lcd_clearScreen(void);
void lcd_noDisplay(void);
void lcd_display(void);
void lcd_noCursor(void);
void lcd_cursor(void);
void lcd_noBlink(void);
void lcd_blink(void);
void lcd_scrollDisplayLeft(void);
void lcd_scrollDisplayRight(void);
void lcd_leftToRight(void);
void lcd_rightToLeft(void);
void lcd_autoscroll(void);
void lcd_noAutoscroll(void);
void lcd_createChar(uint8_t location, uint8_t charmap[]);
void lcd_backlight(void);
void lcd_noBacklight(void);
void lcd_setBackLight(uint8_t new_val);

#ifdef __cplusplus
}
#endif
