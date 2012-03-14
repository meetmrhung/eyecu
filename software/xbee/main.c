/**********************************************************************************/
// Khashi Xiong
// eyeCU
// MSP430 UART HELLO WORLD
//		Setup the UART for the MSP430 and allow for "Hello World" to be sent
//		and received thru the UART ports.
/**********************************************************************************/

#include "msp430x16x.h"
#include <intrinsics.h>

/*
void portinit(void);
void initdev(void);
void pinconfig(void);
void sendstring(char * str, int len);

#define LED_ON                  P6OUT &= ~0x01; P6DIR |= 0x01;
#define LED_OFF                 P6OUT |= 0x01;  P6DIR |= 0x01;
#define LED_Check               (P6IN & 0x01)
#define But_Check               (P6IN & 0x02)
*/

/**********************************************************************************/
// Port Initialization on MSP430
/**********************************************************************************/
void portinit(void)
{
  //input_data=P1IN;          //Read only register
  P1OUT = 0x00;               // The outputs are low
  P1SEL = 0x00;               // Peripheral module function: I/O function is selected.
  P1DIR = 0x00;               // 1 -> Output; 0 -> Input; All inputs
  P1IES = 0x00;               // Interrupt Edge Select Registers
  P1IE = 0x00;                // Disable all PORTx interrupts
  P1IFG = 0x00;               // Clear all interrupt flags

  //input_data=P2IN;          //Read only register
  P2OUT = 0x00;               // The outputs are low
  P2SEL = 0x00;               // Peripheral module function: I/O function is selected.
  P2DIR = 0x00;               // 1 -> Output; 0 -> Input; All inputs
  P2IES = 0x00;               // Interrupt Edge Select Registers
  P2IE = 0x00;                // Disable all PORTx interrupts
  P2IFG = 0x00;               // Clear all interrupt flags
}

/**********************************************************************************/
// Initialize Devices on MSP430
/**********************************************************************************/
void initdev(void)
{
  // Disable interrupts during initialization process
  _BIC_SR(GIE);

  // Call Ports_Initialization
  portinit();

  // Timer_A
  TACTL = 0x0004;  	 // Timer_A clear
  TACCTL0 = 0x0010;	 // Timer_A Capture/compare interrupt enable
  TACCR0 = 0x005F; 	 // Set TACCR0 value
  TACTL = 0x0110;  	 // Selected: ACLK, No divider, Up mode

  // Global Interrupt enabled
  _BIS_SR(GIE);
}

/**********************************************************************************/
// Configure UART on MSP430
/**********************************************************************************/
void uartconfig(void)
{
	  // UART Configuration
	  P3SEL |= 0x60;						// P3.6,7 = USART1 TXD/RXD
	  ME2 |= UTXE1 + URXE1;					// Enable USART1 TXD/RXD
	  UCTL1 |= CHAR;						// 8-bit character
	  UTCTL1 |= SSEL0;						// UCLK = ACLK = 32.768kHz
	  UBR01 = 0x03;							// 32.768kHz/9600 - 3.41
	  UBR11 = 0x00;
	  UMCTL1 = 0x4a;						// Modulation
	  UCTL1 &= ~SWRST;						// Initialize USART state machive
	  IE2 |= URXIE1 + UTXIE1;				// Enable USART1 RX/TX interrupt
	  while (!(IFG2 & UTXIFG1));			// Check if UART TX buffer is ready
	  TXBUF0 = 0x41;
	  while (!(IFG2 & UTXIFG1));			// Check if UART TX buffer is ready
	  TXBUF1 = 0x41;
}

/**********************************************************************************/
// Send string function
/**********************************************************************************/
void sendstring(char str[], int len)
{
	int i=0;
	for(i=0; i<len; i++)
	{
		while (!(IFG2 & UTXIFG1));
		TXBUF0 = str[i];
	}
}

void main(void)
{
  WDTCTL = WDTPW + WDTHOLD;             // Stop Watchdog Timer

  uartconfig();
  initdev();
  sendstring("Hello World\n",12);
}

/**********************************************************************************/
// Interrupt to receive bits
/**********************************************************************************/
/*
#pragma vector=USART1RX_VECTOR
__interrupt void usart1_rx (void)
{
	while (!(IFG2 & UTXIFG1));
	TXBUF1 = RXBUF1;
}
*/
