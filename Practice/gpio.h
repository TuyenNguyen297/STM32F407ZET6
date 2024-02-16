#ifndef GPIO_H
#define GPIO_H

GPIO_TypeDef * GPIO_ADDRESS(unsigned int);
uint16_t pinLength(char *);
uint16_t pinBank(char *);
void GPIO_Init(char *, char);
void Bunk_Init(char *[], uint16_t, uint32_t);
Bool GPIO_Set_AF_Mode(char *, uint32_t, uint32_t, char);
void digitalWrite(char*, Bool);
void Bunk_DigitalWrite(char*[], uint16_t, Bool);
Bool digitalRead(char *);
void Confirm_Set_Mode_Success(char *, char, char*);
void Beep_Once();
void Beep_Twice();

#endif
