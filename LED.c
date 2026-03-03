
#include "tm4c123gh6pm.h"
#include "LED.h"
#include "Systick.h"

void LED_Init(void){
  SYSCTL_RCGCGPIO_R |= 0x20;           // Port F
  volatile uint32_t d = SYSCTL_RCGCGPIO_R; (void)d;

  GPIO_PORTF_DIR_R |= 0x0E;            // PF1-3 outputs
  GPIO_PORTF_DEN_R |= 0x0E;
  GPIO_PORTF_AFSEL_R &= ~0x0E;
  GPIO_PORTF_AMSEL_R &= ~0x0E;

  LED_Off();
}

void LED_Set(uint8_t mask){
  GPIO_PORTF_DATA_R = (GPIO_PORTF_DATA_R & ~0x0E) | (mask & 0x0E);
}

void LED_Off(void){
  LED_Set(0);
}

//Not using Test Modules anymore, full integration testing now
/*void LED_Test(void){
  while(1){
    LED_Set(0x02); Timer1_WaitMs(250); // red
    LED_Set(0x04); Timer1_WaitMs(250); // blue
    LED_Set(0x08); Timer1_WaitMs(250); // green
    LED_Set(0x0E); Timer1_WaitMs(250); // white
    LED_Off();     Timer1_WaitMs(250);
  }
}*/
