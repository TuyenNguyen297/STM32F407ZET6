#include <stdio.h>
#include <stm32f4xx.h>
#include "types.h"
#include "gpio.h"
#include "io.h"
#include "timer.h"
#include "keypad.h"
#include "symbol.h"
#include "LCD_HD44780.h"

volatile Bool readkey_done = true, all_row_output_low = false, key_unread = false;
volatile uint16_t RowPressed = 0xFF, ColPressed = 0xFF, blink_interval_ms = 20;
char pad = 0xFF;

char Keypad_Scan() {

	if (readkey_done && !all_row_output_low) {
		for (uint16_t i = 0; i < KPAD_ROW; i++) {
			GPIO_Init(ROW_PIN[i], OUTPUT);
			digitalWrite(ROW_PIN[i], LOW);
		}
		all_row_output_low = true;
	}

	if (key_unread) {
		key_unread = false;
		return KPAD_ARRAY[RowPressed][ColPressed];
	}
	return ' ';
}

void React_On_KeyPressed(char *c) {
	if (*c != ' ') {
		page = 5;
		blink_interval_ms = 0;
		if ((*c >= '0') && (*c <= '9')) {
			blink_interval_ms = (*c - '0') * 20;
		}
		else if (*c >= 'A' && *c <= 'D') {
			blink_interval_ms = 1000 + (*c - 'A') * 1000;
		}
		else {
			switch (*c) {
				case '*':
					blink_interval_ms = 5000;
					break;
				case '#':
					blink_interval_ms = 6000;
					break;
				default:
					break;
			}
		}
		Trigger_Timer_IRQ_MS(blink_interval_ms);
	}
}
