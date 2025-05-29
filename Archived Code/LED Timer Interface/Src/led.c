#include "stm32f303xc.h"
#include "led.h"
#include "config.h"

volatile uint8_t blink_state = 1;

// LED pins from PE15 to PE8 (countdown order)
const uint8_t led_pins[LED_COUNT] = {15, 14, 13, 12, 3, 10, 2, 8};

void gpio_init(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOEEN;

    for (int i = 0; i < LED_COUNT; ++i) {
        uint8_t pin = led_pins[i];
        GPIOE->MODER &= ~(3 << (2 * pin));
        GPIOE->MODER |=  (1 << (2 * pin));
        GPIOE->ODR    |= (1 << pin);  // Turn on
    }
}

void update_LEDs(uint16_t seconds) {
    uint8_t led_index = seconds / SECONDS_PER_LED;

    for (int i = 0; i < LED_COUNT; ++i) {
        uint8_t pin = led_pins[i];

        if (i < led_index) {
            GPIOE->ODR &= ~(1 << pin);
        } else if (i > led_index) {
            GPIOE->ODR |= (1 << pin);
        } else {
            if (blink_state) GPIOE->ODR |= (1 << pin);
            else GPIOE->ODR &= ~(1 << pin);
        }
    }
}
