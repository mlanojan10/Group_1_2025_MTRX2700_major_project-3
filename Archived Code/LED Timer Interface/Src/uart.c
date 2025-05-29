#include "stm32f303xc.h"
#include "uart.h"
#include "config.h"
#include <stdio.h>

void uart_init(void) {
    RCC->AHBENR  |= RCC_AHBENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    GPIOA->MODER &= ~(3 << (2 * UART_TX_PIN));
    GPIOA->MODER |=  (2 << (2 * UART_TX_PIN));
    GPIOA->AFR[1] |= (UART_TX_AF << ((UART_TX_PIN - 8) * 4));

    USART1->BRR  = SYSTEM_CLOCK_HZ / UART_BAUDRATE;
    USART1->CR1 |= USART_CR1_TE | USART_CR1_UE;
}

void uart_send_char(char c) {
    while (!(USART1->ISR & USART_ISR_TXE));
    USART1->TDR = c;
}

void uart_send_string(const char *s) {
    while (*s) uart_send_char(*s++);
}

void uart_send_time(uint16_t seconds) {
    char buf[16];
    uint8_t min = seconds / 60;
    uint8_t sec = seconds % 60;
    sprintf(buf, "\r%02u:%02u", min, sec);
    uart_send_string(buf);
}
