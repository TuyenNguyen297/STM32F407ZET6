#ifndef ADC_H
#define ADC_H

#define ADC_EXTSEL_TIM2_TRGO 0x06
#define VREF 3.02

//extern volatile uint16_t adc_counter;
//extern volatile uint16_t adc1_result[];
//extern volatile Bool adc_conv_finished[];
//extern volatile uint16_t adc_result_single;

typedef struct __attribute__((__packed__)) {
	char str[8];
	char str_format[5][8];
	ADC_VAL conv_order[5];
	volatile uint16_t counter :4;
	uint16_t conv_len :4;
	volatile Bool finish_flag :1;
	volatile uint16_t raw[5];
	float processed[5];
} ADC_Property;

extern ADC_Property ADC1P;

void ADC_Init(char, uint16_t[], uint16_t, ADC_TRIGGER_SRC);
void ADC_Restart(char);
void List_Conv_Order_To_SQR(ADC_TypeDef **, uint16_t, uint16_t[], uint16_t);
void List_Smp_Time_To_SMP(ADC_TypeDef **, uint16_t, uint16_t);

float Convert_ADC( uint16_t, ADC_VAL);
float Voltage_From_ADC( uint16_t);

void Periodic_Read_ADC();
void Display_ADC_Result();  // Not using

#endif
