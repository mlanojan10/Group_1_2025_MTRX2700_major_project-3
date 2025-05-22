#include <stdint.h>
#include <stdio.h>
#include "stm32f303xc.h"
#include "uart.h"
#include "riddles.h"
#include "potentiometer.h"

volatile uint8_t game_progress = 0b0000;


void island3_game(void) {
    printf("Island 3: placeholder\r\n");
    delay_ms(2000);
    game_progress |= 0b0100;
}

void island4_game(void) {
    printf("Island 4: placeholder\r\n");
    delay_ms(2000);
    game_progress |= 0b1000;
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

    while (1) {
		// Read all 4 LDRs
		uint8_t pa1 = (GPIOA->IDR & (1 << 1)) != 0; // Island 1
		uint8_t pa2 = (GPIOA->IDR & (1 << 2)) != 0; // Island 2
		uint8_t pa3 = (GPIOA->IDR & (1 << 3)) != 0; // Island 3
		uint8_t pa4 = (GPIOA->IDR & (1 << 4)) != 0; // Island 4

		if (pa1 && (game_progress == 0b0000)) {
			riddle_game();               // Sets bit 0
		}
		else if (pa2 && (game_progress == 0b0001)) {
			potentiometer_game();        // Sets bit 1
		}
		else if (pa3 && (game_progress == 0b0011)) {
			island3_game();              // Sets bit 2
		}
		else if (pa4 && (game_progress == 0b0111)) {
			island4_game();              // Sets bit 3
		}
		else if (pa1 || pa2 || pa3 || pa4) {
			printf("\r\nYou must complete the previous island first!\r\n");
		}

		delay_ms(100);
	}
}
