#include <stdint.h>
#include <stdio.h>
#include "stm32f303xc.h"
#include "uart.h"
#include "riddles.h"
#include "led.h"
#include "potentiometer.h"
#include "buzzer.h"
#include "motor.h"
#include "game_progress.h"

volatile uint8_t game_progress = 0b0000;

// === GAME FUNCTIONS ===



// === IO FUNCTIONS ===

int __io_putchar(int ch) {
    SerialOutputChar((uint8_t)ch, &USART1_PORT);
    return ch;
}

int __io_getchar(void) {
    return SerialGetChar(&USART1_PORT);
}

void delay_ms(uint32_t ms) {
    for (volatile uint32_t i = 0; i < ms * 615; i++);
}

void InitialisePE6AsInput(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOEEN;
    GPIOE->MODER &= ~(3 << (2 * 6));  // PE6 as input
}

void InitialisePE7AsInput(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOEEN;
    GPIOE->MODER &= ~(3 << (2 * 7));  // PE7 as input
}

void Initialise_All(void) {
    InitialisePE11AsInput();  // Input 1 (was PE7, originally PC4 / PA11)
    InitialisePA2AsInput();
    InitialisePA3AsInput();
    InitialisePE9AsInput();   // Input 2 (was PC5 / PA12)
    InitialisePA5AsInput();
    InitialisePE3AsInput();   // Timer end signal
    InitialisePE2AsInput();   // Timer start signal
    SerialInitialise(BAUD_115200, &USART1_PORT, NULL);
}

// === MAIN LOOP ===

int main(void) {
    Initialise_All();

    uint8_t prev_pe11 = 0;
    uint8_t prev_pa2  = 0;
    uint8_t prev_pa3  = 0;
    uint8_t prev_pe9  = 0;
    uint8_t game_started = 0;
    uint8_t game_ended = 0;

    printf("\r\r\n");
    delay_ms(3000);
    printf("\r\n Ahoy ye, scallywag! Ye've set sail on the perilous path to treasure!\r\n");
    printf("Four cursed islands lie ahead, each holdin’ a test o’ wit, will, and courage.\r\n");
    printf("Touch the start position ‘n see if ye be brave enough to face Island 1... Yo ho ho!\r\n");

    while (1) {
        uint8_t pe11 = (GPIOE->IDR & (1 << 11)) != 0;  // Replaces PE7 / PC4 / PA11
        uint8_t pa2  = (GPIOA->IDR & (1 << 2))  != 0;
        uint8_t pa3  = (GPIOA->IDR & (1 << 3))  != 0;
        uint8_t pe9  = (GPIOE->IDR & (1 << 9))  != 0;  // Replaces PC5 / PA12
        uint8_t pe3  = (GPIOE->IDR & (1 << 6))  != 0;  // Timer end signal
        uint8_t pe2  = (GPIOE->IDR & (1 << 7))  != 0;  // Timer start signal

        // Start the game only after receiving PE7 high
        if (!game_started && pe3) {
            game_started = 1;
            printf("\r\nTimer started! You may now begin the game.\r\n");
        }

        // If timer has ended via PE6 signal and game not complete
        if (game_started && !game_ended && pe2 && game_progress != 0b1000) {
            game_ended = 1;
            printf("\r\n Time’s up, matey! The treasure slips through yer fingers...\r\n");
        }

        if (!game_started || game_ended) {
            delay_ms(100);
            continue;
        }

        // Check for new press (rising edge)
        if (pe11 && !prev_pe11) {
            if (game_progress == 0b0000) {
                printf("\r\nWelcome to the pirate ship!\r\n");
                printf("\r\nSteer your ship to avoid the obstacles!\r\n");
                printf("\r\nGOOOOO!!!!!\r\n");
                lidar_game();
                game_progress = 0b0001;
            } else {
                printf("\r\nYou cannot do this island!\r\n");
            }
        } else if (pa2 && !prev_pa2) {
            if (game_progress == 0b0001) {
                riddle_game();
                game_progress = 0b0010;
            } else {
                printf("\r\nYou cannot do this island!\r\n");
            }
        } else if (pa3 && !prev_pa3) {
            if (game_progress == 0b0010) {
                led_game();
                game_progress = 0b0100;
            } else {
                printf("\r\nYou cannot do this island!\r\n");
            }
        } else if (pe9 && !prev_pe9) {
            if (game_progress == 0b0100) {
                potentiometer_game();
            } else {
                printf("\r\nYou cannot do this island!\r\n");
            }
        }

        // Save previous states
        prev_pe11 = pe11;
        prev_pa2  = pa2;
        prev_pa3  = pa3;
        prev_pe9  = pe9;

        delay_ms(100);  // Simple debounce delay
    }
}
