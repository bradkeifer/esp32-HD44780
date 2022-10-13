#pragma once
#include <stdio.h>

esp_err_t LCD_init(uint8_t addr, uint8_t dataPin, uint8_t clockPin, uint8_t cols, uint8_t rows);
void LCD_writeChar(char c);
void LCD_writeStr(char* str);
void LCD_home(void);
void LCD_setCursor(uint8_t col, uint8_t row);
void LCD_clearScreen(void);
void LCD_noDisplay(void);
void LCD_display(void);
void LCD_noCursor(void);
void LCD_cursor(void);
void LCD_noBlink(void);
void LCD_blink(void);
void LCD_scrollDisplayLeft(void);
void LCD_scrollDisplayRight(void);
void LCD_leftToRight(void); 
void LCD_rightToLeft(void);
void LCD_autoscroll(void);
void LCD_noAutoscroll(void);
void LCD_createChar(uint8_t location, uint8_t charmap[]);
void LCD_backlight(void);
void LCD_noBacklight(void);
void LCD_setBackLight(uint8_t new_val);