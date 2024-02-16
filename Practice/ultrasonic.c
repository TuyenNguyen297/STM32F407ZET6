#include <stm32f4xx.h>
#include "types.h"
#include "timer.h"
#include "gpio.h"
#include "LCD_HD44780.h"
volatile uint32_t echo_pulse_width = 0;
volatile uint16_t distance = 0;

void Ultrasonic_HCSR04_Init(char* trig_pin, char* echo_pin) {  // Use TIM10/11
	GPIO_Init(trig_pin, AF);  // TIM10_CH1 -> F6
	GPIO_Init(echo_pin, AF);  // TIM11_CH1 -> F7
	GPIO_Set_AF_Mode(trig_pin, AF3, HIGH_SPEED, WEAK);
	GPIO_Set_AF_Mode(echo_pin, AF3, HIGH_SPEED, WEAK);

	TIM_TypeDef * Ultrasonic_Trig = TIM_ADDRESS(Ultrasonic_Trig_TIM);
	TIM_TypeDef * Ultrasonic_Echo = TIM_ADDRESS(Ultrasonic_Echo_TIM);
	RCC->APB2ENR |= RCC_APB2ENR_TIM11EN | RCC_APB2ENR_TIM10EN;

	Ultrasonic_Trig->CR1 &= ~TIM_CR1_CEN;
	Ultrasonic_Trig->SR &= 0x0000;
	Ultrasonic_Trig->CNT = 0;
	Ultrasonic_Trig->ARR = 0xFFFF;
	Ultrasonic_Trig->PSC = CK_PSC_2 / 1000000 - 1;  // CK_CNT = 1MHz, T = 1uS
	Ultrasonic_Trig->CR1 &= ~(TIM_CR1_CKD | TIM_CR1_ARPE | TIM_CR1_UDIS | TIM_CR1_URS | TIM_CR1_OPM);
	Ultrasonic_Trig->CR1 |= TIM_CR1_OPM;
	Ultrasonic_Trig->DIER &= ~(TIM_DIER_CC1IE | TIM_DIER_UIE);
	Ultrasonic_Trig->CCER &= ~TIM_CCER_CC1E;
	Ultrasonic_Trig->CCMR1 &= 0x0000;
	Ultrasonic_Trig->CCMR1 &= ~TIM_CCMR1_CC1S;  // Output Compare mode
	Ultrasonic_Trig->CCMR1 &= ~TIM_CCMR1_OC1M;
	Ultrasonic_Trig->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_0;  // // PWM Mode 1
	Ultrasonic_Trig->CCMR1 &= ~TIM_CCMR1_OC1PE;  // CCR1 is updated each OC
	Ultrasonic_Trig->CCER &= ~(TIM_CCER_CC1NP | TIM_CCER_CC1P);  // Active high
	Ultrasonic_Trig->CCR1 = 0xFFFF - 9;  // minimum 10 pulse-width = 10us to activate trigger pin
	Ultrasonic_Trig->CCER |= TIM_CCER_CC1E;  // Turn on counter
	Ultrasonic_Trig->CR1 |= TIM_CR1_CEN;  // Turn on Timer

	Ultrasonic_Echo->CR1 &= ~TIM_CR1_CEN;
	Ultrasonic_Echo->SR &= 0x0000;
	Ultrasonic_Echo->CNT = 0;
	Ultrasonic_Echo->ARR = 0xFFFF;
	Ultrasonic_Echo->PSC = CK_PSC_2 / 1000000 - 1;  // CK_CNT = 1MHz, T = 1uS
	Ultrasonic_Echo->CR1 &= ~(TIM_CR1_CKD | TIM_CR1_ARPE | TIM_CR1_UDIS | TIM_CR1_URS | TIM_CR1_OPM);
	Ultrasonic_Echo->DIER &= ~(TIM_DIER_CC1IE | TIM_DIER_UIE);
	Ultrasonic_Echo->CCER &= ~TIM_CCER_CC1E;
	Ultrasonic_Echo->CCMR1 &= 0x0000;
	Ultrasonic_Echo->DIER |= TIM_DIER_CC1IE | TIM_DIER_UIE;
	Ultrasonic_Echo->CCER &= ~TIM_CCER_CC1E;
	Ultrasonic_Echo->CCMR1 &= 0x0000;
	Ultrasonic_Echo->CCMR1 |= TIM_CCMR1_CC1S_0;  // 01: Capture/Compare IC1 mapped in TI1
	Ultrasonic_Echo->CCMR1 &= ~(TIM_CCMR1_IC1F | TIM_CCMR1_IC1PSC);  // No filter, No prescaler
	Ultrasonic_Echo->CCER |= TIM_CCER_CC1NP | TIM_CCER_CC1P;  // Trigger by Both edge
	Ultrasonic_Echo->CCER |= TIM_CCER_CC1E;  // Enable CC mode
	Ultrasonic_Echo->CR1 |= TIM_CR1_CEN;  // Enable counter
	uint32_t IRQn = TIM1_TRG_COM_TIM11_IRQn;
	NVIC->ISER[(IRQn >> 5)] |= 1UL << (IRQn & 0x1F);
}

