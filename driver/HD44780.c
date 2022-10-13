#include <driver/i2c.h>
#define LOG_LOCAL_LEVEL ESP_LOG_ERROR
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include "sdkconfig.h"
#include "rom/ets_sys.h"
#include "HD44780.h"

// I2C related defines
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 400000
#define I2C_MASTER_TX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS 1000
#define WRITE_BIT I2C_MASTER_WRITE /*!< I2C master write */
#define READ_BIT I2C_MASTER_READ   /*!< I2C master read */
#define ACK_CHECK_EN 0x1           /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0          /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0                /*!< I2C ack value */
#define NACK_VAL 0x1               /*!< I2C nack value */

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

// flags for display entry mode
#define LCD_ENTRY_RIGHT 0x00
#define LCD_ENTRY_LEFT 0x02
#define LCD_ENTRY_SHIFT_INCREMENT 0x01
#define LCD_ENTRY_SHIFT_DECREMENT 0x00

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
static uint8_t LCD_addr;
static uint8_t SDA_pin;
static uint8_t SCL_pin;
static uint8_t LCD_cols;
static uint8_t LCD_rows;
static uint8_t displayFunction;
static uint8_t displayControl;
static uint8_t displayMode;
static uint8_t backlightVal;

static void LCD_writeNibble(uint8_t nibble, uint8_t mode);
static void LCD_writeByte(uint8_t data, uint8_t mode);
static void LCD_pulseEnable(uint8_t nibble);
static esp_err_t I2C_init(void);
static esp_err_t I2C_detect(uint8_t);

/*
 * @brief Initialise the LCD Display
 * Refer to figure 24 of the Histachi HD44780U datasheet for details of the
 * initialisation sequence and timings.
 */
esp_err_t LCD_init(uint8_t addr, uint8_t dataPin, uint8_t clockPin, uint8_t cols, uint8_t rows)
{
    LCD_addr = addr;
    SDA_pin = dataPin;
    SCL_pin = clockPin;
    LCD_cols = cols;
    LCD_rows = rows;
    displayFunction = LCD_4BIT_MODE | LCD_1LINE | LCD_5x8DOTS;
    displayControl = LCD_DISPLAY_ON | LCD_CURSOR_OFF | LCD_BLINK_OFF;
    // Initialise to default text direction (for roman languages)
    displayMode = LCD_ENTRY_LEFT | LCD_ENTRY_SHIFT_DECREMENT;
    backlightVal = LCD_BACKLIGHT;

    if (rows > 1)
    {
        displayFunction |= LCD_2LINE;
    }

    ESP_ERROR_CHECK_WITHOUT_ABORT(I2C_init());
    vTaskDelay(100 / portTICK_PERIOD_MS); // Initial 40 mSec delay

    // Check for existence of LCD at the address
    if (LOG_LOCAL_LEVEL >= ESP_LOG_DEBUG)
    {
        ESP_LOGD(TAG, "Checking LCD exists at address 0x%x", LCD_addr);
        ESP_ERROR_CHECK_WITHOUT_ABORT(I2C_detect(LCD_addr));
    }

    // Initialise the LCD controller by instruction for 4-bit interface
    LCD_writeNibble(LCD_FUNCTION_SET | LCD_8BIT_MODE, LCD_COMMAND); // First part of reset sequence
    vTaskDelay(10 / portTICK_PERIOD_MS);                 // 4.1 mS delay (min)
    LCD_writeNibble(LCD_FUNCTION_SET | LCD_8BIT_MODE, LCD_COMMAND); // second part of reset sequence
    ets_delay_us(200);                                   // 100 uS delay (min)
    LCD_writeNibble(LCD_FUNCTION_SET | LCD_8BIT_MODE, LCD_COMMAND); // Third time's a charm
    LCD_writeNibble(LCD_FUNCTION_SET | displayFunction, LCD_COMMAND); // Activate 4-bit mode
    ets_delay_us(80);                                    // 40 uS delay (min)

    // --- Busy flag now available ---
    // Set Display Function: # line, font size, etc.
    LCD_writeByte(LCD_FUNCTION_SET | displayFunction, LCD_COMMAND); // Set mode, lines, and font
    ets_delay_us(80);

    // turn the display on with no cursor or blinking default
    LCD_display();

    // Clear Display instruction
    LCD_clearScreen();

    // Entry Mode Set instruction
    LCD_writeByte(LCD_ENTRY_MODE_SET | displayMode, LCD_COMMAND); // Set desired shift characteristics
    ets_delay_us(80);

    LCD_home();
//    LCD_writeByte(LCD_DISPLAY_ON, LCD_COMMAND); // Ensure LCD is set to on

    return ESP_OK;
}

void LCD_writeChar(char c)
{
    LCD_writeByte(c, LCD_WRITE); // Write data to DDRAM
}

void LCD_writeStr(char *str)
{
    while (*str)
    {
        LCD_writeChar(*str++);
    }
}

void LCD_home(void)
{
    LCD_writeByte(LCD_HOME, LCD_COMMAND);
    vTaskDelay(2 / portTICK_PERIOD_MS); // This command takes a while to complete
}

void LCD_setCursor(uint8_t col, uint8_t row)
{
    if (row > LCD_rows - 1)
    {
        ESP_LOGE(TAG, "Cannot write to row %d. Please select a row in the range (0, %d)", row, LCD_rows - 1);
        row = LCD_rows - 1;
    }
    uint8_t row_offsets[] = {LCD_LINEONE, LCD_LINETWO, LCD_LINETHREE, LCD_LINEFOUR};
    LCD_writeByte(LCD_SET_DDRAM_ADDR | (col + row_offsets[row]), LCD_COMMAND);
}

void LCD_clearScreen(void)
{
    LCD_writeByte(LCD_CLEAR, LCD_COMMAND);
    vTaskDelay(2 / portTICK_PERIOD_MS); // This command takes a while to complete
}

// Turn the display on/off (quickly)
void LCD_noDisplay(void)
{
    displayControl &= ~LCD_DISPLAY_ON;
    LCD_writeByte(LCD_DISPLAY_CONTROL | displayControl, LCD_COMMAND);
}

void LCD_display(void)
{
    displayControl |= LCD_DISPLAY_ON;
    LCD_writeByte(LCD_DISPLAY_CONTROL | displayControl, LCD_COMMAND);
}

// Turns the underline cursor on/off
void LCD_noCursor(void)
{
    displayControl &= ~LCD_CURSOR_ON;
    LCD_writeByte(LCD_DISPLAY_CONTROL | displayControl, LCD_COMMAND);
}
void LCD_cursor(void)
{
    displayControl |= LCD_CURSOR_ON;
    LCD_writeByte(LCD_DISPLAY_CONTROL | displayControl, LCD_COMMAND);
}

// Turn on and off the blinking cursor
void LCD_noBlink(void)
{
    displayControl &= ~LCD_BLINK_ON;
    LCD_writeByte(LCD_DISPLAY_CONTROL | displayControl, LCD_COMMAND);
}
void LCD_blink(void)
{
    displayControl |= LCD_BLINK_ON;
    LCD_writeByte(LCD_DISPLAY_CONTROL | displayControl, LCD_COMMAND);
}

// These commands scroll the display without changing the RAM
void LCD_scrollDisplayLeft(void)
{
    LCD_writeByte(LCD_CURSOR_SHIFT | LCD_DISPLAY_MOVE | LCD_MOVE_LEFT, LCD_COMMAND);
}

void LCD_scrollDisplayRight(void)
{
    LCD_writeByte(LCD_CURSOR_SHIFT | LCD_DISPLAY_MOVE | LCD_MOVE_RIGHT, LCD_COMMAND);
}

// This is for text that flows Left to Right
void LCD_leftToRight(void)
{
    displayMode |= LCD_ENTRY_LEFT;
    LCD_writeByte(LCD_ENTRY_MODE_SET | displayMode, LCD_COMMAND);
}

// This is for text that flows Right to Left
void LCD_rightToLeft(void)
{
    displayMode &= ~LCD_ENTRY_LEFT;
    LCD_writeByte(LCD_ENTRY_MODE_SET | displayMode, LCD_COMMAND);
}

// This will 'right justify' text from the cursor
void LCD_Autoscroll(void)
{
    displayMode |= LCD_ENTRY_SHIFT_INCREMENT;
    LCD_writeByte(LCD_ENTRY_MODE_SET | displayMode, LCD_COMMAND);
}

// This will 'left justify' text from the cursor
void LCD_noAutoscroll(void)
{
    displayMode &= ~LCD_ENTRY_SHIFT_INCREMENT;
    LCD_writeByte(LCD_ENTRY_MODE_SET | displayMode, LCD_COMMAND);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void LCD_createChar(uint8_t location, uint8_t charmap[])
{
    location &= 0x7; // we only have 8 locations
    LCD_writeByte(LCD_SET_CGRAM_ADDR | (location << 3), LCD_COMMAND);
    for (int i = 0; i < 8; i++)
    {
        LCD_writeByte(charmap[i], Rs);
    }
}

void LCD_backlight(void)
{
    backlightVal = LCD_BACKLIGHT;
    LCD_writeByte(0x00, LCD_COMMAND);
}

void LCD_noBacklight(void)
{
    backlightVal = LCD_NO_BACKLIGHT;
    LCD_writeByte(0x00, LCD_COMMAND);
}

void LCD_setBackLight(uint8_t new_val)
{
    if (new_val)
        LCD_backlight(); // turn backlight on
    else
        LCD_noBacklight(); // turn backlight off
}

/************ low level data pushing commands **********/

static void LCD_writeNibble(uint8_t nibble, uint8_t mode)
{
    uint8_t data = (nibble & 0xF0) | mode | backlightVal;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (LCD_addr << 1) | WRITE_BIT, ACK_CHECK_EN));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, data, 1));
    ESP_ERROR_CHECK(i2c_master_stop(cmd));
    ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS));
    i2c_cmd_link_delete(cmd);
    ets_delay_us(LCD_PRE_PULSE_DELAY_US); // Need a decent delay here, else display won't work
    ESP_LOGD(TAG,
             "LCD_writeNibble: Wrote nibble 0x%x, with mode 0x%x as data 0x%x to i2c address 0x%x",
             nibble, mode, data, LCD_addr);

    // If there is insufficient time between completion of the above i2c write and the
    // execution of the pluseEnable function, then the display does not work.
    LCD_pulseEnable(data); // Clock data into LCD
}

static void LCD_writeByte(uint8_t data, uint8_t mode)
{
    LCD_writeNibble(data & 0xF0, mode);
    LCD_writeNibble((data << 4) & 0xF0, mode);
}

static void LCD_pulseEnable(uint8_t data)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (LCD_addr << 1) | WRITE_BIT, ACK_CHECK_EN));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, data | LCD_ENABLE, ACK_CHECK_EN));
    ESP_ERROR_CHECK(i2c_master_stop(cmd));
    ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS));
    i2c_cmd_link_delete(cmd);
    ets_delay_us(1); // enable pulse must be >450ns

    cmd = i2c_cmd_link_create();
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (LCD_addr << 1) | WRITE_BIT, ACK_CHECK_EN));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (data & ~LCD_ENABLE), ACK_CHECK_EN));
    ESP_ERROR_CHECK(i2c_master_stop(cmd));
    ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS));
    i2c_cmd_link_delete(cmd);
    ets_delay_us(50); // commands need > 37us to settle
}

/*************** I2C specific fuctions *****************/

static esp_err_t I2C_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = SDA_pin,
        .scl_io_num = SCL_pin,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    ESP_LOGD(TAG,
             "I2C Config:\n\tMode: %d\n\tSDA pin:%d\n\tSCL pin:%d\n\tSDA pullup:%d\n\tSCL pullup:%d\n\tClock speed:%.3fkHz",
             conf.mode, conf.sda_io_num, conf.scl_io_num, conf.sda_pullup_en, conf.scl_pullup_en, conf.master.clk_speed / 1000.0);
    ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_NUM, I2C_MODE_MASTER, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0));
    return i2c_param_config(I2C_MASTER_NUM, &conf);
    ESP_LOGD(TAG, "I2C initialised successfully");
}

static esp_err_t I2C_detect(uint8_t address)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (address << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 50 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    if (ret == ESP_OK)
    {
        ESP_LOGD(TAG, "LCD found at address 0x%x", address);
    }
    else
    {
        ESP_LOGD(TAG, "LCD not found at address 0x%x", address);
    }
    return ret;
}