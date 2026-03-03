

#include "tm4c123gh6pm.h"
#include "Ultrasonic.h"
#include "Delay.h"
#include <stdint.h>

#define TRIG_MASK 0x10   // PB4
#define ECHO_MASK 0x40   // PB6 (T0CCP0)

#define WAIT_RISE_US 30000u
#define MAX_HIGH_US  25000u
#define MIN_CM       2u
#define MAX_CM       400u

static volatile uint32_t tRise = 0;
static volatile uint32_t tFall = 0;
static volatile uint8_t gotRise = 0;
static volatile uint8_t gotFall = 0;
static volatile uint8_t trigDone = 0;



// Using Trigger using Timer2A (16-bit countdown mode)
static void Timer2A_Init(void){
  SYSCTL_RCGCTIMER_R |= 0x04; 
  volatile uint32_t d = SYSCTL_RCGCTIMER_R; (void)d;

  TIMER2_CTL_R &= ~TIMER_CTL_TAEN;
  TIMER2_CFG_R = 0x04;        // 16-bit
  TIMER2_TAMR_R = 0x01;       // one-shot
  TIMER2_TAPR_R = 0;
  TIMER2_ICR_R = 0x01;
  TIMER2_IMR_R |= 0x01;       // arm timeout interrupt
  NVIC_EN0_R |= (1u << 23);   // IRQ 23 = Timer2A
}


static void Timer2A_WaitTicks(uint16_t ticks){
  TIMER2_CTL_R &= ~TIMER_CTL_TAEN;
  TIMER2_TAILR_R = ticks ? (ticks-1u) : 0;
  TIMER2_ICR_R = 0x01;
  TIMER2_CTL_R |= TIMER_CTL_TAEN;
  while((TIMER2_RIS_R & 0x01) == 0){}
  TIMER2_ICR_R = 0x01;
  TIMER2_CTL_R &= ~TIMER_CTL_TAEN;
}

static void Trigger10us(void){
  trigDone = 0;

  GPIO_PORTB_DATA_R &= ~TRIG_MASK;
  Delay_Us(2);

  GPIO_PORTB_DATA_R |= TRIG_MASK;       // start pulse
  TIMER2_TAILR_R = 160 - 1;             // 10us @ 16MHz
  TIMER2_ICR_R = 0x01;
  TIMER2_CTL_R |= TIMER_CTL_TAEN;

  while(!trigDone){}                    // wait for ISR to end pulse
}


// Timer0A Capture (Method B) 
void Timer0A_Handler(void){
  TIMER0_ICR_R = TIMER_ICR_CAECINT;

  uint32_t t = ((uint32_t)TIMER0_TAPS_R << 16) |
               (uint32_t)TIMER0_TAR_R;

  if(!gotRise){
    tRise = t;
    gotRise = 1;
    TIMER0_CTL_R = (TIMER0_CTL_R & ~TIMER_CTL_TAEVENT_M)
                 | TIMER_CTL_TAEVENT_NEG;
  } else if(!gotFall){
    tFall = t;
    gotFall = 1;
    TIMER0_CTL_R = (TIMER0_CTL_R & ~TIMER_CTL_TAEVENT_M)
                 | TIMER_CTL_TAEVENT_POS;
  }
}

static uint32_t Delta24(uint32_t start, uint32_t end){
  return (start - end) & 0x00FFFFFFu;
}

static uint32_t TicksToUs(uint32_t ticks){
  return ticks / 16u;   // 16 ticks per us @16MHz
}

void Ultrasonic_Init(void){

  SYSCTL_RCGCGPIO_R |= 0x02; // Port B
  volatile uint32_t d = SYSCTL_RCGCGPIO_R; (void)d;

  // PB4 TRIG output
  GPIO_PORTB_DIR_R |= TRIG_MASK;
  GPIO_PORTB_DEN_R |= TRIG_MASK;
  GPIO_PORTB_AFSEL_R &= ~TRIG_MASK;
  GPIO_PORTB_AMSEL_R &= ~TRIG_MASK;

  // PB6 as T0CCP0
  GPIO_PORTB_DIR_R &= ~ECHO_MASK;
  GPIO_PORTB_DEN_R |= ECHO_MASK;
  GPIO_PORTB_AFSEL_R |= ECHO_MASK;
  GPIO_PORTB_AMSEL_R &= ~ECHO_MASK;
  GPIO_PORTB_PCTL_R =
    (GPIO_PORTB_PCTL_R & ~(0xFu << (6*4))) |
    (0x7u << (6*4));

  Timer2A_Init();

  // Timer0A capture setup
  SYSCTL_RCGCTIMER_R |= 0x01; // Timer0
  d = SYSCTL_RCGCTIMER_R; (void)d;

  TIMER0_CTL_R &= ~TIMER_CTL_TAEN;
  TIMER0_CFG_R = 0x04; // 16-bit
  TIMER0_TAMR_R =
      TIMER_TAMR_TAMR_CAP |
      TIMER_TAMR_TACMR;

  TIMER0_CTL_R =
    (TIMER0_CTL_R & ~TIMER_CTL_TAEVENT_M)
    | TIMER_CTL_TAEVENT_POS;

  TIMER0_TAPR_R  = 0xFF;   // non-zero prescaler
  TIMER0_TAILR_R = 0xFFFF;

  TIMER0_ICR_R  = TIMER_ICR_CAECINT;
  TIMER0_IMR_R |= TIMER_IMR_CAEIM;

  NVIC_EN0_R |= (1u << 19); // IRQ 19
  TIMER0_CTL_R |= TIMER_CTL_TAEN;
}

uint32_t Ultrasonic_ReadCM(void){

  gotRise = 0;
  gotFall = 0;

  TIMER0_CTL_R =
    (TIMER0_CTL_R & ~TIMER_CTL_TAEVENT_M)
    | TIMER_CTL_TAEVENT_POS;

  Trigger10us();

  uint32_t waited = 0;
  while(!gotRise){
    Delay_Us(50);
    waited += 50;
    if(waited >= WAIT_RISE_US)
      return US_NO_ECHO;
  }

  waited = 0;
  while(!gotFall){
    Delay_Us(50);
    waited += 50;
    if(waited >= MAX_HIGH_US)
      return US_OUT_OF_RANGE;
  }

  uint32_t pulseTicks = Delta24(tRise, tFall);
  uint32_t pulseUs = TicksToUs(pulseTicks);
  uint32_t cm = pulseUs / 58u;

  if(cm < MIN_CM || cm > MAX_CM)
    return US_OUT_OF_RANGE;

  return cm;
}

void Timer2A_Handler(void){
  TIMER2_ICR_R = 0x01;              // clear timeout
  GPIO_PORTB_DATA_R &= ~TRIG_MASK;  // end trigger pulse
  TIMER2_CTL_R &= ~TIMER_CTL_TAEN;  // stop timer
  trigDone = 1;
}
