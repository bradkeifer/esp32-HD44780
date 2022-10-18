#pragma once

#include <stdbool.h>
#include <stdio.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

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
#define LCD_CLEAR 0x01 /*!< Bitmask for "Clear display" instruction */
#define LCD_HOME 0x02  /*!< Bitmask for "Return home" instruction NOTE: Slow execution time */
#define LCD_ENTRY_MODE_SET 0x04 /*!< Bitmask for "Entry mode set" instruction */
#define LCD_DISPLAY_CONTROL 0x08 /*!< Bitmask for "Display on/ff control" instruction */
#define LCD_CURSOR_OR_DISPLAY_SHIFT 0x10 /*!< Bitmask for "Cursor or display shift" instruction */
#define LCD_FUNCTION_SET 0x20 /*!< Bitmask for "Function set" instrution */
#define LCD_SET_CGRAM_ADDR 0x40 /*<! Bitmask for "Set CGRAM address" instruction */
#define LCD_SET_DDRAM_ADDR 0x80 /*!< Bitmask for "Set DDRAM address" instruction */

// LCD Delay times
#define LCD_PRE_PULSE_DELAY_US 1000 /*!< Not sure what this corresponds to in datasheet, but it is necessary */
#define LCD_STD_EXEC_TIME_US 40 /*!< The standard execution time for most instructions */
#define LCD_HOME_EXEC_TIME_US 15200 /*!< Execution time for Return home instruction */

#ifdef __cplusplus
}
#endif
