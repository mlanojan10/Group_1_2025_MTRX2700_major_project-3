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
#include "lidar.h"

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



void Initialise_All(void) {
    InitialisePE11AsInput();  // Input 1 (was PE7, originally PC4 / PA11)
    InitialisePA2AsInput();
    InitialisePA3AsInput();
    InitialisePE9AsInput();   // Input 2 (was PC5 / PA12)
    InitialisePA5AsInput(); //Potentiometer
    InitialisePE7AsInput(); //Button riddle
    SerialInitialise(BAUD_115200, &USART1_PORT, NULL);
}

void SystemClock_HSI_8MHz(void) {
    // Enable the internal high-speed oscillator (HSI)
    RCC->CR |= RCC_CR_HSION;
    while ((RCC->CR & RCC_CR_HSIRDY) == 0);  // Wait until HSI is ready

    // Select HSI as system clock source
    RCC->CFGR &= ~(RCC_CFGR_SW);            // Clear SW bits
    RCC->CFGR |= RCC_CFGR_SW_HSI;           // Set HSI as system clock

    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI);  // Wait until HSI is used

    // Disable PLL (optional if used in lidar_game)
    RCC->CR &= ~RCC_CR_PLLON;
    while ((RCC->CR & RCC_CR_PLLRDY) != 0);  // Wait until PLL is fully stopped

    // Optional: Reset APB/AHB prescalers
    RCC->CFGR &= ~(RCC_CFGR_HPRE | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2);  // Set to /1
}




// === MAIN LOOP ===

int main(void) {
    Initialise_All();

    uint8_t prev_pe11 = 0;
    uint8_t prev_pa2  = 0;
    uint8_t prev_pa3  = 0;
    uint8_t prev_pe9  = 0;

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

        // Check for new press (rising edge)
        if (pe11 && !prev_pe11) {
            if (game_progress == 0b0000) {
            	printf("\r\nWelcome to the pirate ship!\r\n");
            	printf("\r\nSteer your ship to avoid the obstacles!\r\n");
            	printf("\r\nGOOOOO!!!!!\r\n");
            	lidar_game();
            	SystemClock_HSI_8MHz();
            	Initialise_All();

                game_progress = 0b00001;
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
                //led_game();
                game_progress = 0b0100;
            } else {
                printf("\r\nYou cannot do this island!\r\n");
            }
        } else if (pe9 && !prev_pe9) {
            if (game_progress == 0b0100) {
            	// === Force PA5 to be digital input again ===
            	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
            	GPIOA->MODER &= ~(3U << (5 * 2));   // Input mode
            	GPIOA->PUPDR &= ~(3U << (5 * 2));   // No pull-up/down
            	GPIOA->AFR[0] &= ~(0xF << (5 * 4)); // Clear alternate function in case it was set

                potentiometer_game();
                printf("\r\nYou have opened the treasure! HOORAYYYYY!!!!!!\r\n");
                pirate_sound();
                motor_init();
                motor_start_forward();
            	delay_ms(2000);
                motor_stop();
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
