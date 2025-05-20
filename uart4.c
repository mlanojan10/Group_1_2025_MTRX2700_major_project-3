// This is UART4 code that can be included as a .c (and the accompanying .h) in the main integration code and into my touch sensor code. 
// I will add a separate file showing what should go in the mains to transmit

#include "stm32f303xc.h"
#include "uart4.h"

void UART4_Init(void) {
    // Enable clocks for GPIOC and UART4
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    RCC->APB1ENR |= RCC_APB1ENR_UART4EN;

    // PC10 = TX (AF8), PC11 = RX (AF8)
    // Set PC10 and PC11 to alternate function mode (10)
    GPIOC->MODER &= ~(GPIO_MODER_MODER10 | GPIO_MODER_MODER11);
    GPIOC->MODER |= (0b10 << GPIO_MODER_MODER10_Pos) | (0b10 << GPIO_MODER_MODER11_Pos);

    // Set high speed
    GPIOC->OSPEEDR |= (0b11 << GPIO_OSPEEDER_OSPEEDR10_Pos) | (0b11 << GPIO_OSPEEDER_OSPEEDR11_Pos);

    // Set alternate function AF8
    GPIOC->AFR[1] &= ~(0xF << ((10 - 8) * 4)); // clear bits for PC10
    GPIOC->AFR[1] &= ~(0xF << ((11 - 8) * 4)); // clear bits for PC11
    GPIOC->AFR[1] |= (8 << ((10 - 8) * 4)); // AF8 for PC10
    GPIOC->AFR[1] |= (8 << ((11 - 8) * 4)); // AF8 for PC11

    // Set baud rate: 115200 assuming 8 MHz clock
    // BRR = fclk / baud = 8000000 / 115200 = ~69
    UART4->BRR = 69;

    // Enable UART4 for TX and RX
    UART4->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

void UART4_SendChar(uint8_t c) {
    while (!(UART4->ISR & USART_ISR_TXE));
    UART4->TDR = c;
}

uint8_t UART4_ReceiveChar(void) {
    while (!(UART4->ISR & USART_ISR_RXNE));
    return (uint8_t)(UART4->RDR);
}

void UART4_SendString(const char *str) {
    while (*str) {
        UART4_SendChar(*str++);
    }
}
