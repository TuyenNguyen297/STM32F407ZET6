#ifndef IO_H
#define IO_H

/***************************LEDS***************************/
#define HSI_RED_LED "G0"  //
#define PLL_BLUE_LED "G2" //
#define HSE_GREEN_LED "G1" //
#define LED1 "F9"  // Onboard KED D1,
#define LED2 "F10" // Onboard KED D2,
#define TEST_LED_1 "G3" // Blinking test for Timer 7 IRQ,
#define TEST_LED_2 "G4" // Test Input Capture IRQ with corresponding ICPSC/ICFILTER of Input Capture mode of all Timer,
#define TEST_LED_3 "G5" // Test Ultrasonic,
#define BUZZER "G14" //
/************************74HC595 FOR 7SEG*************************/
#define HC595_SCK "F0" //
#define HC595_LCK "F1" //
#define HC595_DS "F2" //
/***************************16X02 LCD****************************/
#define LCD_RS "D10"//
#define LCD_E "D11"//
#define LCD_D4 "D12"//
#define LCD_D5 "D13"//
#define LCD_D6 "D14"//
#define LCD_D7 "D15"//
/*****************************BUTTONS****************************/
#define BTN_K1 "E3" //
#define BTN_K0 "E4" //
/***************************4X4 KEYPAD***************************/
#define KPAD_R0 "E0" //
#define KPAD_R1 "E1" //
#define KPAD_R2 "E2" //
#define KPAD_R3 "E5" //
#define KPAD_C0 "F12" //
#define KPAD_C1 "F13" //
#define KPAD_C2 "F14" //
#define KPAD_C3 "F15" //
/*************************ROTARY ENCODER*************************/
#define ROTARY_A "G11" //
#define ROTARY_B "G13" //
#define ROTARY_SW "G10" //
/****************************TIMER_1*****************************/

#define TIM1_CH1 "A8" // Test PWM/IC Mode TIM1 4CHNs. No wired
#define TIM1_CH2 "A9" // Test PWM/IC Mode TIM1 4CHNs. No wired
#define TIM1_CH3 "A10" // Test PWM/IC Mode TIM14CHNs. No wired
#define TIM1_CH4 "A11" // Test PWM/IC Mode TIM1 4CHNs. No wired
/****************************TIMER_3*****************************/

#define TIM3_CH1 "C6" // Test PWM/IC-Mode/Stepper Motor TIM3 4CHNs.
#define TIM3_CH2 "C7" // Test PWM/IC-Mode/Stepper Motor TIM3 4CHNs.
#define TIM3_CH3 "C8" // Test PWM/IC-Mode/Stepper Motor TIM3 4CHNs.
#define TIM3_CH4 "C9" // Test PWM/IC-Mode/Stepper Motor TIM3 4CHNs.
#define STEPPER_PA_PIN TIM3_CH1
#define STEPPER_NA_PIN TIM3_CH2
#define STEPPER_PB_PIN TIM3_CH3
#define STEPPER_NB_PIN TIM3_CH4

/****************************TIMER_4*****************************/

#define TIM4_CH1 "B6" // Test PWM/IC Mode TIM4 4CHNs.
#define TIM4_CH2 "B7" // Test PWM/IC Mode TIM4 4CHNs.
#define TIM4_CH3 "B8" // Test PWM/IC Mode TIM4 4CHNs.
#define TIM4_CH4 "B9" // Test PWM/IC Mode TIM4 4CHNs.
/****************************TIMER_9*****************************/

#define TIM9_CH1 "A2" // Test PWM/IC Mode TIM9 2CHNs
#define TIM9_CH2 "A3" // Test PWM/IC Mode TIM9 2CHNs
/*******************ULTRASONIC, TIMER10/11***********************/

#define TIM10_CH1 "F6" // HCSR04 Trigger pin.
#define TIM11_CH1 "F7" // HCSR04 Echo pin.
#define ULTRASONIC_TRIG_PIN TIM10_CH1
#define ULTRASONIC_ECHO_PIN TIM11_CH1

/****************************ADC1********************************/
#define AMT_HUMIDI "A1" // ADC123_1
#define AMT_TEMPER "A2" // ADC123_2
#endif
