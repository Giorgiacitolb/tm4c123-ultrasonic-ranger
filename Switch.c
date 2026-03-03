
#include "tm4c123gh6pm.h"
#include "Switch.h"
#include "SysTick.h"
#include <stdint.h>
#include "Delay.h"


#define SW1_MASK 0x10  // PF4

void Switch_Init(void){
  SYSCTL_RCGCGPIO_R |= 0x20;               // Port F
  volatile uint32_t d = SYSCTL_RCGCGPIO_R; (void)d;

  GPIO_PORTF_DIR_R &= ~SW1_MASK;
  GPIO_PORTF_DEN_R |=  SW1_MASK;
  GPIO_PORTF_PUR_R |=  SW1_MASK;
}

uint8_t Switch_ReadRaw(void){
  // pressed when PF4 reads 0
  return ((GPIO_PORTF_DATA_R & SW1_MASK) == 0) ? 0x01 : 0x00;
}


uint8_t Switch_ReadDebounced(void){
  uint8_t a = Switch_ReadRaw();
  Delay_Ms(10);
  uint8_t b = Switch_ReadRaw();
  return (a == b) ? b : 0;
}