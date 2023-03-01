#pragma once

#include <driver/i2c.h>
#include "sdkconfig.h"

#include "control.h"
#include "handle.h"

// Configuration Items
#define I2C_MASTER_SDA_IO CONFIG_SDA_GPIO /*!< GPIO for I2C SDA signal. Set with menuconfig. */
#define I2C_MASTER_SCL_IO CONFIG_SCL_GPIO /*!< GPIO for I2C SCL signal. Set with menuconfig. */
#define I2C_MASTER_NUM I2C_NUM_0          /*!< I2C port number, the number of I2C peripheral interfaces available will depend on the chip. Set with menuconfig. */
#ifdef CONFIG_HARDWARE_I2C_PORT1
#define I2C_MASTER_NUM I2C_NUM_1
#endif
#define I2C_MASTER_FREQ_HZ CONFIG_I2C_CLK_FREQ /*!< I2C master clock frequency. Set with menuconfig. */
#define LCD_ADDR CONFIG_LCD_ADDR               /*!< Address of the display on the I2C bus. Set with menuconfig. */
#define LCD_ROWS CONFIG_LCD_ROWS               /*!< Number of rows in the display. Set with menuconfig. */
#define LCD_COLUMNS CONFIG_LCD_COLUMNS         /*!< Number of columns in the display. Set with menuconfig. */
#define LCD_BACKLIGHT LCD_BACKLIGHT_ON         /*!< Initial state of the backlight. Set with menuconfig. */
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
#define LCD_HANDLE_DEFAULT_CONFIG()                                         \
    {                                                                       \
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
