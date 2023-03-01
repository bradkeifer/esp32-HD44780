#include <stdio.h>
#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#include "esp_log.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lcd.h"

static const char *TAG = "lcd_cgram";

static void initialise(void);
static void lcd_demo(void);

lcd_handle_t lcd_handle = LCD_HANDLE_DEFAULT_CONFIG();

void app_main(void)
{
    initialise();

    while (true)
    {
        ESP_LOGI(TAG, "Running LCD Demo");
        lcd_demo();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

// Perform initilisation functions
static void initialise(void)
{
    i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    // Initialise i2c
    ESP_LOGD(TAG, "Installing i2c driver in master mode on channel %d", I2C_MASTER_NUM);
    ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_NUM, I2C_MODE_MASTER, 0, 0, 0));
    ESP_LOGD(TAG,
             "Configuring i2c parameters.\n\tMode: %d\n\tSDA pin:%d\n\tSCL pin:%d\n\tSDA pullup:%d\n\tSCL pullup:%d\n\tClock speed:%.3fkHz",
             i2c_config.mode, i2c_config.sda_io_num, i2c_config.scl_io_num,
             i2c_config.sda_pullup_en, i2c_config.scl_pullup_en,
             i2c_config.master.clk_speed / 1000.0);
    ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_NUM, &i2c_config));

    // Modify default lcd_handle details
    lcd_handle.i2c_port = I2C_MASTER_NUM;
    lcd_handle.address = LCD_ADDR;
    lcd_handle.columns = LCD_COLUMNS;
    lcd_handle.rows = LCD_ROWS;
    lcd_handle.backlight = LCD_BACKLIGHT;

    // Initialise LCD
    ESP_ERROR_CHECK(lcd_init(&lcd_handle));

    uint8_t pieces[8][8] = {
        {0x07, 0x0F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F},
        {0x1F, 0x1F, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00},
        {0x1C, 0x1E, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F},
        {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x0F, 0x07},
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F},
        {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1E, 0x1C},
        {0x1F, 0x1F, 0x1F, 0x00, 0x00, 0x00, 0x1F, 0x1F},
        {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F},
    };

    for (uint8_t i = 0; i < 8; ++i)
        lcd_write_cgram(&lcd_handle, i, &pieces[i][0]);

    return;
}

/**
 * @brief Demonstrate the LCD
 */
static void lcd_demo(void)
{
    char c = 0; // first char in CGRAM

    ESP_ERROR_CHECK(lcd_probe(&lcd_handle));
    lcd_cursor(&lcd_handle);
    lcd_backlight(&lcd_handle);

    ESP_LOGI(TAG, "Clear screen");
    lcd_clear_screen(&lcd_handle);
    lcd_set_cursor(&lcd_handle, 0, 0);

    vTaskDelay(pdMS_TO_TICKS(1000));

    ESP_LOGI(TAG, "Write custom characters (CGRAM addr 0-15)");

    for (uint8_t i = 0; i < 16; ++i)
    {
        lcd_write_char(&lcd_handle, c++);
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    vTaskDelay(pdMS_TO_TICKS(1000));

    lcd_no_backlight(&lcd_handle);
    ESP_LOGI(TAG, "LCD Demo finished");
}
