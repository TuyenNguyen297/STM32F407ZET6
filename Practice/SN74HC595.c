#include <stm32f4xx.h>
#include "types.h"
#include "gpio.h"
#include "io.h"
#include "symbol.h"

void Shift_Data_MSBFIRST(uchar data) {
	for (int i = 0; i < 8; i++) {
		digitalWrite(HC595_DS, (data & 0x80) ? HIGH : LOW);
		digitalWrite(HC595_SCK, LOW);
		digitalWrite(HC595_SCK, HIGH);
		data <<= 1;
	}
}

void HC595_Write_7SEG(uchar led, uint16_t digit, Bool dp, Bool display) {
	uchar led_off = 0x00, repeat_time = 10;
	for (char i = 0; i < repeat_time; i++) {
		Shift_Data_MSBFIRST(~_7SEG_LED[led]);
		Shift_Data_MSBFIRST(display ? _7SEG_DIG[digit] | (dp ? 0x80 : 0x00) : led_off);
		digitalWrite(HC595_LCK, LOW);
		digitalWrite(HC595_LCK, HIGH);
	}
}
