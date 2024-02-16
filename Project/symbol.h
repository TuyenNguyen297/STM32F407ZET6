#ifndef SYMBOL_H
#define SYMBOL_H
#include "types.h"

extern uchar TIMERS_0CHN[2], TIMERS_1CHN[4], TIMERS_2CHN[2], TIMERS_4CHN[6];
extern uchar _7SEG_LED[4], _7SEG_DIG[10];
extern uchar FullStep[4], HalfStep[8];
extern char KPAD_ARRAY[KPAD_ROW][KPAD_COL];
extern char *ROW_PIN[4], *COL_PIN[4];
extern char *CTRL_PIN[2], *DATA_PIN[4];
extern char *INPUT_PIN[9], *OUTPUT_PIN[18];
extern char *ADC1_PIN[2];
extern uint16_t ADC1_SEQ[2];

#endif
