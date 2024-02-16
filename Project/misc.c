#include "stdio.h"
#include <stm32f4xx.h>
#include "types.h"
#include "misc.h"
#include <stdlib.h>
#include <gpio.h>
#include <io.h>

uchar BCD_To_DEC(uint32_t bcds) {
	uchar portion = 0;
	uchar dec = 0;
	while (bcds) {
		uchar bcd = bcds & 0x0000000F;
		dec += bcd * int_power(10, portion);
		portion++;
		bcds >>= 4;
	}
	return dec;
}

uint32_t int_power(uint32_t x, uint32_t y) {
	if (y == 0) {
		return 1;
	}
	else return x * int_power(x, y - 1);
}

void Int_ToString(char *s, int32_t num, Bool zeroLeading) {
	Bool isPositive = (num >= 0) ? true : false;
	char len = (num == 0) ? 2 : ((num < 10) && isPositive && zeroLeading) ? 1 : 0;
	int32_t n = num;
	while (n) {
		len++;
		n /= 10;
	}
	len += isPositive ? 0 : 1;
	*(s + len) = '\0';
	while (len) {
		*(s + --len) = abs(num) % 10 + '0';
		num /= 10;
	}
	*s = isPositive ? *s : '-';
}

void rotateString(char*str, uchar len) {
	char temp[len];
	for (char i = 0; i < len; i++) {
		temp[(int) i] = *(str + len - i);
	}
	while (--len) {
		*(str + len) = temp[len];
	}
}
