#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

extern volatile uint16_t seconds_remaining;
extern volatile uint8_t blink_state;

void timer_init(void);
void update_timer_leds(uint16_t seconds);

#endif
