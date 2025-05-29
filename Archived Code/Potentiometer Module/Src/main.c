#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "uart.h"
#include "stm32f303xc.h"

// === UART redirection ===
int __io_putchar(int ch) {
    SerialOutputChar((uint8_t)ch, &USART1_PORT);
    return ch;
}

int __io_getchar(void) {
    return SerialGetChar(&USART1_PORT);
}

// === Function Prototypes ===
void initialise_PA1_as_input(void);
void initialise_PE8_as_output(void);
void delay_ms(uint32_t ms);

// === GPIO Setup ===
void initialise_PA1_as_input(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER &= ~(3 << (1 * 2));
    GPIOA->PUPDR &= ~(3 << (1 * 2));
}

void initialise_PE8_as_output(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOEEN;
    GPIOE->MODER &= ~(3 << (8 * 2));
    GPIOE->MODER |= (1 << (8 * 2));
}

// === Crude Delay ===
void delay_ms(uint32_t ms) {
    for (volatile uint32_t i = 0; i < ms * 615; i++);
}

// === MAIN GAME ===
int main(void) {
    initialise_PA1_as_input();
    initialise_PE8_as_output();
    SerialInitialise(BAUD_115200, &USART1_PORT, NULL);

    // One hardcoded math question
    int a = 12;
    int b = 9;
    int result = a + b;

    printf("\r");
    delay_ms(1000);
    printf("=== Crack the code ===\r\n");
    printf("To crack the code and open the treasure chest, you must solve these questions and answer within 3s\r\n");
    printf("Odd answer = Twist LEFT\r\n");
    printf("Even answer = Twist RIGHT\r\n");
    printf("Press ENTER to begin...\r\n");

    // Wait for Enter key (CR = '\r' or LF = '\n')
    while (1) {
        char c = __io_getchar();
        if (c == '\r' || c == '\n') break;
    }
    printf("Ready???\r\n\r\n");
    delay_ms(1000);
    printf("GOOOOO!!!\r\n\r\n");

    printf("Question: %d + %d = ?\r\n", a, b);

    delay_ms(3000);  // Wait 3 seconds

    uint8_t input_state = (GPIOA->IDR & (1 << 1)) ? 1 : 0;
    int is_even = (result % 2 == 0);

    if ((is_even && input_state == 1) || (!is_even && input_state == 0)) {
        printf("Correct! %d is %s.\r\n", result, is_even ? "even" : "odd");
        GPIOE->ODR |= (1 << 8);  // Light up LED
    } else {
        printf("Incorrect. %d is %s.\r\n", result, is_even ? "even" : "odd");
        GPIOE->ODR &= ~(1 << 8); // LED off
    }

    while (1); // Halt
}
