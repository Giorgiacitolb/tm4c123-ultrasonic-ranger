// Blink.c
// Course number: CECS 347
// Project description: Generates 2 Hz red LED blinking using Timer3A interrupt.

#include "tm4c123gh6pm.h"
#include "Blink.h"

#define PF1_RED 0x02

static volatile uint8_t redBlink = 0;

void Blink_Init(void){
  SYSCTL_RCGCTIMER_R |= 0x08;              // Timer3
  volatile uint32_t d = SYSCTL_RCGCTIMER_R; (void)d;

  TIMER3_CTL_R &= ~TIMER_CTL_TAEN;
  TIMER3_CFG_R = 0x04;                    // 16-bit
  TIMER3_TAMR_R = 0x02;                   // periodic down
  TIMER3_TAPR_R = 250 - 1;                // prescale 250 -> 1 tick = 15.625us
  TIMER3_TAILR_R = 16000 - 1;             // 16000 ticks *15.625us = 250ms
  TIMER3_ICR_R = 0x01;
  TIMER3_IMR_R |= 0x01;

  NVIC_EN1_R |= (1u << (35-32));          // IRQ 35 = Timer3A
  TIMER3_CTL_R |= TIMER_CTL_TAEN;
}

void Blink_RedEnable(void){ redBlink = 1; }
void Blink_RedDisable(void){
  redBlink = 0;
  GPIO_PORTF_DATA_R &= ~PF1_RED;
}

void Timer3A_Handler(void){
  TIMER3_ICR_R = 0x01;
  if(redBlink){
    GPIO_PORTF_DATA_R ^= PF1_RED;         // toggle every 250ms => 2Hz blink
  } else {
    GPIO_PORTF_DATA_R &= ~PF1_RED;
  }
}
