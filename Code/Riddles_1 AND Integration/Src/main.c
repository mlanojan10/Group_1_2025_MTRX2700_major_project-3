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
volatile uint8_t game_progress = 0b0000;


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

void InitialisePA5AsInput(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;         // Enable GPIOA clock
    GPIOA->MODER &= ~(3U << (5 * 2));          // Set PA5 as input (00)
    GPIOA->PUPDR &= ~(3U << (5 * 2));          // No pull-up, pull-down
}

void delay_ms(uint32_t ms) {
    // Simple loop delay - calibrate as needed or replace with timer-based delay
    volatile uint32_t count;
    while (ms--) {
        count = 8000;  // Adjust this count for your clock speed to get ~1ms delay
        while (count--) __asm__("nop");
    }
}

int main(void) {
    InitialisePA1AsInput();
    InitialisePA3AsInput();
    InitialisePA5AsInput();
    SerialInitialise(BAUD_115200, &USART1_PORT, NULL);
    SerialSetReceiveCallback(&USART1_PORT, OnLineReceived);

    uint8_t prevPA1State = 0;
    uint8_t prevPA3State = 0;
    uint8_t prevPA5State = 0;

    uint8_t riddleAsked = 0;
    uint8_t completedMessageShown = 0;

    while (1) {
        uint8_t currentPA1State = (GPIOA->IDR & (1 << 1)) != 0;
        uint8_t currentPA3State = (GPIOA->IDR & (1 << 3)) != 0;
        uint8_t currentPA5State = (GPIOA->IDR & (1 << 5)) != 0;

        // --- PA1: Riddle module ---
        if (currentPA1State && !prevPA1State) { // Possible rising edge
            delay_ms(50); // debounce delay
            currentPA1State = (GPIOA->IDR & (1 << 1)) != 0; // re-read pin
            if (currentPA1State) { // Confirmed rising edge after debounce
            	if ((game_progress & 0b0010) != 0) {
            	    printf("\r\nYou have already completed this island.\r\n");
            	} else if ((game_progress & 0b0100) != 0) {
            	    printf("\r\nYou have already completed this island.\r\n");
            	} else if ((game_progress & 0b1000) != 0) {
            	    printf("\r\nYOU HAVE ALREADY WON THE GAME!!\r\n");
            	} else if ((game_progress & 0b0001) != 0) {
            	    AskNewRiddle();
            	} else {
            	    printf("\r\nYou cannot do this module yet!\r\n");
            	}
            }
        }

        if (!currentPA1State && prevPA1State) { // Falling edge
            riddleAsked = 0;
            completedMessageShown = 0;
        }

        // --- PA3: LED module ---
        if (currentPA3State && !prevPA3State) { // Possible rising edge
            delay_ms(50);
            currentPA3State = (GPIOA->IDR & (1 << 3)) != 0;
            if (currentPA3State) {
                if ((game_progress & 0b0010) != 0) {
                    printf("\r\nLED module\r\n");
                    // Call LED module function here if needed
                    // game_progress |= 0b0100;
                } else if ((game_progress & 0b0100) != 0) {
                    printf("\r\nYou have already completed this island.\r\n");
                } else if ((game_progress & 0b1000) != 0) {
                	printf("\r\nYOU HAVE ALREADY WON THE GAME!!\r\n");
                } else {
                    printf("\r\nYou cannot do this module yet!\r\n");
                }
            }
        }

        // --- PA5: Potentiometer module ---
        if (currentPA5State && !prevPA5State) { // Possible rising edge
            delay_ms(50);
            currentPA5State = (GPIOA->IDR & (1 << 5)) != 0;
            if (currentPA5State) {
                if ((game_progress & 0b0100) != 0) {
                    printf("\r\nPotentiometer module\r\n");
                    //potentiometer_game(); // Launch the module
                    // game_progress |= 0b1000;
                } else if ((game_progress & 0b1000) != 0) {
                	printf("\r\nYOU HAVE ALREADY WON THE GAME!!\r\n");
                } else {
                    printf("\r\nYou cannot do this module yet!\r\n");
                }
            }
        }

        prevPA1State = currentPA1State;
        prevPA3State = currentPA3State;
        prevPA5State = currentPA5State;
    }
}
