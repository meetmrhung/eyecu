/**********************************************************************************/
// Khashi Xiong
// eyeCU
// MSP430 UART HELLO WORLD
//		Have MSP430 UART communication
/**********************************************************************************/

#include  "msp430x16x.h"

void main(void)
{
   WDTCTL = WDTPW + WDTHOLD;   // Stop WDT
   BCSCTL1 = CALBC1_1MHZ;      // Set DCO
   DCOCTL = CALDCO_1MHZ;

   P1SEL = 0x00;
   P2SEL = 0x00;
   P3SEL = 0x30;   //P3.4,5 = USCI_A0 TXD/RXD

   P1DIR = 0x00;   //set columns P1.0-P1.3 as inputs
   P2DIR = 0x39;   //set rows P2.3 and P2.4, P2.0 for # pressed = LED on, and P2.5 Xbee0 DI0 as outputs
   P3DIR = 0xC0;   //set rows P3.6 and P3.7 as outputs
   P4DIR = 0x10;   //set P4.6 as fancontrol: fancontrol=0 (off); fancontrol=1 (on)

   P3OUT = 0x00;   //initialize output port to ground
   P2OUT = 0x00;

   __delay_cycles(100000);
   lcd_init();      //get the lcd booted up


  //RX Parameters
  UCA0CTL1 |= UCSSEL_2;                     // SMCLK
  UCA0BR0 = 104;                            // 1MHz 9600
  UCA0BR1 = 0;                              // 1MHz 9600
  UCA0MCTL = UCBRS0;                        // Modulation UCBRSx = 1
  UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
  IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt

  while (1) {
     __bis_SR_register(CPUOFF + GIE);       // Enter LPM0 w/ interrupt
     displayTemp();
     //inputTemp();
  }
}

// Receive RXed character
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{

     apiframe[j] = UCA0RXBUF;
     if ((j == 0 && apiframe[j] == 0x7E) || j > 0) {
         data[j] = apiframe[j];
     }
     if (j == 11) {
        dh = data[j];
     }
     if (j == 12) {
        dl = data[j];
             //dataReady = 1;
     }
     if (j > sizeof apiframe - 1) {
          j = 0;
          __bic_SR_register_on_exit(CPUOFF);        // Return to active mode
     }
     j++;
}
