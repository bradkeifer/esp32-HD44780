/* lcd-tools example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_console.h"
#include "esp_vfs_fat.h"
#include "cmd_system.h"
#include "cmd_lcd_tools.h"

static const char *TAG = "lcd-tools";

#if CONFIG_EXAMPLE_STORE_HISTORY

#define MOUNT_PATH "/data"
#define HISTORY_PATH MOUNT_PATH "/history.txt"

static void initialize_filesystem(void)
{
    static wl_handle_t wl_handle;
    const esp_vfs_fat_mount_config_t mount_config = {
        .max_files = 4,
        .format_if_mount_failed = true
    };
    esp_err_t err = esp_vfs_fat_spiflash_mount(MOUNT_PATH, "storage", &mount_config, &wl_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount FATFS (%s)", esp_err_to_name(err));
        return;
    }
}
#endif // CONFIG_EXAMPLE_STORE_HISTORY

void app_main(void)
{
    esp_console_repl_t *repl = NULL;
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();

#if CONFIG_EXAMPLE_STORE_HISTORY
    initialize_filesystem();
    repl_config.history_save_path = HISTORY_PATH;
#endif
    repl_config.prompt = "lcd-tools>";

    // install console REPL environment
#if CONFIG_ESP_CONSOLE_UART
    esp_console_dev_uart_config_t uart_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_uart(&uart_config, &repl_config, &repl));
#elif CONFIG_ESP_CONSOLE_USB_CDC
    esp_console_dev_usb_cdc_config_t cdc_config = ESP_CONSOLE_DEV_CDC_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_usb_cdc(&cdc_config, &repl_config, &repl));
#elif CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG
    esp_console_dev_usb_serial_jtag_config_t usbjtag_config = ESP_CONSOLE_DEV_USB_SERIAL_JTAG_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_usb_serial_jtag(&usbjtag_config, &repl_config, &repl));
#endif

    register_lcd_tools();
    register_system();

    printf("\n ==============================================================\n");
    printf(" |             Steps to Use lcd-tools                         |\n");
    printf(" |                                                            |\n");
    printf(" |  1. Try 'help', check all supported commands               |\n");
    printf(" |  2. Try 'lcd_detect' to scan devices on the I2C bus        |\n");
    printf(" |  3. Try 'lcd_config' to configure your I2C bus             |\n");
    printf(" |  4. Try 'lcd_handle' to output the LCD handle data         |\n");
    printf(" |  5. Try 'lcd_init' to initialize LCD                       |\n");
    printf(" |  6. Try 'lcd_home' to return the cursor to home and        |\n");
    printf(" |     display to to its original status if it was shifted.   |\n");
    printf(" |  7. Try 'lcd_write_str' to write a string to the LCD       |\n");
    printf(" |  8. Try 'lcd_set_cursor' to position the cursor at a       |\n");
    printf(" |     specified row and column position.                     |\n");
    printf(" |  9. Try 'lcd_clear_screen' to clear the display.           |\n");
    printf(" |  10. Try 'lcd_no_display' to turn the display off.         |\n");
    printf(" |  11. Try 'lcd_display' to turn the display on.             |\n");
    printf(" |  12. Try 'lcd_no_cursor' to turn the cursor off.           |\n");
    printf(" |  13. Try 'lcd_cursor' to turn the cursor on.               |\n");
    printf(" |  14. Try 'lcd_no_blink' to turn blinking of the cursor off.|\n");
    printf(" |  15. Try 'lcd_blink' to turn blinking of the cursor on.    |\n");
    printf(" |  16. Try 'lcd_no_autoscroll' to turn display scroll off.   |\n");
    printf(" |  17. Try 'lcd_autoscroll' to turn display scroll on.       |\n");
    printf(" |  18. Try 'lcd_no_backlight' to turn backlight off.         |\n");
    printf(" |  19. Try 'lcd_backlight' to turn backlight on.             |\n");
    printf(" |  20. Try 'lcd_shift_l' to shift the display left.          |\n");
    printf(" |  21. Try 'lcd_shift_r' to shift the display right.         |\n");
    printf(" |  22. Try 'lcd_l_to_r' set the text direction left to right.|\n");
    printf(" |  23. Try 'lcd_r_to_l' set the text direction right to left.|\n");
    printf(" |                                                            |\n");
    printf(" ==============================================================\n\n");

    // start console REPL
    ESP_ERROR_CHECK(esp_console_start_repl(repl));
}
