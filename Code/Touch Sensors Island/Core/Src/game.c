#include "game.h"
#include "main.h"
#include "led_control.h"
#include "serial.h"
#include "user_interface.h"
#include "stm32f3xx_hal.h"
#include <stdbool.h>
#include <string.h>

#define PATTERN_LENGTH 7
#define TOUCH_TIMEOUT_MS 2000
#define TOUCH_THRESHOLD 2000  // Adjust based on testing
extern TSC_HandleTypeDef htsc;


extern SerialPort USART1_PORT;

static uint8_t pattern[PATTERN_LENGTH] = {0, 2, 4, 6, 1, 5, 7};  // example LED indices corresponding to sensors

uint8_t read_touch_sensor_debug(void)
{
    uint32_t tsc_value;
    char uart_msg[64];

    for (uint8_t group = 1; group < 9; group++)
    {
        if (group == 4) continue;  // Skip Group 4 (PA13/PA14 debugging pins)

        // Disable all groups first
        htsc.Instance->IOGCSR &= ~TSC_IOGCSR_G7E_Msk;

        // Enable only this group
        htsc.Instance->IOGCSR |= (1 << group);

        // Discharge IOs before acquisition
        HAL_TSC_IODischarge(&htsc, ENABLE);
        HAL_Delay(1);
        HAL_TSC_IODischarge(&htsc, DISABLE);

        // Start acquisition
        HAL_TSC_Start(&htsc);

        // Wait for acquisition complete
        while ((htsc.Instance->ISR & TSC_ISR_EOAF) == 0) {}

        // Clear acquisition flag
        htsc.Instance->ICR = TSC_ISR_EOAF;

        // Read raw sampling value
        tsc_value = HAL_TSC_GroupGetValue(&htsc, group - 1);

        // Print raw sampling value for debugging
        snprintf(uart_msg, sizeof(uart_msg), "Group %d raw: %lu\r\n", group, tsc_value);
        SerialOutputString((uint8_t *)uart_msg, &USART1_PORT);

        // Check threshold internally (optional)
        if (tsc_value < TOUCH_THRESHOLD)
        {
            // Optionally indicate touch detected
            snprintf(uart_msg, sizeof(uart_msg), "Group %d TOUCHED!\r\n", group);
            SerialOutputString((uint8_t *)uart_msg, &USART1_PORT);

            // Return the touched group (if you want to exit on first touch)
            return group;
        }
    }

    // Return 0 if no touch detected (optional)
    return 0;
}


void StartMinigame(TSC_HandleTypeDef *htsc) {
    LED_Init();
    UI_PrintWelcomeMessage();
    HAL_Delay(5000);

    char uart_msg[64];

    while (1) {
        LED_DisplayPattern(pattern, PATTERN_LENGTH, 400);

        uint8_t user_input[PATTERN_LENGTH];

        for (uint8_t i = 0; i < PATTERN_LENGTH; i++) {
            // Map pattern index to TSC group (1-based), skipping group 4
            uint8_t group = pattern[i] + 1;
            if (group == 4) group++;

            // Disable all groups first (clear enable mask)
            htsc->Instance->IOGCSR &= ~TSC_IOGCSR_G7E_Msk;
            htsc->Instance->IOGCSR |= (1 << group);

            // Discharge IOs before acquisition
            HAL_TSC_IODischarge(htsc, ENABLE);
            HAL_Delay(1);
            HAL_TSC_IODischarge(htsc, DISABLE);

            // Start acquisition
            HAL_TSC_Start(htsc);

            // Wait for acquisition complete
            while ((htsc->Instance->ISR & TSC_ISR_EOAF) == 0) {}

            // Clear acquisition flag
            htsc->Instance->ICR = TSC_ISR_EOAF;

            // Read raw sampling value
            uint32_t tsc_value = HAL_TSC_GroupGetValue(htsc, group - 1);

            // Print raw value for debugging
            snprintf(uart_msg, sizeof(uart_msg), "Group %d raw: %lu\r\n", group, tsc_value);
            SerialOutputString((uint8_t *)uart_msg, &USART1_PORT);

            // Check touch threshold internally
            bool touch_detected = (tsc_value < TOUCH_THRESHOLD);

            if (touch_detected) {
                snprintf(uart_msg, sizeof(uart_msg), "Group %d TOUCHED!\r\n", group);
                SerialOutputString((uint8_t *)uart_msg, &USART1_PORT);
            }

            // Store detected or no-touch (e.g., 0xFF if no touch)
            user_input[i] = touch_detected ? pattern[i] : 0xFF;

            HAL_Delay(2000); // Small delay between sensor readings
        }

        bool success = Game_CheckPattern(pattern, user_input, PATTERN_LENGTH);
        Game_ShowResult(success, &USART1_PORT);

        if (success) {
            break; // Exit the loop on correct input
        }

        HAL_Delay(1000); // Wait before next round
    }
}



void Game_ShowInstructions(SerialPort *serial) {
    SerialOutputString((uint8_t *)"Watch the LEDs closely.\r\n", serial);
    SerialOutputString((uint8_t *)"Repeat the pattern using the touch sensors.\r\n", serial);
    SerialOutputString((uint8_t *)"You have two seconds per sensor.\r\n\r\n", serial);
}

void Game_ShowResult(bool success, SerialPort *serial) {
    if (success) {
        SerialOutputString((uint8_t *)"\r\n✅ Correct pattern! You win!\r\n\r\n", serial);
    } else {
        SerialOutputString((uint8_t *)"\r\n❌ Wrong pattern. Restarting...\r\n\r\n", serial);
    }
}

bool Game_CheckPattern(uint8_t *correct_pattern, uint8_t *user_input, uint8_t length) {
    for (uint8_t i = 0; i < length; i++) {
        if (correct_pattern[i] != user_input[i]) {
            return false;
        }
    }
    return true;
}
