#ifndef DIO_H
#define DIO_H

#include <avr/io.h>
#include <stdint.h>

#define BUTTON_HIGH_MASK  0x01
#define BUTTON_LOW_MASK   0x02

void DIO_Init(void);
uint8_t DIO_ReadButtons(void);
void DIO_SetOutputs(uint8_t ledEnabled, uint8_t buzzerEnabled);

#endif
