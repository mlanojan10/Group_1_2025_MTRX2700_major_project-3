#include "stm32f303xc.h"
#include "timer.h"

#define MAX_SECONDS         480
#define LED_COUNT           8
#define SECONDS_PER_LED     (MAX_SECONDS / LED_COUNT)

const uint8_t led_pins[LED_COUNT] = { 15, 14, 13, 12, 3, 10, 2, 8 };

volatile uint16_t seconds_remaining = MAX_SECONDS;
volatile uint8_t blink_state = 1;

void timer_init(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOEEN;
    for (int i = 0; i < LED_COUNT; ++i) {
        uint8_t pin = led_pins[i];
        GPIOE->MODER &= ~(3 << (2 * pin));
        GPIOE->MODER |=  (1 << (2 * pin));
        GPIOE->ODR |= (1 << pin);  // All ON
    }

    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    TIM2->PSC = 8000 - 1;
    TIM2->ARR = 1000 - 1;
    TIM2->DIER |= TIM_DIER_UIE;
    TIM2->CR1 |= TIM_CR1_CEN;

    NVIC_SetPriority(TIM2_IRQn, 1);
    NVIC_EnableIRQ(TIM2_IRQn);
}

void update_timer_leds(uint16_t seconds) {
    uint8_t led_index = seconds / SECONDS_PER_LED;
    for (int i = 0; i < LED_COUNT; ++i) {
        uint8_t pin = led_pins[i];
        if (i < led_index) {
            GPIOE->ODR &= ~(1 << pin);
        } else if (i > led_index) {
            GPIOE->ODR |= (1 << pin);
        } else {
            if (blink_state)
                GPIOE->ODR |= (1 << pin);
            else
                GPIOE->ODR &= ~(1 << pin);
        }
    }
}

void TIM2_IRQHandler(void) {
    if (TIM2->SR & TIM_SR_UIF) {
        TIM2->SR &= ~TIM_SR_UIF;
        blink_state ^= 1;
        if (seconds_remaining > 0) {
            seconds_remaining--;
            update_timer_leds(seconds_remaining);
        }
    }
}
