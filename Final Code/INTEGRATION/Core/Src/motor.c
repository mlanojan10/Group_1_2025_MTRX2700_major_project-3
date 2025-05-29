#include "stm32f303xc.h"
#include "motor.h"

#define SERVO_MIN_PULSE 500    // 0.5 ms
#define SERVO_MAX_PULSE 2500   // 2.5 ms
#define SERVO_PERIOD    20000  // 20 ms = 50 Hz

void motor_init(void) {
    // Enable GPIOD and TIM2 clocks
    RCC->AHBENR  |= RCC_AHBENR_GPIODEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    // Set PD6 to Alternate Function mode (AF2 = TIM2_CH4)
    GPIOD->MODER &= ~(3 << (6 * 2));
    GPIOD->MODER |=  (2 << (6 * 2));

    GPIOD->AFR[0] &= ~(0xF << (6 * 4));
    GPIOD->AFR[0] |=  (2 << (6 * 4));  // AF2 = TIM2_CH4

    // Prescaler: 8 MHz / (7 + 1) = 1 MHz (1 µs per tick)
    TIM2->PSC = 7;

    // ARR = 20000 ticks → 20 ms period → 50 Hz
    TIM2->ARR = SERVO_PERIOD - 1;

    // Set default pulse width to center (1.5ms)
    TIM2->CCR4 = 1500;

    // PWM mode 1 on CH4
    TIM2->CCMR2 &= ~TIM_CCMR2_OC4M;
    TIM2->CCMR2 |= (6 << TIM_CCMR2_OC4M_Pos); // PWM Mode 1
    TIM2->CCMR2 |= TIM_CCMR2_OC4PE;

    TIM2->CCER |= TIM_CCER_CC4E;

    TIM2->CR1 |= TIM_CR1_ARPE;
    TIM2->EGR  |= TIM_EGR_UG;   // 🔥 Force update
    TIM2->CR1 |= TIM_CR1_CEN;
}


void motor_set_angle(uint8_t angle) {
    if (angle > 180) angle = 180;

    uint32_t pulse = SERVO_MIN_PULSE +
        ((SERVO_MAX_PULSE - SERVO_MIN_PULSE) * angle) / 180;

    TIM2->CCR4 = pulse;
}

void motor_set_pulse(uint16_t pulse) {
    if (pulse < SERVO_MIN_PULSE) pulse = SERVO_MIN_PULSE;
    if (pulse > SERVO_MAX_PULSE) pulse = SERVO_MAX_PULSE;
    TIM2->CCR4 = pulse;
}


void motor_open(void) {
    motor_set_angle(90);
}

void motor_close(void) {
    motor_set_angle(0);
}
void motor_start_forward(void) {
    motor_set_pulse(1800);  // Adjust to tune speed/direction
}

void motor_start_reverse(void) {
    motor_set_pulse(1300);  // Adjust to tune speed/direction
}

void motor_stop(void) {
    motor_set_pulse(1500);  // Neutral signal
}
