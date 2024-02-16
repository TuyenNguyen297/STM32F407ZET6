#include <stdio.h>
#include <stm32f4xx.h>
#include "types.h"
#include "LCD_HD44780.h"
#include "io.h"
#include "symbol.h"
#include "gpio.h"
#include "timer.h"
#include <rtc.h>
#include <adc.h>
#include <ultrasonic.h>
#include "keypad.h"

volatile uchar page = 1;

void LCD_Latch(char* lt_pin) {
	digitalWrite(lt_pin, HIGH);
	delay_us(40);
	digitalWrite(lt_pin, LOW);
	delay_us(40);
}

void LCD_Clear() {
	LCD_Command(LCD_CLEAR_SCREEN);
	delay_us(1640);
}

void LCD_Clear_Row_From_Col(uchar row, uchar col) {

	uint16_t len = 15 - col + 1;
	char extraSpace[len];
	while (len) {
		extraSpace[--len] = ' ';
	}
	LCD_SetCursor(row, col);
	LCD_Print(extraSpace);
}

void LCD_SetEntry(uchar cmd) {
	LCD_Command(cmd);
	delay_us(40);
}

void LCD_SetDisplay(uchar cmd) {
	LCD_Command(cmd);
	delay_us(40);
}

void LCD_SetFunction(uchar cmd, DELAY_TYPE delay_type, double delay_time) {
	LCD_Command(cmd);
	void (*delay_ptr)(double) = (delay_type == US ) ? &delay_us : &delay_ms;
	(*delay_ptr)(delay_time);
}

void LCD_Init() {
	delay_us(30000);
	LCD_SetFunction(LCD_BUS_8, US, 4100);  // Must have 4100
	LCD_SetFunction(LCD_BUS_8, US, 160);  // Must have 160
	LCD_SetFunction(LCD_BUS_8, US, 160);  // Must have 160

	LCD_SetFunction(LCD_BUS_LENGTH, US, 40);
	LCD_SetFunction(LCD_BUS_LENGTH, US, 40);
	LCD_SetFunction(LCD_BUS_LENGTH | LCD_2_LINES | LCD_SMALL_FONT, US, 40);
	LCD_SetFunction(LCD_BUS_LENGTH | LCD_2_LINES | LCD_SMALL_FONT, US, 40);

	LCD_SetDisplay(LCD_DISPLAY_MODE);
	LCD_Clear();
	LCD_SetEntry(LCD_ENTRY_MODE | LCD_ENTRY_INCREMENT_MODE | LCD_ENTRY_SHIFT_OFF);
	LCD_SetDisplay(LCD_DISPLAY_MODE | LCD_DISPLAY_ON | LCD_CURSOR_OFF | LCD_CURSOR_BLINKOFF);
	LCD_Clear();
}

void LCD_Command(uchar cmd) {
	digitalWrite(LCD_RS, LOW);  //Choose to write to Instruction register
	LCD_PutData(cmd);
}

void LCD_Print(char* str) {
	digitalWrite(LCD_RS, HIGH);  //Choose to write to Instruction register
	while (*str != '\0') {
		LCD_PutData(*str++);
	}
}

void LCD_PutData(uchar data) {
	uint16_t write_cycle, data_pins;
	switch (LCD_BUS_LENGTH) {
		case LCD_BUS_4:
			write_cycle = 2;
			data_pins = 4;
			break;
		case LCD_BUS_8:
			write_cycle = 1;
			data_pins = 8;
			break;
		default:
			break;
	}
	for (int i = (write_cycle - 1); i >= 0; i--) {
		uchar nibble = (data >> (4 * i)) & 0x0F;  // HIGHER NIBBLE is written prior to the LOWER NIBBLE
		for (int j = data_pins - 1; j >= 0; j--) {
			if (nibble & (1 << j)) digitalWrite(DATA_PIN[j], HIGH);
			else digitalWrite(DATA_PIN[j], LOW);
		}
		LCD_Latch(LCD_E);
	}
	delay_us(500);
}

void LCD_SetCursor(uchar row, uchar col) {
	uchar address = row * 0x40 + col;  // row0 start at 0x00, row1 from 0x40
	LCD_Command(LCD_DDRAM_SELECT + address);
}

void LCD_Shift(DIRECTION dir) {
	if ((dir == LEFT) || (dir == RIGHT)) {
		uint16_t cmd;
		switch (dir) {
			case LEFT:
				cmd = LCD_SHIFT_DISPLAY_LEFT;
				break;
			case RIGHT:
				cmd = LCD_SHIFT_DISPLAY_RIGHT;
				break;
			default:
				break;
		}
		LCD_Command(cmd);
	}
}

void LCD_Print_2D_String(char (*p_ch)[][18], uint16_t str_len, uchar (*p_xy)[][2]) {  // Using to print 2D array of n elements of max-7-len strings at given coordinate xy[row][col] (col = 0/1) to LCD)
	for (uchar i = 0; i < str_len; i++) {
		LCD_SetCursor((*p_xy)[i][0], (*p_xy)[i][1]);
		LCD_Print((*p_ch)[i]);
	}
}

void LCD_Display(uchar lcd_page) {  // Display all info of project
	static uchar prev_page = 0;
	void (*p_print)(char (*)[][18], uint16_t, uchar (*)[][2]) = &LCD_Print_2D_String;
	char str[18];
	char (*p_ch)[][18];
	uchar (*p_xy)[][2];
	uchar c_len;
	static Bool page_refresh = false;

	if (prev_page != lcd_page) {
		page_refresh = true;
		LCD_Clear();
		switch (lcd_page) {
			case 1: {  //HCSR04 + RTC
				LCD_SetCursor(0, 0);		// HCSR04
				LCD_Print("Distance: ");
				static char p1_ch[][18] = { ",", "/", "/", " ", ":" };  // weekday, day, month, year, hour, minute, second. second wont be displayed
				static uchar p1_xy[][2] = { { 1, 2 }, { 1, 5 }, { 1, 8 }, { 1, 10 }, { 1, 13 } };
				c_len = 5;
				p_ch = &p1_ch;
				p_xy = &p1_xy;
				break;
			}
			case 2: {  //AMT1001
				static char p2_ch[][18] = { "AMT:", "HUMI", "TMPT", "%", "C" };
				static uchar p2_xy[][2] = { { 0, 0 }, { 0, 5 }, { 0, 11 }, { 1, 9 }, { 1, 15 } };
				c_len = 5;
				p_ch = &p2_ch;
				p_xy = &p2_xy;
				break;
			}
			case 3: {  // Internal
				static char p3_ch[][18] = { "TMPT", "VREF", "VBAT", "C", "V", "V" };
				static uchar p3_xy[][2] = { { 0, 0 }, { 0, 5 }, { 0, 11 }, { 1, 2 }, { 1, 9 }, { 1, 15 } };
				c_len = 6;
				p_ch = &p3_ch;
				p_xy = &p3_xy;
				break;
			}
			case 4: {
				static char p4_ch[][18] = { "STEPPER:F=9000Hz", "MODE: MICRO/16" };
				static uchar p4_xy[][2] = { { 0, 0 }, { 1, 0 } };
				c_len = 2;
				p_ch = &p4_ch;
				p_xy = &p4_xy;
				break;
			}
			case 5: {
				static char p5_ch[][18] = { "Keypad Pressed:", "LED2+PWM:", "Hz" };
				static uchar p5_xy[][2] = { { 0, 0 }, { 1, 0 }, { 1, 14 } };
				c_len = 3;
				p_ch = &p5_ch;
				p_xy = &p5_xy;
				break;
			}
		}
		(*p_print)(p_ch, c_len, p_xy);
		prev_page = lcd_page;
	}

//	};  //3 are internal channels
	switch (lcd_page) {
		case 1: {  //HCSR04 + RTC
			static uint16_t prev_distance = 0;
			if ((prev_distance != distance) || page_refresh) {
				sprintf(str, "%dmm", distance);
				LCD_Clear_Row_From_Col(0, 10);
				LCD_SetCursor(0, 10);
				LCD_Print(str);
				prev_distance = distance;
			}
			Display_RTC_Clock(&rtc_dt, page_refresh);
			page_refresh = false;
			break;
		}
		case 2: {  //as case 3
		}
		case 3:
			{
			//	static float prev_processed[] = { 0, 0, 0, 0, 0 };
			uchar lcd_adc_pos[5][2] = { { 1, 5 }, { 1, 11 }, { 1, 0 }, { 1, 5 }, { 1, 11 } };
			for (uint16_t i = ((lcd_page == 2) ? 0 : 2); i < ((lcd_page == 2) ? 2 : 5); i++) {
				//	if (prev_processed[i] != ADC1P.processed[i]) {
				sprintf(ADC1P.str, ADC1P.str_format[i], ADC1P.processed[i]);
				LCD_SetCursor(lcd_adc_pos[i][0], lcd_adc_pos[i][1]);
				LCD_Print(ADC1P.str);
				//		prev_processed[i] = ADC1P.processed[i];
				//	}
			}
			ADC1P.finish_flag = false;
			break;
		}
		case 4:
			break;
		case 5: {
			static char prev_pad = ' ';
			if ((prev_pad != pad) || page_refresh) {
				char printable[6];
				sprintf(printable, "%c", prev_pad);
				LCD_SetCursor(0, 15);
				LCD_Print(printable);
				LCD_SetCursor(1, 9);
				float frequency = blink_interval_ms ? (1000.0 / blink_interval_ms) : 0;
				sprintf(printable, "%.2f", frequency);
				LCD_Print(printable);
				prev_pad = pad;
				page_refresh = false;
			}
			break;
		}
	}
}
