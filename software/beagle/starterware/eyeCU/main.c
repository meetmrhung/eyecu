/*
 * \file   main.c
 *
 * \brief  This is the main function for the eyeCU eye tracking interface
 * it also uses i2c1, uart, USB, fatfs
 *
 * \author Mike Mozingo
 */

#include "soc_AM335x.h"
#include "interrupt.h"
#include "beaglebone.h"
#include "hw_types.h"
#include "gpio_v2.h"

#include "eyecu_gpio.h"
#include "eyecu_i2c.h"
#include "uart.h"
#include "utilities.h"
#include "camera.h"
#include "eyecu_pins.h"

//#include "hw_control_AM335x.h"
//#include "hw_cm_wkup.h"
//#include "hw_cm_per.h"
//#include "hsi2c.h"

/******************************************************************************
 **              INTERNAL MACRO DEFINITIONS
 ******************************************************************************/
/******************************************************************************
 **              INTERNAL FUNCTION PROTOTYPES
 ******************************************************************************/
/******************************************************************************
 **              GLOBAL VARIABLE DEFINITIONS
 ******************************************************************************/
unsigned char i2cTransmitData[10];

/******************************************************************************
 **              FUNCTION DEFINITIONS
 ******************************************************************************/
int main()
{
	/* Enabling IRQ in CPSR of ARM processor. */
	IntMasterIRQEnable();

	/* Configures AINTC to generate interrupt */
	I2CAintcConfigure();

	// initalize uart
	uartInit();

	// initialize i2c
	SetupI2C(0x3c);

	// setup GPIO pins for use
	gpioInit();

	// initialize camera
	initCamera();

	// just switch pins/leds for now
	while (1)
	{
		/* Driving a logic HIGH on the GPIO pin. */
		GPIOPinWrite(GPIO_BASE1, 23,
				GPIO_PIN_HIGH);
		GPIOPinWrite(GPIO_BASE1, 17, GPIO_PIN_HIGH);
//        GPIOPinWrite(GPIO_BASE0,4,GPIO_PIN_HIGH);
		GPIOPinWrite(GPIO_BASE2, 3, GPIO_PIN_HIGH);

		delay(0x3FFFF);

		/* Driving a logic LOW on the GPIO pin. */
		GPIOPinWrite(GPIO_BASE1, 23,
				GPIO_PIN_LOW);
		GPIOPinWrite(GPIO_BASE1, 17, GPIO_PIN_LOW);
//        GPIOPinWrite(GPIO_BASE0,4,GPIO_PIN_LOW);
		GPIOPinWrite(GPIO_BASE2, 3, GPIO_PIN_LOW);

		delay(0x3FFFF);

	}
}
