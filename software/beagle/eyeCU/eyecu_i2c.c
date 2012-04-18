/*
 * \file   i2c.c
 *
 * \brief  This is an abstraction layer for interfacing to i2c1 on the
 * beagle bone
 *
 * \author Mike Mozingo
 */

/*
 * Some code is written by
 * Texas Instruments Incorporated - http://www.ti.com/
 */

#include "hsi2c.h"
#include "interrupt.h"
#include "soc_AM335x.h"
#include "beaglebone.h"
#include "eyecu_i2c.h"
//#include "cstdlib.h"

/******************************************************************************
 **              INTERNAL MACRO DEFINITIONS
 ******************************************************************************/
#define     CONFIG_DATA(Resolution, Fault, POL, SD, TM, OS )   SD |    \
                                                               TM << 1 | \
                                                               OS << 7 |  \
                                                               POL << 2 |  \
                                                               Fault << 3 | \
                                                               Resolution << 5;

/* I2C address of TMP275 temperature sensor */
#define  I2C_SLAVE_ADDR         (0x5f)

volatile unsigned char dataFromSlave[2];
unsigned char dataToSlave[10];
volatile unsigned int numOfBytes;
volatile unsigned int flag = 1;
volatile unsigned int tCount;
volatile unsigned int rCount;

/******************************************************************************
 **              GLOBAL VARIABLE DEFINITIONS
 ******************************************************************************/
/******************************************************************************
 **              INTERNAL FUNCTION PROTOTYPES
 ******************************************************************************/
void I2CIsr(void);
/******************************************************************************
 **              FUNCTION DEFINITIONS
 ******************************************************************************/
void SetupI2C(unsigned int slaveAddr)
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
	I2CMasterInitExpClk(SOC_I2C_1_REGS, 48000000, 12000000, 150000);

	/* Set i2c slave address */
	I2CMasterSlaveAddrSet(SOC_I2C_1_REGS, slaveAddr);

	/* Bring I2C module out of reset */
	I2CMasterEnable(SOC_I2C_1_REGS);
}

/*
 ** Transmits data over I2C bus
 */
void i2cTransmit(unsigned int dcount, unsigned char data[])
{
	unsigned int i;
	for (i = 0; i < dcount; ++i)
		dataToSlave[i] = data[i];

	tCount = 0;



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
	I2CMasterIntEnableEx(SOC_I2C_1_REGS,
			I2C_INT_TRANSMIT_READY | I2C_INT_STOP_CONDITION);

	/* Generated Start Condition over I2C bus */
	I2CMasterStart(SOC_I2C_1_REGS);

	while (flag);

	/* Wait until I2C registers are ready to access */
	while (0 == (I2CMasterIntRawStatus(SOC_I2C_1_REGS) & I2C_INT_ADRR_READY_ACESS));

	flag = 1;
}

/*
 ** Receives data over I2C bus
 */
void i2cRead(unsigned int dcount)
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

	while (tCount != numOfBytes);

	/* Wait until I2C registers are ready to access */
	while (0 == (I2CMasterIntRawStatus(SOC_I2C_1_REGS) & I2C_INT_ADRR_READY_ACESS));

	/* Data Count specifies the number of bytes to be received */
	I2CSetDataCount(SOC_I2C_1_REGS, dcount);

	numOfBytes = I2CDataCountGet(SOC_I2C_1_REGS);

	cleanupInterrupts();

	/* Configure I2C controller in Master Receiver mode */
	I2CMasterControl(SOC_I2C_1_REGS, I2C_CFG_MST_RX);

	/* Receive and Stop Condition Interrupts are enabled */
	I2CMasterIntEnableEx(SOC_I2C_1_REGS,
			I2C_INT_RECV_READY | I2C_INT_STOP_CONDITION);

	/* Generate Start Condition over I2C bus */
	I2CMasterStart(SOC_I2C_1_REGS);

	while (flag);

	flag = 1;
}

/* Configures AINTC to generate interrupt */
void I2CAintcConfigure(void)
{
	/* Initialize the ARM Interrupt Controller(AINTC) */
	IntAINTCInit();

	/* Registering I2C0 ISR in AINTC */
	IntRegister(SYS_INT_I2C1INT, I2CIsr);

	/* Setting the priority for the system interrupt in AINTC. */
	IntPrioritySet(SYS_INT_I2C1INT, 0, AINTC_HOSTINT_ROUTE_IRQ);

	/* Enabling the system interrupt in AINTC. */
	IntSystemEnable(SYS_INT_I2C1INT);
}

void cleanupInterrupts(void)
{
	I2CMasterIntEnableEx(SOC_I2C_1_REGS, 0x7FF);
	I2CMasterIntClearEx(SOC_I2C_1_REGS, 0x7FF);
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

	if (status & I2C_INT_RECV_READY)
	{
		/* Receive data from data receive register */
		dataFromSlave[rCount++] = I2CMasterDataGet(SOC_I2C_1_REGS);

		/* Clear receive ready interrupt status */
		I2CMasterIntClearEx(SOC_I2C_1_REGS, I2C_INT_RECV_READY);

		if (rCount == numOfBytes)
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

		if (tCount == numOfBytes)
		{
			/* Disable the transmit ready interrupt */
			I2CMasterIntDisableEx(SOC_I2C_1_REGS, I2C_INT_TRANSMIT_READY);

		}

	}

	if (status & I2C_INT_STOP_CONDITION)
	{
		/* Disable transmit data ready and receive data read interrupt */
		I2CMasterIntDisableEx(
				SOC_I2C_1_REGS,
				I2C_INT_TRANSMIT_READY | I2C_INT_RECV_READY
						| I2C_INT_STOP_CONDITION);
		flag = 0;
	}

	if (status & I2C_INT_NO_ACK)
	{
		I2CMasterIntDisableEx(
				SOC_I2C_1_REGS,
				I2C_INT_TRANSMIT_READY | I2C_INT_RECV_READY | I2C_INT_NO_ACK
						| I2C_INT_STOP_CONDITION);
		/* Generate a STOP */
		I2CMasterStop(SOC_I2C_1_REGS);

		flag = 0;
	}

	I2CEndOfInterrupt(SOC_I2C_1_REGS, 0);
}
