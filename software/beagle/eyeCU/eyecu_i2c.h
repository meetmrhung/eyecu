/*
 *
 *
 *
 */

#ifndef EYECU_I2C_H_
#define EYECU_I2C_H_

extern volatile unsigned char dataFromSlave[2];
//extern volatile unsigned char dataToSlave[10];
//extern volatile unsigned int numOfBytes;
//extern volatile unsigned int flag;
//extern volatile unsigned int tCount;
//extern volatile unsigned int rCount;

void i2cTransmit(unsigned int dcount, unsigned char data[]);
void i2cRead(unsigned int dcount);
void SetupI2C(unsigned int slaveAddr);
void I2CAintcConfigure(void);
void cleanupInterrupts(void);


#endif /*EYECU_I2C_H_*/
