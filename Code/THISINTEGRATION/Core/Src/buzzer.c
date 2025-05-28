#include "stm32f303xc.h"
#include "buzzer.h"

void buzzer_init(void) {
    // Enable clocks for GPIOA and TIM2
    RCC->AHBENR  |= RCC_AHBENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    // Set PA0 (TIM2_CH1) to alternate function mode
    GPIOA->MODER &= ~(3 << (0 * 2));
    GPIOA->MODER |=  (2 << (0 * 2));  // AF mode

    GPIOA->AFR[0] &= ~(0xF << (0 * 4));
    GPIOA->AFR[0] |=  (1 << (0 * 4)); // AF1 for TIM2_CH1

    // Configure TIM2 for PWM
    TIM2->PSC = 71;           // Prescaler (72 MHz / 72 = 1 MHz)
    TIM2->ARR = 499;          // Auto-reload: 1 MHz / 500 = 2 kHz
    TIM2->CCR1 = 250;         // 50% duty cycle

    TIM2->CCMR1 &= ~TIM_CCMR1_OC1M;
    TIM2->CCMR1 |= (6 << TIM_CCMR1_OC1M_Pos); // PWM mode 1
    TIM2->CCMR1 |= TIM_CCMR1_OC1PE;           // Enable preload

    TIM2->CCER |= TIM_CCER_CC1E;              // Enable CH1 output

    TIM2->CR1 |= TIM_CR1_ARPE;                // Auto-reload preload enable
    TIM2->EGR |= TIM_EGR_UG;                  // Force update
}

void buzzer_on(void) {
    TIM2->CR1 |= TIM_CR1_CEN; // Start timer
}

void buzzer_off(void) {
    TIM2->CR1 &= ~TIM_CR1_CEN; // Stop timer
}
void buzzer_set_frequency(uint32_t freq_hz) {
    uint32_t timer_clk = 72000000; // 72 MHz APB1 timer clock
    uint32_t prescaler = 71;
    uint32_t period = (timer_clk / (prescaler + 1)) / freq_hz - 1;

    TIM2->PSC = prescaler;
    TIM2->ARR = period;
    TIM2->CCR1 = period / 2; // 50% duty
    TIM2->EGR |= TIM_EGR_UG; // Force update
}

void siren_sound(void) {
	for (uint32_t f = 3000; f <= 10000; f += 100) {
		buzzer_set_frequency(f);
		for (volatile int i = 0; i < 10000; i++); // crude delay (~100 ms)
	}
	for (uint32_t f = 10000; f >= 3000; f -= 100) {
		buzzer_set_frequency(f);
		for (volatile int i = 0; i < 10000; i++); // crude delay (~100 ms)
	}

}

void fail_sound(void) {
	buzzer_set_frequency(5000);
	for (volatile int i = 0; i < 100000; i++);
	buzzer_set_frequency(4000);
	for (volatile int i = 0; i < 100000; i++);
	buzzer_set_frequency(3000);
	for (volatile int i = 0; i < 1000000; i++);
}

void pirate_sound(void) {
    buzzer_on();


    buzzer_set_frequency(2636); // E7
    for (volatile int i = 0; i < 80000; i++); // Quaver

    buzzer_set_frequency(3136); // G7
    for (volatile int i = 0; i < 80000; i++); // Quaver

    buzzer_set_frequency(3520); // A7
    for (volatile int i = 0; i < 80000; i++); // Crotchet
    buzzer_off();
    for (volatile int i = 0; i < 80000; i++);
    buzzer_on();

    buzzer_set_frequency(3520); // A7
    for (volatile int i = 0; i < 80000; i++); // Crotchet
	buzzer_off();
	for (volatile int i = 0; i < 80000; i++);
	buzzer_on();


    buzzer_set_frequency(3520); // A7
    for (volatile int i = 0; i < 80000; i++); // Quaver

    buzzer_set_frequency(3951); // B7
    for (volatile int i = 0; i < 80000; i++); // Quaver

    buzzer_set_frequency(4186); // C8
    for (volatile int i = 0; i < 80000; i++); // Crotchet
	buzzer_off();
	for (volatile int i = 0; i < 80000; i++);
	buzzer_on();

    buzzer_set_frequency(4186); // C8
    for (volatile int i = 0; i < 80000; i++); // Crotchet
	buzzer_off();
	for (volatile int i = 0; i < 80000; i++);
	buzzer_on();

    buzzer_set_frequency(4186); // C8
    for (volatile int i = 0; i < 80000; i++); // Quaver

    buzzer_set_frequency(4698); // D8
    for (volatile int i = 0; i < 80000; i++); // Quaver

    buzzer_set_frequency(3951); // B7
    for (volatile int i = 0; i < 80000; i++); // Crotchet
	buzzer_off();
	for (volatile int i = 0; i < 80000; i++);
	buzzer_on();

    buzzer_set_frequency(3951); // B7
    for (volatile int i = 0; i < 80000; i++); // Crotchet
	buzzer_off();
	for (volatile int i = 0; i < 80000; i++);
	buzzer_on();

    buzzer_set_frequency(3520); // A7
    for (volatile int i = 0; i < 80000; i++); // Quaver

    buzzer_set_frequency(3136); // G7
    for (volatile int i = 0; i < 80000; i++); // Quaver

    buzzer_set_frequency(3520); // A7
    for (volatile int i = 0; i < 160000; i++); // Crotchet
    buzzer_off();
    for (volatile int i = 0; i < 480000; i++); // Crotchet




    buzzer_on();


    buzzer_set_frequency(2636); // E7
    for (volatile int i = 0; i < 80000; i++); // Quaver

    buzzer_set_frequency(3136); // G7
    for (volatile int i = 0; i < 80000; i++); // Quaver

    buzzer_set_frequency(3520); // A7
    for (volatile int i = 0; i < 80000; i++); // Crotchet
    buzzer_off();
    for (volatile int i = 0; i < 80000; i++);
    buzzer_on();

    buzzer_set_frequency(3520); // A7
    for (volatile int i = 0; i < 80000; i++); // Crotchet
	buzzer_off();
	for (volatile int i = 0; i < 80000; i++);
	buzzer_on();


    buzzer_set_frequency(3520); // A7
    for (volatile int i = 0; i < 80000; i++); // Quaver

    buzzer_set_frequency(3951); // B7
    for (volatile int i = 0; i < 80000; i++); // Quaver

    buzzer_set_frequency(4186); // C8
    for (volatile int i = 0; i < 80000; i++); // Crotchet
	buzzer_off();
	for (volatile int i = 0; i < 80000; i++);
	buzzer_on();

    buzzer_set_frequency(4186); // C8
    for (volatile int i = 0; i < 80000; i++); // Crotchet
	buzzer_off();
	for (volatile int i = 0; i < 80000; i++);
	buzzer_on();

    buzzer_set_frequency(4186); // C8
    for (volatile int i = 0; i < 80000; i++); // Quaver

    buzzer_set_frequency(4698); // D8
    for (volatile int i = 0; i < 80000; i++); // Quaver

    buzzer_set_frequency(3951); // B7
    for (volatile int i = 0; i < 80000; i++); // Crotchet
	buzzer_off();
	for (volatile int i = 0; i < 80000; i++);
	buzzer_on();

    buzzer_set_frequency(3951); // B7
    for (volatile int i = 0; i < 80000; i++); // Crotchet
	buzzer_off();
	for (volatile int i = 0; i < 80000; i++);
	buzzer_on();

    buzzer_set_frequency(3520); // A7
    for (volatile int i = 0; i < 80000; i++); // Quaver

    buzzer_set_frequency(3136); // G7
    for (volatile int i = 0; i < 80000; i++); // Quaver

    buzzer_set_frequency(3520); // A7
    for (volatile int i = 0; i < 160000; i++); // Crotchet
    buzzer_off();
    for (volatile int i = 0; i < 480000; i++); // Crotchet





    buzzer_on();


    buzzer_set_frequency(2636); // E7
    for (volatile int i = 0; i < 80000; i++); // Quaver

    buzzer_set_frequency(3136); // G7
    for (volatile int i = 0; i < 80000; i++); // Quaver

    buzzer_set_frequency(3520); // A7
    for (volatile int i = 0; i < 80000; i++); // Crotchet
    buzzer_off();
    for (volatile int i = 0; i < 80000; i++);
    buzzer_on();

    buzzer_set_frequency(3520); // A7
    for (volatile int i = 0; i < 80000; i++); // Crotchet
	buzzer_off();
	for (volatile int i = 0; i < 80000; i++);
	buzzer_on();


    buzzer_set_frequency(3520); // A7
    for (volatile int i = 0; i < 80000; i++); // Quaver

    buzzer_set_frequency(4186); // C8
    for (volatile int i = 0; i < 80000; i++); // Quaver


	buzzer_set_frequency(4698); // D8
	for (volatile int i = 0; i < 80000; i++); // Crotchet
	buzzer_off();
	for (volatile int i = 0; i < 80000; i++);
	buzzer_on();

	buzzer_set_frequency(4698); // D8
	for (volatile int i = 0; i < 80000; i++); // Crotchet
	buzzer_off();
	for (volatile int i = 0; i < 80000; i++);
	buzzer_on();

	buzzer_set_frequency(4698); // D8
	for (volatile int i = 0; i < 80000; i++); // Quaver

	buzzer_set_frequency(5274); // E8
	for (volatile int i = 0; i < 80000; i++); // Quaver


	buzzer_set_frequency(5588); // F8
	for (volatile int i = 0; i < 80000; i++); // Crotchet
	buzzer_off();
	for (volatile int i = 0; i < 80000; i++);
	buzzer_on();

	buzzer_set_frequency(5588); // F8
	for (volatile int i = 0; i < 80000; i++); // Crotchet
	buzzer_off();
	for (volatile int i = 0; i < 80000; i++);
	buzzer_on();


	buzzer_set_frequency(5274); // E8
	for (volatile int i = 0; i < 80000; i++); // Quaver

	buzzer_set_frequency(4698); // D8
	for (volatile int i = 0; i < 80000; i++); // Quaver

	buzzer_set_frequency(5274); // E8
	for (volatile int i = 0; i < 80000; i++); // Quaver

    buzzer_set_frequency(3520); // A7
    for (volatile int i = 0; i < 240000; i++); // Quaver


	buzzer_set_frequency(3520); // A7
	for (volatile int i = 0; i < 80000; i++); // Quaver

	buzzer_set_frequency(3951); // B7
	for (volatile int i = 0; i < 80000; i++); // Quaver

	buzzer_set_frequency(4186); // C8
	for (volatile int i = 0; i < 80000; i++); // Crotchet
	buzzer_off();
	for (volatile int i = 0; i < 80000; i++);
	buzzer_on();

	buzzer_set_frequency(4186); // C8
	for (volatile int i = 0; i < 80000; i++); // Crotchet
	buzzer_off();
	for (volatile int i = 0; i < 80000; i++);
	buzzer_on();

	buzzer_set_frequency(4698); // D8
	for (volatile int i = 0; i < 80000; i++); // Crotchet
	buzzer_off();
	for (volatile int i = 0; i < 80000; i++);
	buzzer_on();

	buzzer_set_frequency(5274); // E8
	for (volatile int i = 0; i < 80000; i++); // Quaver

    buzzer_set_frequency(3520); // A7
    for (volatile int i = 0; i < 240000; i++); // Quaver


	buzzer_set_frequency(3520); // A7
	for (volatile int i = 0; i < 80000; i++); // Quaver

	buzzer_set_frequency(4186); // C8
	for (volatile int i = 0; i < 80000; i++); // Quaver


	buzzer_set_frequency(3951); // B7
	for (volatile int i = 0; i < 80000; i++); // Crotchet
	buzzer_off();
	for (volatile int i = 0; i < 80000; i++);
	buzzer_on();

	buzzer_set_frequency(3951); // B7
	for (volatile int i = 0; i < 80000; i++); // Crotchet
	buzzer_off();
	for (volatile int i = 0; i < 80000; i++);
	buzzer_on();

	buzzer_set_frequency(4186); // C8
	for (volatile int i = 0; i < 80000; i++); // Quaver

	buzzer_set_frequency(3520); // A7
	for (volatile int i = 0; i < 80000; i++); // Quaver

	buzzer_set_frequency(3951); // A7
	for (volatile int i = 0; i < 320000; i++); // Semibrieve


}


