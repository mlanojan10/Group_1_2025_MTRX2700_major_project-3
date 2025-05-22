#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <string.h>
#include "uart.h"
#include "potentiometer.h"
#include "stm32f303xc.h"

extern volatile uint8_t game_progress;


void InitialisePA1AsInput(void) {    //island 1
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER &= ~(3U << (1 * 2));
    GPIOA->PUPDR &= ~(3U << (1 * 2));
}

void InitialisePA2AsInput(void) {    //island 2
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER &= ~(3U << (2 * 2));
    GPIOA->PUPDR &= ~(3U << (2 * 2));
}

void InitialisePA3AsInput(void) {    //island 3
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER &= ~(3U << (3 * 2));
    GPIOA->PUPDR &= ~(3U << (3 * 2));
}

void InitialisePA4AsInput(void) {     //island 4
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;         // Enable GPIOA clock
    GPIOA->MODER &= ~(3U << (4 * 2));          // Set PA5 as input (00)
    GPIOA->PUPDR &= ~(3U << (4 * 2));          // No pull-up, pull-down
}

void InitialisePA5AsInput(void) {    //island 5
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER &= ~(3U << (5 * 2));
    GPIOA->PUPDR &= ~(3U << (5 * 2));
}

void delay_ms(uint32_t ms);

int __io_getchar(void);


void potentiometer_game(void) {
	// Start TIM2 for random seeding
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	TIM2->PSC = 7999;
	TIM2->ARR = 0xFFFFFFFF;
	TIM2->CR1 |= TIM_CR1_CEN;

	while (1) {
		// === Start message ===
		 printf("\r");
		delay_ms(1000);
		printf("YOU HAVE FOUND THE TREASURE, BUT IT IS LOCKED!\r\n");
		printf("To crack the code and open the treasure chest, you must solve each question and answer within 4s\r\n");
		printf("Odd answer = Twist LEFT\r\n");
		printf("Even answer = Twist RIGHT\r\n");
		printf("Press ENTER to begin...\r\n");

		// Wait for Enter key
		while (1) {
			char c = __io_getchar();
			if (c == '\r' || c == '\n') break;
		}

		printf("Ready???\r\n\r\n");
		delay_ms(2000);

		// Seed randomness using TIM2 count
		srand(TIM2->CNT);

		int score = 0;

		while (score < 10) {
			int a = (rand() % 20) + 1;  // 1–20
			int b = (rand() % 20) + 1;
			int result;

			// Transition message before Q6
			if (score == 5) {
				printf("\r\nThat was too easy. Let's spice it up...\r\n");
				delay_ms(2000);
				printf("Ready???\r\n\r\n");
			   delay_ms(2000);
			}

			if (score < 5) {
				result = a + b;
				printf("\r\n%d + %d\r\n", a, b);
			} else {
				result = a * b;
				printf("\r\n%d x %d\r\n", a, b);
			}

			delay_ms(4000);  // Wait 4 seconds

			uint8_t input_state = (GPIOA->IDR & (1 << 1)) ? 1 : 0;
			int is_even = (result % 2 == 0);

			if ((is_even && input_state == 1) || (!is_even && input_state == 0)) {
				printf("Correct! %d is %s.\r\n", result, is_even ? "even" : "odd");
				GPIOE->ODR |= (1 << 8);
				score++;
			} else {
				printf("Incorrect. %d is %s. You failed.\r\n", result, is_even ? "even" : "odd");
				GPIOE->ODR &= ~(1 << 8);
				break;
			}
		}


		if (score == 10) {
			printf("\r\n YOU WIN! The treasure is yours!\r\n");
			game_progress |= 0b0010;  // Set bit 1 after success
		}

		printf("\r\nPress ENTER to try again...\r\n");
		while (1) {
			char c = __io_getchar();
			if (c == '\r' || c == '\n') break;
		}
	}

}
