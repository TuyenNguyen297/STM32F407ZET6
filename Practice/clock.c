#include <stdio.h>
#include <stm32f4xx.h>
#include <core_cm4.h>
#include "types.h"
#include "clock.h"
#include "io.h"
#include "gpio.h"

void Select_Clock_Source(unsigned int clk) {
	RCC->CFGR &= ~(15UL << 4); // Set presc of AHB to 1
	RCC->CFGR &= ~(7UL << 10); // Reset PPRE1
	RCC->CFGR |= (5UL << 10); // PPRE1 presc of 4. APB1CLK = AHBCLK /4 -> Timer Clock = APB1CLK x 2
	RCC->CFGR &= ~(7UL << 13);
	RCC->CFGR |= (4UL << 13); // PPRE2 presc of 2. APB2CLK = AHBCLK /2 -> Timer Clock = APB2CLK x 2
	switch (clk) {
	case 0: // HSI on
		RCC->CR &= ~RCC_CR_HSEON;
		RCC->CR &= ~RCC_CR_PLLON;
		RCC->CR |= RCC_CR_HSION;
		while (!(RCC->CR & RCC_CR_HSIRDY))
			;
		RCC->CFGR &= ~3UL; // choose HSI = 00
		while (RCC->CFGR & RCC_CFGR_SWS_HSI)
			// HSI = 00
			;
		break;
	case 1:
		RCC->CR &= ~RCC_CR_HSION;
		RCC->CR &= ~RCC_CR_PLLON;
		RCC->CR |= RCC_CR_HSEON;
		while (!(RCC->CR & RCC_CR_HSERDY))
			;
		RCC->CFGR |= RCC_CFGR_SW_HSE;
		while (!(RCC->CFGR & RCC_CFGR_SWS_HSE))
			;
		break;
	case 2:
		RCC->APB1ENR |= RCC_APB1ENR_PWREN;
		PWR->CR |= PWR_CR_VOS;
		RCC->CR &= ~RCC_CR_PLLON;
		RCC->CR &= ~RCC_CR_PLLI2SON;
		RCC->CR &= ~RCC_CR_HSION;
		RCC->CR |= RCC_CR_HSEON; // Turn HSE on for input of PLL
		while (!(RCC->CR & RCC_CR_HSERDY))
			;
		uint32_t PLL_P = 2, PLL_N = 168, PLL_M = 4, PLL_Q = 7;
		RCC->PLLCFGR = 0x24003010;
		RCC->PLLCFGR = PLL_M | (PLL_N << 6) | (((PLL_P >> 1) - 1) << 16) | (RCC_PLLCFGR_PLLSRC_HSE) | (PLL_Q << 24);
		RCC->CR |= RCC_CR_PLLON;
		while (!(RCC->CR & RCC_CR_PLLRDY))
			;
		FLASH->ACR &= ~(FLASH_ACR_LATENCY);
		FLASH->ACR = FLASH_ACR_LATENCY_5WS;
		while ((FLASH->ACR & FLASH_ACR_LATENCY) != FLASH_ACR_LATENCY_5WS)
			;
		RCC->CFGR &= ~3UL;
		RCC->CFGR |= RCC_CFGR_SW_PLL;
		while (!(RCC->CFGR & RCC_CFGR_SWS_PLL))
			;
		break;
	default:
		break;
	}
}
void Confirm_Clock_Source(void) {
	if (!(RCC->CFGR & (3UL << 2))) digitalWrite(HSI_RED_LED, LOW);
	if (RCC->CFGR & RCC_CFGR_SWS_HSE) digitalWrite(HSE_GREEN_LED, LOW);
	if (RCC->CFGR & RCC_CFGR_SWS_PLL) digitalWrite(PLL_BLUE_LED, LOW);
}
