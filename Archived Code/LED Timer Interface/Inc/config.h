#ifndef CONFIG_H
#define CONFIG_H

#define MAX_SECONDS         480
#define LED_COUNT           8
#define SECONDS_PER_LED     (MAX_SECONDS / LED_COUNT)

#define SYSTEM_CLOCK_HZ     8000000U
#define UART_BAUDRATE       115200
#define UART_TX_PIN         9
#define UART_TX_AF          7

#define TIMER_PRESCALER     (8000 - 1)
#define TIMER_AUTO_RELOAD   (1000 - 1)

extern volatile uint8_t blink_state;

#endif
