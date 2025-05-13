#include <stdint.h>
#include <stdio.h>
#include "stm32f303xc.h"
#include "riddles.h"

int __io_putchar(int ch) {
    SerialOutputChar((uint8_t)ch, &USART1_PORT);
    return ch;
}

static uint8_t riddleAsked = 0;
static uint8_t completedMessageShown = 0;
static uint8_t prevPA3State = 0;

void InitialisePA1AsInput(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER &= ~(3U << (1 * 2));
    GPIOA->PUPDR &= ~(3U << (1 * 2));
}

void InitialisePA3AsInput(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER &= ~(3U << (3 * 2));
    GPIOA->PUPDR &= ~(3U << (3 * 2));
}

int main(void) {
    InitialisePA1AsInput();
    InitialisePA3AsInput();
    SerialInitialise(BAUD_115200, &USART1_PORT, NULL);
    SerialSetReceiveCallback(&USART1_PORT, OnLineReceived);

    while (1) {
        // PA1 triggers riddle module
        if ((GPIOA->IDR & (1 << 1)) && isMinigame1Completed() && !isMinigame2Completed() && !riddleAsked) {
            AskNewRiddle();
            riddleAsked = 1;
        } else if ((GPIOA->IDR & (1 << 1)) && isMinigame2Completed() && !completedMessageShown) {
            printf("\r\nYou've already completed this riddle challenge! Proceed to the next game.\r\n");
            completedMessageShown = 1;
        }

        if (!(GPIOA->IDR & (1 << 1))) {
            riddleAsked = 0;
            completedMessageShown = 0;
        }

        // PA3 triggers next module
        uint8_t currentPA3State = (GPIOA->IDR & (1 << 3)) != 0;
        if (currentPA3State && !prevPA3State && isMinigame2Completed()) {
            printf("\r\nLED module\r\n");
            // game_progress |= 0b0100;
        }
        prevPA3State = currentPA3State;
    }
}
