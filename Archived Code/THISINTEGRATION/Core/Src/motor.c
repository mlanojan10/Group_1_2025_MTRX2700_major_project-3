#include "stm32f303xc.h"
#include "motor.h"

#define SERVO_MIN_PULSE 500    // 0.5 ms
#define SERVO_MAX_PULSE 2500   // 2.5 ms
#define SERVO_PERIOD    20000  // 20 ms = 50 Hz

void motor_init(void) {
    // Enable GPIOA and TIM2 clocks
    RCC->AHBENR  |= RCC_AHBENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    // Set PA1 to AF1 (TIM2_CH2)
    GPIOA->MODER &= ~(3 << (1 * 2));
    GPIOA->MODER |=  (2 << (1 * 2));

    GPIOA->AFR[0] &= ~(0xF << (1 * 4));
    GPIOA->AFR[0] |=  (1 << (1 * 4));  // AF1 = TIM2_CH2

    // Prescaler: 8 MHz / (7 + 1) = 1 MHz (1 µs per tick)
    TIM2->PSC = 7;

    // ARR = 20000 ticks → 20 ms period → 50 Hz
    TIM2->ARR = SERVO_PERIOD - 1;

    // Set default pulse width to center (1.5ms)
    TIM2->CCR2 = 1500;

    // PWM mode 1 on CH2
    TIM2->CCMR1 &= ~TIM_CCMR1_OC2M;
    TIM2->CCMR1 |= (6 << TIM_CCMR1_OC2M_Pos); // PWM Mode 1
    TIM2->CCMR1 |= TIM_CCMR1_OC2PE;

    TIM2->CCER |= TIM_CCER_CC2E;

    TIM2->CR1 |= TIM_CR1_ARPE;
    TIM2->EGR  |= TIM_EGR_UG;   // 🔥 Force update
    TIM2->CR1 |= TIM_CR1_CEN;
}

void motor_set_angle(uint8_t angle) {
    if (angle > 180) angle = 180;

    uint32_t pulse = SERVO_MIN_PULSE +
        ((SERVO_MAX_PULSE - SERVO_MIN_PULSE) * angle) / 180;

    TIM2->CCR2 = pulse;
}

void motor_set_pulse(uint16_t pulse) {
    if (pulse < SERVO_MIN_PULSE) pulse = SERVO_MIN_PULSE;
    if (pulse > SERVO_MAX_PULSE) pulse = SERVO_MAX_PULSE;
    TIM2->CCR2 = pulse;
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
