#include "led_control.h"
#include "stm32f3xx_hal.h"
#include <stdint.h>

// Simple busy wait delay function, approx 1 ms delay per loop (adjust as needed)
static void delay_ms(uint32_t ms) {
    for (uint32_t i = 0; i < ms * 8000; i++) {
        __NOP();
    }
}

// Initialize LEDs on GPIOE pins 8–15
void LED_Init(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOEEN;
    GPIOE->MODER |= 0x55550000; // Set PE8–PE15 to output mode
    GPIOE->ODR &= ~0xFF00;      // Turn off LEDs initially
}

void LED_Set(uint8_t index, uint8_t state) {
    if (index < 8) {
        if (state)
            GPIOE->ODR |= (1 << (8 + index));
        else
            GPIOE->ODR &= ~(1 << (8 + index));
    }
}

void LED_ClearAll(void) {
    GPIOE->ODR &= ~0xFF00;
}

void LED_DisplayPattern(const uint8_t *pattern, uint8_t length, uint32_t delay_time_ms) {
    for (uint8_t i = 0; i < length; i++) {
        LED_ClearAll();
        LED_Set(pattern[i], 1);
        delay_ms(delay_time_ms);
        LED_ClearAll();
        delay_ms(100); // short off interval between LEDs
    }
}

