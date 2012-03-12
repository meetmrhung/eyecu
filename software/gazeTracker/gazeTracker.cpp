// OpenCV_HelloWorld.cpp : Defines the entry point for the console application.

#include "stdafx.h"
#include <stdio.h>
#include <highgui.h>
#include <time.h>

#include "eyeFeatureExtraction.h"
#include "eFEParam.h"

//  Initialize program parameters
param p = {
	"Nick_Far.avi",		//  inFile
	"out.avi",			//  outFile

	100,				//  iStart
	300,				//  iFinish
	150,				//  jStart
	350,				//  jFinish

	0.5,				//  aspectMin
	2.0,				//  aspectMax

	300,				//  refSize
	0.7,				//  refSizeMinRatio
	1.3,				//  refSizeMaxRatio
	210,				//  refSizeMin
	390,				//  refSizeMax
	
	{0,0},				//  refCentroid
	50,					//  initThreshold

	0,					//  nFrames
	0					//  startFrame
};

int _tmain(int argc, _TCHAR* argv[])
{

#ifdef CAPTURE_CAMERA
	printf("Capturing camerea...\n");
#else
	printf("Capturing video...\n");
#endif

#ifdef CAPTURE_CAMERA
	int c;
#else
	int i;
#endif

	int step = 1920;		//  Number of bytes in a row of image data
	int channels = 3;		//  RGB
	int isColor = 1;		//  
	int fps = 30;			//  Frames per second
	int depth = 8;			//  8 bits per channel per pixel

	uchar* data;			//  Type cast for image data

	
#ifdef DEBUG
	return(1);				//  Used to verify that program compiles then exit.
#endif

#ifdef CAPTURE_CAMERA
	//  Capture from video device #1
	CvCapture* capture = cvCaptureFromCAM(1);
	//  Modify capture resolution
	cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_WIDTH, 640);
	cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_HEIGHT, 480);

	//  Create a window to display the images
	cvNamedWindow("mainWin", CV_WINDOW_AUTOSIZE);
	//  Position the window
	cvMoveWindow("mainWin", 5, 5);
#else
	//  Capture video from file
	CvCapture* capture = cvCaptureFromAVI("Nick_Far.avi");
	int numFrames = (int) cvGetCaptureProperty(capture,  CV_CAP_PROP_FRAME_COUNT);
	//  Setup output
	CvVideoWriter *writer = cvCreateVideoWriter("out.avi", CV_FOURCC('P','I','M','1'), fps, cvSize(frameW,frameH),isColor);
#endif	
	
	int frameW = cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH);
	int frameH = cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT);
	//  Allocate memory for an image
	IplImage *img;
	//  OpenCV reference says we shouldn't modify the output of cvQueryFrame
	//    so this is used to store a copy.
	IplImage *dst  = cvCreateImage(cvSize(frameW, frameH), depth, channels);

	//  Record FPS in camerea mode
#ifdef CAPTURE_CAMERA	
	time_t t_start,t_end;
	time(&t_start);
	int counter = 0;
	while(1)
#else
	for(i = 0; i < numFrames; ++i)
#endif
	{
		//  Retrieve the captured frame
		img = cvQueryFrame(capture);
		cvCopy( img, dst, NULL);
		
		//  In this version, getConnectedRegions thresholds and modifies dst
		getConnectedRegions(dst, 75);
		
#ifdef CAPTURE_CAMERA
		//  Show the image in the window
		cvShowImage("mainWin", dst );
		
		//  Calculate FPS and output
		time(&t_end);
		++counter;
		double sec = difftime(t_end, t_start);
		double fps = counter/sec;
		printf("%lf\n",fps);

		//  Wait 10 ms for a key to be pressed
		c = cvWaitKey(10);

		// escape key terminates program
		if(c == 27)
			break;
#else
		//  Output to video file
		cvWriteFrame(writer,dst);
#endif
	}

#ifndef CAPTURE_CAMERA
	//  Release video writer
	cvReleaseVideoWriter(&writer);
#endif
	//  Release capture 
	cvReleaseCapture(&capture);

	return 0;
}
