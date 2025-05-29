#include "stm32f303xc.h"
#include "timer.h"
#include "led.h"
#include "uart.h"
#include "config.h"

volatile uint16_t seconds_remaining = MAX_SECONDS;

void timer2_init(void) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    TIM2->PSC = TIMER_PRESCALER;
    TIM2->ARR = TIMER_AUTO_RELOAD;
    TIM2->DIER |= TIM_DIER_UIE;
    TIM2->CR1 |= TIM_CR1_CEN;

    NVIC_SetPriority(TIM2_IRQn, 1);
    NVIC_EnableIRQ(TIM2_IRQn);
}

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
        }
    }
}
