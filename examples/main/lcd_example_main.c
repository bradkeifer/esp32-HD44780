#include <stdio.h>
#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "sdkconfig.h"
#include "HD44780.h"

#define I2C_MASTER_SDA_IO CONFIG_SDA_GPIO
#define I2C_MASTER_SCL_IO CONFIG_SCL_GPIO

#ifdef CONFIG_HARDWARE_I2C_PORT0
    #define I2C_MASTER_NUM I2C_NUM_0 /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#endif

#ifdef CONFIG_HARDWARE_I2C_PORT1
    #define I2C_MASTER_NUM I2C_NUM_1 /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#endif

#define I2C_MASTER_FREQ_HZ CONFIG_I2C_CLK_FREQ   /*!< I2C master clock frequency */

#define LCD_ADDR CONFIG_LCD_ADDR /*!< Address of the display on the i2c bus */
#define LCD_ROWS CONFIG_LCD_ROWS
#define LCD_COLUMNS CONFIG_LCD_COLUMNS

static const char *TAG = "lcd_example";

static void initialise(void);
static void lcd_demo(void);

lcd_handle_t lcd_handle = {
    .i2c_port = I2C_MASTER_NUM,
    .address = LCD_ADDR,
    .columns = LCD_COLUMNS,
    .rows = LCD_ROWS,
    .backlight = LCD_BACKLIGHT,
};

void app_main(void)
{
    esp_log_level_set(TAG, ESP_LOG_DEBUG);
    esp_log_level_set("LCD Driver", ESP_LOG_DEBUG);
    initialise();

    while (true)
    {
        ESP_LOGI(TAG, "Running LCD Demo");
        lcd_demo();
        vTaskDelay(3 * 1000 / portTICK_PERIOD_MS);
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

    // Initialise LCD
    ESP_ERROR_CHECK(lcd_init(&lcd_handle));

    return;
}

/**
 * @brief Demonstrate the LCD
 */
static void lcd_demo(void)
{
    char num[20];
    char c = '!'; // first ascii char

    ESP_ERROR_CHECK(lcd_probe(&lcd_handle));
    ESP_LOGI(TAG, "Turn backlight on");
    lcd_setBackLight(1);
    ESP_LOGI(TAG, "Clear screen");
    lcd_clear_screen(&lcd_handle);
    ESP_LOGI(TAG, "Write string:20x4 I2C LCD");
    lcd_writeStr(&lcd_handle, "20x4 I2C LCD");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    ESP_LOGI(TAG, "Clear screen");
    lcd_clear_screen(&lcd_handle);
    ESP_LOGI(TAG, "Write string:Lets write some characters!");
    lcd_writeStr(&lcd_handle, "Lets write some characters!");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    ESP_LOGI(TAG, "Clear screen");
    lcd_clear_screen(&lcd_handle);
    for (int row = 0; row < LCD_ROWS; row++)
    {
        for (int column = 0; column < LCD_COLUMNS; column++)
        {
            ESP_LOGI(TAG, "Set cursor on column %d, row %d", column, row);
            lcd_set_cursor(&lcd_handle, column, row);
            sprintf(num, "%c", c);
            ESP_LOGI(TAG, "Write character:%c", c);
            lcd_write_char(&lcd_handle, c);
            c++;
            vTaskDelay(5 / portTICK_PERIOD_MS);
        }
        ESP_LOGI(TAG, "Finished row %d", row);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
//    ESP_LOGI(TAG, "Turn backlight off");
//    lcd_setBackLight(0); - This is causing display to use different character set

    ESP_LOGI(TAG, "lcd_Demo finished");
}
