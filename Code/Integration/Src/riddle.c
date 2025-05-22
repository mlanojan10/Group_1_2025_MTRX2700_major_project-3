#include <riddles.h>
#include "uart.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "stm32f303xc.h"

typedef struct {
    const char *riddle;
    const char *answer;
} Riddle;

static Riddle riddles[] = {
    {"I follow you all the time and copy your every move, but you can’t touch me or catch me. What am I?", "shadow"},
    {"I speak without a mouth and hear without ears. I have nobody, but I come alive with the wind. What am I?", "echo"},
    {"I have keys but no locks. I have space but no room. You can enter but can’t go outside. What am I?", "keyboard"},
    {"The more of me you take, the more you leave behind. What am I?", "footsteps"},
    {"What gets wetter the more it dries?", "towel"}
};

#define NUM_RIDDLES (sizeof(riddles) / sizeof(Riddle))
#define MAX_INPUT 64

extern volatile uint8_t game_progress;

static Riddle current_riddle = {0};
static uint8_t riddle_step = 0; // 0: riddle, 1: math, 2: cipher
static int math_1 = 0;
static int math_2 = 0;
static int math_answer = 0;

static char input_buffer[MAX_INPUT];
static uint8_t input_index = 0;
static uint8_t prompted = 0;

int __io_getchar(void);


static void ToLowerCase(char *str) {
    while (*str) {
        *str = tolower((unsigned char)*str);
        str++;
    }
}

static void CaesarCipher(char *dest, const char *src, int shift) {
    while (*src) {
        if (isalpha(*src)) {
            char base = islower(*src) ? 'a' : 'A';
            *dest = (char)(((*src - base + shift) % 26) + base);
        } else {
            *dest = *src;
        }
        src++;
        dest++;
    }
    *dest = '\0';
}

static void AskNewRiddle(void) {
    int index = rand() % NUM_RIDDLES;
    current_riddle = riddles[index];
    riddle_step = 0;
    printf("\r\nAHOY - It's Riddle Time! Solve this riddle to find the next step to the treasure.\r\n%s\r\n> ", current_riddle.riddle);
    prompted = 1;
}

static void AskMathQuestion(void) {
    math_1 = rand() % 5;
    math_2 = rand() % 5;
    math_answer = math_1 + math_2;
    printf("\r\nNow answer this: What is %d + %d?\r\n> ", math_1, math_2);
    prompted = 1;
}

static void AskCaesarChallenge(void) {
    printf("\r\nFinal task! Enter the Caesar cipher of the riddle answer with a shift of %d.\r\n> ", math_answer);
    prompted = 1;
}

void riddle_game(void) {
    if (current_riddle.riddle == NULL && !prompted) {
        AskNewRiddle();
        return;
    }

    if (!SerialDataAvailable(&USART1_PORT)) {
        return;
    }

    char c = __io_getchar();

    if (c == '\r' || c == '\n') {
        input_buffer[input_index] = '\0';
        input_index = 0;

        ToLowerCase(input_buffer);

        switch (riddle_step) {
            case 0:
                if (strcmp(input_buffer, current_riddle.answer) == 0) {
                    printf("\r\nCorrect! Step 1 complete.");
                    riddle_step = 1;
                    AskMathQuestion();
                } else {
                    printf("\r\nIncorrect. Try again!\r\n> ");
                }
                break;

            case 1: {
                int answer = atoi(input_buffer);
                if (answer == math_answer) {
                    printf("\r\nWell done! Step 2 complete.");
                    riddle_step = 2;
                    AskCaesarChallenge();
                } else {
                    printf("\r\nThat's not right. Try again!\r\n> ");
                }
                break;
            }

            case 2: {
                char expected[64];
                CaesarCipher(expected, current_riddle.answer, math_answer);
                if (strcmp(input_buffer, expected) == 0) {
                    printf("\r\nIncredible! You completed all steps of the Riddle Challenge!\r\n");
                    game_progress |= 0b0010;
                    current_riddle.riddle = NULL;
                    prompted = 0;
                } else {
                    printf("\r\nHmm, that’s not correct. Try again!\r\n> ");
                }
                break;
            }

            default:
                printf("\r\nUnexpected step. Restarting riddle.\r\n");
                current_riddle.riddle = NULL;
                prompted = 0;
                break;
        }
    } else if (input_index < MAX_INPUT - 1) {
        input_buffer[input_index++] = c;
        SerialOutputChar(c, &USART1_PORT); // Echo
    }
}
