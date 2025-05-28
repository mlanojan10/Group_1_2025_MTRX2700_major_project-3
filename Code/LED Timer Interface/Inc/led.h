#ifndef LED_H
#define LED_H

#include <stdint.h>

void gpio_init(void);
void update_LEDs(uint16_t seconds);

#endif
