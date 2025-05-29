#include "user_interface.h"
#include "serial.h"
#include <stdio.h>


extern SerialPort USART1_PORT;

void UI_PrintWelcomeMessage(void) {
    SerialOutputString((uint8_t*)"Watch the LEDs closely and touch the sensors in the same pattern.\r\n", &USART1_PORT);
    SerialOutputString((uint8_t*)"You have two seconds per sensor.\r\n", &USART1_PORT);
}

void UI_PrintFailureMessage(void) {
    SerialOutputString((uint8_t*)"Incorrect sequence. Restarting game...\r\n", &USART1_PORT);
}

void UI_PrintSuccessMessage(void) {
    SerialOutputString((uint8_t*)"Correct! Well done!\r\n", &USART1_PORT);
}

void UI_PrintPrompt(uint8_t index) {
    char buffer[50];
    sprintf(buffer, "Touch sensor #%d now...\r\n", index + 1);
    SerialOutputString((uint8_t*)buffer, &USART1_PORT);
}
