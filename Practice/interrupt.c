#include <stdio.h>
#include <stm32f4xx.h>
#include <core_cm4.h>
#include "types.h"
#include "gpio.h"
#include "timer.h"
#include "interrupt.h"

void EXTI_Init(uint16_t IRQn, char *pin, uint32_t priority, uint16_t edge) {
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	uint16_t bank = pinBank(pin) >> 8;
	uint16_t pinNo = pinBank(pin) & 0xFF;
	uint16_t word_offset = pinNo / 4;
	uint16_t half_byte_offset = pinNo - pinNo / 4 * 4;
	uint16_t amount_of_shift = half_byte_offset * 4;
	SYSCFG->EXTICR[word_offset] |= bank << amount_of_shift;

	if (edge == RISING) {
		EXTI->RTSR |= 1UL << pinNo;
	}
	else if (edge == FALLING) {
		EXTI->FTSR |= 1UL << pinNo;
	}
	EXTI->IMR |= 1UL << pinNo;
	NVIC->IP[IRQn] |= (priority << 4) & 0xFF;
	word_offset = IRQn >> 5;  // IRQn / 32
	uint16_t bit_offset = IRQn & 0x1F;  // IRQ mod 32
	NVIC->ISER[word_offset] |= 1UL << bit_offset;
}

void EXTI_ReActivate(uint16_t IRQn, uint16_t exti_line) {
	EXTI->IMR |= 1UL << exti_line;  // Turn on line interrupt
	uint32_t word_offset = IRQn >> 5;  // IRQn / 32
	uint32_t bit_offset = IRQn & 0x1F;  // IRQ mod 32
	NVIC->ISER[word_offset] |= 1UL << bit_offset;
	EXTI->PR |= 1 << exti_line;
}

void EXTI_DeActivate(uint16_t IRQn, uint16_t exti_line) {
	EXTI->IMR &= ~(1UL << exti_line);  // Turn off line interrupt
	uint32_t word_offset = IRQn >> 5;  // IRQn / 32
	uint32_t bit_offset = IRQn & 0x1F;  // IRQ mod 32
	NVIC->ISER[word_offset] &= ~(1UL << bit_offset);
}

void Debounce_EXTI(uint16_t exti_irq, uint16_t exti_line, uint16_t debounce_ms) {
	EXTI_DeActivate(exti_irq, exti_line);
	uint32_t reload = SYSTICK_CLK / 1000 * debounce_ms;
	Systick_Init(reload, TRIGGER_HANDLER);
}

