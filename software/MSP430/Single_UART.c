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
	  P3SEL |= 0xFF;						// P3.6,7 = USART1 TXD/RXD
	  ME2 |= UTXE1 + URXE1;					// Enable USART1 TXD/RXD
	  UCTL1 |= CHAR;						// 8-bit character
	  UTCTL1 |= SSEL0;						// UCLK = ACLK = 32.768kHz
	  UBR01 = 0x03;							// 32.768kHz/9600 - 3.41
	  UBR11 = 0x00;
	  UMCTL1 = 0x4a;						// Modulation
	  UCTL1 &= ~SWRST;						// Initialize USART state machine
	  IE2 |= URXIE1;						// Enable USART1 RX/TX interrupt
}


void main(void)
{
  WDTCTL = WDTPW + WDTHOLD;             	// Stop Watchdog Timer

  uartconfig();								// Configure UART Ports
  _BIS_SR(GIE);								// Check Interrupt if Interrupt occurs

}

/**********************************************************************************/
// Interrupt to receive bits and echo back bits received
/**********************************************************************************/

#pragma vector=USART1RX_VECTOR
__interrupt void usart1_rx (void)
{
	while (!(IFG2 & UTXIFG1));				// Check if UART Buffer is ready
	TXBUF1 = RXBUF1;						// TX buffer equals RX buffer
}
