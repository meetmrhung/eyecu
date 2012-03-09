//#include "inc/lm3s6965.h"
#include "inc/hw_i2c.h"
#include "inc/hw_memmap.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/i2c.h"
#include "driverlib/sysctl.h"

#define		I2C0_BIT_DELAY		100
#define		I2C0_PERIPH_GIPO	SYSCTL_PERIPH_GPIOB
#define		I2C0_PIN_SCL		GPIO_PIN_0



//SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
static void GPIO_Config(void)
{

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);				//Enable GPIO clock
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);				//Enable GPIO clock
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);				//Enable GPIO clock
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);				//Enable GPIO clock
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);				//Enable GPIO clock
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);				//Enable GPIO clock
	GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 );	
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_1);		//Configure output pins for LED
    GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, GPIO_PIN_0 | GPIO_PIN_1);		//Configure output pins for LED
	GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);		//Configure output pins for LED
	GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_0);		//Configure output pins for LED
  	GPIOPinTypeGPIOOutput(GPIO_PORTG_BASE, GPIO_PIN_0);		//Configure output pins for LED
    //
    // Set the clocking to run directly from the crystal.
    //
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_8MHZ);

    //
    // Initialize the OLED display.
    //
    RIT128x96x4Init(1000000);
 
	    //
        // Display the text.
        //
	RIT128x96x4StringDraw("Mike is awesome!", 30, 24, 15);

}

static void blink(void)
{
	volatile unsigned long ulLoop;
    for(ulLoop = 0; ulLoop < 200000; ulLoop++)
    {
    }	

	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0x01);		//Turn LED ON
	for(ulLoop = 0; ulLoop < 200000; ulLoop++)
    {
    }	
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0x00);		//Turn LED OFF
    for(ulLoop = 0; ulLoop < 200000; ulLoop++)
    {
    }	
}
//static void toggle_pin(void)
//{
//	volatile unsigned long ulLoop;
//	
//
//	GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_0, 0x01);		//Turn LED ON
//	for(ulLoop = 0; ulLoop < 200000; ulLoop++)
//    {
//    }	
//    GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_0, 0x00);		//Turn LED OFF
//    for(ulLoop = 0; ulLoop < 200000; ulLoop++)
//    {
//    }	
//}
static void toggle_pin(void)
{
	volatile unsigned long ulLoop;
	

	GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, 0x01);		//Turn LED ON
	for(ulLoop = 0; ulLoop < 200000; ulLoop++)
    {
    }	
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, 0x00);		//Turn LED OFF
    for(ulLoop = 0; ulLoop < 200000; ulLoop++)
    {
    }	
}
static void I2C0_SCL_low(unsigned long ulBase, unsigned char ucPin)
{
	GPIOPinWrite(ulBase, GPIO_PIN_0,0x00);				//SCL LOW
//	SysCtlDelay(I2C0_BIT_DELAY);								//Delay
}

static void I2C0_SCL_high(unsigned long ulBase, unsigned char ucPin)
{
	GPIOPinWrite(ulBase, GPIO_PIN_0, 0x01);	//SCL High
//	SysCtlDelay(I2C0_BIT_DELAY);								//Delay
}
	
static void I2C0_SCL_pulse(unsigned long ulBase, unsigned char ucPin)
{
	volatile unsigned long ulLoop;
	I2C0_SCL_high(ulBase, GPIO_PIN_0);											//pulse SCL high
	for(ulLoop = 0; ulLoop < 200000; ulLoop++)
    {
    }
	I2C0_SCL_low(ulBase, GPIO_PIN_0);												//SCL low
	for(ulLoop = 0; ulLoop < 200000; ulLoop++)
    {
    }
}



void main(void)
{
	volatile unsigned long ulLoop;
	GPIO_Config();		
	
	
	I2C0_SCL_low(GPIO_PORTD_BASE,GPIO_PIN_0);
//	GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_1, 0x00);	
	I2C0_SCL_low(GPIO_PORTG_BASE,GPIO_PIN_0);
//	for(ulLoop = 0; ulLoop < 200000; ulLoop++)
//    {
//    }			
	while(1)
	{
//		GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_1, 0x10);
//		for(ulLoop = 0; ulLoop < 200000; ulLoop++)
//	    {
//	    }	
		I2C0_SCL_pulse(GPIO_PORTD_BASE,GPIO_PIN_0);	
		blink();
		I2C0_SCL_pulse(GPIO_PORTG_BASE,GPIO_PIN_0);	
//		GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_1, 0x00);	
//		for(ulLoop = 0; ulLoop < 200000; ulLoop++)
//	    {
//	    }		
//		toggle_pin();
	}							
}

