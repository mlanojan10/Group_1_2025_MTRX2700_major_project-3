#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>

void buzzer_init(void);
void buzzer_on(void);
void buzzer_off(void);
void buzzer_set_frequency(uint32_t freq_hz);
void siren_sound(void);
void fail_sound(void);
void pirate_sound(void);

#endif // BUZZER_H
