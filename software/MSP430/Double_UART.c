/**********************************************************************************/
// Khashi Xiong
// eyeCU
// MSP430 UART
//		Setup the UART for the MSP430 and allow for data to be sent
//		and received thru the UART ports.
/**********************************************************************************/

#include "msp430x16x.h"
#include <intrinsics.h>

void uartconfig(void);

/**********************************************************************************/
// Configure UART on MSP430
/**********************************************************************************/
void uartconfig(void)
{
	  // UART Configuration
	  P3SEL |= 0xF0;				// P3.4,5,6,7 = USART0 and USART1 TXD/RXD
	  ME1 |= UTXE0 + URXE0;			// Enable USART0 TXD/RXD
	  ME2 |= UTXE1 + URXE1;			// Enable USART1 TXD/RXD
	  UCTL0 |= CHAR;				// 8-bit character
	  UCTL1 |= CHAR;				// 8-bit character
	  UTCTL0 |= SSEL0;				// UCLK = ACLK = 32.768kHz
	  UTCTL1 |= SSEL0;				// UCLK = ACLK = 32.768kHz
	  UBR00 = 0x03;					// 32.768kHz/9600 - 3.41
	  UBR01 = 0x03;					// 32.768kHz/9600 - 3.41
	  UBR10 = 0x00;
	  UBR11 = 0x00;
	  UMCTL0 = 0x4a;				// Modulation
	  UMCTL1 = 0x4a;				// Modulation
	  UCTL0 &= ~SWRST;				// Initialize USART state machine
	  UCTL1 &= ~SWRST;				// Initialize USART state machine
	  IE1 |= URXIE0 + UTXIE0;		// Enable USART0 RX/TX interrupt
	  IE2 |= URXIE1 + UTXIE1;		// Enable USART1 RX/TX interrupt
}

/**********************************************************************************/
// Main function
/**********************************************************************************/
void main(void)
{
	WDTCTL = WDTPW + WDTHOLD;		// Stop Watchdog Timer
	uartconfig();					// Configure Ports
	_BIS_SR(GIE);					// Check for interrupt
}

/**********************************************************************************/
// Interrupt to receive bits and transmit to UART 1 RX Buffer
/**********************************************************************************/

#pragma vector=USART0RX_VECTOR
__interrupt void usart0_rx (void)
{
	while (!(IFG1 & UTXIFG0));		// Check if buffer is ready
	TXBUF0 = RXBUF1;
}

/**********************************************************************************/
// Send UART 1 RX buffer to UART 1 TX buffer
/**********************************************************************************/
#pragma vector=USART1RX_VECTOR
__interrupt void usart1_rx (void)
{
	while (!(IFG2 & UTXIFG1));		// Check if buffer is ready
	TXBUF1 = RXBUF1;
}
