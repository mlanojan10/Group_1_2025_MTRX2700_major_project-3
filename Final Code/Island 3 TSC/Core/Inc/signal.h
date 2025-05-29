#ifndef SIGNAL_H
#define SIGNAL_H

#include <stdbool.h>

// Initializes signaling pins (PD3 as output, PD4 as input with pull-down)
void Signal_Init(void);

// Controls PD3 (output)
void Signal_SetHigh(void);
void Signal_SetLow(void);

// Reads PD4 (input) state: returns true if PD4 is HIGH
bool Signal_CheckStart(void);

#endif // SIGNAL_H
