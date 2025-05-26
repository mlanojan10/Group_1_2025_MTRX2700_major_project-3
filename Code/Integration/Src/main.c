
#include <stdint.h>
#include <stdio.h>
#include "stm32f303xc.h"
#include "uart.h"
#include "riddles.h"
#include "led.h"
#include "potentiometer.h"
#include "game_progress.h"

volatile uint8_t game_progress = 0b0000;


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
	InitialisePA1AsInput();
	InitialisePA2AsInput();
	InitialisePA3AsInput();
	InitialisePA4AsInput();
	InitialisePA5AsInput();
	SerialInitialise(BAUD_115200, &USART1_PORT, NULL);
}

int main(void) {
    Initialise_All();

    uint8_t prev_pa1 = 0;
    uint8_t prev_pa2 = 0;
    uint8_t prev_pa3 = 0;
    uint8_t prev_pa4 = 0;

    printf("\r\n Ahoy ye, scallywag! Ye've set sail on the perilous path to treasure!\r\n");
    printf("Four cursed islands lie ahead, each holdin’ a test o’ wit, will, and courage.\r\n");
    printf("Touch the start position ‘n see if ye be brave enough to face Island 1... Yo ho ho!\r\n");


    while (1) {
        uint8_t pa1 = (GPIOA->IDR & (1 << 1)) != 0;
        uint8_t pa2 = (GPIOA->IDR & (1 << 2)) != 0;
        uint8_t pa3 = (GPIOA->IDR & (1 << 3)) != 0;
        uint8_t pa4 = (GPIOA->IDR & (1 << 4)) != 0;

        // Check for new press (rising edge)
        if (pa1 && !prev_pa1) {
            if (game_progress == 0b0000) {
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
                //led_game();
            	island4_game();
            } else {
                printf("\r\nYou must complete the previous riddle island first!\r\n");
            }
        } else if (pa4 && !prev_pa4) {
            if (game_progress == 0b0100) {
                potentiometer_game();
            } else {
                printf("\r\nYou must complete the LED isalnd!\r\n");
            }
        }

        // Save previous states for next loop
        prev_pa1 = pa1;
        prev_pa2 = pa2;
        prev_pa3 = pa3;
        prev_pa4 = pa4;

        delay_ms(100);  // Simple debounce delay
    }
}
