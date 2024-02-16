#include <stm32f4xx.h>
#include "types.h"
#include "timer.h"
#include "misc.h"
#include "rtc.h"
#include "symbol.h"
#include "LCD_HD44780.h"
#include "gpio.h"

DateTime rtc_dt;

void RTC_Init() {
	/************************************Clock Initialization*************************************/
	RCC->APB1ENR |= RCC_APB1ENR_PWREN;  // Enable Power Interface clock
	delay_us(100);

	while (!(RCC->APB1ENR & RCC_APB1ENR_PWREN))
		;
	PWR->CR |= PWR_CR_DBP;  // Enable backup domain write access;
	while (!(PWR->CR & PWR_CR_DBP))
		;

	RCC->BDCR &= ~(RCC_BDCR_LSEON | RCC_BDCR_LSEBYP);  // Turn off all LSE LSEBYP before configuring LSE
	RCC->BDCR |= RCC_BDCR_BDRST;				// RTC Clock selection is only changed if domain backup is prior reset
	RCC->BDCR &= ~RCC_BDCR_BDRST;

	RCC->BDCR |= RCC_BDCR_LSEON;			// Wait until LSE is selected as RTC clock
	while (!(RCC->BDCR & RCC_BDCR_LSERDY))
		;
	RCC->BDCR &= ~RCC_BDCR_RTCSEL;
	RCC->BDCR |= RTC_CLOCK_BY_LSE;
	RCC->APB1ENR &= ~RCC_APB1ENR_PWREN;
	RCC->BDCR |= RCC_BDCR_RTCEN;
	/************************************Clock Initialization*************************************/

	RTC->WPR = 0xCA;
	RTC->WPR = 0x53;

	RTC->ISR |= RTC_ISR_INIT;
	while (!(RTC->ISR & RTC_ISR_INITF))
		;

	RTC->CR &= ~(RTC_CR_FMT | RTC_CR_BYPSHAD);  // | RTC_CR_BYPSHAD Format time as 24hs, calendar values are taken from shadow registers instead of directly read from calendar registers
	RTC->PRER = RTC_PRESCALER_RESET;
	RTC->PRER |= ((1UL << 8) - 1);			// Program Synchronous prescaler first
	RTC->PRER |= ((1UL << 7) - 1) << 16;      // Then program Asynchronous prescaler

	DateTime preset_dt = { 6, 28, 1, 2024, 23, 02, 0 };
	RTC->TR = (0UL << 22) | (preset_dt.hour / 10 << 20) | (preset_dt.hour % 10 << 16) | (preset_dt.minute / 10 << 12) | (preset_dt.minute % 10 << 8) | (preset_dt.second / 10 << 4) | preset_dt.minute % 10;  // 10:10:10 AM
	RTC->DR = (preset_dt.year / 10 % 10 << 20) | (preset_dt.year % 10 << 16) | (preset_dt.weekday << 13) | (preset_dt.month / 10 << 12) | (preset_dt.month % 10 << 8) | (preset_dt.day / 10 << 4) | preset_dt.day % 10;  // Wed, 10/01/24

	RTC->ISR &= ~RTC_ISR_INIT;
	RTC->WPR = 0xFF;  // Input wrong password to re-activate write protection mode

	delay_us(1000);
	if (RTC->ISR & RTC_ISR_INITS) {
		Beep_Twice();
	}
}

void Update_RTC_Clock(DateTime *rtc_dtp) {
	if (RTC->ISR & RTC_ISR_INITS) {
		if (RTC->ISR & RTC_ISR_RSF) {
			static DateTime past, *p;
			uchar hour, min, sec, weekday, month, day;
			uint16_t year;
			p = &past;
			uint32_t time_reg = RTC->TR;
			uint32_t date_reg = RTC->DR;
			hour = BCD_To_DEC((time_reg & (RTC_TR_HT | RTC_TR_HU)) >> 16);
			min = BCD_To_DEC((time_reg & (RTC_TR_MNT | RTC_TR_MNU)) >> 8);
			sec = BCD_To_DEC(time_reg & (RTC_TR_ST | RTC_TR_SU));
			year = 2000 + BCD_To_DEC((date_reg & (RTC_DR_YT | RTC_DR_YU)) >> 16);
			weekday = BCD_To_DEC((date_reg & RTC_DR_WDU) >> 13);
			month = BCD_To_DEC((date_reg & (RTC_DR_MT | RTC_DR_MU)) >> 8);
			day = BCD_To_DEC(date_reg & (RTC_DR_DT | RTC_DR_DU));
			if (p->second != sec) rtc_dtp->second = p->second = sec;
			if (p->minute != min) rtc_dtp->minute = p->minute = min;
			if (p->hour != hour) rtc_dtp->hour = p->hour = hour;
			if (p->year != year) rtc_dtp->year = p->year = year;
			if (p->weekday != weekday) rtc_dtp->weekday = p->weekday = weekday;
			if (p->month != month) rtc_dtp->month = p->month = month;
			if (p->day != day) rtc_dtp->day = p->day = day;
			RTC->ISR &= ~RTC_ISR_RSF;
		}
	}
}

void Display_RTC_Clock(DateTime * real_dtp, Bool refresh) {
	char WeekDay[][3] = { " ", "T3", "T4", "T5", "T6", "T7", "CN", "T2" };
	char cursorPos[] = { 0, 3, 6, 9, 11, 14 };  // weekday, day, month, year, hour, minute
	Bool zeroLeading[] = { 0, 1, 1, 0, 1, 1 };  // weekday, day, month, year, hour, minute
//	char divider[][2] = { ",", "/", "/", " ", ":", " " };  // weekday, day, month, year, hour, minute
	uint16_t current[] = { real_dtp->weekday,
	                       real_dtp->day,
	                       real_dtp->month,
	                       real_dtp->year,
	                       real_dtp->hour,
	                       real_dtp->minute };
	uint16_t currentSize = sizeof(current) / sizeof(uint16_t);
	static uint16_t prev[sizeof(current)];
	static Bool firstInit = false;

	for (uint16_t i = 0; i < currentSize; i++) {
		if ((prev[i] != current[i]) || !firstInit || refresh) {
			char stringFromInt[10];
			LCD_SetCursor(1, cursorPos[i]);
			if (i > 0) {
				Int_ToString(stringFromInt, (i == 3) ? (current[i] - 2020) : current[i], zeroLeading[i]);
			}
			LCD_Print((i == 0) ? WeekDay[current[0]] : stringFromInt);  //
			//LCD_Print(divider[i]); // already done in Display Static LCD
			prev[i] = current[i];
		}
	}
	if (!firstInit) firstInit = true;
	if (refresh) refresh = false;
}

