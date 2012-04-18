#ifndef UART_H_
#define UART_H_

// TODO: redefine interface to make it cleaner
extern unsigned char txArray[50];

void uartInit(void);
void UartInterruptEnable(void);
void UART0AINTCConfigure(void);
void UartFIFOConfigure(void);
void UartBaudRateSet(void);


#endif /*UART_H_*/
