
#include "tm4c123gh6pm.h"
#include <stdint.h>
#include "SysTick.h"


#define PF1_RED   0x02

static volatile uint32_t g_ms = 0;
static volatile uint8_t g_redBlink = 0;
static volatile uint32_t g_redCnt = 0;

void SysTick_Init_1ms(uint32_t busClockHz){
  // reload for 1ms tick
  uint32_t reload = (busClockHz/1000u) - 1u; // for 16MHz -> 15999
  NVIC_ST_CTRL_R = 0;
  NVIC_ST_RELOAD_R = reload;
  NVIC_ST_CURRENT_R = 0;
  // enable SysTick with core clock + interrupts
  NVIC_ST_CTRL_R = 0x07; // ENABLE | INTEN | CLK_SRC
}

uint32_t SysTick_Millis(void){
  return g_ms;
}

void DelayMs(uint32_t ms){
  uint32_t start = SysTick_Millis();
  while((SysTick_Millis() - start) < ms){}
}

void RedBlink_Enable(void){
  g_redBlink = 1;
  g_redCnt = 0;
}

void RedBlink_Disable(void){
  g_redBlink = 0;
  g_redCnt = 0;
  // ensure red off when disabling blink
  GPIO_PORTF_DATA_R &= ~PF1_RED;
}

void SysTick_Handler(void){
  g_ms++;

  if(g_redBlink){
    g_redCnt++;
    if(g_redCnt >= 250){         // toggle every 250ms => 2Hz blink
      g_redCnt = 0;
      GPIO_PORTF_DATA_R ^= PF1_RED;
    }
  }
}
