#include <riddles.h>
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

static Riddle current_riddle;
static char rx_buffers[2][64];
static uint8_t active_rx_buf = 0;
static uint32_t rx_index = 0;
static const char *tx_buffer = NULL;
static uint32_t tx_index = 0;
//static uint8_t game_progress = 0b0001;
extern volatile uint8_t game_progress;


// --- NEW internal state tracking ---
static uint8_t riddle_step = 0; // 0: riddle, 1: math, 2: cipher
static int math_1 = 0;
static int math_2 = 0;
static int math_answer = 0;

// ------------------- Utility Functions -------------------

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

// ------------------- Game Flow -------------------

void AskNewRiddle(void) {
    int index = rand() % NUM_RIDDLES;
    current_riddle = riddles[index];
    riddle_step = 0;  // Reset internal riddle state
    printf("\r\nAHOY - It's Riddle Time! Solve this riddle to find the next step to the treasure.\r\n%s\r\n> ", current_riddle.riddle);
}

static void AskMathQuestion(void) {

	math_1 = (rand() % 5);
    math_2 = (rand() % 5);
	math_answer = math_1 + math_2;
    printf("\r\nNow answer this: What is %d + %d?\r\n> ", math_1, math_2);
}

static void AskCaesarChallenge(void) {
    printf("\r\nFinal task! Enter the Caesar cipher of the riddle answer with a shift of %d.\r\n> ", math_answer);
}

void OnLineReceived(char *string, uint32_t length) {
    static char user_input[64];
    strncpy(user_input, string, sizeof(user_input) - 1);
    user_input[sizeof(user_input) - 1] = '\0';
    ToLowerCase(user_input);

    /*if (!(game_progress & 0b0001)) {
        printf("\r\nYou must complete Minigame 1 before attempting this!\r\n");
        return;
    }

    if (game_progress & 0b0010) {
        printf("\r\nYou've already completed this riddle challenge! Proceed to the next game.\r\n");
        return;
    }*/

    switch (riddle_step) {
        case 0:  // Solve riddle
            if (strcmp(user_input, current_riddle.answer) == 0) {
                printf("\r\nCorrect! Step 1 complete.\r\n");
                riddle_step = 1;
                AskMathQuestion();
            } else {
                printf("\r\nIncorrect. Try again!\r\n> ");
            }
            break;

        case 1: { // Solve math
            int answer = atoi(user_input);
            if (answer == math_answer) {
                printf("\r\nWell done! Step 2 complete.\r\n");
                riddle_step = 2;
                AskCaesarChallenge();
            } else {
                printf("\r\nThat's not right. Try again!\r\n> ");
            }
            break;
        }

        case 2: { // Caesar cipher
            char expected[64];
            CaesarCipher(expected, current_riddle.answer, math_answer);

            if (strcmp(user_input, expected) == 0) {
                printf("\r\nIncredible! You completed all steps of the Riddle Challenge!\r\n");
                game_progress |= 0b0010;
            } else {
                printf("\r\nHmm, that’s not correct. Try again!\r\n> ");
            }
            break;
        }

        default:
            printf("\r\nUnexpected step. Restarting riddle.\r\n");
            AskNewRiddle();
            break;
    }
}

/*// ----------------- Game Progress Accessors -----------------

uint8_t isMinigame1Completed(void) {
    return (game_progress & 0b0001) != 0;
}

uint8_t isMinigame2Completed(void) {
    return (game_progress & 0b0010) != 0;
}*/

//--------------------------USART CONFIGURATION------------------
// Structure defining configuration for a serial port
struct _SerialPort {
    USART_TypeDef *UART;
    GPIO_TypeDef *GPIO;
    volatile uint32_t MaskAPB2ENR;
    volatile uint32_t MaskAPB1ENR;
    volatile uint32_t MaskAHBENR;
    volatile uint32_t SerialPinModeValue;
    volatile uint32_t SerialPinSpeedValue;
    volatile uint32_t SerialPinAlternatePinValueLow;
    volatile uint32_t SerialPinAlternatePinValueHigh;
    void (*completion_function)(uint32_t);
    void (*receive_callback)(char *, uint32_t);
};

SerialPort USART1_PORT = {
    USART1, GPIOC,
    RCC_APB2ENR_USART1EN, 0x00, RCC_AHBENR_GPIOCEN,
    0xA00, 0xF00,
    0x770000, 0x00,
    0x00, 0x00
};

/*// Double buffer for received input lines
static char rx_buffers[2][64];
static uint8_t active_rx_buf = 0;   // Active buffer index
static uint32_t rx_index = 0;       // Index into current buffer

// Pointer to currently transmitting string
static const char *tx_buffer = NULL;
static uint32_t tx_index = 0;*/

// Initializes UART hardware and GPIO settings for communication
void SerialInitialise(uint32_t baudRate, SerialPort *serial_port, void (*completion_function)(uint32_t)) {
    serial_port->completion_function = completion_function;

    // Enable required clocks
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    RCC->AHBENR |= serial_port->MaskAHBENR;

    // Configure GPIO alternate function mode and speed
    serial_port->GPIO->MODER = serial_port->SerialPinModeValue;
    serial_port->GPIO->OSPEEDR = serial_port->SerialPinSpeedValue;
    serial_port->GPIO->AFR[0] |= serial_port->SerialPinAlternatePinValueLow;
    serial_port->GPIO->AFR[1] |= serial_port->SerialPinAlternatePinValueHigh;

    // Enable USART peripheral
    RCC->APB1ENR |= serial_port->MaskAPB1ENR;
    RCC->APB2ENR |= serial_port->MaskAPB2ENR;

    // Set baud rate (hardcoded value for 115200 @ 8 MHz)
    uint16_t *baud_rate_config = (uint16_t*)&serial_port->UART->BRR;
    *baud_rate_config = 0x46;

    // Enable TX, RX, UART and RX interrupt
    serial_port->UART->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
    serial_port->UART->CR1 |= USART_CR1_RXNEIE;

    // Enable USART1 interrupt in NVIC
    NVIC_EnableIRQ(USART1_IRQn);
}

// Transmit a single character (blocking)
void SerialOutputChar(uint8_t data, SerialPort *serial_port) {
    while ((serial_port->UART->ISR & USART_ISR_TXE) == 0);  // Wait for TX to be ready
    serial_port->UART->TDR = data;                          // Write to transmit register
}

// Transmit a null-terminated string using SerialOutputChar
void SerialOutputString(uint8_t *pt, SerialPort *serial_port) {
    while (*pt) {
        SerialOutputChar(*pt++, serial_port);
    }
}

// Blocking read of a single character from UART
uint8_t SerialGetChar(SerialPort *serial_port) {
    while ((serial_port->UART->ISR & USART_ISR_RXNE) == 0);  // Wait for data
    return serial_port->UART->RDR;                           // Return received char
}

// Sets the function to be called when a full line is received via UART
void SerialSetReceiveCallback(SerialPort *serial_port, void (*callback)(char *, uint32_t)) {
    serial_port->receive_callback = callback;
}

// Begins transmission of a string (interrupt-based, non-blocking)
void SerialStartTransmission(const char *str) {
    tx_buffer = str;
    tx_index = 0;
    USART1->CR1 |= USART_CR1_TXEIE;  // Enable TX interrupt
}

// UART interrupt handler (for RX and TX)
void USART1_EXTI25_IRQHandler(void) {
    // ---------- Receive ----------
    if (USART1->ISR & USART_ISR_RXNE) {
        char c = USART1->RDR;  // Read received character
        SerialOutputChar(c, &USART1_PORT);  // Echo it back

        // If Enter pressed, complete the string
        if (c == '\r') return;

        if (c == '\n') {
            rx_buffers[active_rx_buf][rx_index] = '\0';  // Null terminate

            if (USART1_PORT.receive_callback)
                USART1_PORT.receive_callback(rx_buffers[active_rx_buf], rx_index);

            active_rx_buf ^= 1;  // Switch buffer
            rx_index = 0;        // Reset index
        } else if (rx_index < sizeof(rx_buffers[0]) - 1) {
            rx_buffers[active_rx_buf][rx_index++] = c;
        }
    }

    // ---------- Transmit ----------
    if ((USART1->CR1 & USART_CR1_TXEIE) && (USART1->ISR & USART_ISR_TXE)) {
        if (tx_buffer && tx_buffer[tx_index]) {
            USART1->TDR = tx_buffer[tx_index++];  // Send next character
        } else {
            USART1->CR1 &= ~USART_CR1_TXEIE;  // Disable TX interrupt
            tx_buffer = NULL;
            tx_index = 0;
        }
    }
}
