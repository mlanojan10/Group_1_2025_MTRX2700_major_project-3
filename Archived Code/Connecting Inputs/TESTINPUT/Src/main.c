#include "stm32f303xc.h"


//This Code connects the breadboard input to the STM for analysis. Currently input is PA1 and will turn on a LED to show that it is working
void initialise_PA1_as_input(void) {
    // Enable GPIOA clock
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

    // Set PA1 as input (MODER = 00)
    GPIOA->MODER &= ~(3 << (1 * 2)); // Clear bits 3:2

    // No pull-up/pull-down
    GPIOA->PUPDR &= ~(3 << (1 * 2));
}

void initialise_PE8_as_output(void) {
    // Enable GPIOE clock
    RCC->AHBENR |= RCC_AHBENR_GPIOEEN;

    // Set PE8 as output (MODER = 01)
    GPIOE->MODER &= ~(3 << (8 * 2)); // Clear bits 17:16
    GPIOE->MODER |=  (1 << (8 * 2)); // Set bit 16
}

int main(void) {
    initialise_PA1_as_input();
    initialise_PE8_as_output();

    while (1) {
        // Read PA1 input
        if (GPIOA->IDR & (1 << 1)) {
            // If PA1 is high, turn on LED at PE8
            GPIOE->ODR |= (1 << 8);
        } else {
            // If PA1 is low, turn off LED
            GPIOE->ODR &= ~(1 << 8);
        }
    }
}
