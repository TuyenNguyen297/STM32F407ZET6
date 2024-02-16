#include <stdio.h>
#include "types.h"
#include "io.h"

/*************************************TIMERS CLASSIFY***********************************/
uchar TIMERS_0CHN[2] = { TIMER_6, TIMER_7 };
uchar TIMERS_1CHN[4] = { TIMER_10, TIMER_11, TIMER_13, TIMER_14 };
uchar TIMERS_2CHN[2] = { TIMER_9, TIMER_12 };
uchar TIMERS_4CHN[6] = { TIMER_1, TIMER_2, TIMER_3, TIMER_4, TIMER_5, TIMER_8 };

/******************************************I/Os*********************************************/
char *INPUT_PIN[9] = { BTN_K0,
                       BTN_K1,
                       KPAD_C0,
                       KPAD_C1,
                       KPAD_C2,
                       KPAD_C3,
                       ROTARY_A,
                       ROTARY_B,
                       ROTARY_SW };
char *OUTPUT_PIN[18] = { LED1,
                         LED2,
                         TEST_LED_1,
                         TEST_LED_2,
                         TEST_LED_3,
                         HSI_RED_LED,
                         HSE_GREEN_LED,
                         PLL_BLUE_LED,
                         BUZZER,
                         HC595_SCK,
                         HC595_LCK,
                         HC595_DS,
                         LCD_RS,
                         LCD_E,
                         LCD_D4,
                         LCD_D5,
                         LCD_D6,
                         LCD_D7 };  //STEPPER_PA_PIN, STEPPER_NA_PIN, STEPPER_PB_PIN,STEPPER_NB_PIN,

/**************************************KEYPAD 4x4***************************************/
char KPAD_ARRAY[KPAD_ROW][KPAD_COL] = { { '1', '2', '3', 'A' }, { '4', '5', '6', 'B' }, { '7', '8', '9', 'C' }, { '*', '0', '#', 'D' } };
char *ROW_PIN[4] = { KPAD_R0, KPAD_R1, KPAD_R2, KPAD_R3 };
char *COL_PIN[4] = { KPAD_C0, KPAD_C1, KPAD_C2, KPAD_C3 };

/**************************************LCD 16x02***************************************/
char *CTRL_PIN[2] = { LCD_RS, LCD_E };
char *DATA_PIN[4] = { LCD_D4, LCD_D5, LCD_D6, LCD_D7 };

/***************************4 LEDS SERIAL WITH 2 74HC595********************************/
uchar _7SEG_LED[4] = { 0x01, 0x02, 0x04, 0x08 };
uchar _7SEG_DIG[10] = { 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F };

/*************************************STEPPER MOTOR*************************************/
uchar FullStep[4] = { 0x9, 0xA, 0x6, 0x5 };  // PANB PAPB NAPB NANB: 0b1001, 0b1010, 0b0110, 0b0101
uchar HalfStep[8] = { 0x9, 0x8, 0xA, 0x2, 0x6, 0x4, 0x5, 0x1 };  // PANB PA PAPB PB NAPB NA NANB NB: 0b1001, 0b1000, 0b1010,0b0010,0b0110,0b0100,0b0101,0b0001

/*****************************************ADC*****************************************/
char *ADC1_PIN[2] = { AMT_HUMIDI, AMT_TEMPER };  // ADC_CH1, ADC_CH2
uint16_t ADC1_SEQ[2] = {ADC_CHN_1, ADC_CHN_2};

