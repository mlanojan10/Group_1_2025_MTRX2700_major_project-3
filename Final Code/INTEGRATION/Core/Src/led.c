#include "stm32f303xc.h"
#include "led.h"
#include "uart.h"
#include "game_progress.h"



void GPIO_Signal_Init(void) {
    // 1. Enable GPIOA clock
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

    // 2. Configure PA7 as output
    GPIOA->MODER &= ~(3 << (7 * 2));
    GPIOA->MODER |=  (1 << (7 * 2)); // Output mode
    GPIOA->OTYPER &= ~(1 << 7);      // Push-pull
    GPIOA->OSPEEDR |= (3 << (7 * 2));// High speed
    GPIOA->PUPDR &= ~(3 << (7 * 2)); // No pull-up/down

   // Configure PA6 as input
    GPIOA->MODER &= ~(3 << (6 * 2)); // Input mode

    // Enable internal pull-down on PA6
    GPIOA->PUPDR &= ~(3 << (6 * 2)); // Clear
    GPIOA->PUPDR |=  (2 << (6 * 2)); // Pull-down (10)
}


void led_game(void) {
    GPIO_Signal_Init();

    printf("\r\n Welcome to the Blinkin’ Shores, matey!\r\n");
    printf("These cursed lights guard the next piece o’ the treasure map.\r\n");
    printf("Watch the lights closely and remember the pattern,  touch the sensors on the wheel in the same pattern\r\n");
    printf("A tip from one pirate to another matey - ships are hard to steer so touch the first sensor before the led pattern ends as setting sail is hard and make sure you're touching at the blue flash!\r\n");
    printf("Press Enter to begin yer lightin’ trial...\r\n");

        // Wait for user to press Enter
    // Wait for Enter key
	while (1) {
		char c = __io_getchar();
		if (c == '\r' || c == '\n') break;
	}
    // Output HIGH on PA7 to indicate module start
    GPIOA->ODR |= (1 << 7);

    printf("\r\nWaiting for signal on PA6 from other board...\r\n");//

	// Wait indefinitely until PA6 goes HIGH
	while (!(GPIOA->IDR & (1 << 6))) {
		// Constantly sampling PA6, do nothing until it's HIGH
	}

	// Input received
	printf("\r\nYo-ho-ho! The LED trial be complete! Onwards to the next island if you dare \r\n");
	//game_progress |= 0b0100;
}
