#include <stm32f4xx.h>
#include "types.h"
#include "io.h"
#include "timer.h"
#include "gpio.h"

void Systick_Init(uint32_t reload, Bool trigger) {
	SysTick->LOAD = reload - 1;
	SysTick->VAL = 0;
	SCB->SHP[11] |= ((1 << __NVIC_PRIO_BITS) - 1);  // highest priority
	SysTick->CTRL &= ~(1UL << 2);  // Use AHB/8 Clock
	if (trigger)
	SysTick->CTRL |= 1UL << 1;  //Counting down to 0  trigger an exception request (interrupt)
	else
	SysTick->CTRL &= ~(1UL << 1);  //Counting down to 0 doesn't trigger an exception request (interrupt)
	SysTick->CTRL |= 1UL;  // Enable Counter down
}
void Systick_Reset() {
	SysTick->CTRL &= 0;
}

TIM_TypeDef * TIM_ADDRESS(uchar tim) {
	uint32_t APBPERIPH_BASE = 0;
	uint32_t tim_offset = 0;
	switch (tim) {
	case 1:
		APBPERIPH_BASE = APB2PERIPH_BASE;
		break;
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
		APBPERIPH_BASE = APB1PERIPH_BASE;
		tim_offset = (tim - 2) * 0x0400;
		break;
	case 8:
		APBPERIPH_BASE = APB2PERIPH_BASE;
		tim_offset = 0x0400;
		break;
	case 9:
	case 10:
	case 11:
		APBPERIPH_BASE = APB2PERIPH_BASE;
		tim_offset = (tim - 9) * 0x0400 + 0x4000;
		break;
	case 12:
	case 13:
	case 14:
		APBPERIPH_BASE = APB1PERIPH_BASE;
		tim_offset = (tim - 12) * 0x0400 + 0x1800;
		break;
	default:
		break;
	}
	return (TIM_TypeDef*) (APBPERIPH_BASE + tim_offset);
}

void Basic_Timer_Init(uchar tim, TIMER_ROLE role) {
	TIM_TypeDef *TIM = TIM_ADDRESS(tim);
	char first_tim_offset_in_APB1_bus = 2;
	RCC->APB1ENR |= 1UL << (tim - first_tim_offset_in_APB1_bus);
	TIM->CR1 &= ~(TIM_CR1_CEN | TIM_CR1_ARPE | TIM_CR1_UDIS | TIM_CR1_URS | TIM_DIER_UDE);  //Turn off TIMER. Enable UEV. Disable DMA IRQ
	switch (role) {
	case DELAY:
		TIM->CR1 |= TIM_CR1_OPM;  // Stop counting at the next update event.
		TIM->DIER &= ~TIM_DIER_UIE;
		break;
	case IRQ:
		TIM->CR1 &= ~TIM_CR1_OPM;
		TIM->SR = 0;
		TIM->DIER |= TIM_DIER_UIE;
		TIM->CNT = 0;
		TIM->PSC = CK_PSC_1 / 2000 - 1;
		TIM->ARR = 100 / 0.5 - 1;  // 1 pulse - > 0.5ms, ? pulse -> N(s) / 0.5ms. Initially set to 100ms bink
		uint32_t IRQn = 54 + (tim - 6);  // 54 is base position of TIM6
		NVIC->ISER[(IRQn >> 5)] |= 1UL << (IRQn & 0x1F);
		NVIC->IP[IRQn] = (BASIC_TIM_PRI << 4) & 0xFF;
		TIM->CR1 |= TIM_CR1_CEN;
		break;
	default:
		break;
	}
}

void Trigger_Timer_IRQ_MS(uint16_t ms) {
	TIM_TypeDef * TIM = TIM_ADDRESS(KPAD_TIMER);
	TIM->CR1 &= ~TIM_CR1_CEN;
	TIM->CNT = 0;
	TIM->SR = 0;
	TIM->ARR = ms / 0.5 - 1;
	TIM->CR1 |= TIM_CR1_CEN;
}

void delay_ms(double ms) {
	TIM_TypeDef *TIM = TIM_ADDRESS(SYSTEM_TIMER);
//	TIM->CR1 &= ~TIM_CR1_CEN;
	TIM->SR = 0;
	TIM->CNT = 0;
	TIM->PSC = CK_PSC_1 / 2000 - 1;  // Create 2000Hz CLK -> 1 pulse = 0.5ms
	TIM->ARR = (uint16_t)(ms / 0.5 - 1);  // 2000Hz -> 1 pulse = 0.5(ms) -> M(pulse) = N(ms) / 0.5
	TIM->CR1 |= TIM_CR1_CEN;
	while (!(TIM->SR & TIM_SR_UIF))
		;
}

void delay_us(double us) {
	TIM_TypeDef * TIM = TIM_ADDRESS(SYSTEM_TIMER);
//	TIM->CR1 &= ~TIM_CR1_CEN;
	TIM->SR = 0;
	TIM->CNT = 0;
	TIM->PSC = CK_PSC_1 / 1000000 - 1;  //83;
	TIM->ARR = (uint16_t)(us - 1);
	TIM->CR1 |= TIM_CR1_CEN;
	while (!(TIM->SR & TIM_SR_UIF))
		;
//	double reload = SYSTICK_CLK * us / 1000000; // Convert ms to s
//	Systick_Init(reload, NO_TRIGGER_HANDLER);
//	while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk))
//		;
//	Systick_Reset();
}

void Capture_Init(uchar tim, uchar channel, uchar countMode, uchar ACTIVE_EDGE, uint16_t N_FILTER, uint16_t ICPSC) {
	TIM_TypeDef * TIM = TIM_ADDRESS(tim);
	TIM->CR1 &= ~TIM_CR1_CEN;  // Turn off Counter before other configs
	TIM->CR1 &= ~(TIM_CR1_ARPE | TIM_CR1_OPM | TIM_CR1_URS | TIM_CR1_UDIS | TIM_CR1_CKD);  // Turn off Auto-Reload Preload on UEV, turn off UEV
	TIM->ARR = 59999;  //Overflow point. OC/UC each 1 second (CK_CNT = 60Khz)
	uint16_t IRQn;
	char offset;
	switch (tim) {
	case TIMER_1:
	case TIMER_8:
		offset = (tim == 1) ? 0 : 1;
		RCC->APB2ENR |= 1UL << offset;
		TIM->PSC = CK_PSC_2 / 60000 - 1;
		if (countMode != CENTER_ALIGN) {
			TIM->CR1 &= ~TIM_CR1_DIR;  // Reset DIR
			TIM->CR1 |= countMode << (char) TIM_CR1_DIR;  // Choose UP/DOWN counter
			TIM->CR1 &= ~(TIM_CR1_CMS_1 | TIM_CR1_CMS_0);  // Set edge-align depending on DIR
		}
		else {
			TIM->CR1 |= TIM_CR1_CMS_1 | TIM_CR1_CMS_0;
		}
		TIM->CCER &= ~(TIM_CCER_CC1E << ((channel - 1) << 2));

		if (channel <= 2) {
			TIM->CCMR1 &= ~(TIM_CCMR1_CC1S << ((channel - 1) << 3));
			TIM->CCMR1 |= TIM_CCMR1_CC1S_0 << ((channel - 1) << 3);
			TIM->CCMR1 |= (ICPSC << ((channel - 1) << 3));
			TIM->CCMR1 &= ~(TIM_CCMR1_IC1F << ((channel - 1) << 3));
			TIM->CCMR1 |= (N_FILTER << ((channel - 1) << 3));
		}
		else if (channel <= 4) {
			TIM->CCMR2 &= ~(TIM_CCMR2_CC3S << (((channel >> 1) - 1) << 3));
			TIM->CCMR2 |= TIM_CCMR2_CC3S_0 << (((channel >> 1) - 1) << 3);
			TIM->CCMR2 |= (ICPSC << (((channel >> 1) - 1) << 3));
			TIM->CCMR2 &= ~(TIM_CCMR2_IC3F << (((channel >> 1) - 1) << 3));
			TIM->CCMR2 |= (N_FILTER << (((channel >> 1) - 1) << 3));
		}
		TIM->CCER &= ~((TIM_CCER_CC1NP | TIM_CCER_CC1P) << ((channel - 1) << 2));
		TIM->CCER |= (ACTIVE_EDGE << ((channel - 1) << 2));
		TIM->CCER |= (TIM_CCER_CC1E << ((channel - 1) << 2));
		TIM->DIER |= (TIM_DIER_CC1IE << (channel - 1));
		TIM->CR1 |= TIM_CR1_CEN;
		IRQn = tim == TIMER_1 ? TIM1_CC_IRQn : TIM8_CC_IRQn;
		break;  // Advanced, 4 CHNs, 16-bit up/down, complementary output
	case TIMER_2:  // 2,5: GP, 4 CHNs, 32-bit up/down, center-align mode
	case TIMER_3:  // 3,4: GP, 4 CHNs, 16-bit up/down, center-align mode
	case TIMER_4:
	case TIMER_5:
		offset = tim - 2;
		RCC->APB1ENR |= 1UL << offset;
		TIM->PSC = CK_PSC_1 / 60000 - 1;
		if (countMode != CENTER_ALIGN) {
			TIM->CR1 &= ~TIM_CR1_DIR;  // Reset DIR
			TIM->CR1 |= countMode << (char) TIM_CR1_DIR;  // Choose UP/DOWN counter
			TIM->CR1 &= ~(TIM_CR1_CMS_1 | TIM_CR1_CMS_0);  // Set edge-align depending on DIR
		}
		else {
			TIM->CR1 |= TIM_CR1_CMS_1 | TIM_CR1_CMS_0;
		}
		TIM->CCER &= ~(TIM_CCER_CC1E << ((channel - 1) << 2));

		if (channel <= 2) {
			TIM->CCMR1 &= ~(TIM_CCMR1_CC1S << ((channel - 1) << 3));
			TIM->CCMR1 |= TIM_CCMR1_CC1S_0 << ((channel - 1) << 3);
			TIM->CCMR1 |= (ICPSC << ((channel - 1) << 3));
			TIM->CCMR1 &= ~(TIM_CCMR1_IC1F << ((channel - 1) << 3));
			TIM->CCMR1 |= (N_FILTER << ((channel - 1) << 3));
		}
		else if (channel <= 4) {
			TIM->CCMR2 &= ~(TIM_CCMR2_CC3S << (((channel >> 1) - 1) << 3));
			TIM->CCMR2 |= TIM_CCMR2_CC3S_0 << (((channel >> 1) - 1) << 3);
			TIM->CCMR2 |= (ICPSC << (((channel >> 1) - 1) << 3));
			TIM->CCMR2 &= ~(TIM_CCMR2_IC3F << (((channel >> 1) - 1) << 3));
			TIM->CCMR2 |= (N_FILTER << (((channel >> 1) - 1) << 3));
		}
		TIM->CCER &= ~((TIM_CCER_CC1NP | TIM_CCER_CC1P) << ((channel - 1) << 2));
		TIM->CCER |= (ACTIVE_EDGE << ((channel - 1) << 2));
		TIM->CCER |= (TIM_CCER_CC1E << ((channel - 1) << 2));
		TIM->DIER |= (TIM_DIER_CC1IE << (channel - 1));
		TIM->CR1 |= TIM_CR1_CEN;
		break;
	case TIMER_9:
	case TIMER_12:
		if (tim == 9) {
			RCC->APB2ENR |= RCC_APB2ENR_TIM9EN;
			TIM->PSC = CK_PSC_2 / 60000 - 1;
			IRQn = TIM1_BRK_TIM9_IRQn;
		}
		else {
			RCC->APB1ENR |= RCC_APB1ENR_TIM12EN;
			TIM->PSC = CK_PSC_1 / 60000 - 1;
			IRQn = TIM8_BRK_TIM12_IRQn;
		}
		TIM->CCER &= ~(TIM_CCER_CC1E << ((channel - 1) << 2));  //CCxE: ~(0X0001<< (channel - 1) * 4)// Turn off corresponding channel before writing CCxS
		TIM->CCMR1 &= ~(TIM_CCMR1_CC1S << ((channel - 1) << 3));  //CCxS: ~(0x0003 << ((channel - 1) *8)) clear
		TIM->CCMR1 |= TIM_CCMR1_CC1S_0 << ((channel - 1) << 3);  // ICx map to TIx
		TIM->CCMR1 |= (ICPSC << ((channel - 1) << 3));  // (ICPSC << (channel - 1) * 8)// each channel occupies 8 bit
		TIM->CCMR1 &= ~(TIM_CCMR1_IC1F << ((channel - 1) << 3));  //ICxF: ~((0xF << (channel - 1) * 8)) clear
		TIM->CCMR1 |= (N_FILTER << ((channel - 1) << 3));  //ICxF:(N_FILTER & 0xFFFF) << ((channel - 1) * 8))
		TIM->CCER &= ~((TIM_CCER_CC1NP | TIM_CCER_CC1P) << ((channel - 1) << 2));  //CCxNP/CCxP: ~(0x000A << ((channel - 1) * 4)) clear
		TIM->CCER |= (ACTIVE_EDGE << ((channel - 1) << 2));  //CCxNP/CCxP: ~(0x000A << ((channel - 1) * 4)) Active both
		TIM->CCER |= (TIM_CCER_CC1E << ((channel - 1) << 2));  // Re-Enable Input Capture after configs
		TIM->DIER |= (TIM_DIER_CC1IE << (channel - 1));  //| TIM_DIER_UIE; // Turn on corresponding capture interrupt
		TIM->CR1 |= TIM_CR1_CEN;
		break;  // GP, 2 CHNs, 16-bit up
	case TIMER_10:
	case TIMER_11:
	case TIMER_13:
	case TIMER_14:
		if (tim <= 11) {
			offset = 17 + (tim - 10);
			RCC->APB2ENR |= 1UL << offset;
			TIM->PSC = CK_PSC_2 / 60000 - 1;
			IRQn = tim == 10 ? TIM1_UP_TIM10_IRQn : TIM1_TRG_COM_TIM11_IRQn;
		}
		else {
			offset = 7 + (tim - 13);
			RCC->APB1ENR |= 1UL << offset;
			TIM->PSC = CK_PSC_1 / 60000 - 1;
			IRQn = tim == 13 ? TIM8_UP_TIM13_IRQn : TIM8_TRG_COM_TIM14_IRQn;
		}
		TIM->CCER &= ~TIM_CCER_CC1E;
		TIM->CCMR1 &= ~TIM_CCMR1_CC1S;
		TIM->CCMR1 |= TIM_CCMR1_CC1S_0;
		TIM->CCMR1 |= ICPSC & TIM_CCMR1_IC1PSC;
		TIM->CCMR1 &= ~TIM_CCMR1_IC1F;
		TIM->CCMR1 |= N_FILTER;
		TIM->CCER &= ~(TIM_CCER_CC1NP | TIM_CCER_CC1P);
		TIM->CCER |= ACTIVE_EDGE;
		TIM->CCER |= TIM_CCER_CC1E;
		TIM->DIER |= TIM_DIER_CC1IE;
		TIM->CR1 |= TIM_CR1_CEN;
		break;  // GP, 1 CHN, 16-bit up
	default:
		break;  // Bypass TIM6/7 (No output)
	}
	//NVIC->IP[IRQn] |= (1 << 4) & 0xFF;
	uint32_t word_offset = IRQn >> 5;  // IRQn / 32
	uint32_t bit_offset = IRQn & 0x1F;  // IRQ mod 32
	NVIC->ISER[word_offset] |= 1UL << bit_offset;
}

void Setup_IC_Mode(uchar tim) {
	switch (tim) {
	case TIMER_1:
		GPIO_Init(TIM1_CH1, AF);  // PA8
		GPIO_Init(TIM1_CH2, AF);  // PA9
		GPIO_Init(TIM1_CH3, AF);  // PA10
		GPIO_Init(TIM1_CH4, AF);  // PA11
		GPIO_Set_AF_Mode(TIM1_CH1, AF1, HIGH_SPEED, WEAK);
		GPIO_Set_AF_Mode(TIM1_CH2, AF1, HIGH_SPEED, WEAK);
		GPIO_Set_AF_Mode(TIM1_CH3, AF1, HIGH_SPEED, WEAK);
		GPIO_Set_AF_Mode(TIM1_CH4, AF1, HIGH_SPEED, WEAK);
		Capture_Init(TIMER_1, TIM_CHN_1, COUNTUP, NONINVERTING, FILTER_8, NO_ICPSC);  // A8  -> FILTER 8, NO ICPSC
		Capture_Init(TIMER_1, TIM_CHN_2, COUNTUP, NONINVERTING, FILTER_8, ICPSC_2);  // A9  -> FILTER 8, ICPSC_2
		Capture_Init(TIMER_1, TIM_CHN_3, COUNTUP, NONINVERTING, FILTER_8, ICPSC_4);  // A10 -> FILTER 8, ICPSC_4
		Capture_Init(TIMER_1, TIM_CHN_4, COUNTUP, NONINVERTING, FILTER_8, ICPSC_8);  // A11 -> FILTER 8, ICPSC_8
		break;
	case TIMER_3:
		GPIO_Init(TIM3_CH1, AF);  // PC6
		GPIO_Init(TIM3_CH2, AF);  // PC7
		GPIO_Init(TIM3_CH3, AF);  // PC8
		GPIO_Init(TIM3_CH4, AF);  // PC9
		GPIO_Set_AF_Mode(TIM3_CH1, AF2, HIGH_SPEED, WEAK);
		GPIO_Set_AF_Mode(TIM3_CH2, AF2, HIGH_SPEED, WEAK);
		GPIO_Set_AF_Mode(TIM3_CH3, AF2, HIGH_SPEED, WEAK);
		GPIO_Set_AF_Mode(TIM3_CH4, AF2, HIGH_SPEED, WEAK);
		Capture_Init(TIMER_3, TIM_CHN_1, COUNTUP, NONINVERTING, FILTER_8, NO_ICPSC);  // C6  -> FILTER 8, NO ICPSC
		Capture_Init(TIMER_3, TIM_CHN_2, COUNTUP, NONINVERTING, FILTER_8, ICPSC_2);  // C7  -> FILTER 8, ICPSC_2
		Capture_Init(TIMER_3, TIM_CHN_3, COUNTUP, NONINVERTING, FILTER_8, ICPSC_4);  // C8  -> FILTER 8, ICPSC_4
		Capture_Init(TIMER_3, TIM_CHN_4, COUNTUP, NONINVERTING, FILTER_8, ICPSC_8);  // C9  -> FILTER 8, ICPSC_8
		break;
	case TIMER_9:
		GPIO_Init(TIM9_CH1, AF);  // PA2
		GPIO_Init(TIM9_CH2, AF);  // PA3
		GPIO_Set_AF_Mode(TIM9_CH1, AF3, HIGH_SPEED, WEAK);
		GPIO_Set_AF_Mode(TIM9_CH2, AF3, HIGH_SPEED, WEAK);
		Capture_Init(TIMER_9, TIM_CHN_1, COUNTUP, NONINVERTING, FILTER_8, ICPSC_8);
		Capture_Init(TIMER_9, TIM_CHN_2, COUNTUP, BOTH_EDGE, FILTER_8, ICPSC_2);
		break;
	case TIMER_10:
		GPIO_Init(TIM10_CH1, AF);  // PF6
		GPIO_Set_AF_Mode(TIM10_CH1, AF3, HIGH_SPEED, WEAK);  //LED4 green
		Capture_Init(TIMER_10, TIM_CHN_1, COUNTUP, NONINVERTING, FILTER_8, ICPSC_8);
		break;
	default:
		break;
	}
}

void PWM_Init(uchar tim, uchar countMode, uint16_t pwmMode, uint32_t CK_CNT, float PWM_FREQ, TRIGGER_IRQ trigger_irq, TIMER_SIGNAL tim_sig) {
	TIM_TypeDef * TIM = TIM_ADDRESS(tim);
	TIM->CR1 &= ~(TIM_CR1_CEN | TIM_CR1_ARPE | TIM_CR1_OPM | TIM_CR1_UDIS);  // Turn off Auto-Reload Preload on UEV, turn on UEV
	TIM->CR1 |= TIM_CR1_URS;  // Only overflow/underflow trigger UEV
	TIM->DIER &= 0x0000;  // Turn off all interrupts
	uint16_t IRQn;
	char offset;
	switch (tim) {
	case TIMER_1:  // Advanced, 4 CHNs, 16-bit up/down, complementary output, , TRGO
	case TIMER_8:  // Advanced, 4 CHNs, 16-bit up/down, complementary output, , TRGO
		offset = (tim == 1) ? 0 : 1;
		RCC->APB2ENR |= 1UL << offset;
		TIM->PSC = CK_PSC_2 / CK_CNT - 1;
		if (countMode != CENTER_ALIGN) {
			TIM->CR1 &= ~TIM_CR1_DIR;  // Reset DIR
			TIM->CR1 |= countMode << (char) TIM_CR1_DIR;  // Choose UP/DOWN counter
			TIM->CR1 &= ~(TIM_CR1_CMS_1 | TIM_CR1_CMS_0);  // Set edge-align depending on DIR
		}
		else {
			TIM->CR1 |= TIM_CR1_CMS_1 | TIM_CR1_CMS_0;
		}
		TIM->ARR = CK_CNT / PWM_FREQ - 1;
		TIM->CNT &= 0;
		TIM->BDTR &= ~TIM_BDTR_LOCK;
		TIM->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | (pwmMode << TIM_CCMR1_OC1M_0);
		TIM->CCMR1 |= TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1 | (pwmMode << (char) TIM_CCMR1_OC2M_0);
		TIM->CCMR2 |= TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_1 | (pwmMode << TIM_CCMR2_OC3M_0);
		TIM->CCMR2 |= TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4M_1 | (pwmMode << (char) TIM_CCMR2_OC4M_0);
		TIM->CCMR1 |= (TIM_CCMR1_OC1PE | TIM_CCMR1_OC2PE);  //CCR1/CCR2 value is updated only in next update event
		TIM->CCMR2 |= (TIM_CCMR2_OC3PE | TIM_CCMR2_OC4PE);  //CCR3/CCR4 value is updated only in next update event
		switch (tim_sig) {
		case INTERNAL:
			TIM->CR2 &= ~TIM_CR2_MMS;
			//TIM->CR2 |= TIM_CR2_MMS_1;  // Update Event as TRGO signal, UEV interval = 1/ CNT_CLK * TIM->ARR
			TIM->CR2 |= TIM_CR2_MMS_2;  // OC1REF as TRGO signal
			break;
		case EXTERNAL:
			TIM->CCER &= ~(TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E);  // Disable CC1/CC2 before being able to write CCMR1_CC1S/CC2S
			TIM->CCMR1 &= ~(TIM_CCMR1_CC2S | TIM_CCMR1_CC1S);  // Set CC2/CC1 as outputs
			TIM->CCMR2 &= ~(TIM_CCMR2_CC3S | TIM_CCMR2_CC4S);  // Set CC4/CC3 as outputs
			break;
		default:
			break;
		}
		TIM->CCER &= ~(TIM_CCER_CC4P | TIM_CCER_CC3P | TIM_CCER_CC2P | TIM_CCER_CC1P);  // Active high polarity bit
		TIM->CCR1 = TIM->CCR2 = TIM->CCR3 = TIM->CCR4 = 0;
		TIM->BDTR |= TIM_BDTR_MOE;
		IRQn = tim == TIMER_1 ? TIM1_CC_IRQn : TIM8_CC_IRQn;
		break;
	case TIMER_2:  // 2,5: GP, 4 CHNs, 32-bit up/down, center-align mode, TRGO
	case TIMER_3:  // 3,4: GP, 4 CHNs, 16-bit up/down, center-align mode, TRGO
	case TIMER_4:
	case TIMER_5:
		offset = tim - 2;
		RCC->APB1ENR |= 1UL << offset;
		TIM->PSC = CK_PSC_1 / CK_CNT - 1;
		if (countMode != CENTER_ALIGN) {
			TIM->CR1 &= ~TIM_CR1_DIR;  // Reset DIR
			TIM->CR1 |= countMode << (char) TIM_CR1_DIR;  // Choose UP/DOWN counter
			TIM->CR1 &= ~(TIM_CR1_CMS_1 | TIM_CR1_CMS_0);  // Set edge-align depending on DIR
		}
		else {
			TIM->CR1 |= TIM_CR1_CMS_1 | TIM_CR1_CMS_0;
		}
		float arr = (float) CK_CNT / PWM_FREQ - 1;
		TIM->ARR = (uint32_t) arr;
		TIM->CNT &= 0;
		TIM->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | (pwmMode << TIM_CCMR1_OC1M_0);
		TIM->CCMR1 |= TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1 | (pwmMode << (char) TIM_CCMR1_OC2M_0);
		TIM->CCMR2 |= TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_1 | (pwmMode << TIM_CCMR2_OC3M_0);
		TIM->CCMR2 |= TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4M_1 | (pwmMode << (char) TIM_CCMR2_OC4M_0);
		TIM->CCMR1 |= (TIM_CCMR1_OC1PE | TIM_CCMR1_OC2PE);  //CCR1/CCR2 value is updated only in next update event
		TIM->CCMR2 |= (TIM_CCMR2_OC3PE | TIM_CCMR2_OC4PE);  //CCR3/CCR4 value is updated only in next update event
		switch (tim_sig) {
		case INTERNAL:
			TIM->CR2 &= ~TIM_CR2_MMS;
			//TIM->CR2 |= TIM_CR2_MMS_1;  // Update Event as TRGO signal, UEV interval = 1/ CNT_CLK * TIM->ARR
			TIM->CR2 |= TIM_CR2_MMS_2;  // OC1REF as TRGO signal
			break;
		case EXTERNAL:
			TIM->CCER &= ~(TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E);  // Disable CC1/CC2 before being able to write CCMR1_CC1S/CC2S
			TIM->CCMR1 &= ~(TIM_CCMR1_CC2S | TIM_CCMR1_CC1S);  // Set CC2/CC1 as outputs
			TIM->CCMR2 &= ~(TIM_CCMR2_CC4S | TIM_CCMR2_CC3S);  // Set CC4/CC3 as outputs
			break;
		default:
			break;
		}
		TIM->CCER &= ~(TIM_CCER_CC4P | TIM_CCER_CC3P | TIM_CCER_CC2P | TIM_CCER_CC1P);  // Active high polarity bit
		TIM->CCR1 = TIM->CCR2 = TIM->CCR3 = TIM->CCR4 = 0;
		IRQn = (tim == TIMER_5) ? TIM5_IRQn : TIM2_IRQn + offset;
		break;
	case TIMER_9:   // GP, 2 CHNs, 16-bit up, No TRGO
	case TIMER_12:  // GP, 2 CHNs, 16-bit up, No TRGO
		if (tim == 9) {
			RCC->APB2ENR |= RCC_APB2ENR_TIM9EN;
			TIM->PSC = CK_PSC_2 / CK_CNT - 1;
			IRQn = TIM1_BRK_TIM9_IRQn;
		}
		else {
			RCC->APB1ENR |= RCC_APB1ENR_TIM12EN;
			TIM->PSC = CK_PSC_1 / CK_CNT - 1;
			IRQn = TIM8_BRK_TIM12_IRQn;
		}
		TIM->ARR = CK_CNT / PWM_FREQ;  // Overflow point
		TIM->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | (pwmMode << TIM_CCMR1_OC1M_0);
		TIM->CCMR1 |= TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1 | (pwmMode << (char) TIM_CCMR1_OC2M_0);  // Choose PWM mode 1 /2
		TIM->CCMR1 |= (TIM_CCMR1_OC1PE | TIM_CCMR1_OC2PE);  //CCR1/CCR2 value is updated only in next update event
		TIM->CCER &= ~(TIM_CCER_CC1E | TIM_CCER_CC2E);  // Disable CC1/CC2 before being able to write CCMR1_CC1S/CC2S
		TIM->CCMR1 &= ~(TIM_CCMR1_CC1S | TIM_CCMR1_CC2S);  // Set CC1/CC2 as outputs
		TIM->CCER &= ~(TIM_CCER_CC2P | TIM_CCER_CC1P);  // Active high polarity bit
		TIM->CCR1 = TIM->CCR2 = 0;
		break;
	case TIMER_10:  // GP, 1 CHN, 16-bit up, , No TRGO
	case TIMER_11:  // GP, 1 CHN, 16-bit up, , No TRGO
	case TIMER_13:  // GP, 1 CHN, 16-bit up, , No TRGO
	case TIMER_14:  // GP, 1 CHN, 16-bit up, , No TRGO
		if (tim <= 11) {
			offset = 17 + (tim - 10);
			RCC->APB2ENR |= 1UL << offset;
			TIM->PSC = CK_PSC_2 / CK_CNT - 1;
			IRQn = tim == 10 ? TIM1_UP_TIM10_IRQn : TIM1_TRG_COM_TIM11_IRQn;
		}
		else {
			offset = 7 + (tim - 13);
			RCC->APB1ENR |= 1UL << offset;
			TIM->PSC = CK_PSC_1 / CK_CNT - 1;
			IRQn = tim == 13 ? TIM8_UP_TIM13_IRQn : TIM8_TRG_COM_TIM14_IRQn;
		}
		TIM->ARR = CK_CNT / (uint16_t) PWM_FREQ - 1;  // Overflow point
		TIM->CCMR1 &= ~TIM_CCMR1_OC1M;  // Reset = Output mode;
		TIM->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | (pwmMode << TIM_CCMR1_OC1M_0);  // Choose PWM mode 1 /2
		TIM->CCMR1 |= TIM_CCMR1_OC1PE;  // CCR value is updated only in next update event
		TIM->CCER &= ~TIM_CCER_CC1E;  // Disable CC1 before being able to write CCMR1_CC1/CC2
		TIM->CCMR1 &= ~TIM_CCMR1_CC1S;  // Set CC1 as outputs
		TIM->CCER &= ~TIM_CCER_CC1P;  // Default active high polarity
		TIM->CCR1 = 0;
		TIM->CCER |= TIM_CCER_CC1E;
//		TIM->CR1 |= TIM_CR1_CEN;
		break;
	default:
		break;  // Bypass TIM6/7 (No output)
	}
	if (trigger_irq) {
		TIM->DIER |= TIM_DIER_UIE;
		//NVIC->IP[IRQn] |= (1 << 4) & 0xFF;
		uint32_t word_offset = IRQn >> 5;  // IRQn / 32
		uint32_t bit_offset = IRQn & 0x1F;  // IRQ mod 32
		NVIC->ISER[word_offset] |= 1UL << bit_offset;
	}
}

void PWM_ENA_CHN(uchar tim, uchar channel) {
	TIM_TypeDef * TIM = TIM_ADDRESS(tim);
	TIM->CCER |= 1 << ((channel - 1) * 4);  // Turn on CCxE
	TIM->CR1 |= TIM_CR1_CEN;
}

void Set_PWM(uchar tim, uchar channel, uint32_t duty) {
	TIM_TypeDef * TIM = TIM_ADDRESS(tim);
	uint32_t arr = TIM->ARR;
	duty *= arr / 100;
	switch (channel) {
	case TIM_CHN_1:
		TIM->CCR1 = duty;
		break;
	case TIM_CHN_2:
		TIM->CCR2 = duty;
		break;
	case TIM_CHN_3:
		TIM->CCR3 = duty;
		break;
	case TIM_CHN_4:
		TIM->CCR4 = duty;
		break;
	default:
		break;
	}
}

void Setup_PWM_Mode(uchar tim, float freq, TRIGGER_IRQ trigger_irq, TIMER_SIGNAL tim_sig) {
	switch (tim) {
	case TIMER_1:
		if (tim_sig == EXTERNAL) {
			GPIO_Init(TIM1_CH1, AF);  // PA8
			GPIO_Init(TIM1_CH2, AF);  // PA9
			GPIO_Init(TIM1_CH3, AF);  // PA10
			GPIO_Init(TIM1_CH4, AF);  // PA11
			GPIO_Set_AF_Mode(TIM1_CH1, AF1, HIGH_SPEED, WEAK);
			GPIO_Set_AF_Mode(TIM1_CH2, AF1, HIGH_SPEED, WEAK);
			GPIO_Set_AF_Mode(TIM1_CH3, AF1, HIGH_SPEED, WEAK);
			GPIO_Set_AF_Mode(TIM1_CH4, AF1, HIGH_SPEED, WEAK);
		}
		PWM_Init(tim, COUNTUP, PWM1, 100000, freq, trigger_irq, tim_sig);
		PWM_ENA_CHN(TIMER_1, TIM_CHN_1);  // See TIM6 Handler
		PWM_ENA_CHN(TIMER_1, TIM_CHN_2);  // See TIM6 Handler
		PWM_ENA_CHN(TIMER_1, TIM_CHN_3);  // See TIM6 Handler
		PWM_ENA_CHN(TIMER_1, TIM_CHN_4);  // See TIM6 Handler
		break;
	case TIMER_2:  // default TRGO for ADC
		PWM_Init(tim, COUNTUP, PWM1, 2000, freq, trigger_irq, tim_sig);
		PWM_ENA_CHN(TIMER_2, TIM_CHN_1);
		Set_PWM(TIMER_2, TIM_CHN_1, 50);  // 1%*arr-pulse width TRGO to trigger ADC
		break;
	case TIMER_3:
		if (tim_sig == EXTERNAL) {
			GPIO_Init(TIM3_CH1, AF);  // PC6
			GPIO_Init(TIM3_CH2, AF);  // PC7
			GPIO_Init(TIM3_CH3, AF);  // PC8
			GPIO_Init(TIM3_CH4, AF);  // PC9
			GPIO_Set_AF_Mode(TIM3_CH1, AF2, HIGH_SPEED, STRONG);
			GPIO_Set_AF_Mode(TIM3_CH2, AF2, HIGH_SPEED, STRONG);
			GPIO_Set_AF_Mode(TIM3_CH3, AF2, HIGH_SPEED, STRONG);
			GPIO_Set_AF_Mode(TIM3_CH4, AF2, HIGH_SPEED, STRONG);
		}
		PWM_Init(tim, COUNTUP, PWM1, 100000, freq, trigger_irq, tim_sig);
		PWM_ENA_CHN(TIMER_3, TIM_CHN_1);
		PWM_ENA_CHN(TIMER_3, TIM_CHN_2);
		PWM_ENA_CHN(TIMER_3, TIM_CHN_3);
		PWM_ENA_CHN(TIMER_3, TIM_CHN_4);
		break;
	case TIMER_4:
		if (tim_sig == EXTERNAL) {
			GPIO_Init(TIM4_CH1, AF);  // = LED pin = PB6
			GPIO_Init(TIM4_CH2, AF);  // = LED pin = PB7
			GPIO_Init(TIM4_CH3, AF);  // = LED pin = PB8
			GPIO_Init(TIM4_CH4, AF);  // = LED pin = PB9
			GPIO_Set_AF_Mode(TIM4_CH1, AF2, HIGH_SPEED, WEAK);  // PB6
			GPIO_Set_AF_Mode(TIM4_CH2, AF2, HIGH_SPEED, WEAK);  // PB7
			GPIO_Set_AF_Mode(TIM4_CH3, AF2, HIGH_SPEED, WEAK);  // PB8
			GPIO_Set_AF_Mode(TIM4_CH4, AF2, HIGH_SPEED, WEAK);  // PB9
		}
		PWM_Init(tim, COUNTUP, PWM1, 100000, freq, trigger_irq, tim_sig);
		PWM_ENA_CHN(TIMER_4, TIM_CHN_1);  // See TIM6 Handler
		PWM_ENA_CHN(TIMER_4, TIM_CHN_2);  // See TIM6 Handler
		PWM_ENA_CHN(TIMER_4, TIM_CHN_3);  // See TIM6 Handler
		PWM_ENA_CHN(TIMER_4, TIM_CHN_4);  // See TIM6 Handler
		break;
	case TIMER_9:
		GPIO_Init(TIM9_CH1, AF);  // PA2
		GPIO_Init(TIM9_CH2, AF);  // PA3
		GPIO_Set_AF_Mode(TIM9_CH1, AF3, HIGH_SPEED, WEAK);
		GPIO_Set_AF_Mode(TIM9_CH2, AF3, HIGH_SPEED, WEAK);
		PWM_Init(tim, COUNTUP, PWM1, 1000000, 1, trigger_irq, tim_sig);
		PWM_ENA_CHN(TIMER_9, TIM_CHN_1);  // See TIM6 Handler
		PWM_ENA_CHN(TIMER_9, TIM_CHN_2);  // See TIM6 Handler
		Set_PWM(TIMER_9, TIM_CHN_1, 1);
		break;
	case TIMER_10:
		GPIO_Init(TIM10_CH1, AF);  // PF6
		GPIO_Set_AF_Mode(TIM10_CH1, AF3, HIGH_SPEED, WEAK);  //LED4 green
		PWM_Init(tim, COUNTUP, PWM1, 100000, freq, trigger_irq, tim_sig);     // See TIM6 Handler
		PWM_ENA_CHN(TIMER_10, TIM_CHN_1);                    // See TIM6 Handler
		Set_PWM(TIMER_10, TIM_CHN_1, 50);
		break;
	default:
		break;
	}
}

Bool is_Timer_Belong_To(uchar tim, uchar *timer_group, uchar size) {
	while (size--) {
		if (tim == *timer_group) return true;
		timer_group++;
	}
	return false;
}

