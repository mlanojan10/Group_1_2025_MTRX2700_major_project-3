#include <stdint.h>
#include <stdio.h>
#include "stm32f303xc.h"
#include "uart.h"
#include "riddles.h"
#include "led.h"
#include "potentiometer.h"
#include "game_progress.h"
#include "timer.h"

volatile uint8_t game_progress = 0b0000;

// === GAME FUNCTIONS ===

void lidar_game(void) {
    printf("Island 3: placeholder\r\n");
    delay_ms(2000);
    game_progress |= 0b0001;
}

void island4_game(void) {
    printf("Island 4: placeholder\r\n");
    delay_ms(2000);
    game_progress |= 0b0100;
}

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



void Initialise_All(void) {
    InitialisePE11AsInput();  // Input 1 (was PE7, originally PC4 / PA11)
    InitialisePA2AsInput();
    InitialisePA3AsInput();
    InitialisePE9AsInput();   // Input 2 (was PC5 / PA12)
    InitialisePA5AsInput();
    SerialInitialise(BAUD_115200, &USART1_PORT, NULL);
}

// === MAIN LOOP ===

int main(void) {
    Initialise_All();

    uint8_t prev_pe11 = 0;
    uint8_t prev_pa2  = 0;
    uint8_t prev_pa3  = 0;
    uint8_t prev_pe9  = 0;

    printf("\r\n Ahoy ye, scallywag! Ye've set sail on the perilous path to treasure!\r\n");
    printf("Four cursed islands lie ahead, each holdin’ a test o’ wit, will, and courage.\r\n");
    printf("Touch the start position ‘n see if ye be brave enough to face Island 1... Yo ho ho!\r\n");

    while (1) {
        uint8_t pe11 = (GPIOE->IDR & (1 << 11)) != 0;  // Replaces PE7 / PC4 / PA11
        uint8_t pa2  = (GPIOA->IDR & (1 << 2))  != 0;
        uint8_t pa3  = (GPIOA->IDR & (1 << 3))  != 0;
        uint8_t pe9  = (GPIOE->IDR & (1 << 9))  != 0;  // Replaces PC5 / PA12

        // Check for new press (rising edge)
        if (pe11 && !prev_pe11) {
            if (game_progress == 0b0000) {
            	timer_init();
                lidar_game();
            } else {
                printf("\r\nYou have already done this island!\r\n");
            }
        } else if (pa2 && !prev_pa2) {
            if (game_progress == 0b0001) {
                riddle_game();
            } else {
                printf("\r\nYou must complete the previous lidar island first!\r\n");
            }
        } else if (pa3 && !prev_pa3) {
            if (game_progress == 0b0010) {
                island4_game();
            } else {
                printf("\r\nYou must complete the previous riddle island first!\r\n");
            }
        } else if (pe9 && !prev_pe9) {
            if (game_progress == 0b0100) {
                potentiometer_game();
            } else {
                printf("\r\nYou must complete the LED island!\r\n");
            }
        }

        // === Timeout Check ===
        if (seconds_remaining == 0 && game_progress != 0b1111) {
        	printf("\r\n Time’s up, matey! The treasure slips through yer fingers...\r\n");
        	while (1); // Game over — freeze or replace with reset logic
        }

        // Save previous states
        prev_pe11 = pe11;
        prev_pa2  = pa2;
        prev_pa3  = pa3;
        prev_pe9  = pe9;

        delay_ms(100);  // Simple debounce delay
    }
}
