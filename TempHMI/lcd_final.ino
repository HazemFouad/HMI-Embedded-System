#define F_CPU 16000000UL

#include "lcd_final.h"
#include <util/delay.h>

#define LCD_ADDRESS 0x27

#define LCD_RS 0x01
#define LCD_EN 0x04
#define LCD_BACKLIGHT 0x08

static void I2C_Init(void)
{
    // 100 kHz I2C clock at 16 MHz
    TWSR = 0x00;
    TWBR = 72;
    TWCR = (1 << TWEN);
}

static void I2C_Start(void)
{
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);

    while (!(TWCR & (1 << TWINT)));
}

static void I2C_Stop(void)
{
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}

static void I2C_Write(uint8_t data)
{
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);

    while (!(TWCR & (1 << TWINT)));
}

static void LCD_ExpanderWrite(uint8_t data)
{
    I2C_Start();

    I2C_Write(LCD_ADDRESS << 1);
    I2C_Write(data | LCD_BACKLIGHT);

    I2C_Stop();
}

static void LCD_EnablePulse(uint8_t data)
{
    LCD_ExpanderWrite(data | LCD_EN);
    _delay_us(1);

    LCD_ExpanderWrite(data & ~LCD_EN);
    _delay_us(50);
}

static void LCD_Write4Bits(uint8_t value)
{
    LCD_ExpanderWrite(value);
    LCD_EnablePulse(value);
}

static void LCD_Send(uint8_t value, uint8_t mode)
{
    uint8_t highNibble = value & 0xF0;
    uint8_t lowNibble = (value << 4) & 0xF0;

    LCD_Write4Bits(highNibble | mode);
    LCD_Write4Bits(lowNibble | mode);
}

static void LCD_Command(uint8_t command)
{
    LCD_Send(command, 0);
}

static void LCD_Data(uint8_t data)
{
    LCD_Send(data, LCD_RS);
}

void LCD_Init(void)
{
    I2C_Init();

    _delay_ms(50);

    // Initialize LCD in 4-bit mode
    LCD_Write4Bits(0x30);
    _delay_ms(5);

    LCD_Write4Bits(0x30);
    _delay_us(150);

    LCD_Write4Bits(0x30);
    LCD_Write4Bits(0x20);

    LCD_Command(0x28); // 4-bit, 2 lines, 5x8 font
    LCD_Command(0x0C); // Display ON, cursor OFF
    LCD_Command(0x06); // Entry mode
    LCD_Command(0x01); // Clear display

    _delay_ms(2);
}

void LCD_Clear(void)
{
    LCD_Command(0x01);
    _delay_ms(2);
}

void LCD_SetCursor(uint8_t row, uint8_t column)
{
    uint8_t address;

    if (row == 0)
        address = 0x00 + column;
    else
        address = 0x40 + column;

    LCD_Command(0x80 | address);
}

void LCD_Print(const char *text)
{
    while (*text)
    {
        LCD_Data(*text);
        text++;
    }
}

void LCD_PrintNumber(uint16_t number)
{
    char digits[6];
    uint8_t i = 0;

    if (number == 0)
    {
        LCD_Data('0');
        return;
    }

    while (number > 0 && i < 5)
    {
        digits[i++] = '0' + (number % 10);
        number /= 10;
    }

    while (i > 0)
    {
        LCD_Data(digits[--i]);
    }
}
