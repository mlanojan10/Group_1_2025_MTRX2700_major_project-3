
#include "uart4.h"

#include "stm32f3xx.h"

void UART4_Init(void)
{
    // Enable clocks for GPIOC and UART4
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    RCC->APB1ENR |= RCC_APB1ENR_UART4EN;

    // Configure PC10 (TX) and PC11 (RX) for alternate function mode AF5
    GPIOC->MODER &= ~(GPIO_MODER_MODER10_Msk | GPIO_MODER_MODER11_Msk);
    GPIOC->MODER |= (0b10 << GPIO_MODER_MODER10_Pos) | (0b10 << GPIO_MODER_MODER11_Pos); // AF mode

    GPIOC->AFR[1] &= ~((0xF << 8) | (0xF << 12));
    GPIOC->AFR[1] |= (5 << 8) | (5 << 12);

    // Configure UART4
    UART4->BRR = HAL_RCC_GetPCLK1Freq() / 115200;
    UART4->CR1 = USART_CR1_TE | USART_CR1_RE; // Enable Transmit and Receive
    UART4->CR1 |= USART_CR1_UE;               // Enable UART
}


void UART4_SendByte(uint8_t byte)
{
    while (!(UART4->ISR & USART_ISR_TXE));  // Wait until transmit buffer is empty
    UART4->TDR = byte;                      // Send byte
    while (!(UART4->ISR & USART_ISR_TC));   // Wait until transmission is complete
}

uint8_t UART4_ReceiveByte(void)
{
    while (!(UART4->ISR & USART_ISR_RXNE));
    return (uint8_t)(UART4->RDR);
}

