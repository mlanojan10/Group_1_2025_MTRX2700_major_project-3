// common header file 

#ifndef UART4_H
#define UART4_H

#include <stdint.h>

void UART4_Init(void);
void UART4_SendChar(uint8_t c);
uint8_t UART4_ReceiveChar(void);
void UART4_SendString(const char *str);

#endif
