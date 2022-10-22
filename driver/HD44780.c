#include <stdio.h>
#include "driver/i2c.h"
#include "esp_log.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "rom/ets_sys.h"
#include "lcd.h"
#include "hd44780.h"

// Pin mappings
// P0 -> RS
// P1 -> RW
// P2 -> E
// P3 -> Backlight
// P4 -> D4
// P5 -> D5
// P6 -> D6
// P7 -> D7

// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set:
//    DL = 1; 8-bit interface data
//    N = 0; 1-line display
//    F = 0; 5x8 dot character font
// 3. Display on/off control:
//    D = 0; Display off
//    C = 0; Cursor off
//    B = 0; Blinking off
// 4. Entry mode set:
//    I/D = 1; Increment by 1
//    S = 0; No shift
//
// Note, however, that resetting the esp32 doesn't reset the LCD, so we
// can't assume that its in that state when task starts.
//
// Also note, that on power up the LCD internal reset circuitry remains in a
// busy state for 10 ms after Vcc rises to 4.5V.

static const char *TAG = "LCD Driver";

/**
 * @brief Transmit 4 bits of data to the LCD panel
 *
 * @param[in] handle The LCD handle
 * @param[in] nibble The 4 bits of data to be sent.
 * @param[in] mode [** to be defined **]
 */
static esp_err_t lcd_write_nibble(lcd_handle_t *handle, uint8_t nibble, uint8_t mode);

/**
 * @brief Manage incrementing the cursor column of the LCD handle
 *
 * @details The cursor sits one position ahead of the last character written and
 *          this means that the cursor will overflow into a new row on a multi-row
 *          display when the last charater is written into a row or if the display
 *          is shifted
 *
 * @param[inout] handle The LCD handle. Cursor position details will be updated
 *
 * @returns - ESP_OK Success
 *          - ESP_ERR_INVALID_ARG   Invalid paramater
*/
static esp_err_t lcd_handle_increment_cursor(lcd_handle_t *handle);

/**
 * @brief Manage decrementing the cursor column of the LCD handle
 *
 * @details The cursor sits one position ahead of the last character written and
 *          this means that the cursor will overflow into a new row on a multi-row
 *          display when the last charater is written into a row or if the display
 *          is shifted
 *
 * @param[inout] handle The LCD handle. Cursor position details will be updated
 *
 * @returns - ESP_OK Success
 *          - ESP_ERR_INVALID_ARG   Invalid paramater
*/
static esp_err_t lcd_handle_decrement_cursor(lcd_handle_t *handle);

static esp_err_t lcd_write_byte(lcd_handle_t *handle, uint8_t data, uint8_t mode);
static esp_err_t lcd_pulse_enable(lcd_handle_t *handle, uint8_t nibble);
static esp_err_t lcd_i2c_detect(i2c_port_t port, uint8_t address);
static esp_err_t lcd_i2c_write(i2c_port_t port, uint8_t address, uint8_t data);

esp_err_t lcd_init(lcd_handle_t *handle)
{
    esp_err_t ret = ESP_OK;

    ESP_LOGD(TAG,
             "Initialising LCD with:\n\ti2c_port: %d\n\tAddress: 0x%0x\n\tColumns: %d\n\tRows: %d\n\tDisplay Function: 0x%0x\n\tDisplay Control: 0x%0x\n\tDisplay Mode: 0x%0x\n\tCursor Column: %d\n\tCursor Row: %d\n\tBacklight: %d\n\tInitialised: %d",
             handle->i2c_port, handle->address, handle->columns, handle->rows,
             handle->display_function, handle->display_control, handle->display_mode,
             handle->cursor_column, handle->cursor_row, handle->backlight,
             handle->initialized);

    if (handle->display_function & LCD_8BIT_MODE)
    {
        ESP_LOGE(TAG, "8 bit mode not yet supported");
        return ESP_ERR_INVALID_ARG;
    }

    if (handle->initialized)
    {
        ESP_LOGE(TAG, "LCD already initialized");
        return ESP_ERR_INVALID_STATE;
    }

    // Initialise the LCD controller by instruction for 4-bit interface
    // First part of reset sequence
    ESP_GOTO_ON_ERROR(
        lcd_write_nibble(handle, LCD_FUNCTION_SET | LCD_8BIT_MODE, LCD_COMMAND),
        err, TAG, "Unable to complete Reset by Instruction. Part 1.");
     // 4.1 mS delay (min)
    vTaskDelay(10 / portTICK_PERIOD_MS);
    // second part of reset sequence
    ESP_GOTO_ON_ERROR(
        lcd_write_nibble(handle, LCD_FUNCTION_SET | LCD_8BIT_MODE, LCD_COMMAND),
        err, TAG, "Unable to complete Reset by Instruction. Part 2.");
     // 100 uS delay (min)
    ets_delay_us(200);
    // Third time's a charm
    ESP_GOTO_ON_ERROR(
        lcd_write_nibble(handle, LCD_FUNCTION_SET | LCD_8BIT_MODE, LCD_COMMAND),
        err, TAG, "Unable to complete Reset by Instruction. Part 3.");
    // Activate 4-bit mode
    ESP_GOTO_ON_ERROR(
        lcd_write_nibble(handle, LCD_FUNCTION_SET | LCD_4BIT_MODE, LCD_COMMAND),
        err, TAG, "Unable to activate 4-bit mode.");
    // 40 uS delay (min)
    ets_delay_us(80);

    // --- Busy flag now available ---
    // Set Display Function: # line, font size, etc.
    // 37us max execution time with 270kHz clock
     // Set mode, lines, and font
    ESP_GOTO_ON_ERROR(
        lcd_write_byte(handle, LCD_FUNCTION_SET | handle->display_function, LCD_COMMAND),
        err, TAG, "Unable to perform Set Display Function instruction.");
    ets_delay_us(LCD_STD_EXEC_TIME_US);

    // turn the display on with no cursor or blinking default
    ESP_GOTO_ON_ERROR(
        lcd_display(handle),
        err, TAG, "Error with lcd_display()");

    // Clear Display instruction
    ESP_GOTO_ON_ERROR(
        lcd_clear_screen(handle),
        err, TAG, "Error with lcd_clear_screen()");

    // Entry Mode Set instruction.
    // Sets cursor move direction and specifies display shift
    // 37us max execution time with 270kHz clock
    ESP_GOTO_ON_ERROR(
        lcd_write_byte(handle, LCD_ENTRY_MODE_SET | handle->display_mode, LCD_COMMAND),
        err, TAG, "Unable to perform Entry Mode Set instruction.");
    ets_delay_us(LCD_STD_EXEC_TIME_US);

    ESP_GOTO_ON_ERROR(
        lcd_home(handle),
        err, TAG, "Error with lcd_home()");
    handle->initialized = true;

    return ret;
err:
    if (ret == ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "I2C driver must be installed before attempting to initalize LCD.");
    }
    return ret;
}

esp_err_t lcd_write_char(lcd_handle_t *handle, char c)
{
    esp_err_t ret = ESP_OK;

    ESP_GOTO_ON_FALSE(handle, ESP_ERR_INVALID_ARG, err, TAG, "Invalid argument");
    ESP_GOTO_ON_FALSE(c, ESP_ERR_INVALID_ARG, err, TAG, "Invalid argument");

    // Write data to DDRAM
    ESP_GOTO_ON_ERROR(
        lcd_write_byte(handle, c, LCD_WRITE),
        err, TAG, "Error with lcd_write_byte()");
    // 37us + 4us execution time for 270kHz oscillator frequency
    ets_delay_us(LCD_STD_EXEC_TIME_US);

    // Update the cursor position details in the LCD handle
    if (handle->display_mode & LCD_ENTRY_INCREMENT)
    {
        lcd_handle_increment_cursor(handle);
    }
    else
    {
        lcd_handle_decrement_cursor(handle);
    }
    return ret;
err:
    return ret;
}

esp_err_t lcd_write_str(lcd_handle_t *handle, char *str)
{
    esp_err_t ret = ESP_OK;

    while (*str)
    {
        ESP_GOTO_ON_ERROR(
            lcd_write_char(handle, *str++),
        err, TAG, "Error with lcd_write_char()");
    }
    return ret;
err:
    return ret;
}

esp_err_t lcd_home(lcd_handle_t *handle)
{
    esp_err_t ret = ESP_OK;

    ESP_GOTO_ON_FALSE(handle, ESP_ERR_INVALID_ARG, err, TAG, "Invalid argument");

    ESP_GOTO_ON_ERROR(
        lcd_write_byte(handle, LCD_HOME, LCD_COMMAND),
        err, TAG, "Error with lcd_write_byte()");
    // 1.52ms execution time for 270kHz oscillator frequency
    vTaskDelay((LCD_HOME_EXEC_TIME_US / 1000) / portTICK_PERIOD_MS);
    handle->cursor_row = 0;
    handle->cursor_column = 0;

    return ESP_OK;
err:
    ESP_LOGE(TAG, "lcd_home:%s", esp_err_to_name(ret));
    return ret;
}

esp_err_t lcd_set_cursor(lcd_handle_t *handle, uint8_t column, uint8_t row)
{
    esp_err_t ret;
    bool valid_arg = false;
    uint8_t row_offsets[] = {LCD_LINEONE, LCD_LINETWO, LCD_LINETHREE, LCD_LINEFOUR};

    ESP_GOTO_ON_FALSE(handle, ESP_ERR_INVALID_ARG, err, TAG, "Invalid argument");

    valid_arg = ((column < handle->columns) ? true : false);
    ESP_GOTO_ON_FALSE(valid_arg, ESP_ERR_INVALID_ARG, err, TAG, "Invalid column argument");

    valid_arg = ((row < handle->rows) ? true : false);
    ESP_GOTO_ON_FALSE(valid_arg, ESP_ERR_INVALID_ARG, err, TAG, "Invalid row argument");

    // Why is this not using Cursor/Display Shift Instruction??
    ESP_GOTO_ON_ERROR(
        lcd_write_byte(handle, LCD_SET_DDRAM_ADDR | (column + row_offsets[row]), LCD_COMMAND),
        err, TAG, "Error with lcd_set_cursor()");
    // 37us execution time for 270kHz oscillator frequency
    ets_delay_us(LCD_STD_EXEC_TIME_US);
    handle->cursor_column = column;
    handle->cursor_row = row;
    return ESP_OK;
err:
    ESP_LOGE(TAG, "lcd_set_cursor:%s", esp_err_to_name(ret));
    return ret;
}

esp_err_t lcd_clear_screen(lcd_handle_t *handle)
{
    esp_err_t ret = ESP_OK;

    ESP_GOTO_ON_FALSE(handle, ESP_ERR_INVALID_ARG, err, TAG, "Invalid argument");

    // Max execution time not specified. Assume it is 0
    ESP_GOTO_ON_ERROR(
        lcd_write_byte(handle, LCD_CLEAR, LCD_COMMAND),
        err, TAG, "Error with lcd_write_byte()");
    handle->cursor_row = 0;
    handle->cursor_column = 0;
    // This instruction also sets I/D bit to 1 (increment mode)
    handle->display_mode |= LCD_ENTRY_INCREMENT;
    return ESP_OK;
err:
    ESP_LOGE(TAG, "lcd_clear_screen:%s", esp_err_to_name(ret));
    return ret;
}

esp_err_t lcd_no_display(lcd_handle_t *handle)
{
    esp_err_t ret = ESP_OK;

    ret = lcd_write_byte(handle,
            LCD_DISPLAY_CONTROL | (handle->display_control & ~LCD_DISPLAY_ON),
            LCD_COMMAND);
    // 37us execution time for 270kHz oscillator frequency
    ets_delay_us(LCD_STD_EXEC_TIME_US);
    if (ret != ESP_OK) goto err;
    handle->display_control &= ~LCD_DISPLAY_ON;

    return ESP_OK;
err:
    ESP_LOGE(TAG, "lcd_no_display:%s", esp_err_to_name(ret));
    return ret;
}

esp_err_t lcd_display(lcd_handle_t *handle)
{
    esp_err_t ret = ESP_OK;

    ret = lcd_write_byte(handle,
            LCD_DISPLAY_CONTROL | (handle->display_control | LCD_DISPLAY_ON),
            LCD_COMMAND);
    // 37us max execution time with 270kHz clock
    ets_delay_us(LCD_STD_EXEC_TIME_US);
    if (ret != ESP_OK) goto err;
    handle->display_control |= LCD_DISPLAY_ON;

    return ESP_OK;
err:
    ESP_LOGE(TAG, "lcd_display:%s", esp_err_to_name(ret));
    return ret;
}

esp_err_t lcd_no_cursor(lcd_handle_t *handle)
{
    esp_err_t ret = ESP_OK;

    ret = lcd_write_byte(handle,
        LCD_DISPLAY_CONTROL | (handle->display_control & ~LCD_CURSOR_ON),
        LCD_COMMAND);
    // 37us execution time for 270kHz oscillator frequency
    ets_delay_us(LCD_STD_EXEC_TIME_US);
    if (ret != ESP_OK) goto err;
    handle->display_control &= ~LCD_CURSOR_ON;

    return ESP_OK;
err:
    ESP_LOGE(TAG, "lcd_no_cursor:%s", esp_err_to_name(ret));
    return ret;
}
esp_err_t lcd_cursor(lcd_handle_t *handle)
{
    esp_err_t ret = ESP_OK;

    ret = lcd_write_byte(handle,
        LCD_DISPLAY_CONTROL | (handle->display_control | LCD_CURSOR_ON),
        LCD_COMMAND);
    // 37us execution time for 270kHz oscillator frequency
    ets_delay_us(LCD_STD_EXEC_TIME_US);
    if (ret != ESP_OK) goto err;
    handle->display_control |= LCD_CURSOR_ON;

    return ESP_OK;
err:
    ESP_LOGE(TAG, "lcd_cursor:%s", esp_err_to_name(ret));
    return ret;
}

esp_err_t lcd_no_blink(lcd_handle_t *handle)
{
    esp_err_t ret = ESP_OK;

    ret = lcd_write_byte(handle,
        LCD_DISPLAY_CONTROL | (handle->display_control & ~LCD_BLINK_ON),
        LCD_COMMAND);
    // 37us execution time for 270kHz oscillator frequency
    ets_delay_us(LCD_STD_EXEC_TIME_US);
    if (ret != ESP_OK) goto err;
    handle->display_control &= ~LCD_BLINK_ON;

    return ESP_OK;
err:
    ESP_LOGE(TAG, "lcd_no_blink:%s", esp_err_to_name(ret));
    return ret;
}

esp_err_t lcd_blink(lcd_handle_t *handle)
{
    esp_err_t ret = ESP_OK;

    ret = lcd_write_byte(handle,
        LCD_DISPLAY_CONTROL | (handle->display_control | LCD_BLINK_ON),
        LCD_COMMAND);
    // 37us execution time for 270kHz oscillator frequency
    ets_delay_us(LCD_STD_EXEC_TIME_US);
    if (ret != ESP_OK) goto err;
    handle->display_control |= LCD_BLINK_ON;

    return ESP_OK;
err:
    ESP_LOGE(TAG, "lcd_blink:%s", esp_err_to_name(ret));
    return ret;
}

esp_err_t lcd_display_shift_left(lcd_handle_t *handle)
{
    esp_err_t ret = ESP_OK;

    ret = lcd_write_byte(handle,
            LCD_CURSOR_OR_DISPLAY_SHIFT | LCD_DISPLAY_MOVE | LCD_MOVE_LEFT,
            LCD_COMMAND);
    // 37us execution time for 270kHz oscillator frequency
    ets_delay_us(LCD_STD_EXEC_TIME_US);
    if (ret != ESP_OK) goto err;
    return lcd_handle_decrement_cursor(handle);
err:
    ESP_LOGE(TAG, "lcd_display_shift_left:%s", esp_err_to_name(ret));
    return ret;
}

esp_err_t lcd_display_shift_right(lcd_handle_t *handle)
{
    esp_err_t ret = ESP_OK;

    ret = lcd_write_byte(handle,
            LCD_CURSOR_OR_DISPLAY_SHIFT | LCD_DISPLAY_MOVE | LCD_MOVE_RIGHT,
            LCD_COMMAND);
    // 37us execution time for 270kHz oscillator frequency
    ets_delay_us(LCD_STD_EXEC_TIME_US);
    if (ret != ESP_OK) goto err;
    return lcd_handle_increment_cursor(handle);
err:
    ESP_LOGE(TAG, "lcd_display_shift_right:%s", esp_err_to_name(ret));
    return ret;
}

// This is for text that flows Left to Right
esp_err_t lcd_left_to_right(lcd_handle_t *handle)
{
    esp_err_t ret = ESP_OK;

    ret = lcd_write_byte(handle,
        LCD_ENTRY_MODE_SET | (handle->display_mode | LCD_ENTRY_INCREMENT),
        LCD_COMMAND);
    // 37us execution time for 270kHz oscillator frequency
    ets_delay_us(LCD_STD_EXEC_TIME_US);
    if (ret != ESP_OK) goto err;
    handle->display_mode |= LCD_ENTRY_INCREMENT;

    return ESP_OK;
err:
    ESP_LOGE(TAG, "lcd_left_to_right:%s", esp_err_to_name(ret));
    return ret;
}

// This is for text that flows Right to Left
esp_err_t lcd_right_to_left(lcd_handle_t *handle)
{
    esp_err_t ret = ESP_OK;

    ret = lcd_write_byte(handle,
        LCD_ENTRY_MODE_SET | (handle->display_mode & ~LCD_ENTRY_INCREMENT),
        LCD_COMMAND);
    // 37us execution time for 270kHz oscillator frequency
    ets_delay_us(LCD_STD_EXEC_TIME_US);
    if (ret != ESP_OK) goto err;
    handle->display_mode &= ~LCD_ENTRY_INCREMENT;

    return ESP_OK;
err:
    ESP_LOGE(TAG, "lcd_right_to_left:%s", esp_err_to_name(ret));
    return ret;
}

esp_err_t lcd_autoscroll(lcd_handle_t *handle)
{
    esp_err_t ret = ESP_OK;
    ESP_LOGE(TAG, "lcd_autoscroll: Function not yet supported\n");
    ret = ESP_ERR_NOT_SUPPORTED;
    goto err;

    ret = lcd_write_byte(handle,
        LCD_ENTRY_MODE_SET | (handle->display_mode | LCD_ENTRY_DISPLAY_SHIFT),
        LCD_COMMAND);
    // 37us execution time for 270kHz oscillator frequency
    ets_delay_us(LCD_STD_EXEC_TIME_US);
    if (ret != ESP_OK) goto err;
    handle->display_mode |= LCD_ENTRY_DISPLAY_SHIFT;
    return ESP_OK;
err:
    ESP_LOGE(TAG, "lcd_autoscroll:%s", esp_err_to_name(ret));
    return ret;
}

esp_err_t lcd_no_autoscroll(lcd_handle_t *handle)
{
    esp_err_t ret = ESP_OK;

    ret = lcd_write_byte(handle,
        LCD_ENTRY_MODE_SET | (handle->display_mode & ~LCD_ENTRY_DISPLAY_SHIFT),
        LCD_COMMAND);
    // 37us execution time for 270kHz oscillator frequency
    ets_delay_us(LCD_STD_EXEC_TIME_US);
    if (ret != ESP_OK) goto err;
    handle->display_mode &= ~LCD_ENTRY_DISPLAY_SHIFT;
    return ESP_OK;
err:
    ESP_LOGE(TAG, "lcd_no_autoscroll:%s", esp_err_to_name(ret));
    return ret;
}
/*
// Allows us to fill the first 8 CGRAM locations
// with custom characters
void lcd_createChar(uint8_t location, uint8_t charmap[])
{
    location &= 0x7; // we only have 8 locations
    lcd_write_byte(LCD_SET_CGRAM_ADDR | (location << 3), LCD_COMMAND);
    for (int i = 0; i < 8; i++)
    {
        lcd_write_byte(charmap[i], Rs);
    }
}
*/

esp_err_t lcd_backlight(lcd_handle_t *handle)
{
    handle->backlight = LCD_BACKLIGHT_ON;
    // Closest thing we have to a noop. We need an instruction to effect the backlight change
    return(lcd_set_cursor(handle, handle->cursor_column, handle->cursor_row));
}

esp_err_t lcd_no_backlight(lcd_handle_t *handle)
{
    handle->backlight = LCD_BACKLIGHT_OFF;
    // Closest thing we have to a noop. We need an instruction to effect the backlight change
    return(lcd_set_cursor(handle, handle->cursor_column, handle->cursor_row));
}

static esp_err_t lcd_handle_increment_cursor(lcd_handle_t *handle)
{
    esp_err_t ret = ESP_OK;

    ESP_GOTO_ON_FALSE(handle, ESP_ERR_INVALID_ARG, err, TAG, "Invalid argument");
    handle->cursor_column++;
    if (handle->cursor_column == handle->columns)
    {
        /**
         * Cursor will have overflowed into a new row. Unfortunately, the row
         * overflow logic doesn't map nicely to rows. It is simple from a DDRAM
         * perspective, but that doesn't translate nicely to rows.
         * It would be nice to find a more robust algorithm than that which
         * is used below
        */
        handle->cursor_column = 0;
        if (handle->rows == 4)
        {
            if (handle->cursor_row == 0)
                handle->cursor_row = 2;
            else if (handle->cursor_row == 1)
                handle->cursor_row = 3;
            else if (handle->cursor_row == 2)
                handle->cursor_row = 1;
            else if (handle->cursor_row == 3)
                handle->cursor_row = 0;
            else
            {
                ESP_LOGE(TAG,"Invalid cursor row (%d). Range is [0 - %d]",
                    handle->cursor_row, handle->rows);
                return ESP_ERR_INVALID_STATE;
            }
        }
        else if (handle->rows == 2)
            handle->cursor_row = (handle->cursor_row + 1) % handle->rows;
        else
        {
            ESP_LOGE(TAG, "Untested lcd_increment_cursor scenaro. Rows =%d\n",
                handle->rows);
            return ESP_ERR_INVALID_SIZE;
        }
    }
    return ret;
err:
    ESP_LOGE(TAG, "lcd_handle_increment_cursor:%s", esp_err_to_name(ret));
    return ret;
}

static esp_err_t lcd_handle_decrement_cursor(lcd_handle_t *handle)
{
    esp_err_t ret = ESP_OK;

    ESP_GOTO_ON_FALSE(handle, ESP_ERR_INVALID_ARG, err, TAG, "Invalid argument");
    if (handle->cursor_column == 0)
    {
        // Cursor will have underflowed
        /**
         * Cursor will have underflowed into a new row. Unfortunately, the row
         * underflow logic doesn't map nicely to rows. It is simple from a DDRAM
         * perspective, but that doesn't translate nicely to rows.
         * It would be nice to find a more robust algorithm than that which
         * is used below
        */
        handle->cursor_column = (handle->columns - 1);
        if (handle->rows == 4)
        {
            if (handle->cursor_row == 0)
                handle->cursor_row = 3;
            else if (handle->cursor_row == 1)
                handle->cursor_row = 2;
            else if (handle->cursor_row == 2)
                handle->cursor_row = 0;
            else if (handle->cursor_row == 3)
                handle->cursor_row = 1;
            else
            {
                ESP_LOGE(TAG,"Invalid cursor row (%d). Range is [0 - %d]",
                    handle->cursor_row, handle->rows);
                return ESP_ERR_INVALID_STATE;
            }
        }
        else if (handle->rows == 2)
            handle->cursor_row = (handle->cursor_row + 1) % handle->rows;
    }
    else
    {
        handle->cursor_column--;
    }
    return ret;
err:
    ESP_LOGE(TAG, "lcd_handle_increment_cursor:%s", esp_err_to_name(ret));
    return ret;
}


/************ low level data pushing commands **********/

static esp_err_t lcd_write_nibble(lcd_handle_t *handle, uint8_t nibble, uint8_t mode)
{
    esp_err_t ret = ESP_OK;
    uint8_t data = 0;

    if (handle->backlight)
    {
        data = (nibble & 0xF0) | mode | LCD_BACKLIGHT_CONTROL_ON;
    }
    else
    {
        data = (nibble & 0xF0) | mode | LCD_BACKLIGHT_CONTROL_OFF;
    }

    ESP_GOTO_ON_ERROR(
        lcd_i2c_write(I2C_MASTER_NUM, handle->address, data),
        err, TAG, "Error with lcd_i2c_write()");

    ets_delay_us(LCD_PRE_PULSE_DELAY_US); // Need a decent delay here, else display won't work

    // Clock the data into the LCD
    ESP_GOTO_ON_ERROR(
        lcd_pulse_enable(handle, data),
        err, TAG, "Error with lcd_pulse_enable()");

    return ESP_OK;
err:
    ESP_LOGE(TAG, "lcd_write_nibble:%s", esp_err_to_name(ret));
    return ret;
}

static esp_err_t lcd_write_byte(lcd_handle_t *handle, uint8_t data, uint8_t mode)
{
    esp_err_t ret;

    ESP_GOTO_ON_ERROR(
        lcd_write_nibble(handle, data & 0xF0, mode),
        err, TAG, "Error with lcd_write_nibble()");

    ESP_GOTO_ON_ERROR(
        lcd_write_nibble(handle, (data << 4) & 0xF0, mode),
        err, TAG, "Error with lcd_write_nibble()");

    return ESP_OK;
err:
    ESP_LOGE(TAG, "lcd_write_byte:%s", esp_err_to_name(ret));
    return ret;
}

static esp_err_t lcd_pulse_enable(lcd_handle_t *handle, uint8_t data)
{
    esp_err_t ret = ESP_OK;

    ESP_GOTO_ON_ERROR(
        lcd_i2c_write(I2C_MASTER_NUM, handle->address, data | LCD_ENABLE),
        err, TAG, "Error with lcd_i2c_write()");
    ets_delay_us(1); // enable pulse must be >450ns
    ESP_GOTO_ON_ERROR(
        lcd_i2c_write(I2C_MASTER_NUM, handle->address, data & ~LCD_ENABLE),
        err, TAG, "Error with lcd_i2c_write()");
    // 37us + 4us execution time for 270kHz oscillator frequency
    ets_delay_us(LCD_STD_EXEC_TIME_US);
    return ESP_OK;
err:
    ESP_LOGE(TAG, "lcd_pulse_enable:%s", esp_err_to_name(ret));
    return ret;
}

esp_err_t lcd_probe(lcd_handle_t *handle)
{
    esp_err_t ret = ESP_OK;

    ESP_GOTO_ON_FALSE(handle, ESP_ERR_INVALID_ARG, err, TAG, "Invalid argument");
    return lcd_i2c_detect(handle->i2c_port, handle->address);
err:
    ESP_LOGE(TAG, "lcd_probe:%s", esp_err_to_name(ret));
    return ret;
}

/**
 * @brief check if LCD exists on the I2C bus
 *
 * @param[in] port I2C controller port
 * @param[in] address LCD address
 * @return  - ESP_OK                Success
 *          - ESP_ERR_NOT_FOUND     LCD not found
 *          - ESP_ERR_INVALID_ARG   Parameter error
 *          - ESP_ERR_INVALID_STATE I2C driver not installed or not in master mode
 *          - ESP_ERR_TIMEOUT       Operation timeout because the bus is busy
 */
static esp_err_t lcd_i2c_detect(i2c_port_t port, uint8_t address)
{
    esp_err_t ret = ESP_OK;

    ret = lcd_i2c_write(port, address, 0);
    switch (ret)
    {
    case ESP_OK:
        ESP_LOGD(TAG, "LCD found at address 0x%x", address);
        break;

    case ESP_FAIL: // Slave hasn't ACK the transfer
        ESP_LOGE(TAG, "LCD not found at address 0x%x", address);
        return ESP_ERR_NOT_FOUND;

    default:
        ESP_LOGE(TAG, "ic2_detect:%s", esp_err_to_name(ret));
        break;
    }
    return ret;
}

static esp_err_t lcd_i2c_write(i2c_port_t port, uint8_t address, uint8_t data)
{
    esp_err_t ret = ESP_OK;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    ESP_GOTO_ON_ERROR(
        i2c_master_start(cmd),
        err, TAG, "Error with i2c_master_start()");

    ESP_GOTO_ON_ERROR(
        i2c_master_write_byte(cmd, (address << 1) | WRITE_BIT, ACK_CHECK_EN),
        err, TAG, "Error with ic2_master_write_byte()");

    if (data != 0)
    {
        ESP_GOTO_ON_ERROR(
            i2c_master_write_byte(cmd, data, ACK_CHECK_EN),
            err, TAG, "Error with ic2_master_write_byte()");
    }

    ESP_GOTO_ON_ERROR(
        i2c_master_stop(cmd),
        err, TAG, "Error with i2c_master_stop()");

    ESP_GOTO_ON_ERROR(
        i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS),
        err, TAG, "Error with i2c_master_cmd_begin()");

    i2c_cmd_link_delete(cmd);

    return ESP_OK;
err:
    ESP_LOGE(TAG, "lcd_i2c_write:%s", esp_err_to_name(ret));
    return ret;
}
