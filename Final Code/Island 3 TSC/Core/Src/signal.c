#include "signal.h"
#include "stm32f3xx_hal.h"

void Signal_Init(void) {
    // Enable GPIOD clock
    RCC->AHBENR |= RCC_AHBENR_GPIODEN;

    // Configure PD3 as output, start LOW
    GPIOD->MODER &= ~(0b11 << (3 * 2));
    GPIOD->MODER |=  (0b01 << (3 * 2));
    GPIOD->ODR &= ~(1 << 3);

    // Configure PD4 as input with pull-down
    GPIOD->MODER &= ~(0b11 << (4 * 2));
    GPIOD->PUPDR &= ~(0b11 << (4 * 2));
    GPIOD->PUPDR |=  (0b10 << (4 * 2));
}

void Signal_SetHigh(void) {
    GPIOD->ODR |= (1 << 3); // Set PD3 HIGH
}

void Signal_SetLow(void) {
    GPIOD->ODR &= ~(1 << 3); // Set PD3 LOW
}

bool Signal_CheckStart(void) {
    return (GPIOD->IDR & (1 << 4)) != 0; // Check if PD4 is HIGH
}
