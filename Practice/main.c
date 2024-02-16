#include <stdio.h>
#include <stm32f4xx.h>
#include <types.h>
#include <io.h>
#include <symbol.h>
#include <clock.h>
#include <gpio.h>
#include <timer.h>
#include <interrupt.h>
#include <keypad.h>
#include <stepper.h>
#include "SN74HC595.h"
#include <ultrasonic.h>
#include <LCD_HD44780.h>
#include <rtc.h>
#include <adc.h>
#include "misc.h"

volatile uint16_t exti_line_pressed = 16, irq_pressed = 256;
Bool TIM_HAS_1CHN = false, TIM_HAS_2CHN = false, TIM_HAS_4CHN = false;

void SysTick_Handler(void) {
	EXTI_ReActivate(irq_pressed, exti_line_pressed);
	Systick_Reset();
}

void TIM1_CC_IRQHandler() {
	static Bool stt_test = HIGH;
	if ((TIM1->DIER & TIM_DIER_CC1IE) & (TIM1->SR & TIM_SR_CC1IF)) {
		digitalWrite(TEST_LED_2, stt_test);
		stt_test = stt_test ? LOW : HIGH;
		TIM1->SR &= ~(TIM_SR_CC1IF | TIM_SR_CC1OF);
	}
	if ((TIM1->DIER & TIM_DIER_CC2IE) & (TIM1->SR & TIM_SR_CC2IF)) {
		digitalWrite(TEST_LED_2, stt_test);
		stt_test = stt_test ? LOW : HIGH;
		TIM1->SR &= ~(TIM_SR_CC2IF | TIM_SR_CC2OF);
	}
	if ((TIM1->DIER & TIM_DIER_CC3IE) & (TIM1->SR & TIM_SR_CC3IF)) {
		digitalWrite(TEST_LED_2, stt_test);
		stt_test = stt_test ? LOW : HIGH;
		TIM1->SR &= ~(TIM_SR_CC3IF | TIM_SR_CC3OF);
	}
	if ((TIM1->DIER & TIM_DIER_CC4IE) & (TIM1->SR & TIM_SR_CC4IF)) {
		digitalWrite(TEST_LED_2, stt_test);
		stt_test = stt_test ? LOW : HIGH;
		TIM1->SR &= ~(TIM_SR_CC4IF | TIM_SR_CC4OF);
	}
	if (TIM1->SR & (TIM_SR_UIF | TIM_SR_TIF)) {
		TIM1->SR &= ~(TIM_SR_UIF | TIM_SR_TIF);
	}
}

void TIM3_IRQHandler() {  // CC mode test and control stepper
	/********************Input Capture Mode*********************/
//	static Bool stt_test = HIGH;
//	if ((TIM3->DIER & TIM_DIER_CC1IE) & (TIM3->SR & TIM_SR_CC1IF)) {
//		digitalWrite(TEST_LED_2, stt_test);
//		stt_test = stt_test ? LOW : HIGH;
//		TIM3->SR &= ~(TIM_SR_CC1IF | TIM_SR_CC1OF);
//	}
//	if ((TIM3->DIER & TIM_DIER_CC2IE) & (TIM3->SR & TIM_SR_CC2IF)) {
//		digitalWrite(TEST_LED_2, stt_test);
//		stt_test = stt_test ? LOW : HIGH;
//		TIM3->SR &= ~(TIM_SR_CC2IF | TIM_SR_CC2OF);
//	}
//	if ((TIM3->DIER & TIM_DIER_CC3IE) & (TIM3->SR & TIM_SR_CC3IF)) {
//		digitalWrite(TEST_LED_2, stt_test);
//		stt_test = stt_test ? LOW : HIGH;
//		TIM3->SR &= ~(TIM_SR_CC3IF | TIM_SR_CC3OF);
//	}
//	if ((TIM3->DIER & TIM_DIER_CC4IE) & (TIM3->SR & TIM_SR_CC4IF)) {
//		digitalWrite(TEST_LED_2, stt_test);
//		stt_test = stt_test ? LOW : HIGH;
//		TIM3->SR &= ~(TIM_SR_CC4IF | TIM_SR_CC4OF);
//	}
	/************Microstepping Stepper Motor with  UEV*************/
	if (TIM3->SR & TIM_SR_UIF) {
		static volatile uint16_t STEP = 1, microstep = 1;
		if (microstep == (MICROSTEP_COEFFICIENT + 1)) {
			microstep = 1;
			STEP = (STEP == (PAIRS_OF_PHASES * 2)) ? 1 : (STEP + 1);
		}
		else {
			uint16_t STEP_by_dir =
			(STEPPER_DIR == CLOCKWISE) ? STEP : (PAIRS_OF_PHASES * 2 - STEP + 1);
			uint16_t microstep_by_dir =
			(STEPPER_DIR == CLOCKWISE) ? microstep : (MICROSTEP_COEFFICIENT - microstep);
			Stepper_MicroMove(TIMER_3, STEP_by_dir, microstep_by_dir, MICROSTEP_COEFFICIENT);
			microstep++;
		}
		TIM3->SR &= ~TIM_SR_UIF;
	}
}

void TIM6_DAC_IRQHandler(void) {  // KPAD_TIMER
	TIM6->SR &= 0;
	volatile static Bool toggle_led2 = HIGH;
	digitalWrite(LED2, toggle_led2);
	toggle_led2 = toggle_led2 ? LOW : HIGH;

	if (TIM_HAS_1CHN) {
		static volatile int brightness_1 = 1;
		static volatile int step_1 = 1;
		if ((brightness_1 > 99) || (brightness_1 <= 0)) step_1 = -step_1;
		brightness_1 += step_1;
		Set_PWM(PWM_TEST_TIMER, TIM_CHN_1, brightness_1);
	}
	if (TIM_HAS_2CHN) {
		static volatile int brightness_2 = 8;
		static volatile int step_2 = 1;
		if ((brightness_2 > 99) || (brightness_2 <= 0)) step_2 = -step_2;
		brightness_2 += step_2;
		Set_PWM(PWM_TEST_TIMER, TIM_CHN_2, brightness_2);
	}
	if (TIM_HAS_4CHN) {
		static volatile int brightness_3 = 16, brightness_4 = 24;
		static volatile int step_3 = 1, step_4 = 1;
		if ((brightness_3 > 99) || (brightness_3 <= 0)) step_3 = -step_3;
		if ((brightness_4 > 99) || (brightness_4 <= 0)) step_4 = -step_4;
		brightness_3 += step_3;
		brightness_4 += step_4;
		Set_PWM(PWM_TEST_TIMER, TIM_CHN_3, brightness_3);
		Set_PWM(PWM_TEST_TIMER, TIM_CHN_4, brightness_4);
	}
}

void TIM7_IRQHandler(void) {  // Default as System Timer for Delay. Wont be call if chosen as SYSTEM_DELAY
	TIM7->SR &= 0;
	volatile static Bool toggle_led1 = HIGH;
	digitalWrite(LED1, toggle_led1);
	digitalWrite(TEST_LED_1, toggle_led1 ? LOW : HIGH);
	toggle_led1 = toggle_led1 ? LOW : HIGH;
}

void TIM1_BRK_TIM9_IRQHandler() {
	if ((TIM9->DIER & TIM_DIER_CC1IE) & (TIM9->SR & TIM_SR_CC1IF)) {
		static Bool stt_test = HIGH;
		digitalWrite(TEST_LED_2, stt_test);
		stt_test = stt_test ? LOW : HIGH;
		TIM9->SR &= ~(TIM_SR_CC1IF | TIM_SR_CC1OF);
	}
	else if ((TIM9->DIER & TIM_DIER_CC2IE) & (TIM9->SR & TIM_SR_CC2IF)) {
		static Bool stt_test = HIGH;
		digitalWrite(TEST_LED_2, stt_test);
		stt_test = stt_test ? LOW : HIGH;
		TIM9->SR &= ~(TIM_SR_CC2IF | TIM_SR_CC2OF);
	}
	if (TIM9->SR & (TIM_SR_UIF | TIM_SR_TIF)) {
		TIM9->SR &= ~(TIM_SR_UIF | TIM_SR_TIF);
	}
}

void TIM2_IRQHandler() {  // Check TRGO validity of ADC1 and restart ADC1
//	if (TIM2->SR & TIM_SR_CC1IF) {
//		static Bool stt_test_trgo = HIGH;
//		digitalWrite(TEST_LED_3, stt_test_trgo);
//		stt_test_trgo = stt_test_trgo ? LOW : HIGH;
//		TIM2->SR &= ~(TIM_SR_CC1IF | TIM_SR_CC1OF);
//	}
	if (TIM2->SR & TIM_SR_UIF) {
		ADC1P.counter = 0;
		ADC_Restart(ADC_1);
		TIM2->SR &= ~TIM_SR_UIF;
	}
}

void TIM1_UP_TIM10_IRQHandler() {
	if (TIM10->SR & TIM_SR_CC1IF) {
		static Bool stt_test = HIGH;
		digitalWrite(TEST_LED_2, stt_test);
		stt_test = stt_test ? LOW : HIGH;
		TIM10->SR &= ~(TIM_SR_CC1IF | TIM_SR_CC1OF);
	}
	if (TIM10->SR & TIM_SR_UIF) {
		TIM10->SR &= ~TIM_SR_UIF;
	}
}

void TIM1_TRG_COM_TIM11_IRQHandler() {  // Capture response pulse from echo pin
	static volatile uint32_t last_captured, signal_polarity, overFlow;
	static Bool echo_edge_detected = false;

	if (TIM11->SR & TIM_SR_CC1IF) {
		uint32_t current_capture = TIM11->CCR1;
		signal_polarity = 1 - signal_polarity;

		if (signal_polarity) {
			echo_edge_detected = true;
			last_captured = current_capture;
		}
		else {
			echo_pulse_width = current_capture - last_captured + 65536 * overFlow;  // uS
			overFlow = 0;
			echo_edge_detected = false;
		}

		TIM11->SR &= ~(TIM_SR_CC1OF | TIM_SR_CC1IF);
	}
	if (TIM11->SR & TIM_SR_UIF) {
		overFlow += echo_edge_detected ? 1 : 0;
		TIM11->SR &= ~TIM_SR_UIF;
	}
}

void EXTI3_IRQHandler() {  // Everytime press button will send 10us-pulse width to trigger pin to command ultrasonic to measure
	irq_pressed = EXTI3_IRQn;
	exti_line_pressed = 3;
	Debounce_EXTI(irq_pressed, exti_line_pressed, 50);
	TIM_TypeDef * Ultrasonic_Trig = TIM_ADDRESS(Ultrasonic_Trig_TIM);
	Ultrasonic_Trig->CR1 |= TIM_CR1_CEN;
	EXTI->PR |= 1 << exti_line_pressed;
}

void EXTI4_IRQHandler() {
	irq_pressed = EXTI4_IRQn;
	exti_line_pressed = 4;
	Debounce_EXTI(irq_pressed, exti_line_pressed, 200);
	static volatile Bool lcd_screen_stt = OFF;
	LCD_SetDisplay(LCD_DISPLAY_MODE | (lcd_screen_stt ? LCD_DISPLAY_ON : LCD_DISPLAY_OFF));
	lcd_screen_stt = lcd_screen_stt ? OFF : ON;
	RTC_Init();
//	TIM_TypeDef * TIM = TIM_ADDRESS(SYSTEM_TIMER);
//	static volatile uint16_t blink_period;
//	blink_period += blink_period == 1000 ? -900 : 100;  // blink period
//	TIM->CR1 &= ~TIM_CR1_CEN;
//	TIM->CNT = 0;
//	TIM->SR = 0;
//	TIM->ARR = blink_period / 0.5 - 1;
//	TIM->CR1 |= TIM_CR1_CEN;
	EXTI->PR |= 1UL << exti_line_pressed;
}

void EXTI15_10_IRQHandler() {
	irq_pressed = EXTI15_10_IRQn;
	/**************************EXTI_11****************************/
	volatile Bool Rotary_B_logic = digitalRead(ROTARY_B);
	if (EXTI->PR & (1UL << 11)) {       // Rotary_A -> Exti_line = 11
		exti_line_pressed = 11;
		Debounce_EXTI(irq_pressed, exti_line_pressed, 250);
		if (!Rotary_B_logic) {
			page++;
			digitalWrite(TEST_LED_3, HIGH);
		}
		else if (Rotary_B_logic) {
			digitalWrite(TEST_LED_3, LOW);
			page--;
		}

		if (page < MIN_PAGE) page = MAX_PAGE;
		else if (page > MAX_PAGE) page = MIN_PAGE;
	}
	/**************************EXTI_10****************************/
	else if (EXTI->PR & (1UL << 10)) {  // Rotary_SW -> Exti_line = 10
		exti_line_pressed = 10;
		Debounce_EXTI(irq_pressed, exti_line_pressed, 300);
		Bool *dir_ptr = &STEPPER_DIR;
		*dir_ptr = (*dir_ptr == CLOCKWISE) ? COUNTER_CLOCKWISE : CLOCKWISE;
	}
	/*************************EXTI_12-15***************************/
	else {
		readkey_done = false;
		RowPressed = ColPressed = 0xFF;
		for (uint16_t i = 0; i < KPAD_COL; i++) {
			if (!digitalRead(COL_PIN[i])) {
				ColPressed = i;
				exti_line_pressed = 12 + ColPressed;  // Col0 - Col3 -> Exti_line = 12 - 15
				break;
			}
		}
		Debounce_EXTI(irq_pressed, exti_line_pressed, 20);
		if (ColPressed != 0xFF) {
			for (uint16_t i = 0; i < KPAD_ROW; i++) {
				GPIO_Init(ROW_PIN[i], INPUT);
			}
			for (uint16_t i = 0; i < KPAD_ROW; i++) {
				GPIO_Init(ROW_PIN[i], OUTPUT);
				digitalWrite(ROW_PIN[i], LOW);
				if (!digitalRead(COL_PIN[ColPressed])) {
					RowPressed = i;
					key_unread = true;
					break;
				}
				else key_unread = false;
				GPIO_Init(ROW_PIN[i], INPUT);
			}
		}
		readkey_done = true;
		all_row_output_low = false;
	}
	EXTI->PR |= (1UL << exti_line_pressed);
}

void ADC_IRQHandler() {
	if ((ADC1->SR & ADC_SR_EOC) == ADC_SR_EOC) {
		ADC1P.raw[ADC1P.counter] = ADC1->DR;
		ADC1P.counter++;
		if (!(ADC1P.counter % ADC1P.conv_len)) {
			ADC1P.counter = 0;
			ADC1P.finish_flag = true;
		}
	}
}

void Display_Distance_HCSR04(uint32_t pulse_width) {  //in us
	distance = pulse_width * 10 / 58;  //us - > cm, x10 = mm
	uint16_t hundreds = distance / 1000;
	uint16_t tens = distance / 100 % 10;
	uint16_t ones = distance / 10 % 10;
	uint16_t one_fraction = distance % 10;
	HC595_Write_7SEG(3, (uchar) hundreds, NO_DP, hundreds ? ON : OFF);
	HC595_Write_7SEG(2, (uchar) tens, NO_DP, hundreds ? ON : tens ? ON : OFF);
	HC595_Write_7SEG(1, (uchar) ones, DP, ON);
	HC595_Write_7SEG(0, (uchar) one_fraction, NO_DP, ON);
}

int main(void) {
	TIM_HAS_1CHN = is_Timer_Belong_To(PWM_TEST_TIMER, TIMERS_1CHN, sizeof(TIMERS_1CHN)) | is_Timer_Belong_To(
	               PWM_TEST_TIMER, TIMERS_2CHN, sizeof(TIMERS_2CHN)) | is_Timer_Belong_To(
	               PWM_TEST_TIMER, TIMERS_4CHN, sizeof(TIMERS_4CHN));
	TIM_HAS_2CHN = is_Timer_Belong_To(PWM_TEST_TIMER, TIMERS_2CHN, sizeof(TIMERS_2CHN)) | is_Timer_Belong_To(
	               PWM_TEST_TIMER, TIMERS_4CHN, sizeof(TIMERS_4CHN));
	TIM_HAS_4CHN = is_Timer_Belong_To(PWM_TEST_TIMER, TIMERS_4CHN, sizeof(TIMERS_4CHN));

	Select_Clock_Source(CLK_SRC);

	Bunk_Init(OUTPUT_PIN, sizeof(OUTPUT_PIN) / MAX_PIN_CHAR, OUTPUT);
	Bunk_Init(INPUT_PIN, sizeof(INPUT_PIN) / MAX_PIN_CHAR, INPUT);
	Bunk_Init(ADC1_PIN, sizeof(ADC1_PIN) / MAX_PIN_CHAR, ANALOG);
	digitalWrite(LED1, HIGH);           // PF9
	digitalWrite(LED2, HIGH);           // PF10
	digitalWrite(TEST_LED_1, LOW);      // PG3
	digitalWrite(TEST_LED_2, LOW);      // PG4
	digitalWrite(TEST_LED_3, LOW);      // PG5
	digitalWrite(HSI_RED_LED, HIGH);    // PG0
	digitalWrite(HSE_GREEN_LED, HIGH);  // PG1
	digitalWrite(PLL_BLUE_LED, HIGH);   // PG2
	digitalWrite(PLL_BLUE_LED, LOW);   // PG2
	digitalWrite(HC595_DS, LOW);        // PF2
	digitalWrite(HC595_LCK, LOW);       // PF0
	digitalWrite(HC595_SCK, LOW);       // PF1

	EXTI_Init(EXTI4_IRQn, BTN_K0, BTN_K0_PRI, RISING);             // EXTI_4
	EXTI_Init(EXTI3_IRQn, BTN_K1, BTN_K1_PRI, RISING);             // EXTI_3
	EXTI_Init(EXTI15_10_IRQn, ROTARY_SW, ROTARY_SW_PRI, RISING);  // EXTI_10
	EXTI_Init(EXTI15_10_IRQn, ROTARY_A, ROTARY_A_PRI, FALLING);    // EXTI_11
	EXTI_Init(EXTI15_10_IRQn, KPAD_C0, KPAD_C0_PRI, FALLING);      // EXTI_12
	EXTI_Init(EXTI15_10_IRQn, KPAD_C1, KPAD_C1_PRI, FALLING);      // EXTI_13
	EXTI_Init(EXTI15_10_IRQn, KPAD_C2, KPAD_C2_PRI, FALLING);      // EXTI_14
	EXTI_Init(EXTI15_10_IRQn, KPAD_C3, KPAD_C3_PRI, FALLING);      // EXTI_15

	Confirm_Clock_Source();                 // RED: HSI 16Mhz, GREEN: HSE 8Mhz, BLUE: PLL 168Mhz
	Basic_Timer_Init(SYSTEM_TIMER, DELAY);  // TIM7 for delay function
	Basic_Timer_Init(KPAD_TIMER, IRQ);      // TIM6 for changing freq of led when pressing kpads
	Trigger_Timer_IRQ_MS(20);               // Changing ARR of TIM6
	//Setup_IC_Mode(ICM_TEST_TIMER);
	Setup_PWM_Mode(PWM_TEST_TIMER, 200, NO_IRQ, EXTERNAL);
	Setup_PWM_Mode(STEPPER_TIMER, MICROSTEPPING_16, TRG_IRQ, EXTERNAL);
	Setup_PWM_Mode(ADC_TRIG_TIMER, 0.25, TRG_IRQ, INTERNAL);
	ADC_Init(ADC_1, ADC1_SEQ, sizeof(ADC1_SEQ) / sizeof(uint16_t), TIMER_TRIGGER);
	Ultrasonic_HCSR04_Init(ULTRASONIC_TRIG_PIN, ULTRASONIC_ECHO_PIN);  // Press K1 to measure. // TIM10_CH1 -> F6: Make 10us width trigger pulse, // TIM11_CH1 -> F7: Capture response pulse from echo pin
	LCD_Init();
//	Confirm_Set_Mode_Success(TIM4_CH1, AF2, TEST_LED);

	while (1) {
		pad = Keypad_Scan();
		React_On_KeyPressed(&pad);
		Display_Distance_HCSR04(echo_pulse_width);
		Update_RTC_Clock(&rtc_dt);
		Periodic_Read_ADC(&ADC1P);
		LCD_Display(page);
	}
}
