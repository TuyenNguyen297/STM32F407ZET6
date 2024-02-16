#ifndef ULTRASONIC_H
#define ULTRASONIC_H

extern volatile uint32_t echo_pulse_width;
extern volatile uint16_t distance;
void Ultrasonic_HCSR04_Init(char*, char*);
void Display_Distance_HCSR04();

#endif
