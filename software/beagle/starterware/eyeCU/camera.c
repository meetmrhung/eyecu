#include "camera.h"
#include "eyecu_i2c.h"


/******************************************************************************
 **              FUNCTION DEFINITIONS
 ******************************************************************************/
char initCamera(void)
{
	int i;

	// TODO: fix config to RGB + 30fps etc


	// TODO: do I need to fix this? the camera does init, maybe by luck
	// wait for camera before bringing up reset
	// set cam-rst low
//	for (i = 0; i < 0xffff; ++i);
	// set cam-rst high

	// i2c initialization commands
	i2cTransmitData[0] = 0x03;
	i2cTransmitData[1] = 0x00;

	i2cTransmit(2, i2cTransmitData);

	i2cTransmitData[0] = 0x22;
	i2cTransmitData[1] = 0x28;

	i2cTransmit(2, i2cTransmitData);

	i2cTransmitData[0] = 0x02;
	i2cTransmitData[1] = 0xC0;

	i2cTransmit(2, i2cTransmitData);

	return 0;
}
void readFIFO(void)
{

}
char resetCamera(void)
{
	return 0;
}
