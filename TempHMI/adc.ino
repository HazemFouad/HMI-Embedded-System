#include "adc.h"

void ADC_Init(void)
{
    // AVCC reference, right-adjusted result
    ADMUX = (1 << REFS0);

    // Enable ADC, prescaler = 128
    ADCSRA = (1 << ADEN) |
             (1 << ADPS2) |
             (1 << ADPS1) |
             (1 << ADPS0);
}

uint16_t ADC_Read(uint8_t channel)
{
    channel &= 0x07;

    // Select ADC channel
    ADMUX = (ADMUX & 0xF0) | channel;

    // Start conversion
    ADCSRA |= (1 << ADSC);

    // Wait until conversion finishes
    while (ADCSRA & (1 << ADSC));

    return ADC;
}
