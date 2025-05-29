#include "stm32f303xc.h"
#include <stdint.h>
#include <stdio.h>

// --- Timer / UART / GPIO Constants ---
#define MAX_SECONDS         480
#define LED_COUNT           8
#define SECONDS_PER_LED     (MAX_SECONDS / LED_COUNT)

#define SYSTEM_CLOCK_HZ     8000000U
#define UART_BAUDRATE       115200
#define UART_TX_PIN         9
#define UART_TX_AF          7

#define TIMER_PRESCALER     (8000 - 1)    // 1 ms tick
#define TIMER_AUTO_RELOAD   (1000 - 1)    // 1 second

#define LED_X_PIN           7   // PE5 = Timer Start LED
#define LED_Y_PIN           6   // PE6 = Timer End LED

const uint8_t led_pins[LED_COUNT] = {
    15, 14, 13, 12, 3, 10, 2, 8
};

// --- Globals ---
volatile uint16_t seconds_remaining = MAX_SECONDS;
volatile uint8_t blink_state = 1;

// --- UART ---
void uart_init(void) {
    RCC->AHBENR  |= RCC_AHBENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    GPIOA->MODER &= ~(3 << (2 * UART_TX_PIN));
    GPIOA->MODER |=  (2 << (2 * UART_TX_PIN));
    GPIOA->AFR[1] |= (UART_TX_AF << ((UART_TX_PIN - 8) * 4));

    USART1->BRR  = SYSTEM_CLOCK_HZ / UART_BAUDRATE;
    USART1->CR1 |= USART_CR1_TE | USART_CR1_UE;
}

void uart_send_char(char c) {
    while (!(USART1->ISR & USART_ISR_TXE));
    USART1->TDR = c;
}

void uart_send_string(const char *s) {
    while (*s) uart_send_char(*s++);
}

void uart_send_time(uint16_t seconds) {
    char buf[16];
    uint8_t min = seconds / 60;
    uint8_t sec = seconds % 60;
    sprintf(buf, "\r%02u:%02u", min, sec);
    uart_send_string(buf);
}

// --- Timer2 ---
void timer2_init(void) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    TIM2->PSC = TIMER_PRESCALER;
    TIM2->ARR = TIMER_AUTO_RELOAD;
    TIM2->DIER |= TIM_DIER_UIE;
    TIM2->CR1 |= TIM_CR1_CEN;

    NVIC_SetPriority(TIM2_IRQn, 1);
    NVIC_EnableIRQ(TIM2_IRQn);
}

// --- GPIO ---
void gpio_init(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOEEN;

    // Main 8 LEDs
    for (int i = 0; i < LED_COUNT; ++i) {
        uint8_t pin = led_pins[i];
        GPIOE->MODER &= ~(3 << (2 * pin));
        GPIOE->MODER |=  (1 << (2 * pin));
        GPIOE->ODR    |= (1 << pin);  // Start ON
    }

    // LED X (Start)
    GPIOE->MODER &= ~(3 << (2 * LED_X_PIN));
    GPIOE->MODER |=  (1 << (2 * LED_X_PIN));
    GPIOE->ODR |= (1 << LED_X_PIN);  // ON at start

    // LED Y (End)
    GPIOE->MODER &= ~(3 << (2 * LED_Y_PIN));
    GPIOE->MODER |=  (1 << (2 * LED_Y_PIN));
    GPIOE->ODR &= ~(1 << LED_Y_PIN);  // OFF at start
}

void update_LEDs(uint16_t seconds) {
    uint8_t led_index = seconds / SECONDS_PER_LED;

    // Introduce a 1-sec delay before LED 0 starts blinking
    if (seconds >= MAX_SECONDS - 1) {
        // Do NOT start LED index 0 until next second
        led_index = 0xFF;
    }

    for (int i = 0; i < LED_COUNT; ++i) {
        uint8_t pin = led_pins[i];

        if (i < led_index) {
            GPIOE->ODR &= ~(1 << pin);  // OFF
        } else if (i > led_index) {
            GPIOE->ODR |= (1 << pin);   // ON
        } else {
            // Blink only the active LED
            if (blink_state)
                GPIOE->ODR |= (1 << pin);
            else
                GPIOE->ODR &= ~(1 << pin);
        }
    }
}

// --- Timer2 ISR ---
void TIM2_IRQHandler(void) {
    if (TIM2->SR & TIM_SR_UIF) {
        TIM2->SR &= ~TIM_SR_UIF;
        blink_state ^= 1;

        if (seconds_remaining > 0) {
            seconds_remaining--;
            update_LEDs(seconds_remaining);
            uart_send_time(seconds_remaining);
        } else {
            uart_send_string("\r00:00 - Game Over!\n");

            GPIOE->ODR &= ~(1 << LED_X_PIN);  // LED X OFF
            GPIOE->ODR |=  (1 << LED_Y_PIN);  // LED Y ON
        }
    }
}

// --- Main ---
int main(void) {
    gpio_init();
    uart_init();
    timer2_init();

    uart_send_string("\n--- Timer Started ---\n");
    uart_send_time(seconds_remaining);

    while (1) {
        // Everything handled in ISR
    }
}

