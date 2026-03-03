
#include "tm4c123gh6pm.h"
#include <stdint.h>

/* ========= MODULE ENABLE FLAGS ========= */
// Uncomment the module you want to test in isolation
#define ENABLE_MODULE_SW1_TEST
#define TEST_ULTRASONIC
#define TEST_UART0
#define TEST_TIMER1
#define TEST_LED

/* ========= PIN, MASKS,VAR DEFINITIONS ========= */
#define SW1_MASK 0x10		  // PF4 (SW1)
#define PF1_MASK 0x02     // PF1 (Red LED)
#define SW1  (*((volatile uint32_t *)0x40025040))   // PORTF4 (masked)
#define LED  (*((volatile uint32_t *)0x40025038))   // PORTF3-1 (masked)
#define SYSDIV2 7 //should change to 16Mhz, currently at 50MHz
#define US_NO_ECHO        0u
#define US_OUT_OF_RANGE   0xFFFFFFFFu
#define TRIG_MASK 0x10   // PB4
#define ECHO_MASK 0x40   // PB6 (T0CCP0)
#define WAIT_RISE_US 30000u
#define MAX_PULSE_US 25000u
#define MIN_CM       2u
#define MAX_CM       400u
static volatile uint32_t tRise = 0;
static volatile uint32_t tFall = 0;
static volatile uint8_t gotRise = 0;
static volatile uint8_t gotFall = 0;
#define CR   0x0D //For UART0 test, Carriage Return
#define LF   0x0A
#define BS   0x08
#define ESC  0x1B
#define SP   0x20
#define DEL  0x7F
#define NULL 0

/* ========= FUNCTION PROTOTYPES ========= */
void System_Init(void);
#if defined(ENABLE_MODULE_SW1_TEST)
void Module_SW1_Init(void);
void MODULE_SW1_TEST(void);
#endif

#if defined(TEST_ULTRASONIC)
void Ultrasonic_Init(void);	
void Ultrasonic_Test(void);
uint32_t Ultrasonic_ReadCM(void);
uint32_t Ultrasonic_ReadCM_Filtered(void);
#endif

#if defined(TEST_UART0)
void UART0_Init(void);
void UART0_Test(void);
void UART0_OutCRLF(void);
uint8_t UART0_InChar(void);
void UART0_OutChar(uint8_t data);
void UART0_OutString(uint8_t *pt);
#endif

#if defined(TEST_TIMER1)
void Timer1_Init(void);
uint32_t Timer1_Now(void);
void Timer1_WaitTicks(uint32_t ticks);
void Timer1_WaitUs(uint32_t us);   // assumes 50 MHz, but want 16MHz
void Timer1_WaitMs(uint32_t ms);   // assumes 50 MHz, but want 16MHz
void Timer1_Test(void);
#endif

#if defined(TEST_LED)
void LED_Init(void);
void LED_Set(uint8_t mask);   // PF1=0x02 PF2=0x04 PF3=0x08
void LED_Off(void);
void LED_Test(void);
#endif

/* ========= MAIN ========= */
int main(void){
	System_Init();

	while(1){
		#if defined(ENABLE_MODULE_SW1_TEST)
		MODULE_SW1_TEST();
		#endif
		#if defined(TEST_ULTRASONIC)
		Ultrasonic_Test();
		#endif
		#if defined(TEST_UART0)
		UART0_Test();
		#endif
		#if defined(TEST_TIMER1)
		Timer1_Test();
		#endif
		#if defined(TEST_LED)
		LED_Test();
		#endif
	}
}

/* ========= SYSTEM INIT ========= */
void System_Init(void){
	#if defined(ENABLE_MODULE_SW1_TEST)
	Module_SW1_Init();
	#endif
	#if defined(TEST_ULTRASONIC)
	Ultrasonic_Init();
	#endif
	#if defined(TEST_UART0)
	UART0_Init();
	#endif
	#if defined(TEST_TIMER1)
	Timer1_Init();
	#endif
	#if defined(TEST_LED)
	LED_Init();
	#endif
}

/* ========= SW1 MODULE ========= */
#if defined(ENABLE_MODULE_SW1_TEST)
	/*
	* Module_SW1_Init
	* - PF4: input with pull-up and interrupt
	* - PF1: output (red LED)
	*/
	void Module_SW1_Init(void){
		/* Enable clock for Port F */
		SYSCTL_RCGCGPIO_R |= 0x20;
		while((SYSCTL_PRGPIO_R & 0x20) == 0){}

		/* Unlock PF4 (required) */
		GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY;
		GPIO_PORTF_CR_R  |= SW1_MASK;


		/* Configure directions */
		GPIO_PORTF_DIR_R &= ~SW1_MASK;   // PF4 input
		GPIO_PORTF_DIR_R |= PF1_MASK;    // PF1 output

		/* Digital enable */
		GPIO_PORTF_DEN_R |= (SW1_MASK | PF1_MASK);

		/* Pull-up on PF4 */
		GPIO_PORTF_PUR_R |= SW1_MASK;

		/* Disable analog and alternate functions */
		GPIO_PORTF_AMSEL_R &= ~(SW1_MASK | PF1_MASK);
		GPIO_PORTF_AFSEL_R &= ~(SW1_MASK | PF1_MASK);
		GPIO_PORTF_PCTL_R &= ~0x000F0000;   // Clear PCTL for PF4


		/* Turn LED off */
		GPIO_PORTF_DATA_R &= ~PF1_MASK;

		/* Configure PF4 interrupt: falling edge */
		GPIO_PORTF_IS_R  &= ~SW1_MASK;    // Edge-sensitive
		GPIO_PORTF_IBE_R &= ~SW1_MASK;    // Single edge
		GPIO_PORTF_IEV_R &= ~SW1_MASK;    // Falling edge
		GPIO_PORTF_ICR_R  =  SW1_MASK;    // Clear interrupt flag
		GPIO_PORTF_IM_R  |=  SW1_MASK;    // Arm interrupt

		/* Set GPIO Port F interrupt (IRQ 30) priority to 2
		* Priority bits for IRQ 30 are in NVIC_PRI7_R[23:21]
		* 2 << 21 = 0x00400000
		*/
		NVIC_PRI7_R = (NVIC_PRI7_R & 0xFF1FFFFF) | 0x00400000;

		/* Enable GPIO Port F interrupt (IRQ 30) */
		NVIC_EN0_R |= 0x40000000;
	}

	/*
	* GPIO Port F Interrupt Handler
	* - Simple for-loop debounce
	* - Toggle red LED on valid press
	*/
	void GPIOPortF_Handler(void){
		volatile uint32_t i;

		/* Clear interrupt flag */
		GPIO_PORTF_ICR_R = SW1_MASK;

		/*  debounce delay */
		for(i = 0; i < 2000; i++){
		}

		/* Confirm switch still pressed (active low) */
		if((GPIO_PORTF_DATA_R & SW1_MASK) == 0){
			GPIO_PORTF_DATA_R ^= PF1_MASK;   // toggle red LED
		}
	}

	/*
	* MODULE_SW1_TEST
	* - No polling needed; ISR handles behavior
	*/
	void MODULE_SW1_TEST(void){
		/* Intentionally empty */
	}

#endif  /* ENABLE_MODULE_SW1_TEST */

//Module UART Init 
#if defined(TEST_ULTRASONIC)
	static void Trigger10us(void){
		GPIO_PORTB_DATA_R &= ~TRIG_MASK;
		Timer1_WaitUs(2);
		GPIO_PORTB_DATA_R |= TRIG_MASK;
		Timer1_WaitUs(10);
		GPIO_PORTB_DATA_R &= ~TRIG_MASK;
	}

	// Timer0A capture ISR (IRQ 19)
	void Timer0A_Handler(void){  			//Uses Interupt capture to measure pulse. Uses rising AND falling edge.
	TIMER0_ICR_R = TIMER_ICR_CAECINT;
	uint32_t t = TIMER0_TAR_R;

	if(!gotRise){
		tRise = t; 							//rising edge capture on tRise
		gotRise = 1;
		TIMER0_CTL_R = (TIMER0_CTL_R & ~TIMER_CTL_TAEVENT_M) | TIMER_CTL_TAEVENT_NEG;
	} else if(!gotFall){
		tFall = t;							//falling edge capture on tFall
		gotFall = 1;
		TIMER0_CTL_R = (TIMER0_CTL_R & ~TIMER_CTL_TAEVENT_M) | TIMER_CTL_TAEVENT_POS;
	}
	}

	void Ultrasonic_Init(void){
	SYSCTL_RCGCGPIO_R |= 0x02;                 // Port B
	volatile uint32_t d = SYSCTL_RCGCGPIO_R; (void)d;

	// PB4 TRIG output
	GPIO_PORTB_DIR_R |= TRIG_MASK;
	GPIO_PORTB_DEN_R |= TRIG_MASK;
	GPIO_PORTB_AFSEL_R &= ~TRIG_MASK;
	GPIO_PORTB_AMSEL_R &= ~TRIG_MASK;

	// PB6 ECHO as T0CCP0
	GPIO_PORTB_DIR_R &= ~ECHO_MASK;
	GPIO_PORTB_DEN_R |= ECHO_MASK;
	GPIO_PORTB_AFSEL_R |= ECHO_MASK;
	GPIO_PORTB_AMSEL_R &= ~ECHO_MASK;
	GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R & ~(0xF << (6*4))) | (0x7 << (6*4)); // PB6=7

	// Timer0A capture edge-time
	SYSCTL_RCGCTIMER_R |= 0x01;                // Timer0
	d = SYSCTL_RCGCTIMER_R; (void)d;

	TIMER0_CTL_R &= ~TIMER_CTL_TAEN;
	TIMER0_CFG_R = 0x04;                       // 16-bit
	TIMER0_TAMR_R = TIMER_TAMR_TAMR_CAP | TIMER_TAMR_TACMR;
	TIMER0_CTL_R = (TIMER0_CTL_R & ~TIMER_CTL_TAEVENT_M) | TIMER_CTL_TAEVENT_POS;
	TIMER0_TAPR_R = 0xFF;
	TIMER0_TAILR_R = 0xFFFF;
	TIMER0_ICR_R = TIMER_ICR_CAECINT;
	TIMER0_IMR_R |= TIMER_IMR_CAEIM;

	NVIC_EN0_R |= (1 << 19);                   // enable IRQ 19
	TIMER0_CTL_R |= TIMER_CTL_TAEN;
	}

	static uint32_t TicksToUs(uint32_t ticks){
	return ticks / 50u;                        // 50MHz
	}

	uint32_t Ultrasonic_ReadCM(void){ // Returns distance in cm, or US_NO_ECHO, or US_OUT_OF_RANGE if there is a time out, waiting for echo.
	gotRise = 0; gotFall = 0;
	tRise = 0; tFall = 0;

	TIMER0_CTL_R = (TIMER0_CTL_R & ~TIMER_CTL_TAEVENT_M) | TIMER_CTL_TAEVENT_POS; //Timer0A will capture the next rising edge

	Trigger10us(); //trigger pulse sent

	uint32_t wait = 0; //wait for rising edge, with timeout if nothing is recieved
	while(!gotRise){
		Timer1_WaitUs(50);
		wait += 50;
		if(wait >= WAIT_RISE_US) return US_NO_ECHO;
	}

	wait = 0;
	while(!gotFall){
		Timer1_WaitUs(50);
		wait += 50;
		if(wait >= MAX_PULSE_US) return US_OUT_OF_RANGE;	//if no response, print out of range.
	}

	uint32_t pulseTicks = (tRise - tFall);     // down-counter wrap ok
	uint32_t pulseUs = TicksToUs(pulseTicks);
	uint32_t cm = pulseUs / 58u;

	if(cm < MIN_CM || cm > MAX_CM) return US_OUT_OF_RANGE;
	return cm;
	}

	uint32_t Ultrasonic_ReadCM_Filtered(void){
	uint32_t good[5];
	uint32_t n = 0;

	for(int i=0;i<5;i++){
		uint32_t d = Ultrasonic_ReadCM();
		if(d != US_NO_ECHO && d != US_OUT_OF_RANGE){
		good[n++] = d;
		}
		Timer1_WaitMs(60);                       // spacing between pings
	}
	if(n == 0) return US_NO_ECHO;
	if(n == 1) return good[0];

	for(uint32_t i=0;i<n;i++){
		for(uint32_t j=i+1;j<n;j++){
		if(good[j] < good[i]){
			uint32_t t = good[i]; good[i] = good[j]; good[j] = t;
		}
		}
	}
	return good[n/2];
	}

	void Ultrasonic_Test(void){
	char buf[64];
	UART0_OutString((uint8_t*)"\n\rUltrasonic test: TRIG=PB4, ECHO=PB6(T0CCP0)\n\r");
	
	while(1){
		uint32_t d = Ultrasonic_ReadCM_Filtered();
		snprintf(buf, sizeof(buf), "Distance: %lu cm\n\r", d);
		UART0_OutString((uint8_t*)buf);
		Timer1_WaitMs(200);
	}
}
#endif

//Module UART0 Init (Used example UART0Out project example)
#if defined(TEST_UART0)
	void UART0_Init(void){
	SYSCTL_RCGCUART_R |= 0x01;             // activate UART0
	SYSCTL_RCGCGPIO_R |= 0x01;             // activate port A
		
	UART0_CTL_R = 0;                      // disable UART
	UART0_IBRD_R = 54;                    // IBRD = int(50,000,000 / (16 * 115,200)) = int(27.1267)
	UART0_FBRD_R = 16;                     // FBRD = int(0.1267 * 64 + 0.5) = 8
											// 8 bit word length (no parity bits, one stop bit,  FIFOs enabled)
	UART0_LCRH_R = UART_LCRH_WLEN_8|UART_LCRH_FEN; // For simulator FIFO has to be enabled.
		
	UART0_CTL_R |= UART_CTL_RXE|UART_CTL_TXE|UART_CTL_UARTEN;// enable Tx, RX and UART
	GPIO_PORTA_AFSEL_R |= 0x03;           // enable alt funct on PA1-0
	GPIO_PORTA_DEN_R |= 0x03;             // enable digital I/O on PA1-0
											// configure PA1-0 as UART
	GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R&0xFFFFFF00)+0x00000011;
	GPIO_PORTA_AMSEL_R &= ~0x03;          // disable analog functionality on PA
	}

	void UART0_OutCRLF(void){
	UART0_OutChar(CR);
	UART0_OutChar(LF);
	}

	uint8_t UART0_InChar(void){
	while((UART0_FR_R&UART_FR_RXFE) != 0); // wait until the receiving FIFO is not empty
	return((uint8_t)(UART0_DR_R&0xFF));
	}
	
	void UART0_OutChar(uint8_t data){
	while((UART0_FR_R&UART_FR_TXFF) != 0);
	UART0_DR_R = data;
	}

	void UART0_OutString(uint8_t *pt){
	while(*pt){
		UART0_OutChar(*pt);
		pt++;
	}
	}
#endif 


//Module LED Init 
#if defined(TEST_LED)
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

	void LED_Test(void){
	while(1){
		LED_Set(0x02); Timer1_WaitMs(250); // red
		LED_Set(0x04); Timer1_WaitMs(250); // blue
		LED_Set(0x08); Timer1_WaitMs(250); // green
		LED_Set(0x0E); Timer1_WaitMs(250); // white
		LED_Off();     Timer1_WaitMs(250);
	}
	}
	#endif 

//Module Timer1 Init 
#if defined(TEST_TIMER1)
	void Timer1_Init(void){
	SYSCTL_RCGCTIMER_R |= 0x02;            // Timer1 clock
	volatile uint32_t d = SYSCTL_RCGCTIMER_R; (void)d;

	TIMER1_CTL_R = 0;
	TIMER1_CFG_R = 0x0;                    // 32-bit
	TIMER1_TAMR_R = 0x02;                  // periodic
	TIMER1_TAILR_R = 0xFFFFFFFF;
	TIMER1_TAPR_R = 0;
	TIMER1_ICR_R = 0x1;
	TIMER1_CTL_R = 0x1;                    // enable
	}

	uint32_t Timer1_Now(void){
	return TIMER1_TAR_R;                   // down-counting
	}

	static inline uint32_t Elapsed(uint32_t start, uint32_t now){
	return (start - now);
	}

	void Timer1_WaitTicks(uint32_t ticks){
	uint32_t start = Timer1_Now();
	while(Elapsed(start, Timer1_Now()) < ticks){}
	}

	void Timer1_WaitUs(uint32_t us){
	Timer1_WaitTicks(us * 50u);            // 16 ticks per us @ 16MHz
	}

	void Timer1_WaitMs(uint32_t ms){
	while(ms--){
		Timer1_WaitUs(1000);
	}
	}

	void Timer1_Test(void){
	UART0_OutString((uint8_t*)"Timer1: wait 1000ms...\n\r");
	Timer1_WaitMs(1000);
	UART0_OutString((uint8_t*)"Done.\n\r");
	while(1){}
	}
#endif 
