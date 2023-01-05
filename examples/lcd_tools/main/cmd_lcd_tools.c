/* cmd_lcdtools.c

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "argtable3/argtable3.h"
#include "esp_console.h"
#include "esp_log.h"
#include "lcd.h"

// I2C Tools defines
#define I2C_MASTER_TX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define WRITE_BIT I2C_MASTER_WRITE  /*!< I2C master write */
#define READ_BIT I2C_MASTER_READ    /*!< I2C master read */
#define ACK_CHECK_EN 0x1            /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0           /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0                 /*!< I2C ack value */
#define NACK_VAL 0x1                /*!< I2C nack value */
// end I2C Tools defines

static const char *TAG = "cmd_lcd_tools";

static lcd_handle_t lcd_handle = LCD_HANDLE_DEFAULT_CONFIG();

static esp_err_t lcd_set_port(int port, lcd_handle_t *handle)
{
    if (port >= I2C_NUM_MAX)
    {
        ESP_LOGE(TAG, "Wrong port number: %d", port);
        return ESP_FAIL;
    }
    handle->i2c_port = port;
    return ESP_OK;
}

static esp_err_t i2c_master_driver_initialize(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
        // .clk_flags = 0,          /*!< Optional, you can use I2C_SCLK_SRC_FLAG_* flags to choose i2c source clock here. */
    };
    return i2c_param_config(lcd_handle.i2c_port, &conf);
}

static struct
{
    struct arg_int *i2c_port;
    struct arg_int *address;
    struct arg_int *columns;
    struct arg_int *rows;
    struct arg_end *end;
} lcd_config_args;

static int do_lcd_config_cmd(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&lcd_config_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, lcd_config_args.end, argv[0]);
        return 0;
    }

    /* Check "--port" option */
    if (lcd_config_args.i2c_port->count)
    {
        if (lcd_set_port(lcd_config_args.i2c_port->ival[0], &lcd_handle) != ESP_OK)
        {
            return 1;
        }
    }
    /* Check "--address" option */
    lcd_handle.address = lcd_config_args.address->ival[0];
    /* Check "columns" option */
    lcd_handle.columns = lcd_config_args.columns->ival[0];
    /* Check "rows" option */
    lcd_handle.rows = lcd_config_args.rows->ival[0];
    return 0;
}

static void register_lcd_config(void)
{
    lcd_config_args.i2c_port = arg_int1(NULL, "i2c_port", "<0|1>", "Set the I2C bus port number");
    lcd_config_args.address = arg_int1(NULL, "address", "<0xaddr>", "Set the address of the LCD on the I2C bus");
    lcd_config_args.columns = arg_int1(NULL, "columns", "<columns>", "Set the number of columns of the LCD");
    lcd_config_args.rows = arg_int1(NULL, "rows", "<rows>", "Set the number of rows of the LCD");
    lcd_config_args.end = arg_end(2);
    const esp_console_cmd_t lcd_config_cmd = {
        .command = "lcd_config",
        .help = "Config LCD Parameters",
        .hint = NULL,
        .func = &do_lcd_config_cmd,
        .argtable = &lcd_config_args};
    ESP_ERROR_CHECK(esp_console_cmd_register(&lcd_config_cmd));
}

static struct
{
    struct arg_int *column;
    struct arg_int *row;
    struct arg_end *end;
} lcd_set_cursor_args;

static int do_lcd_set_cursor_cmd(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&lcd_set_cursor_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, lcd_set_cursor_args.end, argv[0]);
        return 0;
    }

    /* Check "--row" and "--column" options */
    if (lcd_set_cursor_args.row->count && lcd_set_cursor_args.column->count)
    {
        if (lcd_set_cursor(&lcd_handle,
                            lcd_set_cursor_args.column->ival[0],
                            lcd_set_cursor_args.row->ival[0]) != ESP_OK)
        {
            return 1;
        }
    }
    return 0;
}

static void register_lcd_set_cursor(void)
{
    lcd_set_cursor_args.column = arg_int1("c", "column", "<column>", "Set the column number to move to");
    lcd_set_cursor_args.row = arg_int1("r", "row", "<row>", "Set the row number to move to");
    lcd_set_cursor_args.end = arg_end(2);
    const esp_console_cmd_t lcd_set_cursor_cmd = {
        .command = "lcd_set_cursor",
        .help = "Set the cursor position",
        .hint = NULL,
        .func = &do_lcd_set_cursor_cmd,
        .argtable = &lcd_set_cursor_args};
    ESP_ERROR_CHECK(esp_console_cmd_register(&lcd_set_cursor_cmd));
}

static int do_lcd_detect_cmd(int argc, char **argv)
{
    i2c_driver_install(lcd_handle.i2c_port, I2C_MODE_MASTER, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
    i2c_master_driver_initialize();
    uint8_t address;
    printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\r\n");
    for (int i = 0; i < 128; i += 16)
    {
        printf("%02x: ", i);
        for (int j = 0; j < 16; j++)
        {
            fflush(stdout);
            address = i + j;
            i2c_cmd_handle_t cmd = i2c_cmd_link_create();
            i2c_master_start(cmd);
            i2c_master_write_byte(cmd, (address << 1) | WRITE_BIT, ACK_CHECK_EN);
            i2c_master_stop(cmd);
            esp_err_t ret = i2c_master_cmd_begin(lcd_handle.i2c_port, cmd, 50 / portTICK_PERIOD_MS);
            i2c_cmd_link_delete(cmd);
            if (ret == ESP_OK)
            {
                printf("%02x ", address);
            }
            else if (ret == ESP_ERR_TIMEOUT)
            {
                printf("UU ");
            }
            else
            {
                printf("-- ");
            }
        }
        printf("\r\n");
    }

    i2c_driver_delete(lcd_handle.i2c_port);
    return 0;
}

static void register_lcd_detect(void)
{
    const esp_console_cmd_t lcd_detect_cmd = {
        .command = "lcd_detect",
        .help = "Scan I2C bus for devices (may or may not be LCD's)",
        .hint = NULL,
        .func = &do_lcd_detect_cmd,
        .argtable = NULL};
    ESP_ERROR_CHECK(esp_console_cmd_register(&lcd_detect_cmd));
}

static int do_i2c_init_cmd(int argc, char **argv)
{
    esp_err_t ret = ESP_OK;

    ret = i2c_driver_install(lcd_handle.i2c_port, I2C_MODE_MASTER, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
    if (ret != ESP_OK) {
        printf("Unable to install i2c driver.\n");
        fflush(stdout);
        return 1;
    }
    ret = i2c_master_driver_initialize();
    if (ret != ESP_OK) {
        printf("Unable to initialize i2c driver.\n");
        fflush(stdout);
        return 1;
    }
    printf("I2C Driver installed and initalized.\n");
    fflush(stdout);
    return 0;
}

static void register_i2c_init(void)
{
    const esp_console_cmd_t i2c_init_cmd = {
        .command = "i2c_init",
        .help = "Install an initialise the I2C driver",
        .hint = NULL,
        .func = &do_i2c_init_cmd,
        .argtable = NULL};
    ESP_ERROR_CHECK(esp_console_cmd_register(&i2c_init_cmd));
}

static int do_lcd_init_cmd(int argc, char **argv)
{
    esp_err_t ret = ESP_OK;

    ret = lcd_init(&lcd_handle);
    if (ret == ESP_OK)
        printf("LCD successfully initialised\n");
    else
        printf("Unable to initialise LCD.\n");
    fflush(stdout);
    return 0;
}

static void register_lcd_init(void)
{
    const esp_console_cmd_t lcd_init_cmd = {
        .command = "lcd_init",
        .help = "Initialise the LCD panel",
        .hint = NULL,
        .func = &do_lcd_init_cmd,
        .argtable = NULL};
    ESP_ERROR_CHECK(esp_console_cmd_register(&lcd_init_cmd));
}

static int do_lcd_home_cmd(int argc, char **argv)
{
    esp_err_t ret = ESP_OK;

    ret = lcd_home(&lcd_handle);
    if (ret == ESP_OK)
        printf("lcd_home success\n");
    else
        printf("Unable to home the LCD.\n");
    fflush(stdout);
    return 0;
}

static void register_lcd_home(void)
{
    const esp_console_cmd_t lcd_home_cmd = {
        .command = "lcd_home",
        .help = "Return home",
        .hint = NULL,
        .func = &do_lcd_home_cmd,
        .argtable = NULL};
    ESP_ERROR_CHECK(esp_console_cmd_register(&lcd_home_cmd));
}

static int do_lcd_clear_screen_cmd(int argc, char **argv)
{
    esp_err_t ret = ESP_OK;

    ret = lcd_clear_screen(&lcd_handle);
    if (ret == ESP_OK)
        printf("lcd_clear_screen success\n");
    else
        printf("Unable to home the LCD.\n");
    fflush(stdout);
    return 0;
}

static void register_lcd_clear_screen(void)
{
    const esp_console_cmd_t lcd_clear_screen_cmd = {
        .command = "lcd_clear_screen",
        .help = "Clear the display",
        .hint = NULL,
        .func = &do_lcd_clear_screen_cmd,
        .argtable = NULL};
    ESP_ERROR_CHECK(esp_console_cmd_register(&lcd_clear_screen_cmd));
}

static int do_lcd_no_display_cmd(int argc, char **argv)
{
    esp_err_t ret = ESP_OK;

    ret = lcd_no_display(&lcd_handle);
    if (ret == ESP_OK)
        printf("lcd_no_display success\n");
    else
        printf("Unable to turn off the LCD display.\n");
    fflush(stdout);
    return 0;
}

static void register_lcd_no_display(void)
{
    const esp_console_cmd_t lcd_no_display_cmd = {
        .command = "lcd_no_display",
        .help = "Turn the display off",
        .hint = NULL,
        .func = &do_lcd_no_display_cmd,
        .argtable = NULL};
    ESP_ERROR_CHECK(esp_console_cmd_register(&lcd_no_display_cmd));
}

static int do_lcd_display_cmd(int argc, char **argv)
{
    esp_err_t ret = ESP_OK;

    ret = lcd_display(&lcd_handle);
    if (ret == ESP_OK)
        printf("lcd_display success\n");
    else
        printf("Unable to turn on the LCD display.\n");
    fflush(stdout);
    return 0;
}

static void register_lcd_display(void)
{
    const esp_console_cmd_t lcd_display_cmd = {
        .command = "lcd_display",
        .help = "Turn the display on",
        .hint = NULL,
        .func = &do_lcd_display_cmd,
        .argtable = NULL};
    ESP_ERROR_CHECK(esp_console_cmd_register(&lcd_display_cmd));
}

static int do_lcd_no_cursor_cmd(int argc, char **argv)
{
    esp_err_t ret = ESP_OK;

    ret = lcd_no_cursor(&lcd_handle);
    if (ret == ESP_OK)
        printf("lcd_no_cursor success\n");
    else
        printf("Unable to turn off the LCD cursor.\n");
    fflush(stdout);
    return 0;
}

static void register_lcd_no_cursor(void)
{
    const esp_console_cmd_t lcd_no_cursor_cmd = {
        .command = "lcd_no_cursor",
        .help = "Turn the cursor off",
        .hint = NULL,
        .func = &do_lcd_no_cursor_cmd,
        .argtable = NULL};
    ESP_ERROR_CHECK(esp_console_cmd_register(&lcd_no_cursor_cmd));
}

static int do_lcd_cursor_cmd(int argc, char **argv)
{
    esp_err_t ret = ESP_OK;

    ret = lcd_cursor(&lcd_handle);
    if (ret == ESP_OK)
        printf("lcd_cursor success\n");
    else
        printf("Unable to turn on the cursor.\n");
    fflush(stdout);
    return 0;
}

static void register_lcd_cursor(void)
{
    const esp_console_cmd_t lcd_cursor_cmd = {
        .command = "lcd_cursor",
        .help = "Turn the cursor on",
        .hint = NULL,
        .func = &do_lcd_cursor_cmd,
        .argtable = NULL};
    ESP_ERROR_CHECK(esp_console_cmd_register(&lcd_cursor_cmd));
}

static int do_lcd_no_blink_cmd(int argc, char **argv)
{
    esp_err_t ret = ESP_OK;

    ret = lcd_no_blink(&lcd_handle);
    if (ret == ESP_OK)
        printf("lcd_no_blink success\n");
    else
        printf("Unable to turn off the LCD cursor blink.\n");
    fflush(stdout);
    return 0;
}

static void register_lcd_no_blink(void)
{
    const esp_console_cmd_t lcd_no_blink_cmd = {
        .command = "lcd_no_blink",
        .help = "Turn the cursor blink off",
        .hint = NULL,
        .func = &do_lcd_no_blink_cmd,
        .argtable = NULL};
    ESP_ERROR_CHECK(esp_console_cmd_register(&lcd_no_blink_cmd));
}

static int do_lcd_blink_cmd(int argc, char **argv)
{
    esp_err_t ret = ESP_OK;

    ret = lcd_blink(&lcd_handle);
    if (ret == ESP_OK)
        printf("lcd_blink success\n");
    else
        printf("Unable to turn on the cursor blink.\n");
    fflush(stdout);
    return 0;
}

static void register_lcd_blink(void)
{
    const esp_console_cmd_t lcd_blink_cmd = {
        .command = "lcd_blink",
        .help = "Turn the cursor blink on",
        .hint = NULL,
        .func = &do_lcd_blink_cmd,
        .argtable = NULL};
    ESP_ERROR_CHECK(esp_console_cmd_register(&lcd_blink_cmd));
}

static int do_lcd_no_autoscroll_cmd(int argc, char **argv)
{
    esp_err_t ret = ESP_OK;

    ret = lcd_no_autoscroll(&lcd_handle);
    if (ret == ESP_OK)
        printf("lcd_no_autoscroll success\n");
    else
        printf("Unable to disable autoscroll\n");
    fflush(stdout);
    return 0;
}

static void register_lcd_no_autoscroll(void)
{
    const esp_console_cmd_t lcd_no_autoscroll_cmd = {
        .command = "lcd_no_autoscroll",
        .help = "Disable autoscroll",
        .hint = NULL,
        .func = &do_lcd_no_autoscroll_cmd,
        .argtable = NULL};
    ESP_ERROR_CHECK(esp_console_cmd_register(&lcd_no_autoscroll_cmd));
}

static int do_lcd_autoscroll_cmd(int argc, char **argv)
{
    esp_err_t ret = ESP_OK;

    ret = lcd_autoscroll(&lcd_handle);
    if (ret == ESP_OK)
        printf("lcd_autoscroll success\n");
    else
        printf("Unable to enable autoscroll\n");
    fflush(stdout);
    return 0;
}

static void register_lcd_autoscroll(void)
{
    const esp_console_cmd_t lcd_autoscroll_cmd = {
        .command = "lcd_autoscroll",
        .help = "Enable autoscroll",
        .hint = NULL,
        .func = &do_lcd_autoscroll_cmd,
        .argtable = NULL};
    ESP_ERROR_CHECK(esp_console_cmd_register(&lcd_autoscroll_cmd));
}

static int do_lcd_no_backlight_cmd(int argc, char **argv)
{
    esp_err_t ret = ESP_OK;

    ret = lcd_no_backlight(&lcd_handle);
    if (ret == ESP_OK)
        printf("lcd_no_backlight success\n");
    else
        printf("Unable to disable backlight\n");
    fflush(stdout);
    return 0;
}

static void register_lcd_no_backlight(void)
{
    const esp_console_cmd_t lcd_no_backlight_cmd = {
        .command = "lcd_no_backlight",
        .help = "Disable backlight",
        .hint = NULL,
        .func = &do_lcd_no_backlight_cmd,
        .argtable = NULL};
    ESP_ERROR_CHECK(esp_console_cmd_register(&lcd_no_backlight_cmd));
}

static int do_lcd_backlight_cmd(int argc, char **argv)
{
    esp_err_t ret = ESP_OK;

    ret = lcd_backlight(&lcd_handle);
    if (ret == ESP_OK)
        printf("lcd_backlight success\n");
    else
        printf("Unable to enable backlight\n");
    fflush(stdout);
    return 0;
}

static void register_lcd_backlight(void)
{
    const esp_console_cmd_t lcd_backlight_cmd = {
        .command = "lcd_backlight",
        .help = "Enable backlight",
        .hint = NULL,
        .func = &do_lcd_backlight_cmd,
        .argtable = NULL};
    ESP_ERROR_CHECK(esp_console_cmd_register(&lcd_backlight_cmd));
}

static int do_lcd_l_to_r_cmd(int argc, char **argv)
{
    esp_err_t ret = ESP_OK;

    ret = lcd_left_to_right(&lcd_handle);
    if (ret == ESP_OK)
        printf("LCD entry mode set for left to right\n");
    else
        printf("Unable to set entry mode for left to right\n");
    fflush(stdout);
    return 0;
}

static void register_lcd_l_to_r(void)
{
    const esp_console_cmd_t lcd_l_to_r_cmd = {
        .command = "lcd_l_to_r",
        .help = "Set text direction to be left to right",
        .hint = NULL,
        .func = &do_lcd_l_to_r_cmd,
        .argtable = NULL};
    ESP_ERROR_CHECK(esp_console_cmd_register(&lcd_l_to_r_cmd));
}

static int do_lcd_r_to_l_cmd(int argc, char **argv)
{
    esp_err_t ret = ESP_OK;

    ret = lcd_right_to_left(&lcd_handle);
    if (ret == ESP_OK)
        printf("LCD entry mode set for right to left\n");
    else
        printf("Unable to set entry mode for right to left\n");
    fflush(stdout);
    return 0;
}

static void register_lcd_r_to_l(void)
{
    const esp_console_cmd_t lcd_r_to_l_cmd = {
        .command = "lcd_r_to_l",
        .help = "Set text direction to be right to left",
        .hint = NULL,
        .func = &do_lcd_r_to_l_cmd,
        .argtable = NULL};
    ESP_ERROR_CHECK(esp_console_cmd_register(&lcd_r_to_l_cmd));
}

static int do_lcd_shift_r_cmd(int argc, char **argv)
{
    esp_err_t ret = ESP_OK;

    ret = lcd_display_shift_right(&lcd_handle);
    if (ret == ESP_OK)
        printf("LCD display shifted right\n");
    else
        printf("Unable to shift display right\n");
    fflush(stdout);
    return 0;
}

static void register_lcd_shift_r(void)
{
    const esp_console_cmd_t lcd_shift_r_cmd = {
        .command = "lcd_shift_r",
        .help = "Shift display right",
        .hint = NULL,
        .func = &do_lcd_shift_r_cmd,
        .argtable = NULL};
    ESP_ERROR_CHECK(esp_console_cmd_register(&lcd_shift_r_cmd));
}

static int do_lcd_shift_l_cmd(int argc, char **argv)
{
    esp_err_t ret = ESP_OK;

    ret = lcd_display_shift_left(&lcd_handle);
    if (ret == ESP_OK)
        printf("LCD display shifted left\n");
    else
        printf("Unable to shift display left\n");
    fflush(stdout);
    return 0;
}

static void register_lcd_shift_l(void)
{
    const esp_console_cmd_t lcd_shift_l_cmd = {
        .command = "lcd_shift_l",
        .help = "Shift display left",
        .hint = NULL,
        .func = &do_lcd_shift_l_cmd,
        .argtable = NULL};
    ESP_ERROR_CHECK(esp_console_cmd_register(&lcd_shift_l_cmd));
}

static int do_lcd_handle_cmd(int argc, char **argv)
{
    printf("lcd_handle:\n\ti2c_port: %d\n\taddress: 0x%0x\n\tcolumns: %d\n\trows: %d\n",
           lcd_handle.i2c_port, lcd_handle.address, lcd_handle.columns, lcd_handle.rows);
    printf("\tdisplay function: 0x%0x\n\tdisplay_control: 0x%0x\n\t",
           lcd_handle.display_function, lcd_handle.display_control);
    printf("display mode: 0x%0x\n\tcursor column: %d\n\tcursor row: %d\n",
           lcd_handle.display_mode, lcd_handle.cursor_column, lcd_handle.cursor_row);
    printf("\tbacklight: %d\n\tinitialized state: %s\n",
           lcd_handle.backlight, lcd_handle.initialized ? "true" : "false");
    fflush(stdout);
    return 0;
}

static void register_lcd_handle(void)
{
    const esp_console_cmd_t lcd_handle_cmd = {
        .command = "lcd_handle",
        .help = "Output the LCD handle data",
        .hint = NULL,
        .func = &do_lcd_handle_cmd,
        .argtable = NULL};
    ESP_ERROR_CHECK(esp_console_cmd_register(&lcd_handle_cmd));
}

static struct
{
    struct arg_str *str;
    struct arg_end *end;
} lcd_write_str_args;

static int do_lcd_write_str_cmd(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&lcd_write_str_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, lcd_write_str_args.end, argv[0]);
        return 0;
    }

    /* Check "--string" option */
    if (lcd_write_str_args.str->count)
    {
        if (lcd_write_str(&lcd_handle, (char *)lcd_write_str_args.str->sval[0]) != ESP_OK)
        {
            printf("Error writing string: %s\n", lcd_write_str_args.str->sval[0]);
            fflush(stdout);
            return 1;
        }
        printf("Success writing string: %s\n", lcd_write_str_args.str->sval[0]);
        fflush(stdout);
    }
    return 0;
}

static void register_lcd_write_str(void)
{
    lcd_write_str_args.str = arg_str1(NULL, "string", "<string>", "Character string to write to LCD");
    lcd_write_str_args.end = arg_end(2);
    const esp_console_cmd_t lcd_write_str_cmd = {
        .command = "lcd_write_str",
        .help = "Write a string of character to the LCD",
        .hint = NULL,
        .func = &do_lcd_write_str_cmd,
        .argtable = &lcd_write_str_args};
    ESP_ERROR_CHECK(esp_console_cmd_register(&lcd_write_str_cmd));
}

void register_lcd_tools(void)
{
    register_lcd_config();
    register_i2c_init();
    register_lcd_init();
    register_lcd_detect();
    register_lcd_handle();
    register_lcd_home();
    register_lcd_write_str();
    register_lcd_set_cursor();
    register_lcd_clear_screen();
    register_lcd_no_display();
    register_lcd_display();
    register_lcd_no_cursor();
    register_lcd_cursor();
    register_lcd_no_blink();
    register_lcd_blink();
    register_lcd_no_autoscroll();
    register_lcd_autoscroll();
    register_lcd_no_backlight();
    register_lcd_backlight();
    register_lcd_shift_l();
    register_lcd_shift_r();
    register_lcd_l_to_r();
    register_lcd_r_to_l();
}
