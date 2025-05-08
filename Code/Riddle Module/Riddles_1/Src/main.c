#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <riddles.h>
#include "stm32f303xc.h"

int __io_putchar(int ch) {
    SerialOutputChar((uint8_t)ch, &USART1_PORT);
    return ch;
}

// This struct can be used to handle modes of the game, so user can play the game more than once and have different questions
typedef struct {
	const char *riddle;
    const char *answer;
} Riddle;

Riddle riddles[] = {
    {
        "I follow you all the time and copy your every move, but you can’t touch me or catch me. What am I?",
        "shadow"
    },
    {
        "I speak without a mouth and hear without ears. I have nobody, but I come alive with the wind. What am I?",
        "echo"
    },
    {
        "I have keys but no locks. I have space but no room. You can enter but can’t go outside. What am I?",
        "keyboard"
    },
    {
        "The more of me you take, the more you leave behind. What am I?",
        "footsteps"
    },
    {
        "What gets wetter the more it dries?",
        "towel"
    }
};

#define NUM_RIDDLES (sizeof(riddles) / sizeof(Riddle))


static Riddle current_riddle;
static uint8_t game_progress = 0b0001;   //change this to test if the game progress section is working


//--------------------------------RIDDLE MANAGEMENT-------------------------------
// Convert a string to lowercase (in-place)
void ToLowerCase(char *str) {
    while (*str) {
        *str = tolower((unsigned char)*str);
        str++;
    }
}

// Seed RNG from SysTick timer
void SeedRandomGenerator(void) {
    srand(SysTick->VAL);
}

// Select and print a new random riddle
void AskNewRiddle(void) {
    int index = rand() % NUM_RIDDLES;
    current_riddle = riddles[index];

    printf("\r\n AHOY - It's Riddle Time! Solve this riddle to find the next step to the treasure. \r\n%s\r\n> ", current_riddle.riddle);
}

// Handle received input from user
void OnLineReceived(char *string, uint32_t length) {
    static char user_input[64];
    strncpy(user_input, string, sizeof(user_input) - 1);
    user_input[sizeof(user_input) - 1] = '\0';
    ToLowerCase(user_input);

    if ((game_progress & 0b0001) && !(game_progress & 0b0010)) {
        if (strcmp(user_input, current_riddle.answer) == 0) {
            printf("\r\n Correct! You solved it!\r\n");
            game_progress |= 0b0010;  // Mark minigame 2 (riddle) as completed
            printf("\r\nYou've unlocked the next challenge!\r\n");
        } else {
            printf("\r\n Incorrect. Try again!\r\n> ");
        }
    } else if (!(game_progress & 0b0001)) {
        printf("\r\nYou must complete Minigame 1 before attempting this!\r\n");
    } else if (game_progress & 0b0010) {
        printf("\r\nYou've already completed this riddle challenge! Proceed to the next game.\r\n");
    }
}


int main(void) {
    SerialInitialise(BAUD_115200, &USART1_PORT, NULL);
    SerialSetReceiveCallback(&USART1_PORT, OnLineReceived);

    //SeedRandomGenerator();

    if (game_progress == 0b0001) {
        AskNewRiddle();
    } else if (game_progress == 0b0000) {
        printf("\r\nWelcome Adventurer! Complete the first challenge to unlock the riddle game.\r\n");
    } else if (game_progress & 0b0010) {
        printf("\r\nRiddle challenge already completed. Proceed to the next stage!\r\n");
    }

    while (1) {
        // UART handled via interrupts
    }
}

