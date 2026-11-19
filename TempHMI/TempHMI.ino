#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#include "adc.h"
#include "dio.h"
#include "lcd_final.h"

#define MIN_TEMPERATURE 0
#define MAX_TEMPERATURE 99

#define SAMPLE_TIME_MS 10
#define DEBOUNCE_TICKS 3
#define LONG_PRESS_TICKS 50
#define REPEAT_TICKS 25

uint8_t highLimit = 40;
uint8_t lowLimit = 20;

typedef struct
{
    uint8_t lastRawState;
    uint8_t stableState;
    uint8_t debounceCount;
    uint16_t holdTicks;
    uint16_t repeatTicks;
} ButtonState;

static ButtonState highButton = {0, 0, 0, 0, 0};
static ButtonState lowButton  = {0, 0, 0, 0, 0};

static uint8_t temperature = 0;

static uint8_t ReadTemperature(void)
{
    uint16_t adcValue;
    uint32_t millivolts;

    adcValue = ADC_Read(0);

    // LM35: 10 mV per degree C
    // ADC reference = 5 V, resolution = 1023
    millivolts = ((uint32_t)adcValue * 5000UL) / 1023UL;

    return (uint8_t)(millivolts / 10UL);
}

static void UpdateOneButton(
    ButtonState *button,
    uint8_t rawPressed,
    uint8_t isHighButton)
{
    // Debounce the raw button state
    if (rawPressed != button->lastRawState)
    {
        button->lastRawState = rawPressed;
        button->debounceCount = 0;
    }
    else if (button->debounceCount < DEBOUNCE_TICKS)
    {
        button->debounceCount++;

        if (button->debounceCount == DEBOUNCE_TICKS)
        {
            if (button->stableState != rawPressed)
            {
                button->stableState = rawPressed;

                if (rawPressed)
                {
                    // Button has just been pressed
                    button->holdTicks = 0;
                    button->repeatTicks = 0;
                }
                else
                {
                    // Button has just been released
                    // Increase only if it was a short press
                    if (button->holdTicks < LONG_PRESS_TICKS)
                    {
                        if (isHighButton)
                        {
                            if (highLimit < MAX_TEMPERATURE)
                                highLimit++;
                        }
                        else
                        {
                            if (lowLimit < highLimit - 1)
                                lowLimit++;
                        }
                    }

                    button->holdTicks = 0;
                    button->repeatTicks = 0;
                }
            }
        }
    }

    // Process a button that is being held
    if (button->stableState)
    {
        if (button->holdTicks < LONG_PRESS_TICKS)
        {
            button->holdTicks++;

            if (button->holdTicks == LONG_PRESS_TICKS)
            {
                // First decrease after 500 ms
                if (isHighButton)
                {
                    if (highLimit > lowLimit + 1)
                        highLimit--;
                }
                else
                {
                    if (lowLimit > MIN_TEMPERATURE)
                        lowLimit--;
                }

                button->repeatTicks = 0;
            }
        }
        else
        {
            button->repeatTicks++;

            if (button->repeatTicks >= REPEAT_TICKS)
            {
                if (isHighButton)
                {
                    if (highLimit > lowLimit + 1)
                        highLimit--;
                }
                else
                {
                    if (lowLimit > MIN_TEMPERATURE)
                        lowLimit--;
                }

                button->repeatTicks = 0;
            }
        }
    }
}

static void UpdateButtons(void)
{
    uint8_t buttons = DIO_ReadButtons();

    UpdateOneButton(
        &highButton,
        (buttons & BUTTON_HIGH_MASK) ? 1 : 0,
        1
    );

    UpdateOneButton(
        &lowButton,
        (buttons & BUTTON_LOW_MASK) ? 1 : 0,
        0
    );
}

static void UpdateOutputs(void)
{
    uint8_t ledEnabled = 0;
    uint8_t buzzerEnabled = 0;

    if (temperature > lowLimit && temperature <= highLimit)
    {
        ledEnabled = 1;
        buzzerEnabled = 0;
    }
    else if (temperature > highLimit)
    {
        ledEnabled = 0;
        buzzerEnabled = 1;
    }

    DIO_SetOutputs(ledEnabled, buzzerEnabled);
}

static void DisplayStatus(void)
{
    LCD_SetCursor(0, 0);
    LCD_Print("Temp:");

    LCD_PrintNumber(temperature);
    LCD_Print("C     ");

    LCD_SetCursor(1, 0);
    LCD_Print("LH:");
    LCD_PrintNumber(highLimit);

    LCD_Print(" LL:");
    LCD_PrintNumber(lowLimit);

    LCD_Print("   ");
}

void setup(void)
{
    ADC_Init();
    DIO_Init();
    LCD_Init();

    LCD_Clear();

    LCD_SetCursor(0, 0);
    LCD_Print("Temperature HMI");

    _delay_ms(1000);
    LCD_Clear();
}

void loop(void)
{
    static uint8_t displayCounter = 0;

    // Read and process both buttons every 10 ms
    UpdateButtons();

    // Update temperature and outputs periodically
    displayCounter++;

    if (displayCounter >= 20)
    {
        displayCounter = 0;

        temperature = ReadTemperature();

        UpdateOutputs();
        DisplayStatus();
    }

    _delay_ms(SAMPLE_TIME_MS);
}
