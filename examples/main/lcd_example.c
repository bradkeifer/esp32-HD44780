#include <stdio.h>
#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include "esp_log.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "sdkconfig.h"
#include "HD44780.h"

// Use project configuration menu (idf.py menuconfig) to choose the GPIO assignments,
// or you can edit the following lines and set numbers here
#define I2C_MASTER_SDA_IO 13
#define I2C_MASTER_SCL_IO 16
#define I2C_MASTER_NUM 0            /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ 400000   /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS 1000

#define LCD_ADDR 0x3f

static const char *TAG = "lcd_example";

static void initialise(void);
static void LCD_Demo(void);

void app_main(void)
{
    initialise();

    while (true)
    {
        ESP_LOGI(TAG, "Running LCD Demo");
        LCD_Demo();
        vTaskDelay(3 * 1000 / portTICK_PERIOD_MS);
    }
}

// Perform initilisation functions
static void initialise(void)
{
    ESP_LOGI(TAG, "Setting log level for all components to %d.", LOG_LOCAL_LEVEL);
    esp_log_level_set("*", LOG_LOCAL_LEVEL);

    ESP_LOGD(TAG,
             "Initialising LCD at i2c address 0x%0x, sda pin %d, scl pin %d, %d columns, %d rows.",
             LCD_ADDR, I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO, 20, 4);
    LCD_init(LCD_ADDR, I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO, 20, 4);

    return;
}

/**
 * @brief Demonstrate the LCD
 */
static void LCD_Demo(void)
{
    char num[20];

    ESP_LOGD(TAG, "Turn backlight on");
    LCD_setBackLight(1);
    ESP_LOGD(TAG, "Run LCD.home()");
    LCD_home();
    ESP_LOGD(TAG, "Clear screen");
    LCD_clearScreen();
    ESP_LOGD(TAG, "Write string:20x4 I2C LCD");
    LCD_writeStr("20x4 I2C LCD");
    ESP_LOGD(TAG, "Wait 3000 ms");
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    ESP_LOGD(TAG, "Clear screen");
    LCD_clearScreen();
    ESP_LOGD(TAG, "Write string:Lets Count 0-10!");
    LCD_writeStr("Lets Count 0-10!");
    ESP_LOGD(TAG, "Wait 3000 ms");
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    ESP_LOGD(TAG, "Clear screen");
    LCD_clearScreen();
    for (int i = 0; i <= 10; i++)
    {
        ESP_LOGD(TAG, "Set cursor on column %d, row %d", 8, 2);
        LCD_setCursor(8, 2);
        sprintf(num, "%d", i);
        ESP_LOGD(TAG, "Write string:%s", num);
        LCD_writeStr(num);
        ESP_LOGD(TAG, "Wait 1000 ms");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    ESP_LOGD(TAG, "Turn backlight off");
    LCD_setBackLight(0);
    
    ESP_LOGD(TAG, "LCD_Demo finished");
}