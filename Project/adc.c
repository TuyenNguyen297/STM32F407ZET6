#include <stdio.h>
#include <stm32f4xx.h>
#include "types.h"
#include "gpio.h"
#include "adc.h"
#include "io.h"
#include "math.h"
#include "symbol.h"
#include "LCD_HD44780.h"

/*Vc(t) = Vin * (1- e^(-t/Tc))
 * Tc = (Rain + Radc)* Cadc = (Rain + 6000)* 4*10^-12
 * Vc ~ Vin for high accuracy result which is depending on "t"
 * */

/* fADC = 84/8 = 10.5 Mhz
 * min sampling time for 3 internal channel is 10us
 * Ts = 1/fADC x cycle => Min cycles = 10 / 1 / 10.5 = 105. Choose 144 cycles 0x06
 * */

ADC_Property ADC1P = { "",
                       { "%.1f%%", "%.1fC", "%.0fC", "%.2fV", "%.2fV" },  // formats of AMT_HUMI, AMT_TEMP, INTERNAL_TEMP, INTERNAL_VREF, INTERNAL_VBATE
                       { AMT_HUMI, AMT_TEMP, INTERNAL_TEMP, INTERNAL_VREF, INTERNAL_VBATE },
                       0,
                       sizeof(ADC1_SEQ) / sizeof(uint16_t) + 3,
                       false,
                       { 0, 0, 0, 0, 0 },
                       { 0, 0, 0, 0, 0 }
};  //3 are internal channels

void ADC_Init(char adc_module, uint16_t conversion_seq[], uint16_t conversion_length,
ADC_TRIGGER_SRC src) {
	uint32_t adc_address = ADC1_BASE + (uint32_t) (0x00000100 * adc_module);
	ADC_TypeDef * ADC_n = (ADC_TypeDef*) adc_address;

	void (*list_sqr_ptr)(ADC_TypeDef**, uint16_t, uint16_t[], uint16_t) = &List_Conv_Order_To_SQR;
	void (*list_smp_ptr)(ADC_TypeDef**, uint16_t, uint16_t) = &List_Smp_Time_To_SMP;
	Bool internal_measurement = (adc_module == ADC_1) ? true : false;
	uint16_t nth_conversion = 0;

	RCC->APB2ENR |= (1UL << (8 + adc_module));  // 8 is offset of ADC1EN
	ADC_n->CR2 &= ~(ADC_CR2_ADON | ADC_CR2_EXTEN | ADC_CR2_EXTSEL);
	ADC->CCR &= ~(ADC_CCR_ADCPRE | ADC_CCR_MULTI);  // Independent single mode
	ADC->CCR |= ((SLOW << 16) & ADC_CCR_ADCPRE) | ADC_CCR_DELAY;  //ADC prescaler for all ADCs. delay 20 ADCCLK cycles before starting the next sampling process
	ADC_n->CR1 &= ~(ADC_CR1_RES | ADC_CR1_DISCEN | ADC_CR1_OVRIE);  //  12-bit resolution as default
	ADC_n->CR1 |= ADC_CR1_SCAN | ADC_CR1_EOCIE;
	ADC_n->CR2 &= ~(ADC_CR2_CONT | ADC_CR2_ALIGN);  //  discontinue,Right align
	ADC_n->CR2 |= ADC_CR2_EOCS;
	switch (src) {
	case TIMER_TRIGGER:
		ADC_n->CR2 |= ADC_CR2_EXTEN_1;  // trigger on rising edge
		ADC_n->CR2 |= ADC_EXTSEL_TIM2_TRGO << 24;  // TIM2 TRGO is external trigger as default
		break;
	case SOFTWARE_TRIGGER:
		default:
		break;
	}
	ADC_n->SQR1 &= ~ADC_SQR1_L;
	ADC_n->SQR1 |= (((conversion_length - 1) + (internal_measurement ? 3 : 0)) << 20);

	while ((nth_conversion < conversion_length) && (nth_conversion < 16)) {  // maximum 18 channel
		(*list_sqr_ptr)(&ADC_n, nth_conversion, conversion_seq, 0);
		nth_conversion++;
	}
	if (internal_measurement) {
		ADC->CCR |= ADC_CCR_TSVREFE | ADC_CCR_VBATE;  // ADC contains 3 common registers of 3 ADCs (CSR/CCR/CDR).// internal temperature, vbat, vref are internally connected to ADC1_CH16->18
		uint16_t internal_chns[] = { ADC_INTERNAL_TEMP, ADC_INTERNAL_VREF, ADC_INTERNAL_VBAT };
		for (uint16_t i = 0; i < sizeof(internal_chns) / sizeof(uint16_t); i++) {
			(*list_sqr_ptr)(&ADC_n, nth_conversion, internal_chns, nth_conversion - i);
			nth_conversion++;
		}
	}
	for (uint16_t i = 0; i < 19; i++) {
		(*list_smp_ptr)(&ADC_n, i, (i < 16) ? 7UL : 7UL);  // 0x06 = 144 cycles for sampling. 0x05 = 112 cycles for sampling = 112 * 1 / 10.5 = 10.6 us
	}

	uint16_t word_offset = ADC_IRQn >> 5;  // IRQn / 32
	uint16_t bit_offset = ADC_IRQn & 0x1F;  // IRQ mod 32

	NVIC->ISER[word_offset] |= 1UL << bit_offset;
	ADC_n->CR2 |= ADC_CR2_ADON;
}

float Convert_ADC(uint16_t adc, ADC_VAL adc_type) {
	float (*p_convert)( uint16_t) = &Voltage_From_ADC;
	float voltage = (*p_convert)(adc);
	switch (adc_type) {
	case AMT_HUMI:
		return voltage / 0.03;
	case AMT_TEMP: {
		uint16_t R1 = 10000;
		uint16_t R2 = 14970;
		float Beta = 3435, R0 = 10000, T25K = 298.15;
		float NTC_Resistor = (R1 + R2) / (VREF / (VREF - voltage * (R1 / R2 + 1)) - 1);
		return (1.0 / (1.0 / Beta * log(NTC_Resistor / R0) + 1 / T25K) - 273.15);
	}
	case INTERNAL_TEMP: {
		//uint16_t *TS_CAL1 = (uint16_t*) 0x1FFF7A2C;  // calib at 30*C, 910
		//uint16_t *TS_CAL2 = (uint16_t*) 0x1FFF7A2E;  // calib at 110*C, 1195
		//float refactor_avgSlope = (*TS_CAL1 * 3.3 / 4095.0 - 0.76) * 1000.0 / (30.0 - 25.0);  // 3.3 and 30 is tested condition at manufacture
		return (voltage - 0.76) * 1000.0 / 2.5 + 25.0;
	}
	case INTERNAL_VREF:
		return voltage;
	case INTERNAL_VBATE:
		return voltage * 2;
	default:
		return 0;
	}
}

float Voltage_From_ADC(uint16_t adc) {
	return (float) adc * VREF / 4095.0;
}

void ADC_Restart(char adc_module) {
	ADC_TypeDef * ADC_n = (ADC_TypeDef*) (ADC1_BASE + (uint32_t) (0x00000100 * adc_module));
	ADC_n->CR2 &= ~ADC_CR2_ADON;
	ADC_n->CR2 |= ADC_CR2_ADON;
}

uint16_t ADC_Get_Value(char adc_module) {
	uint32_t adc_address = ADC1_BASE + (uint32_t) (0x00000100 * adc_module);
	ADC_TypeDef * ADC_n = (ADC_TypeDef*) adc_address;
	return ADC_n->DR;
}

void List_Conv_Order_To_SQR(ADC_TypeDef ** ADC_n, uint16_t nth_conversion,
uint16_t conversion_seq[], uint16_t merge_offset) {
	uint16_t reg_offset = nth_conversion / 6;  // each word has 6 sequences
	uint16_t seq_offset = (nth_conversion % 6) * 5;  // find start bit of sequence. a word has max 6 sequences, a sequence has 5 bits.
	__IO uint32_t *ADC_SQR_N;
	switch (reg_offset) {
	case 0:
		ADC_SQR_N = &((*ADC_n)->SQR3);
		break;
	case 1:
		ADC_SQR_N = &((*ADC_n)->SQR2);
		break;
	case 2:
		ADC_SQR_N = &((*ADC_n)->SQR1);
		break;
	default:
		break;
	}
	*ADC_SQR_N |= (conversion_seq[nth_conversion - merge_offset] & 0x1F) << seq_offset;
}
void List_Smp_Time_To_SMP(ADC_TypeDef ** ADCn, uint16_t nth_chn, uint16_t sampling_time_level) {
	uint16_t reg_offset = nth_chn / 10;  // each word has 6 sequences
	uint16_t chn_offset = (nth_chn % 10) * 3;  // find start bit of sequence. a word has max 6 sequences, a sequence has 5 bits.
	__IO uint32_t *ADC_SMP_N;
	switch (reg_offset) {
	case 0:
		ADC_SMP_N = &((*ADCn)->SMPR2);
		break;
	case 1:
		ADC_SMP_N = &((*ADCn)->SMPR1);
		break;
	default:
		break;
	}
	*ADC_SMP_N |= (sampling_time_level & 7UL) << chn_offset;
}

void Periodic_Read_ADC(ADC_Property * ADC_n) { // Update ADC processed array each time adc finished a sequence of 5 conversion (each 5s when COUNTER = CCR of Timer 2 as internal TRGO trigger to ADC module)
	if (ADC_n->finish_flag) {
		for (uint16_t i = 0; i < ADC_n->conv_len; i++) {
			ADC_n->processed[i] = Convert_ADC(ADC_n->raw[i], ADC_n->conv_order[i]);
		}
	}
}

void Display_ADC_Result(ADC_Property * ADC_n) { // Not using. It is currently alternated by LCD_Display(page) for all info
	if (ADC_n->finish_flag) {
		uchar lcd_pos[5][2] = { { 0, 0 }, { 0, 5 }, { 0, 11 }, { 1, 5 }, { 1, 11 } };
		for (uint16_t i = 0; i < ADC_n->conv_len; i++) {
			sprintf(ADC_n->str, ADC_n->str_format[i], ADC_n->processed[i]);
			LCD_SetCursor(lcd_pos[i][0], lcd_pos[i][1]);
			LCD_Print(ADC_n->str);
			ADC_n->finish_flag = false;
		}
	}
}

