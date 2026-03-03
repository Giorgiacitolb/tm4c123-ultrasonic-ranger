
#ifndef LED_H
#define LED_H

#include <stdint.h>

#define LED_RED   0x02  // PF1
#define LED_BLUE  0x04  // PF2
#define LED_GREEN 0x08  // PF3

void LED_Init(void);
void LED_Set(uint8_t mask);   // PF1=0x02 PF2=0x04 PF3=0x08
void LED_Off(void);
//void LED_Test(void);

#endif
