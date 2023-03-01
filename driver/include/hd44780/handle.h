#pragma once

#include <driver/i2c.h>

/**
 * @brief LCD handle
 *
 * @details This is required to be populated with LCD Display configuration
 *          data prior to calling lcd_init().
 */
typedef struct lcd_handle_t
{
    i2c_port_t i2c_port;      /*!< I2C controller used. Must be populated prior to calling lcd_init(). */
    uint8_t address;          /*!< Address of the LCD on the I2C bus. Must be populated prior to calling lcd_init(). */
    uint8_t columns;          /*!< Number of columns. Must be populated prior to calling lcd_init(). */
    uint8_t rows;             /*!< Number of rows. Must be populated prior to calling lcd_init(). */
    uint8_t display_function; /*!< Current state of display function flag. Must be populated prior to calling lcd_init(). */
    uint8_t display_control;  /*!< Current state of display control flag. Must be populated prior to calling lcd_init(). */
    uint8_t display_mode;     /*!< Current state of display mode flag. Must be populated prior to calling lcd_init(). */
    uint8_t cursor_column;    /*!< Current column position of cursor. First column is position 0. */
    uint8_t cursor_row;       /*!< Current row position of cursor. First row is position 0. */
    uint8_t backlight;        /*!< Current state of backlight. */
    bool initialized;         /*!< Private flag to reflect initialization state. */

} lcd_handle_t;
