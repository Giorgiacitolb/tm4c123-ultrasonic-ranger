
#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include <stdint.h>

#define US_NO_ECHO        0u
#define US_OUT_OF_RANGE   0xFFFFFFFFu

void Ultrasonic_Init(void);
uint32_t Ultrasonic_ReadCM(void);

#endif