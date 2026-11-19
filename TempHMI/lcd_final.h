#ifndef LCD_FINAL_H
#define LCD_FINAL_H

#include <avr/io.h>
#include <stdint.h>

void LCD_Init(void);
void LCD_Clear(void);
void LCD_SetCursor(uint8_t row, uint8_t column);
void LCD_Print(const char *text);
void LCD_PrintNumber(uint16_t number);

#endif
