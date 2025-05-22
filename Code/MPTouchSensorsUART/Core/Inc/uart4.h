
#ifndef UART4_H
#define UART4_H

#include <stdint.h>

void UART4_Init(void);
void UART4_SendByte(uint8_t byte);
uint8_t UART4_ReceiveByte(void);

#endif // UART4_H
