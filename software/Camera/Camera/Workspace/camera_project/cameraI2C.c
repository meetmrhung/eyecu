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
 
/**********************************************************************************/
/* 									Includes                                      */
/**********************************************************************************/
 
#include "cameraI2C.h"


 ///////////////////////////////////////NOTES/////////////
 // SLAVE BASE ADDRESS 0x78 	//This is for write 
 								//read is 0x79
 // Camera Address 0x3c
  // Suggested camera command initialization order (by hex register address): 02,1e,03
 // RapidFire order - 02,03,04,1E,03
 
 // #######################DONT FORGET PULLUP RESISTORS
 ////////////////////////////////////////////////////////
 
 /*	Frame rate, frequency, DCLK polarity */
 //Address 02h	1100 0000 - 0xC0
 // Frame Rate			(0 = 30fps, 	1 = 15fps) 			B7
 // AC Freq	(flckr)		(0 = 50Hz,		1 = 60Hz)			B6
 // RESERVED												B5-2
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

//Name: I2CMCUInit()
//Precondition: No other function has been called
//Postcondition: Clock speed is set directly from crystal. GPIOs have been enabled
//	for I2C communication. Master/Slave have been enabled and the I2C SCL speed 
//	is set
//Description: Stellaris board I2C initialization
 void I2CMCUInit()
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

//Name: I2CInitCamera()
//Precondition: I2CMCUInit() has been called
//Postcondition: I2CMasterWrite has been called
//Description: Camera has been initialized and configured for operation.
//	Frame rate, Image output format, synchronizations, and output mode 
void I2CInitCamera()
{
	/*	Frame rate, frequency, DCLK polarity */
	I2CMasterWrite(0x02,0xC0);
	/*	Testing for Color bar, Enable Synchronizations*/
	I2CMasterWrite(0x1E,0x6C);	
	/* Enable data outputs, set camera resolution to SUBQCIF full for testing, data output to format YUV422, and image color */
	I2CMasterWrite(0x03,0x20);			
}

//Name: I2CMasterWrite()
//Precondition: I2CMCUInit() has been called
//Postcondition: Byte of data has been written to camera
//Description: Preforms necessary protocol to write a byte of data to camera
void I2CMasterWrite(unsigned char reg_address, unsigned char data)
{
	//###Don't forget to disable interrupts
	
	//Set slave and define whether transfer is a send (write from master to slave), or receive.
	//		true  = I2C Master initiating READ from the Slave
	//		false = I2C Master initiating a WRITE to the Slave		
	I2CMasterSlaveAddrSet(I2C0_MASTER_BASE, CAMERA_BASE, false);		//POSSIBLE SOURCE FOR ERROR, THIS FUNCTION SHIFTS SLAVE BY 1
	//Load SLAVE ADDRESS 	
	I2CMasterDataPut(I2C0_MASTER_BASE, CAMERA_BASE);
	//Send SLAVE ADDRESS with START condition
	I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_START);
	while(I2CMasterBusy(I2C0_MASTER_BASE)){}

	//Wait for ACK ###################
	//###ERROR CHECKING - unsigned long I2CMasterErr(unsigned long ulBase)
	
	//Load SLAVE REGISTER ADDRESS 	
	I2CMasterDataPut(I2C0_MASTER_BASE, reg_address);	
	//Send SLAVE REGISTER ADDRESS 
	I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
	//Wait until Master is idle 
	while(I2CMasterBusy(I2C0_MASTER_BASE)){}

	//Wait for ACK on the read after the write ##########
	//###ERROR CHECKING - unsigned long I2CMasterErr(unsigned long ulBase)	
	
	//Load DATA 
	I2CMasterDataPut(I2C0_MASTER_BASE, data);	
	//Send DATA with STOP CONDITION
	I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);		//###WHAT ABOUT GETTING ACK FROM SLAVE BEFORE SEND STOP?????
	//Wait until Master is idle 
	while(I2CMasterBusy(I2C0_MASTER_BASE)){}
	
	//###ERROR CHECKING - unsigned long I2CMasterErr(unsigned long ulBase)	
	
	
	//###Don't forget to enable interrupts
}
