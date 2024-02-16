#ifndef INTERRUPT_H
#define INTERRUPT_H

void EXTI_Init(uint16_t, char *, uint32_t, uint16_t);
void EXTI_DeActivate(uint16_t, uint16_t);
void EXTI_ReActivate(uint16_t, uint16_t);
void Debounce_EXTI(uint16_t, uint16_t, uint16_t);

#endif
