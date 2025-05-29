#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include <stdint.h>

void LED_Init(void);
void LED_DisplayPattern(const uint8_t *pattern, uint8_t length, uint32_t delay_ms);
void LED_ClearAll(void);
void LED_Set(uint8_t index, uint8_t state);

#endif
