#include "led_control.h"
#include "stm32f3xx_hal.h"
#include <stdint.h>

// Simple busy wait delay function, approx 1 ms delay per loop
static void delay_ms(uint32_t ms) {
    for (uint32_t i = 0; i < ms * 8000; i++) {
        __NOP();
    }
}

// Initialize LEDs on GPIOE pins 8–15 and PC7, PC8, PC9
void LED_Init(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOEEN;
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;

    // Configure PE8–PE15 as output
    GPIOE->MODER |= 0x55550000; // Set PE8–PE15 to output
    GPIOE->ODR &= ~0xFF00;

    // Configure PC7, PC8, PC9 as output
    GPIOC->MODER &= ~((0b11 << (7 * 2)) | (0b11 << (8 * 2)) | (0b11 << (9 * 2)));
    GPIOC->MODER |=  ((0b01 << (7 * 2)) | (0b01 << (8 * 2)) | (0b01 << (9 * 2)));
    GPIOC->ODR &= ~((1 << 7) | (1 << 8) | (1 << 9));
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
        delay_ms(100);
    }
}

// Blue LED (PC7)
void LED_BlueOn(void) {
    GPIOC->ODR |= (1 << 7);
}

void LED_BlueOff(void) {
    GPIOC->ODR &= ~(1 << 7);
}

// Red LED (PC8) - Failure Signal
void LED_SignalFailure(void) {
    GPIOC->ODR |= (1 << 8);
    delay_ms(2000);
    GPIOC->ODR &= ~(1 << 8);
}

// Green LED (PC9) - Success Signal
void LED_SignalSuccess(void) {
    GPIOC->ODR |= (1 << 9);
    delay_ms(2000);
    GPIOC->ODR &= ~(1 << 9);
}

