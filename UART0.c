
#include "PLL.h"
#include "UART0.h"
#include "Systick.h"
#include "LED.h"
#include "Switch.h"
#include "Ultrasonic.h"
#include <stdint.h>
#include <stdio.h>

// SELECT ONE TEST
//#define TEST_ULTRASONIC
//#define TEST_UART0
//#define TEST_TIMER1
//#define TEST_LED
//#define TEST_SWITCH

void System_Init(void){
  PLL_Init();       // 50 MHz
  Timer1_Init();    // delay + time base
  UART0_Init();     // 57600 baud (after you edit UART0.c)
  LED_Init();
  Switch_Init();
  Ultrasonic_Init();

  UART0_OutString((uint8_t*)"\n\r Module Test Harness: \n\r");
}

int main(void){
  System_Init();

#ifdef TEST_UART0
  UART0_OutString((uint8_t*)"UART0 OK @57600\n\r");
  while(1){
    UART0_OutString((uint8_t*)"ping\n\r");
    Timer1_WaitMs(500);
  }
#endif

#ifdef TEST_TIMER1
  Timer1_Test();
#endif

#ifdef TEST_LED
  LED_Test();
#endif

#ifdef TEST_SWITCH
  Switch_Test();
#endif

#ifdef TEST_ULTRASONIC
  Ultrasonic_Test();
#endif

  while(1){}
}
