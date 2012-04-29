#ifndef CAMERA_H_
#define CAMERA_H_

extern unsigned char i2cTransmitData[10];

char initCamera(void);
void readFIFO(void);
char resetCamera(void); // TODO:

#endif /* CAMERA_H_ */
