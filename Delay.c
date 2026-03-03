
#include "tm4c123gh6pm.h"
#include "Delay.h"
#include <stdint.h>

void Delay_Init(void){
  SYSCTL_RCGCTIMER_R |= 0x20;              // Timer5
  volatile uint32_t d = SYSCTL_RCGCTIMER_R; (void)d;

  TIMER5_CTL_R &= ~TIMER_CTL_TAEN;
  TIMER5_CFG_R = 0x04;                    // 16-bit
  TIMER5_TAMR_R = 0x02;                   // periodic down-count
  TIMER5_TAPR_R = 0;                      // no prescale
  TIMER5_ICR_R = 0x01;
}

static void WaitTicks5A(uint32_t ticks){
  TIMER5_CTL_R &= ~TIMER_CTL_TAEN;
  TIMER5_TAILR_R = (ticks > 0) ? (ticks - 1u) : 0;
  TIMER5_ICR_R = 0x01;
  TIMER5_CTL_R |= TIMER_CTL_TAEN;
  while((TIMER5_RIS_R & 0x01) == 0){}
  TIMER5_ICR_R = 0x01;
  TIMER5_CTL_R &= ~TIMER_CTL_TAEN;
}

void Delay_Us(uint32_t us){
  // 16 ticks/us @ 16MHz
  while(us--){
    WaitTicks5A(16);
  }
}

void Delay_Ms(uint32_t ms){
  while(ms--){
    WaitTicks5A(16000); // 16000 ticks/ms @ 16MHz
  }
}


void Delay_Init(void){
  SYSCTL_RCGCTIMER_R |= 0x20;              // Timer5
  volatile uint32_t d = SYSCTL_RCGCTIMER_R; (void)d;

  TIMER5_CTL_R &= ~TIMER_CTL_TAEN;
  TIMER5_CFG_R = 0x04;                    // 16-bit
  TIMER5_TAMR_R = 0x02;                   // periodic down-count
  TIMER5_TAPR_R = 0;                      // no prescale
  TIMER5_ICR_R = 0x01;
}

static void WaitTicks5A(uint32_t ticks){
  TIMER5_CTL_R &= ~TIMER_CTL_TAEN;
  TIMER5_TAILR_R = (ticks > 0) ? (ticks - 1u) : 0;
  TIMER5_ICR_R = 0x01;
  TIMER5_CTL_R |= TIMER_CTL_TAEN;
  while((TIMER5_RIS_R & 0x01) == 0){}
  TIMER5_ICR_R = 0x01;
  TIMER5_CTL_R &= ~TIMER_CTL_TAEN;
}

void Delay_Us(uint32_t us){
  // 16 ticks/us @ 16MHz
  while(us--){
    WaitTicks5A(16);
  }
}

void Delay_Ms(uint32_t ms){
  while(ms--){
    WaitTicks5A(16000); // 16000 ticks/ms @ 16MHz
  }
}INT_LEAST16_MAX

