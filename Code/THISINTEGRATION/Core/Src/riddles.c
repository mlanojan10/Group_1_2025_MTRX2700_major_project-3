#include <riddles.h>
#include "uart.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "stm32f303xc.h"
#include "game_progress.h"

typedef struct {
    const char *riddle;
    const char *answer;
    const char *hint1;
    const char *hint2;
} Riddle;

static Riddle riddles[] = {
    {
        "I follow you all the time and copy your every move, but you can’t touch me or catch me. What am I?",
        "shadow",
        "You see it when there's light.",
        "It mimics your actions."
    },
    {
        "I speak without a mouth and hear without ears. I have nobody, but I come alive with the wind. What am I?",
        "echo",
        "It bounces back.",
        "It’s commonly heard in caves."
    },
    {
        "I have keys but no locks. I have space but no room. You can enter but can’t go outside. What am I?",
        "keyboard",
        "Let's give you a few letters: _ _ y _ _ a _ d",
        "It helps you type."
    },
    {
        "The more of me you take, the more you leave behind. What am I?",
        "footsteps",
        "You leave them behind.",
        "They make a sound."
    },
    {
        "What gets wetter the more it dries?",
        "towel",
        "You use it after a shower.",
        "It soaks water."
    }
};

#define NUM_RIDDLES (sizeof(riddles) / sizeof(Riddle))
#define MAX_INPUT 64



static Riddle current_riddle = {0};
static uint8_t riddle_step = 0; // 0: riddle, 1: math, 2: cipher
static int math_1 = 0;
static int math_2 = 0;
static int math_answer = 0;

static char input_buffer[MAX_INPUT];
static uint8_t input_index = 0;
static uint8_t prompted = 0;
uint32_t SystemCoreClock = 72000000;

int __io_getchar(void);


//---------------------Button Things--------------------------------
volatile uint32_t millis = 0;

void SysTick_Handler(void) {
    millis++;
}

void SysTick_Init(void) {
    SysTick_Config(SystemCoreClock / 1000);  // 1 ms tick
}


static void ToLowerCase(char *str) {
    while (*str) {
        *str = tolower((unsigned char)*str);
        str++;
    }
}

//----------------- Game Flow Submodules----------------------------------
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
    static int last_index = -1;
    int index;

    do {
        index = rand() % NUM_RIDDLES;
    } while (index == last_index);  // Ensure it’s not the same as the last one

    last_index = index;
    current_riddle = riddles[index];
    riddle_step = 0;

    printf("\r\nSolve this riddle:\r\n\n%s\r\n> ", current_riddle.riddle);
    prompted = 1;
}

static void AskMathQuestion(void) {
    do {
        math_1 = rand() % 5;
        math_2 = rand() % 5;
        math_answer = math_1 + math_2;
    } while (math_answer == 0);  // Avoid 0 + 0

    printf("\r\nNow answer this: What is %d + %d?\r\n> ", math_1, math_2);
    prompted = 1;
}

static void AskCaesarChallenge(void) {
    printf("\r\nFinal task! Enter the Caesar cipher of the riddle answer with a shift of %d.\r\n> ", math_answer);
    prompted = 1;
}

//---------------------RIDDLE GAME FLOW----------------------------------
void riddle_game(void) {
    SysTick_Init();  // Initialize the millisecond timer

    // Seed RNG based on system time
    srand(millis);
    prompted = 0;
    input_index = 0;
    memset(input_buffer, 0, sizeof(input_buffer));

    printf("\r\n🧠 Welcome to Riddle Island, ye clever seadog!\r\n");
    printf("Prepare yer noggin'—it’s time for brain teasers ‘n trickery!\r\n");

    AskNewRiddle();

    uint32_t start_time = millis;
    uint32_t last_hint_time = 0;
    uint8_t hint_count = 0;
    uint8_t hint_button_used = 0;
    uint8_t button_hint_shown = 0;
    uint8_t early_solve = 0;

    //const char *hint1 = "Let's give you a few letters: _ _ y _ _ a _ d ";
    //const char *hint2 = "It helps you type.";

    while (riddle_step < 3) {
        uint32_t elapsed = millis - start_time;

        // === After 45s, reveal that there's a hidden button ===
        if (!button_hint_shown && elapsed >= 1000 && riddle_step == 0 && !hint_button_used) {
            printf("\r\nThere's a button hidden on this island. Find it and gain a hint!\r\n\n> ");
            button_hint_shown = 1;
        }

        // === Hint button check (PE7) ===
        if (riddle_step == 0 && (GPIOE->IDR & GPIO_IDR_7) != 0) {
            hint_button_used = 1;

            if (hint_count == 0) {
                if (elapsed >= 1000 || early_solve) {
                    printf("\r\nHint: %s\r\n> ", current_riddle.hint1);
                    last_hint_time = millis;
                    hint_count++;
                } else {
                    printf("\r\nYou have to wait 45 seconds before getting a hint.\r\n> ");
                }
            } else if (hint_count == 1 && (millis - last_hint_time) > 1000) {
                printf("\r\nLast hint: %s\r\n> ", current_riddle.hint2);
                hint_count++;
            }
            for (volatile int i = 0; i < 100000; ++i);  // Debounce
        }


        // === User input handling ===
        if (SerialDataAvailable(&USART1_PORT)) {
            char c = SerialGetChar(&USART1_PORT);
            if (c == '\r' || c == '\n') {
                input_buffer[input_index] = '\0';
                ToLowerCase(input_buffer);

                if (riddle_step == 0) {
                    if (strcmp(input_buffer, current_riddle.answer) == 0) {
                        printf("\r\n\nCorrect!\r\n");

                        // Track if it was solved within 45s
                        if (elapsed < 1000) {
                        	printf("\r\nWow, you're quick. So heres another one!\r\n");
                            early_solve = 1;
                            srand(millis);
                            AskNewRiddle();
                            riddle_step = 0;
                        } else{
                        printf("\r\nGet Ready for the next challenge...\n\r\n");
                        // Reset for new riddle stage
                        riddle_step = 1;
                        AskMathQuestion();
                        }
                    } else {
                        printf("\r\n\nWrong! Try again.\r\n%s\r\n> ", current_riddle.riddle);
                    }
                } else if (riddle_step == 1) {
                    if (atoi(input_buffer) == math_answer) {
                        printf("\r\n\nNicely done! Final challenge...\r\n");
                        riddle_step = 2;
                        AskCaesarChallenge();
                    } else {
                        printf("\r\n\nIncorrect. What is %d + %d?\r\n> ", math_1, math_2);
                    }
                } else if (riddle_step == 2) {
                    char expected_cipher[64];
                    CaesarCipher(expected_cipher, current_riddle.answer, math_answer);
                    ToLowerCase(expected_cipher);

                    if (strcmp(input_buffer, expected_cipher) == 0) {
                        printf("\r\n\nWell done! You've completed the riddle island!\r\n");
                        game_progress |= 0b0010;
                        return;
                    } else {
                        printf("\r\n\nNot quite! Try the Caesar cipher again with a shift of %d.\r\n> ", math_answer);
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
