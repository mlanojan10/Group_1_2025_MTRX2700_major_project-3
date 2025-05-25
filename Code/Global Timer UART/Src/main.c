#include "stm32f303xc.h"
#include <stdio.h>
#include <string.h>

// Global 8-minute countdown (480 seconds)
volatile uint16_t seconds_remaining = 480;
char uart_buffer[64];

// UART send function
void uart_send_string(const char *str) {
    while (*str) {
        while (!(USART1->ISR & USART_ISR_TXE));
        USART1->TDR = *str++;
    }
}

// Display formatted time over UART
void display_time_uart(uint16_t secs) {
    uint8_t min = secs / 60;
    uint8_t sec = secs % 60;
    sprintf(uart_buffer, "Time left: %02d:%02d\r\n", min, sec);
    uart_send_string(uart_buffer);
}

// TIM2 interrupt handler – 1s interval
void TIM2_IRQHandler(void) {
    if (TIM2->SR & TIM_SR_UIF) {
        TIM2->SR &= ~TIM_SR_UIF;

        // Toggle PE9 LED for test
        GPIOE->ODR ^= (1 << 9);

        // Decrement timer + UART print
        if (seconds_remaining > 0) {
            seconds_remaining--;
            display_time_uart(seconds_remaining);
        }
    }
}

void timer2_init(void) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    TIM2->PSC = 8000 - 1;       // 1 ms tick
    TIM2->ARR = 1000 - 1;       // 1000 ticks = 1 sec
    TIM2->DIER |= TIM_DIER_UIE;
    TIM2->CR1 |= TIM_CR1_CEN;

    NVIC_SetPriority(TIM2_IRQn, 1);
    NVIC_EnableIRQ(TIM2_IRQn);
}

void uart1_init(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    // PC4 = TX, PC5 = RX
    GPIOC->MODER &= ~((3 << (2*4)) | (3 << (2*5)));
    GPIOC->MODER |= (2 << (2*4)) | (2 << (2*5));  // Alternate function
    GPIOC->AFR[0] |= (7 << (4*4)) | (7 << (4*5)); // AF7 = USART1

    USART1->BRR = 8000000 / 115200;
    USART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

void gpio_led_init(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOEEN;
    GPIOE->MODER &= ~(3 << (2 * 9));
    GPIOE->MODER |= (1 << (2 * 9)); // PE9 = output
    GPIOE->ODR &= ~(1 << 9);        // Turn off initially
}

int main(void) {
    gpio_led_init();
    uart1_init();
    timer2_init();
    display_time_uart(seconds_remaining); // Show initial time

    while (1) {
        // Main loop not used — logic in ISR
    }
}

