#ifndef CAMERAI2C_H_
#define CAMERAI2C_H_
/**********************************************************************************/
/* 									Includes                                      */
/**********************************************************************************/
 
 // Provides I2C0_MASTER_BASE
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_i2c.h"
// Provides all I2C_MASTER_CMD_...
#include "driverlib/i2c.h"
// Provides all SYSCTL_PERIPH_...
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "utils/uartstdio.h"


/**********************************************************************************/
/* 									Defines                                       */
/**********************************************************************************/
// Set the address for slave module. This is a 7-bit address sent in the
// following format:
//                      [A6:A5:A4:A3:A2:A1:A0:RS]
//
// A zero in the "RS" position of the first byte means that the master
// transmits (sends) data to the selected slave, and a one in this position
// means that the master receives data from the slave.
 #define	CAMERA_BASE		0x78		//Camera Base Address WRITE
//							0x79							  READ
//																	######%%%%%MAY CHANGE BASE TO 3C IF I2CMasterSlaveAddrSet SHIFTS THE ADDRESS!!!!

/*********************************************************************************/
/* 								    Prototypes                                	 */
/*********************************************************************************/

//Name: I2CMCUInit()
//Precondition: No other function has been called
//Postcondition: Clock speed is set directly from crystal. GPIOs have been enabled
//	for I2C communication. Master/Slave have been enabled and the I2C SCL speed 
//	is set
//Description: Stellaris board I2C initialization
void I2CMCUInit();

//Name: I2CInitCamera()
//Precondition: I2CMCUInit() has been called
//Postcondition: I2CMasterWrite has been called
//Description: Camera has been initialized and configured for operation.
//	Frame rate, Image output format, synchronizations, and output mode 
void I2CInitCamera();

//Name: I2CMasterWrite()
//Precondition: I2CMCUInit() has been called
//Postcondition: Byte of data has been written to camera
//Description: Preforms necessary protocol to write a byte of data to camera
void I2CMasterWrite(unsigned char reg_address, unsigned char data);

#endif /*CAMERAI2C_H_*/
