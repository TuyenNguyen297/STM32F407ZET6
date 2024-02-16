#include <stdio.h>
#include <stm32f4xx.h>
#include <core_cm4.h>
#include "types.h"
#include "io.h"
#include "gpio.h"
#include "timer.h"

GPIO_TypeDef * GPIO_ADDRESS(unsigned int bank) {
	return (GPIO_TypeDef*) (AHB1PERIPH_BASE + (bank >> 8) * (GPIOB_BASE - AHB1PERIPH_BASE));
}

uint16_t pinLength(char *pin) {
	uint16_t length = 0;
	while (*pin++ != '\0') {
		length++;
	}
	return length;
}

uint16_t pinBank(char *pin) {
	char bank = *pin, pinNo;
	uint16_t gpioPin;
	uint16_t pinL = pinLength(pin);

	if (pinL == 2) {
		pinNo = *(pin + 1) - '0';
	}
	else if (pinL == 3) {
		pinNo = (*(pin + 1) - '0') * 10 + (*(pin + 2) - '0');  // Convert Pin string to number
	}
	if ((bank >= 'A') && (bank <= 'I')) {
		gpioPin = ((bank - 'A') << 8) | pinNo;
	}
	return gpioPin;
}

void GPIO_Init(char *pin, char mode) {
	uint16_t bank = pinBank(pin);
	uint16_t pinNo = bank & 0xFF;
	GPIO_TypeDef *GPIO = GPIO_ADDRESS(bank);
	RCC->AHB1ENR |= 1 << (bank >> 8);
	GPIO->MODER &= ~(3UL << (pinNo << 1));  // clear old Mode
	GPIO->MODER |= (mode & 3) << (pinNo << 1);  // Set mode, << 1 = x2
	GPIO->PUPDR &= ~(3UL << (pinNo << 1));  // Clears PUPDR to 00 also means sets pin no pull up no pull down

	switch (mode) {
	case INPUT:
		GPIO->PUPDR |= 1UL << (pinNo << 1);  //Weak Pull up
		break;
	case OUTPUT:
		GPIO->OTYPER &= ~(1UL << pinNo);  // Set push-pull output type
		GPIO->OSPEEDR &= ~(3UL << (pinNo << 1));  //Set Output speed to Low Speed
		break;
	case AF:
		GPIO->OTYPER |= 1 << pinNo;  // Set open-drain output type;
		//GPIO->OTYPER &= ~(1UL << pinNo); // Set push-pull output type;
		break;
	case ANALOG:
		break;
	default:
		break;
	};
}

void Bunk_Init(char *pin_array[], uint16_t length, uint32_t mode) {
	void (*ptr)(char*, char) = &GPIO_Init;
	while (length--) {
		(*ptr)(pin_array[length], mode);
	}
}

Bool GPIO_Set_AF_Mode(char *pin, uint32_t af, uint32_t speed, char pull_type) {
	uint16_t bank = pinBank(pin);
	uint16_t pinNo = bank & 0xFF;
	GPIO_TypeDef *GPIO = GPIO_ADDRESS(bank);

	if (GPIO->MODER & (2UL << (pinNo << 1))) {
		GPIO->OSPEEDR &= ~(3UL << (pinNo << 1));
		GPIO->OSPEEDR |= speed << (pinNo << 1);
		switch (pull_type) {
		case WEAK:  // No need external pull-up resistor.
			GPIO->PUPDR |= (1 << (pinNo << 1));  // 1 << (pinNo *2) // Internal pull-up
			break;
		case STRONG:  // Already set to no pull up no pull down in GPIO_Init. Need pulling up by external several of K resistor < 10k.
		default:
			break;
		}
		if (af < 16) {
			GPIO->AFR[pinNo >> 3] &= ~(0xF << ((pinNo & 0x7) << 2));  // GPIO->AFR[pinNo / 8] &= ~(0xF << ((pinNo mod 8) * 4));
			GPIO->AFR[pinNo >> 3] |= af << ((pinNo & 0x7) << 2);     // GPIO->AFR[pinNo / 8] &= ~(af << ((pinNo mod 8) * 4));
//			if (pinNo < 8) {
//				GPIO->AFR[0] &= ~(0xF << (pinNo * 4));
//				GPIO->AFR[0] |= af << (pinNo * 4);
//			}
//			else {
//				GPIO->AFR[1] &= ~(0xF << ((pinNo - 8) * 4));
//				GPIO->AFR[1] |= af << ((pinNo - 8) * 4);
//			}
		}
		else {
			return false;
		}
		return true;
	}
	return false;
}

void digitalWrite(char *pin, Bool bit) {
	uint16_t bank = pinBank(pin);
	uint16_t pinNo = bank & 0xFF;
	GPIO_TypeDef *GPIO = GPIO_ADDRESS(bank);
	switch (bit) {
	case HIGH:
		GPIO->BSRRL |= (1 << pinNo);
		GPIO->BSRRH &= ~(1 << pinNo);
		//GPIO->ODR |= (1 << pinNo);
		break;
	case LOW:
		GPIO->BSRRL &= ~(1 << pinNo);
		GPIO->BSRRH |= (1 << pinNo);
		//GPIO->ODR &= ~(1 << pinNo);
		break;
	default:
		break;
	}
}
void Bunk_DigitalWrite(char *pin_array[], uint16_t length, Bool bit) {
	void (*ptr)(char*, Bool) = &digitalWrite;
	while (length--) {
		(*ptr)(pin_array[length], bit);
	}
}

Bool digitalRead(char* pin) {
	uint16_t bank = pinBank(pin);
	uint16_t pinNo = bank & 0xFF;
	GPIO_TypeDef *GPIO = GPIO_ADDRESS(bank);
	return ((GPIO->IDR & (1UL << pinNo)) ? HIGH : LOW);
}

void Confirm_Set_Mode_Success(char *pin, char af, char* led) {
	uint16_t bank = pinBank(pin);
	uint16_t pinNo = bank & 0xFF;
	GPIO_TypeDef *GPIO = GPIO_ADDRESS(bank);
	uint32_t bitOffset = (pinNo & 0x7) << 2;
	uint32_t pinAF = af << bitOffset;
	if ((GPIO->AFR[pinNo >> 3] & (0xF << bitOffset)) == pinAF) digitalWrite(led, HIGH);  // AF << ((pin mod 8) * 4)
}

void Beep_Once() {
	digitalWrite(BUZZER, HIGH);
	delay_us(65000);
	digitalWrite(BUZZER, LOW);
}

void Beep_Twice() {
	for (int i = 0; i < 2; i++) {
		digitalWrite(BUZZER, HIGH);
		delay_us(40000);
		digitalWrite(BUZZER, LOW);
		delay_us(40000);
	}
}
