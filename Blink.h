#ifndef BLINK_H
#define BLINK_H
#include <stdint.h>

void Blink_Init(void);         // Timer3A interrupt every 250ms
void Blink_RedEnable(void);
void Blink_RedDisable(void);

#endif
