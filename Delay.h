#ifndef DELAY_H
#define DELAY_H
#include <stdint.h>

void Delay_Init(void);
void Delay_Us(uint32_t us);   // 16MHz
void Delay_Ms(uint32_t ms);   // 16MHz

#endif