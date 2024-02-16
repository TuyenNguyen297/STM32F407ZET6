#ifndef TIMER_H
#define TIMER_H

TIM_TypeDef * TIM_ADDRESS(uchar);

void Systick_Init(uint32_t, Bool);
void Systick_Reset(void);

void Basic_Timer_Init(uchar, TIMER_ROLE);
void Trigger_Timer_IRQ_MS( uint16_t);
void delay_ms(double);
void delay_us(double);

void PWM_Init(uchar, uchar, uint16_t, uint32_t, float, TRIGGER_IRQ, TIMER_SIGNAL);
void PWM_ENA_CHN(uchar, uchar);
void Set_PWM(uchar, uchar, uint32_t);
void Setup_PWM_Mode(uchar, float, TRIGGER_IRQ, TIMER_SIGNAL);

void Capture_Init(uchar, uchar, uchar, uchar, uint16_t, uint16_t);
void Setup_IC_Mode(uchar);

Bool is_Timer_Belong_To(uchar, uchar *, uchar);

#endif
