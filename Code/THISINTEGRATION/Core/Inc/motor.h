#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>

void motor_init(void);
void motor_set_angle(uint8_t angle);  // Optional for standard servo mode
void motor_open(void);
void motor_close(void);

// ✅ Add this line:
void motor_set_pulse(uint16_t pulse);

void motor_start_forward(void);
void motor_start_reverse(void);
void motor_stop(void);

#endif
