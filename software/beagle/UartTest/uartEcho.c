/*
 * \file   uartEcho.c
 *
 * \brief  This is a sample application file which invokes some APIs
 *         from the UART/IrDA/CIR device abstraction library to perform
 *         configuration, transmission and reception operations.
 */

/* Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 */

#include "uart_irda_cir.h"
#include "soc_AM335x.h"
#include "interrupt.h"
#include "beaglebone.h"
#include "hw_types.h"

#include "hsi2c.h"
#include "gpio_v2.h"

#include "hw_control_AM335x.h"
#include "hw_cm_wkup.h"
#include "hw_cm_per.h"

/******************************************************************************
**              INTERNAL MACRO DEFINITIONS
******************************************************************************/
#define BAUD_RATE_115200          (115200)
#define UART_MODULE_INPUT_CLK     (48000000)

/* I2C address of TMP275 temperature sensor */
#define  I2C_SLAVE_ADDR         (0x48)

#define GPIO_INSTANCE_ADDRESS           (SOC_GPIO_1_REGS)
#define GPIO_INSTANCE_PIN_NUMBER        (23)
#define GPIO_BASE0						(SOC_GPIO_0_REGS)
#define GPIO_0_4						(4)
#define GPIO_0_5						(5)
#define GPIO_1_17						(17)
#define CONTROL_CONF_GPMC1_AD(n)   (0x804 + (n * 4))
#define GPIO_BASE2						(SOC_GPIO_2_REGS)
#define GPIO_2_3						(3)

/******************************************************************************
**              INTERNAL FUNCTION PROTOTYPES
******************************************************************************/
static void UartInterruptEnable(void);
static void UART0AINTCConfigure(void);
static void UartFIFOConfigure(void);
static void UartBaudRateSet(void);
static void UARTIsr(void);

void I2CIsr(void);
static void SetupI2C(void);
static void I2CAintcConfigure(void);
static void cleanupInterrupts(void);
static void SetupI2CTransmit(unsigned int dcount);
static void SetupI2CReception(unsigned int dcount);

static void Delay(unsigned int count);
void GPIOPinPinMuxSetup(void);
void GPIO2ModuleClkConfig(void);

/******************************************************************************
**              GLOBAL VARIABLE DEFINITIONS
******************************************************************************/
unsigned char txArray[] = "StarterWare AM335X UART Interrupt application\r\n";

volatile unsigned char dataFromSlave[2];
volatile unsigned char dataToSlave[3];
volatile unsigned int numOfBytes;
volatile unsigned int flag = 1;
volatile unsigned int tCount;
volatile unsigned int rCount;

/******************************************************************************
**              FUNCTION DEFINITIONS
******************************************************************************/

int main()
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

    /* Enable IRQ in CPSR */
    IntMasterIRQEnable();

    /* Configures AINTC to generate interrupt */
    I2CAintcConfigure();

    /*
    ** Configures HSI2C to Master mode to generate start
    ** condition on HSI2C bus and to transmit data at a
    ** speed of 100khz
    */
    SetupI2C();

    /* Performing Interrupt configurations. */
    UartInterruptEnable();

    dataToSlave[0] = 0x45;
    dataToSlave[1] = 0xfd;


    /* Enabling functional clocks for GPIO0 instance. */
    GPIO1ModuleClkConfig();
    GPIO0ModuleClkConfig();
    GPIO2ModuleClkConfig();

    /* Selecting GPIO0[19] pin for use. */
    GPIO1Pin23PinMuxSetup();
    GPIOPinPinMuxSetup();

    /* Enabling the GPIO module. */
    GPIOModuleEnable(GPIO_INSTANCE_ADDRESS);
    GPIOModuleEnable(GPIO_BASE0);
    GPIOModuleEnable(GPIO_BASE2);

    /* Resetting the GPIO module. */
    GPIOModuleReset(GPIO_INSTANCE_ADDRESS);
    GPIOModuleReset(GPIO_BASE0);
    GPIOModuleReset(GPIO_BASE2);

    /* Setting the GPIO pin as an output pin. */
    GPIODirModeSet(GPIO_INSTANCE_ADDRESS,
                   GPIO_INSTANCE_PIN_NUMBER,
                   GPIO_DIR_OUTPUT);
    GPIODirModeSet(GPIO_INSTANCE_ADDRESS,
    			   GPIO_1_17,
    			   GPIO_DIR_OUTPUT);
    GPIODirModeSet(GPIO_BASE0, GPIO_0_5, GPIO_DIR_OUTPUT);
    GPIODirModeSet(GPIO_BASE2, GPIO_2_3, GPIO_DIR_OUTPUT);
    while(1)
    {
        /* Driving a logic HIGH on the GPIO pin. */
        GPIOPinWrite(GPIO_INSTANCE_ADDRESS,
                     GPIO_INSTANCE_PIN_NUMBER,
                     GPIO_PIN_HIGH);
        GPIOPinWrite(GPIO_INSTANCE_ADDRESS,
        			 GPIO_1_17,
            		 GPIO_PIN_HIGH);
        GPIOPinWrite(GPIO_BASE0,GPIO_0_4,GPIO_PIN_HIGH);
        GPIOPinWrite(GPIO_BASE2,GPIO_2_3,GPIO_PIN_HIGH);

        Delay(0x3FFFF);

        /* Driving a logic LOW on the GPIO pin. */
        GPIOPinWrite(GPIO_INSTANCE_ADDRESS,
                     GPIO_INSTANCE_PIN_NUMBER,
                     GPIO_PIN_LOW);
        GPIOPinWrite(GPIO_INSTANCE_ADDRESS,
                	 GPIO_1_17,
               		 GPIO_PIN_LOW);
        GPIOPinWrite(GPIO_BASE0,GPIO_0_4,GPIO_PIN_LOW);
        GPIOPinWrite(GPIO_BASE2,GPIO_2_3,GPIO_PIN_LOW);

        Delay(0x3FFFF);
    }
}

/*
** A wrapper function performing FIFO configurations.
*/

static void UartFIFOConfigure(void)
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

static void UartBaudRateSet(void)
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

static void UartInterruptEnable(void)
{
    /* Enabling IRQ in CPSR of ARM processor. */
    IntMasterIRQEnable();

    /* Configuring AINTC to receive UART0 interrupts. */
    UART0AINTCConfigure();

    /* Enabling the specified UART interrupts. */
    UARTIntEnable(SOC_UART_0_REGS, (UART_INT_LINE_STAT | UART_INT_THR |
                                    UART_INT_RHR_CTI));
}

/*
** Interrupt Service Routine for UART.
*/

static void UARTIsr(void)
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

static void UART0AINTCConfigure(void)
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
/*
** Configures i2c bus frequency and slave address.
** It also enable the clock for i2c module and
** does pinmuxing for the i2c.
*/
static void SetupI2C(void)
{
   I2C1ModuleClkConfig();

   I2CPinMuxSetup(1);

   /* Put i2c in reset/disabled state */
   I2CMasterDisable(SOC_I2C_1_REGS);

   /*
   ** Upon reset Auto Idle is enabled.
   ** Hence it is disabled after reset
   */
   I2CAutoIdleDisable(SOC_I2C_1_REGS);

   /* Configure i2c bus speed to 100khz */
   I2CMasterInitExpClk(SOC_I2C_1_REGS, 48000000, 12000000, 100000);

   /* Set i2c slave address */
   I2CMasterSlaveAddrSet(SOC_I2C_1_REGS, I2C_SLAVE_ADDR);

   /* Bring I2C module out of reset */
   I2CMasterEnable(SOC_I2C_1_REGS);
}

/*
** Transmits data over I2C bus
*/
static void SetupI2CTransmit(unsigned int dcount)
{
   /* Data Count specifies the number of bytes to be transferred */
   I2CSetDataCount(SOC_I2C_1_REGS, dcount);

   numOfBytes = I2CDataCountGet(SOC_I2C_1_REGS);


   cleanupInterrupts();

   /*
   ** Configure I2C controller in Master Transmitter mode.A stop
   ** condition will be generated after data count number of
   ** bytes are transferred.
   */
   I2CMasterControl(SOC_I2C_1_REGS, I2C_CFG_MST_TX | I2C_CFG_STOP);

   /* Transmit and Stop Condition Interrupts are enabled */
   I2CMasterIntEnableEx(SOC_I2C_1_REGS, I2C_INT_TRANSMIT_READY |
                                        I2C_INT_STOP_CONDITION );

   /* Generated Start Condition over I2C bus */
   I2CMasterStart(SOC_I2C_1_REGS);

   while(flag);

   /* Wait untill I2C registers are ready to access */
   while(0 == (I2CMasterIntRawStatus(SOC_I2C_1_REGS) & I2C_INT_ADRR_READY_ACESS));

   flag = 1;
}

/*
** Receives data over I2C bus
*/
static void SetupI2CReception(unsigned int dcount)
{
   /* Data Count specifies the number of bytes to be transmitted */
   I2CSetDataCount(SOC_I2C_1_REGS, 1);

   numOfBytes = I2CDataCountGet(SOC_I2C_1_REGS);

   cleanupInterrupts();

   /* Configure I2C controller in Master Transmitter mode */
   I2CMasterControl(SOC_I2C_1_REGS, I2C_CFG_MST_TX);

   /* Transmit interrupt is enabled */
   I2CMasterIntEnableEx(SOC_I2C_1_REGS, I2C_INT_TRANSMIT_READY);

   /* Generate Start Condition over I2C bus */
   I2CMasterStart(SOC_I2C_1_REGS);

   while(tCount != numOfBytes);

   /* Wait untill I2C registers are ready to access */
   while(0 == (I2CMasterIntRawStatus(SOC_I2C_1_REGS) & I2C_INT_ADRR_READY_ACESS));

   /* Data Count specifies the number of bytes to be received */
   I2CSetDataCount(SOC_I2C_1_REGS, dcount);

   numOfBytes = I2CDataCountGet(SOC_I2C_1_REGS);

   cleanupInterrupts();

   /* Configure I2C controller in Master Receiver mode */
   I2CMasterControl(SOC_I2C_1_REGS, I2C_CFG_MST_RX);

   /* Receive and Stop Condition Interrupts are enabled */
   I2CMasterIntEnableEx(SOC_I2C_1_REGS, I2C_INT_RECV_READY |
                                        I2C_INT_STOP_CONDITION);

   /* Generate Start Condition over I2C bus */
   I2CMasterStart(SOC_I2C_1_REGS);

   while(flag);

   flag = 1;
}

/* Configures AINTC to generate interrupt */
static void I2CAintcConfigure(void)
{
   /* Intialize the ARM Interrupt Controller(AINTC) */
   IntAINTCInit();

   /* Registering I2C0 ISR in AINTC */
   IntRegister(SYS_INT_I2C1INT, I2CIsr);

   /* Setting the priority for the system interrupt in AINTC. */
   IntPrioritySet(SYS_INT_I2C1INT, 0, AINTC_HOSTINT_ROUTE_IRQ );

   /* Enabling the system interrupt in AINTC. */
   IntSystemEnable(SYS_INT_I2C1INT);
}

static void cleanupInterrupts(void)
{
   I2CMasterIntEnableEx(SOC_I2C_1_REGS, 0x7FF);
   I2CMasterIntClearEx(SOC_I2C_1_REGS,  0x7FF);
   I2CMasterIntDisableEx(SOC_I2C_1_REGS, 0x7FF);
}


/*
** I2C Interrupt Service Routine. This function will read and write
** data through I2C bus.
*/
void I2CIsr(void)
{
   unsigned int status = 0;

   /* Get only Enabled interrupt status */
   status = I2CMasterIntStatus(SOC_I2C_1_REGS);

   /*
   ** Clear all enabled interrupt status except receive ready and
   ** transmit ready interrupt status
   */
   I2CMasterIntClearEx(SOC_I2C_1_REGS,
                       (status & ~(I2C_INT_RECV_READY | I2C_INT_TRANSMIT_READY)));

   if(status & I2C_INT_RECV_READY)
   {
        /* Receive data from data receive register */
        dataFromSlave[rCount++] = I2CMasterDataGet(SOC_I2C_1_REGS);

        /* Clear receive ready interrupt status */
        I2CMasterIntClearEx(SOC_I2C_1_REGS,  I2C_INT_RECV_READY);

        if(rCount == numOfBytes)
        {
             /* Disable the receive ready interrupt */
             I2CMasterIntDisableEx(SOC_I2C_1_REGS, I2C_INT_RECV_READY);

             /* Generate a STOP */
             I2CMasterStop(SOC_I2C_1_REGS);

        }


   }
   if (status & I2C_INT_TRANSMIT_READY)
   {
        /* Put data to data transmit register of i2c */
        I2CMasterDataPut(SOC_I2C_1_REGS, dataToSlave[tCount++]);

        /* Clear Transmit interrupt status */
        I2CMasterIntClearEx(SOC_I2C_1_REGS, I2C_INT_TRANSMIT_READY);

        if(tCount == numOfBytes)
        {
             /* Disable the transmit ready interrupt */
             I2CMasterIntDisableEx(SOC_I2C_1_REGS, I2C_INT_TRANSMIT_READY);

        }

   }

   if (status & I2C_INT_STOP_CONDITION)
   {
          /* Disable transmit data ready and receive data read interupt */
        I2CMasterIntDisableEx(SOC_I2C_1_REGS, I2C_INT_TRANSMIT_READY |
                                              I2C_INT_RECV_READY     |
                          I2C_INT_STOP_CONDITION);
        flag = 0;
   }

   if(status & I2C_INT_NO_ACK)
   {
        I2CMasterIntDisableEx(SOC_I2C_1_REGS, I2C_INT_TRANSMIT_READY  |
                                              I2C_INT_RECV_READY      |
                                              I2C_INT_NO_ACK          |
                                              I2C_INT_STOP_CONDITION);
        /* Generate a STOP */
        I2CMasterStop(SOC_I2C_1_REGS);

        flag = 0;
   }



   I2CEndOfInterrupt(SOC_I2C_1_REGS, 0);
}

/*
** A function which is used to generate a delay.
*/

static void Delay(volatile unsigned int count)
{
    while(count--);
}

void GPIOPinPinMuxSetup(void)
{
	HWREG(SOC_CONTROL_REGS + CONTROL_CONF_SPI0_CS0) = CONTROL_CONF_MUXMODE(7);
	HWREG(SOC_CONTROL_REGS + CONTROL_CONF_SPI0_D1) = CONTROL_CONF_MUXMODE(7);
	HWREG(SOC_CONTROL_REGS + CONTROL_CONF_GPMC_AD(2)) = CONTROL_CONF_MUXMODE(7);
	HWREG(SOC_CONTROL_REGS + CONTROL_CONF_GPMC_OEN_REN) = CONTROL_CONF_MUXMODE(7);
}

void GPIO2ModuleClkConfig(void)
{

    /* Writing to MODULEMODE field of CM_PER_GPIO2_CLKCTRL register. */
    HWREG(SOC_CM_PER_REGS + CM_PER_GPIO2_CLKCTRL) |=
          CM_PER_GPIO2_CLKCTRL_MODULEMODE_ENABLE;

    /* Waiting for MODULEMODE field to reflect the written value. */
    while(CM_PER_GPIO2_CLKCTRL_MODULEMODE_ENABLE !=
          (HWREG(SOC_CM_PER_REGS + CM_PER_GPIO2_CLKCTRL) &
           CM_PER_GPIO2_CLKCTRL_MODULEMODE));
    /*
    ** Writing to OPTFCLKEN_GPIO_2_GDBCLK bit in CM_PER_GPIO2_CLKCTRL
    ** register.
    */
    HWREG(SOC_CM_PER_REGS + CM_PER_GPIO2_CLKCTRL) |=
          CM_PER_GPIO2_CLKCTRL_OPTFCLKEN_GPIO_2_GDBCLK;

    /*
    ** Waiting for OPTFCLKEN_GPIO_2_GDBCLK bit to reflect the desired
    ** value.
    */
    while(CM_PER_GPIO2_CLKCTRL_OPTFCLKEN_GPIO_2_GDBCLK !=
          (HWREG(SOC_CM_PER_REGS + CM_PER_GPIO2_CLKCTRL) &
           CM_PER_GPIO2_CLKCTRL_OPTFCLKEN_GPIO_2_GDBCLK));

    /*
    ** Waiting for IDLEST field in CM_PER_GPIO2_CLKCTRL register to attain the
    ** desired value.
    */
    while((CM_PER_GPIO2_CLKCTRL_IDLEST_FUNC <<
           CM_PER_GPIO2_CLKCTRL_IDLEST_SHIFT) !=
           (HWREG(SOC_CM_PER_REGS + CM_PER_GPIO2_CLKCTRL) &
            CM_PER_GPIO2_CLKCTRL_IDLEST));

    /*
    ** Waiting for CLKACTIVITY_GPIO_2_GDBCLK bit in CM_PER_L4LS_CLKSTCTRL
    ** register to attain desired value.
    */
    while(CM_PER_L4LS_CLKSTCTRL_CLKACTIVITY_GPIO_2_GDBCLK !=
          (HWREG(SOC_CM_PER_REGS + CM_PER_L4LS_CLKSTCTRL) &
           CM_PER_L4LS_CLKSTCTRL_CLKACTIVITY_GPIO_2_GDBCLK));
}

/******************************* End of file *********************************/
