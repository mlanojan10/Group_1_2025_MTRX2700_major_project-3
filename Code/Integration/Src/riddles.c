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
    printf("\r\n🧠 Welcome to Riddle Island, ye clever seadog!\r\n");
    printf("Prepare yer noggin'—it’s time for brain teasers ‘n trickery!\r\n");
    printf("\r\nSolve this riddle to find the next step to the treasure.\r\n\n%s\r\n> ", current_riddle.riddle);
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
    prompted = 0;
    input_index = 0;
    memset(input_buffer, 0, sizeof(input_buffer));
    AskNewRiddle();

    while (riddle_step < 3) {
        if (SerialDataAvailable(&USART1_PORT)) {
            char c = SerialGetChar(&USART1_PORT);
            if (c == '\r' || c == '\n') {
                input_buffer[input_index] = '\0';
                ToLowerCase(input_buffer);

                if (riddle_step == 0) {
                    if (strcmp(input_buffer, current_riddle.answer) == 0) {
                        printf("\r\n\nCorrect! On to the next challenge...\r\n");
                        riddle_step = 1;
                        AskMathQuestion();
                    } else {
                        printf("\r\nWrong! Try again.\r\n%s\r\n> ", current_riddle.riddle);
                    }
                } else if (riddle_step == 1) {
                    if (atoi(input_buffer) == math_answer) {
                        printf("\r\nNicely done! Final challenge...\r\n");
                        riddle_step = 2;
                        AskCaesarChallenge();  // <-- Call your existing function here
                    } else {
                        printf("\r\nIncorrect. What is %d + %d?\r\n> ", math_1, math_2);
                    }
                } else if (riddle_step == 2) {
                    char expected_cipher[64];
                    CaesarCipher(expected_cipher, current_riddle.answer, math_answer);
                    ToLowerCase(expected_cipher);

                    if (strcmp(input_buffer, expected_cipher) == 0) {
                        printf("\r\nWell done! You've completed the riddle island!\r\n");
                        game_progress |= 0b0010;
                        return;
                    } else {
                        printf("\r\nNot quite! Try the Caesar cipher again with a shift of %d.\r\n> ", math_answer);
                    }
                }

                input_index = 0;
                memset(input_buffer, 0, sizeof(input_buffer));
            } else if (input_index < MAX_INPUT - 1) {
                SerialOutputChar(c, &USART1_PORT);  // Echo
                input_buffer[input_index++] = c;
            }
        }
    }
}
