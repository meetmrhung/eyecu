i/* Stellaris Peripheral Driver Library User's Guide
 * http://www.ti.com/lit/ug/spmu019m/spmu019m.pdf
 * Stellaris LM3S6965 Microcontroller Datasheet 
*/
/*
 ********* WRITE MODE ********
 * Start
 * Slave Address (MSB 7bit)	
 * 0
 * <Acknowledge>
 * Sub Address (8 bit)
 * <Acknowledge>
 * Data 1 (8 bit)
 * <Acknowledge>
 * Data n (8 bit)
 * <Acknowledge>
 * Stop
 * 
 ******** READ MODE ********
 * Start
 * Slave Address (MSB 7 bit)
 * 0
 * <Acknowledge>
 * Sub Address (8 bit)
 * <Acknowledge>
 * Start
 * Slave Address (MSB 7 bit)
 * 1
 * <Data 1 (8 bit)>
 * Acknowledge
 * <Data n (8 bit)>
 * Acknowledge
 * Stop
 */
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_i2c.h"
#include "driverlib/i2c.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "utils/uartstdio.h"

// Set the address for slave module. This is a 7-bit address sent in the
// following format:
//                      [A6:A5:A4:A3:A2:A1:A0:RS]
//
// A zero in the "RS" position of the first byte means that the master
// transmits (sends) data to the selected slave, and a one in this position
// means that the master receives data from the slave.
 #define	I2C0_SLAVE_BASE		0x78		//Camera
 #define	I2C0_MASTER_BASE	0x0000.0020	//Stellaris
 //Stellaris Slave Address: 0x0000.0000
 #define	SYSCTL_PERIPH_GPIOB				//%%%%%%%%%%%///
 #define	SYSCTL_PERIPH_I2C0				//%%%%%%%%%%%///
 ///////////////////////////////////////NOTES/////////////
 // SLAVE BASE ADDRESS 0x78 	//This is for write 
 								//read is 0x79
 // Camera Address 0x3c
  // Suggested camera command initialization order (by hex register address): 02,1e,03
 // RapidFire order - 02,03,04,1E,03
 
 // DONT FORGET PULLUP RESISTORS
 ////////////////////////////////////////////////////////
 
 /*	Frame rate, frequency, DCLK polarity */
 //Address 02h	11XX XX00 - 0x30
 // Frame Rate			(0 = 30fps, 	1 = 15fps) 			B7
 // AC Freq	(flckr)		(0 = 50Hz,		1 = 60Hz)			B6
 // Polarity of DCLK	(0 = norm,  	1 = inverted)		B1
 // Auto flckr det		(0 = on,		1 = off)			B0
 
 /* Enable data outputs, set camera resolution to SUBQCIF full for testing, data output to format YUV422, and image color */
 //Address 03h	0010 0000 - 0x20
 // DOUTSW	dout,hd,vd	(0 = enable,	1 = disable)		B7
 // DATAHZ	dout,hd,vd	(0 = enable,	1 = disable HI-Z)	B6
 // PICSZ 	SUBQCIF full 128x96 - decimal 8, b1000 			B5-2 
 // PICFMT				(0 = YUV422,	1 = RGB565)			B1
 // CM					(0 = color,		1 = B/W)			B0
 
 /* Set image inversion preferences and exposure length/time */
 //Address 04h	1100 0000 - C0
 // V_INV vert flip		(0 = normal,	1 = vertical flip)	B7	
 // H_INV horizontal flp(0 = normal,	1 = hrzntl flip)	B6	
 // ESRLSW (fps expose)  0 = normal 15||30fps				B5-4
 //						 1 = long 3.75fps
 //					   2,3 = Extra long
 // V_LENGTH (only for ESRLSW 2-3)							B3-0
 //					   0,1 = 1 frame interval
 //					   2-F = 2-15 frame intervals
 
 //Address 05h	
 // ALCSW luminance crl (0 = ALC ON, 	1 = ALC OFF)		B7
 // ESRLIM max exp tim 0-3									B6-5	?? 0

 /*	Testing for Color bar, Enable Synchronizations*/
 //Address 1eh	0110 1100 - 6C 						
 // D_MASK 01h: 1V (one frame interval) mask				B7-6	
 // CODESW data out syn	(0 = w/o synch,	1 = w/ synch)		B5
 // CODESEL slct syn	(0 = FS FE etc,	1 = ITU656)			B4
 // HSYNCHSEL h syn		(0 = normal,	1 = blanking)		B3
 // TESTPIC	output test	(0 = test OFF,	1 = test ON)		B2
 // PICSEL test pattern										B1-0
 //						0 = color bar
 //						1 = ramp wave after gamma
 //						2 = ramp wave before gamma
 //						3 = normal picture
 
 void I2CInit()
 {
 	//Set clock to run directly from crystal
	SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_8MHZ);
	//Enable GPIOs for I2C
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	//Enable specific I2C0 pins
	SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
 	//Enable Master and Slave, set SCL speed
 	//		true  = 400kbps data transfer
 	//		false = 100kbps data transfer
	I2CMasterInitExpClk(I2C0_MASTER_BASE, SysCtlClockGet(), false);	 //Master addr, clk, scl

//		#Useless after I2CMasterInitExpClk()
//	 //Enable and Initialize Master/Slave
//	 I2CMasterEnable(MASTER_BASE);

}

void I2CWrite()
{
	//Set slave and define whether transfer is a send (write from master to slave), or receive.
	//		true  = I2C Master initiating READ from the Slave
	//		false = I2C Master initiating a WRITE to the Slave		
	I2CMasterSlaveAddrSet(I2C0_MASTER_BASE, I2C0_SLAVE_BASE, true);
	
			
}