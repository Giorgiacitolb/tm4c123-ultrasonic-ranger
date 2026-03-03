
#include <stdint.h>
#include <stdio.h>
#include "tm4c123gh6pm.h"
//include all header files for modules
#include "PLL.h"
#include "UART0.h"
#include "LED.h"
#include "Switch.h"
#include "Delay.h"
#include "Blink.h"
#include "Ultrasonic.h"

// function prototypes
void System_Init(void);
static void ApplyLED(uint32_t d);

int main(void){	
  System_Init();

  
  UART0_OutString((uint8_t*)"\n\r Integration Test (Method B) \n\r");
  //UART0_OutString((uint8_t*)"Set terminal to 115200. Press SW1 to measure.\n\r");

  uint8_t prev = 0;
	
  while(1){
    uint8_t now = Switch_ReadDebounced();     // bit0 = SW1

    // one measurement per press (rising edge of "pressed" state)
    if((now & 0x01) && !(prev & 0x01)){

      uint32_t d = Ultrasonic_ReadCM();

      if(d == US_NO_ECHO || d == US_OUT_OF_RANGE){
        UART0_OutString((uint8_t*)"OUT OF RANGE\n\r");
        ApplyLED(US_OUT_OF_RANGE);
      } else {
        char buf[64];
        snprintf(buf, sizeof(buf), "Distance: %lu cm\n\r", d);
        UART0_OutString((uint8_t*)buf);
        ApplyLED(d);
      }
      
			
			while(Switch_ReadRaw() & 0x01){}   // wait until button released
			Delay_Ms(30);  
  }
}
}

static void ApplyLED(uint32_t d){
  Blink_RedDisable();
  LED_Off();

  if(d == US_NO_ECHO || d == US_OUT_OF_RANGE){
    return;                 // keep LEDs off
  }
  if(d < 10u){
    Blink_RedEnable();      // red blink 2Hz
  } else if(d <= 70u){
    LED_Set(LED_GREEN);
  } else {
    LED_Set(LED_BLUE);
  }
}

void System_Init(void) {
  PLL_Init();
  SysTick_Init();
  LED_Init();
  UART0_Init();      // 115200 @ 16MHz (IBRD=8, FBRD=44)
  Delay_Init();
  Blink_Init();
  Switch_Init();
  Ultrasonic_Init();
}
