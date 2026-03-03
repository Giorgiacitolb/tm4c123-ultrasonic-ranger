
#ifndef SYSTICK_H
#define SYSTICK_H

#include <stdint.h>

void SysTick_Init_1ms(uint32_t busClockHz);
uint32_t SysTick_Millis(void);
void DelayMs(uint32_t ms);

// Red blink control (PF1 toggles at 2Hz: 0.25s on / 0.25s off)
void RedBlink_Enable(void);
void RedBlink_Disable(void);

#endif

