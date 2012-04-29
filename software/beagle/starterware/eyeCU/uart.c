// TODO: Switch to not interrupt driven maybe - for simplicity
// look to hsi2c_eeprom for example of writing to uart with no interrupts

/*
** A wrapper function performing FIFO configurations.
*/

#include "uart.h"
#include "uart_irda_cir.h"
#include "soc_AM335x.h"
#include "interrupt.h"
#include "beaglebone.h"

/******************************************************************************
**              INTERNAL MACRO DEFINITIONS
******************************************************************************/
#define BAUD_RATE_115200          (115200)
#define UART_MODULE_INPUT_CLK     (48000000)
unsigned char txArray[] = "StarterWare AM335X UART Interrupt application\r\n";

/******************************************************************************
 **              FUNCTION DEFINITIONS
 ******************************************************************************/

// TODO: need to init uart1 for use with XBee
void uartInit(void)
{
	/* Configuring the system clocks for UART0 instance. */
	UART0ModuleClkConfig();

	/* Performing the Pin Multiplexing for UART0 instance. */
	UARTPinMuxSetup(0);

	/* Performing a module reset. */
	UARTModuleReset(SOC_UART_0_REGS);

	/* Performing FIFO configurations. */
	UartFIFOConfigure();

	/* Performing Baud Rate settings. */
	UartBaudRateSet();

	/* Switching to Configuration Mode B. */
	UARTRegConfigModeEnable(SOC_UART_0_REGS, UART_REG_CONFIG_MODE_B);

	/* Programming the Line Characteristics. */
	UARTLineCharacConfig(SOC_UART_0_REGS,
			(UART_FRAME_WORD_LENGTH_8 | UART_FRAME_NUM_STB_1),
			UART_PARITY_NONE);

	/* Disabling write access to Divisor Latches. */
	UARTDivisorLatchDisable(SOC_UART_0_REGS);

	/* Disabling Break Control. */
	UARTBreakCtl(SOC_UART_0_REGS, UART_BREAK_COND_DISABLE);

	/* Switching to UART16x operating mode. */
	UARTOperatingModeSelect(SOC_UART_0_REGS, UART16x_OPER_MODE);

	/* Performing Interrupt configurations. */
	UartInterruptEnable();
}
void UartFIFOConfigure(void)
{
    unsigned int fifoConfig = 0;

    /* Setting the TX and RX FIFO Trigger levels as 1. No DMA enabled. */
    fifoConfig = UART_FIFO_CONFIG(UART_TRIG_LVL_GRANULARITY_1,
                                  UART_TRIG_LVL_GRANULARITY_1,
                                  1,
                                  1,
                                  1,
                                  1,
                                  UART_DMA_EN_PATH_SCR,
                                  UART_DMA_MODE_0_ENABLE);

    /* Configuring the FIFO settings. */
    UARTFIFOConfig(SOC_UART_0_REGS, fifoConfig);
}

/*
** A wrapper function performing Baud Rate settings.
*/

void UartBaudRateSet(void)
{
    unsigned int divisorValue = 0;

    /* Computing the Divisor Value. */
    divisorValue = UARTDivisorValCompute(UART_MODULE_INPUT_CLK,
                                         BAUD_RATE_115200,
                                         UART16x_OPER_MODE,
                                         UART_MIR_OVERSAMPLING_RATE_42);

    /* Programming the Divisor Latches. */
    UARTDivisorLatchWrite(SOC_UART_0_REGS, divisorValue);
}

/*
** A wrapper function performing Interrupt configurations.
*/

void UartInterruptEnable(void)
{
    /* Configuring AINTC to receive UART0 interrupts. */
    UART0AINTCConfigure();

    /* Enabling the specified UART interrupts. */
    UARTIntEnable(SOC_UART_0_REGS, (UART_INT_LINE_STAT | UART_INT_THR |
                                    UART_INT_RHR_CTI));
}

/*
** Interrupt Service Routine for UART.
*/

void UARTIsr(void)
{
    static unsigned int txStrLength = sizeof(txArray);
    static unsigned int count = 0;
    unsigned char rxByte = 0;
    unsigned int intId = 0;

    /* Checking ths source of UART interrupt. */
    intId = UARTIntIdentityGet(SOC_UART_0_REGS);

    switch(intId)
    {
        case UART_INTID_TX_THRES_REACH:

            if(0 != txStrLength)
            {
                UARTCharPut(SOC_UART_0_REGS, txArray[count]);
                txStrLength--;
                count++;
            }
            else
            {
                /* Disabling the THR interrupt. */
                UARTIntDisable(SOC_UART_0_REGS, UART_INT_THR);
            }

        break;

        case UART_INTID_RX_THRES_REACH:
            rxByte = UARTCharGetNonBlocking(SOC_UART_0_REGS);
            UARTCharPutNonBlocking(SOC_UART_0_REGS, rxByte);
        break;

        case UART_INTID_RX_LINE_STAT_ERROR:
        case UART_INTID_CHAR_TIMEOUT:
            rxByte = UARTCharGetNonBlocking(SOC_UART_0_REGS);
        break;

        default:
        break;
    }

}

/*
** This function configures the AINTC to receive UART interrupts.
*/

void UART0AINTCConfigure(void)
{
    /* Initializing the ARM Interrupt Controller. */
    IntAINTCInit();

    /* Registering the Interrupt Service Routine(ISR). */
    IntRegister(SYS_INT_UART0INT, UARTIsr);

    /* Setting the priority for the system interrupt in AINTC. */
    IntPrioritySet(SYS_INT_UART0INT, 0, AINTC_HOSTINT_ROUTE_IRQ);

    /* Enabling the system interrupt in AINTC. */
    IntSystemEnable(SYS_INT_UART0INT);
}
