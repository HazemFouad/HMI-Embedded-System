#include "dio.h"

void DIO_Init(void)
{
    // D2 (PD2) and D3 (PD3): button inputs
    DDRD &= ~((1 << PD2) | (1 << PD3));

    // Enable internal pull-up resistors
    PORTD |= (1 << PD2) | (1 << PD3);

    // D8 (PB0): LED output
    // D9 (PB1): buzzer output
    DDRB |= (1 << PB0) | (1 << PB1);

    // Initially turn both OFF
    PORTB &= ~((1 << PB0) | (1 << PB1));
}

uint8_t DIO_ReadButtons(void)
{
    uint8_t buttons = 0;

    // A pressed button reads LOW
    if (!(PIND & (1 << PD2)))
        buttons |= BUTTON_HIGH_MASK;

    if (!(PIND & (1 << PD3)))
        buttons |= BUTTON_LOW_MASK;

    return buttons;
}

void DIO_SetOutputs(uint8_t ledEnabled, uint8_t buzzerEnabled)
{
    if (ledEnabled)
        PORTB |= (1 << PB0);
    else
        PORTB &= ~(1 << PB0);

    if (buzzerEnabled)
        PORTB |= (1 << PB1);
    else
        PORTB &= ~(1 << PB1);
}
