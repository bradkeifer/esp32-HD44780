#include <stdio.h>
#include "driver/i2c.h"
#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "rom/ets_sys.h"
#include "HD44780.h"

// I2C related defines
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 400000
#define I2C_MASTER_TX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define WRITE_BIT I2C_MASTER_WRITE  /*!< I2C master write */
#define READ_BIT I2C_MASTER_READ    /*!< I2C master read */
#define ACK_CHECK_EN 0x1            /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0           /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0                 /*!< I2C ack value */
#define NACK_VAL 0x1                /*!< I2C nack value */

// LCD module defines
#define LCD_LINEONE 0x00   // start of line 1
#define LCD_LINETWO 0x40   // start of line 2
#define LCD_LINETHREE 0x14 // start of line 3
#define LCD_LINEFOUR 0x54  // start of line 4

#define LCD_ENABLE 0x04
#define LCD_COMMAND 0x00
#define LCD_WRITE 0x01
#define Rs 0x01 // Register select bit

// LCD instructions - refer Table 6 of Hitachi HD44780U datasheet
#define LCD_CLEAR 0x01 // replace all characters with ASCII 'space'
#define LCD_HOME 0x02  // return cursor to first position on first line
#define LCD_ENTRY_MODE_SET 0x04
#define LCD_DISPLAY_CONTROL 0x08
#define LCD_CURSOR_SHIFT 0x10
#define LCD_FUNCTION_SET 0x20
#define LCD_SET_CGRAM_ADDR 0x40
#define LCD_SET_DDRAM_ADDR 0x80

// LCD Delay times
#define LCD_PRE_PULSE_DELAY_US 1000

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
// can't assume that its in that state when task starts (and the
// LiquidCrystal constructor is called).
//
// Also note, that on power up the LCD internal reset circuitry remains in a
// busy state for 10 ms after Vcc ries to 4.5V.

static const char *TAG = "LCD Driver";

// TODO: To align with esp32 best practice, put these into a struct and
// develop esp-idf standard initialisation pattern(s)
static uint8_t lcd_addr;
static uint8_t lcd_rows;
static uint8_t displayControl;
static uint8_t backlightVal;

/**
 * @brief Transmit 4 bits of data to the LCD panel
 *
 * @param[in] nibble The 4 bits of data to be sent.
 * @param[in] mode [** to be defined **]
*/
static esp_err_t lcd_write_nibble(uint8_t nibble, uint8_t mode);
static esp_err_t lcd_write_byte(uint8_t data, uint8_t mode);
static esp_err_t lcd_pulse_enable(uint8_t nibble);
static esp_err_t lcd_i2c_detect(i2c_port_t port, uint8_t address);
static esp_err_t lcd_i2c_write(i2c_port_t port, uint8_t address, uint8_t data);

esp_err_t lcd_init(lcd_handle_t *handle)
{
    // to be deprecated!!
    lcd_addr = handle->address;

    // Initialise the LCD handle
    if (handle->rows == 1)
        handle->display_function = LCD_4BIT_MODE | LCD_1LINE | LCD_5x8DOTS;
    else
        handle->display_function = LCD_4BIT_MODE | LCD_2LINE | LCD_5x8DOTS;
    handle->display_control = LCD_DISPLAY_ON | LCD_CURSOR_OFF | LCD_BLINK_OFF;
    handle->display_mode = LCD_ENTRY_INCREMENT | LCD_ENTRY_DISPLAY_NO_SHIFT;
    handle->cursor_column = 0;
    handle->cursor_row = 0;
    handle->backlight = LCD_BACKLIGHT;

    // Initialise the LCD controller by instruction for 4-bit interface
    lcd_write_nibble(LCD_FUNCTION_SET | LCD_8BIT_MODE, LCD_COMMAND);   // First part of reset sequence
    vTaskDelay(10 / portTICK_PERIOD_MS);                              // 4.1 mS delay (min)
    lcd_write_nibble(LCD_FUNCTION_SET | LCD_8BIT_MODE, LCD_COMMAND);   // second part of reset sequence
    ets_delay_us(200);                                                // 100 uS delay (min)
    lcd_write_nibble(LCD_FUNCTION_SET | LCD_8BIT_MODE, LCD_COMMAND);   // Third time's a charm
    lcd_write_nibble(LCD_FUNCTION_SET | LCD_4BIT_MODE, LCD_COMMAND); // Activate 4-bit mode
    ets_delay_us(80);                                                 // 40 uS delay (min)

    // --- Busy flag now available ---
    // Set Display Function: # line, font size, etc.
    // 37us max execution time with 270kHz clock
    lcd_write_byte(LCD_FUNCTION_SET | handle->display_function, LCD_COMMAND); // Set mode, lines, and font
    ets_delay_us(80);

    // turn the display on with no cursor or blinking default
    // 37us max execution time with 270kHz clock
    lcd_display();

    // Clear Display instruction
    // Max execution time not specified
    lcd_clear_screen(handle);

    // Entry Mode Set instruction.
    // Sets cursor move direction and specifies display shift
    // 37us max execution time with 270kHz clock
    lcd_write_byte(LCD_ENTRY_MODE_SET | handle->display_mode, LCD_COMMAND);
    ets_delay_us(80);

    lcd_home(handle);

    return ESP_OK;

}

esp_err_t lcd_write_char(lcd_handle_t *handle, char c)
{
    esp_err_t ret = ESP_OK;
    int8_t new_column = (int8_t) handle->cursor_column;

    ESP_GOTO_ON_FALSE(handle, ESP_ERR_INVALID_ARG, err, TAG, "Invalid argument");
    ESP_GOTO_ON_FALSE(c, ESP_ERR_INVALID_ARG, err, TAG, "Invalid argument");

    // Check for column overflow. Need to understand display behaviour and then
    // determine appropriate handling procedure
    if (handle->display_mode && LCD_ENTRY_INCREMENT)
    {
        if(++new_column > handle->columns)
        {
            ESP_LOGW(TAG,
                "lcd_write_char(): Column overflow. Current=%d, new=%d",
                handle->cursor_column, new_column
            );
        }
    }
    else if (handle->display_mode && LCD_ENTRY_DECREMENT)
    {
        if(--new_column < 0)
        {
            ESP_LOGW(TAG,
                "lcd_write_char(): Column underflow. Current=%d, new=%d",
                handle->cursor_column, new_column
            );
            new_column = 0; // prevent actual underflow
        }
    }

    // Write data to DDRAM
    ESP_GOTO_ON_ERROR(
        lcd_write_byte(c, LCD_WRITE),
        err, TAG, "Error with lcd_write_byte()"
    );

    // Update the cursor position details in the LCD handle
    ESP_LOGD(TAG,"lcd_write_char:Current column=%d, new column=%d",
        handle->cursor_column, new_column
    );
    handle->cursor_column = (uint8_t) new_column;
    return ret;
err:
    return ret;
}

void lcd_writeStr(lcd_handle_t *handle, char *str)
{
    while (*str)
    {
        lcd_write_char(handle, *str++);
    }
}

esp_err_t lcd_home(lcd_handle_t *handle)
{
    esp_err_t ret = ESP_OK;

    ESP_GOTO_ON_FALSE(handle, ESP_ERR_INVALID_ARG, err, TAG, "Invalid argument");

    ESP_GOTO_ON_ERROR(
        lcd_write_byte(LCD_HOME, LCD_COMMAND),
        err, TAG, "Error with lcd_write_byte()"
    );

    handle->cursor_row = 0;
    handle->cursor_column = 0;
    vTaskDelay(2 / portTICK_PERIOD_MS); // This command takes a while to complete

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

    ESP_GOTO_ON_ERROR(
        lcd_write_byte(LCD_SET_DDRAM_ADDR | (column + row_offsets[row]), LCD_COMMAND),
        err, TAG, "Error with lcd_set_cursor()"
    );
    ets_delay_us(40); // 37us execution time for Set DDRAM address
    handle->cursor_column = column;
    handle->cursor_row = row;
    return ESP_OK;
err:
    ESP_LOGE(TAG, "lcd_set_cursor:%s", esp_err_to_name(ret));
    return ret;
}

void lcd_setCursor(uint8_t col, uint8_t row)
{
    if (row > lcd_rows - 1)
    {
        ESP_LOGW(TAG, "Cannot write to row %d. Please select a row in the range (0, %d)", row, lcd_rows - 1);
        row = lcd_rows - 1;
    }
    uint8_t row_offsets[] = {LCD_LINEONE, LCD_LINETWO, LCD_LINETHREE, LCD_LINEFOUR};
    lcd_write_byte(LCD_SET_DDRAM_ADDR | (col + row_offsets[row]), LCD_COMMAND);
}

esp_err_t lcd_clear_screen(lcd_handle_t *handle)
{
    esp_err_t ret = ESP_OK;

    ESP_GOTO_ON_FALSE(handle, ESP_ERR_INVALID_ARG, err, TAG, "Invalid argument");

    ESP_GOTO_ON_ERROR(
        lcd_write_byte(LCD_CLEAR, LCD_COMMAND),
        err, TAG, "Error with lcd_write_byte()"
    );
    handle->cursor_row = 0;
    handle->cursor_column = 0;

    return ESP_OK;
err:
    ESP_LOGE(TAG, "lcd_clear_screen:%s", esp_err_to_name(ret));
    return ret;
}

// Turn the display on/off (quickly)
void lcd_noDisplay(void)
{
    displayControl &= ~LCD_DISPLAY_ON;
    lcd_write_byte(LCD_DISPLAY_CONTROL | displayControl, LCD_COMMAND);
}

void lcd_display(void)
{
    displayControl |= LCD_DISPLAY_ON;
    lcd_write_byte(LCD_DISPLAY_CONTROL | displayControl, LCD_COMMAND);
}
/*
// Turns the underline cursor on/off
void lcd_noCursor(void)
{
    displayControl &= ~LCD_CURSOR_ON;
    lcd_write_byte(LCD_DISPLAY_CONTROL | displayControl, LCD_COMMAND);
}
void lcd_cursor(void)
{
    displayControl |= LCD_CURSOR_ON;
    lcd_write_byte(LCD_DISPLAY_CONTROL | displayControl, LCD_COMMAND);
}

// Turn on and off the blinking cursor
void lcd_noBlink(void)
{
    displayControl &= ~LCD_BLINK_ON;
    lcd_write_byte(LCD_DISPLAY_CONTROL | displayControl, LCD_COMMAND);
}
void lcd_blink(void)
{
    displayControl |= LCD_BLINK_ON;
    lcd_write_byte(LCD_DISPLAY_CONTROL | displayControl, LCD_COMMAND);
}

// These commands scroll the display without changing the RAM
void lcd_scrollDisplayLeft(void)
{
    lcd_write_byte(LCD_CURSOR_SHIFT | LCD_DISPLAY_MOVE | LCD_MOVE_LEFT, LCD_COMMAND);
}

void lcd_scrollDisplayRight(void)
{
    lcd_write_byte(LCD_CURSOR_SHIFT | LCD_DISPLAY_MOVE | LCD_MOVE_RIGHT, LCD_COMMAND);
}

// This is for text that flows Left to Right
void lcd_leftToRight(void)
{
    displayMode |= LCD_ENTRY_LEFT;
    lcd_write_byte(LCD_ENTRY_MODE_SET | displayMode, LCD_COMMAND);
}

// This is for text that flows Right to Left
void lcd_rightToLeft(void)
{
    displayMode &= ~LCD_ENTRY_LEFT;
    lcd_write_byte(LCD_ENTRY_MODE_SET | displayMode, LCD_COMMAND);
}

// This will 'right justify' text from the cursor
void lcd_Autoscroll(void)
{
    displayMode |= LCD_ENTRY_SHIFT_INCREMENT;
    lcd_write_byte(LCD_ENTRY_MODE_SET | displayMode, LCD_COMMAND);
}

// This will 'left justify' text from the cursor
void lcd_noAutoscroll(void)
{
    displayMode &= ~LCD_ENTRY_SHIFT_INCREMENT;
    lcd_write_byte(LCD_ENTRY_MODE_SET | displayMode, LCD_COMMAND);
}

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

void lcd_backlight(void)
{
    backlightVal = LCD_BACKLIGHT;
    lcd_write_byte(0x00, LCD_COMMAND);
}

void lcd_noBacklight(void)
{
    backlightVal = LCD_NO_BACKLIGHT;
    lcd_write_byte(0x00, LCD_COMMAND);
}

void lcd_setBackLight(uint8_t new_val)
{
    if (new_val)
        lcd_backlight(); // turn backlight on
    else
        lcd_noBacklight(); // turn backlight off
}

/************ low level data pushing commands **********/

static esp_err_t lcd_write_nibble(uint8_t nibble, uint8_t mode)
{
    esp_err_t ret;
    uint8_t data = (nibble & 0xF0) | mode | backlightVal;

    ESP_GOTO_ON_ERROR(
        lcd_i2c_write(I2C_MASTER_NUM, lcd_addr, data),
        err, TAG, "Error with lcd_i2c_write()"
    );

    ets_delay_us(LCD_PRE_PULSE_DELAY_US); // Need a decent delay here, else display won't work

    ESP_LOGD(TAG,
             "lcd_write_nibble: Wrote nibble 0x%x, with mode 0x%x as data 0x%x to i2c address 0x%x",
             nibble, mode, data, lcd_addr);

    // Clock the data into the LCD
    ESP_GOTO_ON_ERROR(
        lcd_pulse_enable(data),
        err, TAG, "Error with lcd_pulse_enable()"
    );

    return ESP_OK;
err:
    ESP_LOGE(TAG, "lcd_write_nibble:%s", esp_err_to_name(ret));
    return ret;
}

static esp_err_t lcd_write_byte(uint8_t data, uint8_t mode)
{
    esp_err_t ret;

    ESP_GOTO_ON_ERROR(
        lcd_write_nibble(data & 0xF0, mode),
        err, TAG, "Error with lcd_write_nibble()"
    );

    ESP_GOTO_ON_ERROR(
        lcd_write_nibble((data << 4) & 0xF0, mode),
        err, TAG, "Error with lcd_write_nibble()"
    );

    return ESP_OK;
err:
    ESP_LOGE(TAG, "lcd_write_byte:%s", esp_err_to_name(ret));
    return ret;
}

static esp_err_t lcd_pulse_enable(uint8_t data)
{
    esp_err_t ret = ESP_OK;

    ESP_GOTO_ON_ERROR(
        lcd_i2c_write(I2C_MASTER_NUM, lcd_addr, data | LCD_ENABLE),
        err, TAG, "Error with lcd_i2c_write()"
    );
    ets_delay_us(1); // enable pulse must be >450ns
    ESP_GOTO_ON_ERROR(
        lcd_i2c_write(I2C_MASTER_NUM, lcd_addr, data & ~LCD_ENABLE),
        err, TAG, "Error with lcd_i2c_write()"
    );
    ets_delay_us(50); // commands need > 37us to settle
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
        err, TAG, "Error with i2c_master_start()"
    );

    ESP_GOTO_ON_ERROR(
        i2c_master_write_byte(cmd, (address << 1) | WRITE_BIT, ACK_CHECK_EN),
        err, TAG, "Error with ic2_master_write_byte()"
    );

    if (data != 0)
    {
        ESP_GOTO_ON_ERROR(
            i2c_master_write_byte(cmd, data, ACK_CHECK_EN),
            err, TAG, "Error with ic2_master_write_byte()"
        );
    }

    ESP_GOTO_ON_ERROR(
        i2c_master_stop(cmd),
        err, TAG, "Error with i2c_master_stop()"
    );

    ESP_GOTO_ON_ERROR(
        i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS),
        err, TAG, "Error with i2c_master_cmd_begin()"
    );

    i2c_cmd_link_delete(cmd);

    return ESP_OK;
err:
    ESP_LOGE(TAG, "lcd_i2c_write:%s", esp_err_to_name(ret));
    return ret;
}
