#include <stdio.h>
#include <math.h>
#include <stm32f4xx.h>
#include "types.h"
#include "io.h"
#include "symbol.h"
#include "gpio.h"
#include "timer.h"
#include "stepper.h"

Bool STEPPER_DIR = CLOCKWISE;

void Stepper_NoneMicroMove(STEPPER_MODE stepper_mode, float round, uint16_t freq, Bool dir) {
	uchar *Step_Mode = (stepper_mode == FULLSTEP) ? FullStep : HalfStep;
	uint16_t steps_per_revolution = 360 / 1.8 / 4;  // 1.8°/step, need 4 consecutive step each sweepping
	float steps_to_round = steps_per_revolution * round;
	uint16_t step_delay_time = 1000 / freq;  // T = 1/f * 1000 (ms)
	uint16_t i = 0, j = 0;

	while (j++ <= steps_to_round) {
		i = 0;
		while (i < (PAIRS_OF_PHASES * 2 * stepper_mode)) {  //1 phase has 2 polars
			uint16_t step_order = (dir == CLOCKWISE) ? i : (4 * stepper_mode - 1) - i;
			Bool Widing_PA_Logic = (Step_Mode[step_order] & 0x8) >> 3;
			Bool Widing_NA_Logic = (Step_Mode[step_order] & 0x4) >> 2;
			Bool Widing_PB_Logic = (Step_Mode[step_order] & 0x2) >> 1;
			Bool Widing_NB_Logic = (Step_Mode[step_order] & 0x1);
			digitalWrite(STEPPER_PA_PIN, Widing_PA_Logic);
			digitalWrite(STEPPER_NA_PIN, Widing_NA_Logic);
			digitalWrite(STEPPER_PB_PIN, Widing_PB_Logic);
			digitalWrite(STEPPER_NB_PIN, Widing_NB_Logic);
			i++;
			delay_ms(step_delay_time);
		}
	}
}

void Stepper_NoneMicroMove_Once(STEPPER_MODE stepper_mode, short int step) {
	uint16_t STEP_LIMIT = stepper_mode == FULLSTEP ? 3 : 7;
	if (step == STEP_LIMIT)
	step = 0;
	else
	step++;

	if (step == -1)
	step = STEP_LIMIT;
	else
	step--;

	unsigned char *Step_Mode = (stepper_mode == FULLSTEP) ? FullStep : HalfStep;
	Bool Widing_PA_Logic = (Step_Mode[step] & 0x8) >> 3;
	Bool Widing_NA_Logic = (Step_Mode[step] & 0x4) >> 2;
	Bool Widing_PB_Logic = (Step_Mode[step] & 0x2) >> 1;
	Bool Widing_NB_Logic = (Step_Mode[step] & 0x1);
	digitalWrite(STEPPER_PA_PIN, Widing_PA_Logic);
	digitalWrite(STEPPER_NA_PIN, Widing_NA_Logic);
	digitalWrite(STEPPER_PB_PIN, Widing_PB_Logic);
	digitalWrite(STEPPER_NB_PIN, Widing_NB_Logic);
}

void Stepper_DePower() {
	char *Stepper_Pins[] = { STEPPER_PA_PIN, STEPPER_NA_PIN, STEPPER_PB_PIN, STEPPER_NB_PIN };
	Bunk_DigitalWrite(Stepper_Pins, sizeof(Stepper_Pins) / MAX_PIN_CHAR, OFF);
}

void Stepper_MicroMove(uchar tim, uint16_t STEP, float step, float micro_divider) {
	TIM_TypeDef * TIM = TIM_ADDRESS(tim);
	float alpha = (STEP % 2) ? (step / micro_divider * M_PI_2) : ((micro_divider - step) / micro_divider * M_PI_2);
	uint16_t peak = TIM->ARR;	//
	uint16_t sina = roundf(sinf(alpha) * peak);  // sina goes from 0 - ARR
	uint16_t cosa = roundf(cosf(alpha) * peak);  // cosa goes from 0 - ARR
	switch (STEP) {
		case STEP_1:
			TIM->CCR1 = sina;  // PA
			TIM->CCR4 = cosa;  // NB
			break;
		case STEP_2:
			TIM->CCR1 = sina;  // PA
			TIM->CCR3 = cosa;  // PB
			break;
		case STEP_3:
			TIM->CCR2 = sina;  // NA
			TIM->CCR3 = cosa;  // PB
			break;
		case STEP_4:
			TIM->CCR2 = sina;  // NA
			TIM->CCR4 = cosa;  // NB
			break;
	}
}
