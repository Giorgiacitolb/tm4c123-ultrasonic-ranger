
#ifndef SWITCH_H
#define SWITCH_H

#include <stdint.h>

void Switch_Init(void);
uint8_t Switch_ReadRaw(void);
uint8_t Switch_ReadDebounced(void);
void Switch_Test(void);

#endif
