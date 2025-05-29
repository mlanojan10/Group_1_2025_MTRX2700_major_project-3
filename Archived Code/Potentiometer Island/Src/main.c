#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "uart.h"
#include "stm32f303xc.h"

int __io_putchar(int ch) {
    SerialOutputChar((uint8_t)ch, &USART1_PORT);
    return ch;
}

int __io_getchar(void) {
    return SerialGetChar(&USART1_PORT);
}

void initialise_PA1_as_input(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER &= ~(3 << (1 * 2));
    GPIOA->PUPDR &= ~(3 << (1 * 2));
}

void initialise_PE8_as_output(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOEEN;
    GPIOE->MODER &= ~(3 << (8 * 2));
    GPIOE->MODER |= (1 << (8 * 2));
}

void delay_ms(uint32_t ms) {
    for (volatile uint32_t i = 0; i < ms * 615; i++);
}

int main(void) {
    initialise_PA1_as_input();
    initialise_PE8_as_output();
    SerialInitialise(BAUD_115200, &USART1_PORT, NULL);

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
        }

        printf("\r\nPress ENTER to try again...\r\n");
        while (1) {
            char c = __io_getchar();
            if (c == '\r' || c == '\n') break;
        }
    }
}
