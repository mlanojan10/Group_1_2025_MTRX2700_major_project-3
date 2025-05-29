#include "uart.h"
#include "led.h"
#include "timer.h"

int main(void) {
    gpio_init();
    uart_init();
    timer2_init();

    uart_send_string("\n--- 8-Minute Timer Started ---\n");
    uart_send_time(seconds_remaining);

    while (1) {
        // Timer ISR handles blinking and countdown
    }
}

