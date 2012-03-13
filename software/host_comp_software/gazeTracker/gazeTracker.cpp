// OpenCV_HelloWorld.cpp : Defines the entry point for the console application.

#include "stdafx.h"
#include <stdio.h>
#include <highgui.h>
#include <time.h>
#include <conio.h>

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



#ifdef DEBUG
int _tmain(int argc, _TCHAR* argv[])
{
	return(1);
}
#endif


#ifdef CAPTURE_CAMERA
int _tmain(int argc, _TCHAR* argv[])
{
	printf("Capturing camerea...\n");

	int c;
	int i;

	int step = 1920;		//  Number of bytes in a row of image data
	int channels = 3;		//  RGB
	int isColor = 1;		//  
	int fps = 30;			//  Frames per second
	int depth = 8;			//  8 bits per channel per pixel

	uchar* data;			//  Type cast for image data
	int threshold = 75;
	
	//  Capture from video device #1
	CvCapture* capture = cvCaptureFromCAM(1);

	//  Modify capture resolution
	cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_WIDTH, 640);
	cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_HEIGHT, 480);

	#ifdef DISPLAY_OUTPUT
		//  Create a window to display the images
		cvNamedWindow("mainWin", CV_WINDOW_AUTOSIZE);
		//  Position the window
		cvMoveWindow("mainWin", 5, 5);
	#endif

	#ifdef RECORD_OUTPUT
		//  Setup output
		CvVideoWriter *writer = cvCreateVideoWriter("out.avi", CV_FOURCC('P','I','M','1'), fps, cvSize(640,480),isColor);
	#endif

	int frameW = cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH);
	int frameH = cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT);
	
	//  Allocate memory for an image
	IplImage *img;
	
	//  OpenCV reference says we shouldn't modify the output of cvQueryFrame
	//    so this is used to store a copy.
	IplImage *dst  = cvCreateImage(cvSize(frameW, frameH), depth, channels);

	//  Record FPS in camerea mode
	time_t t_start,t_end;
	time(&t_start);
	int counter = 0;
	while(1)
	{
		//  Retrieve the captured frame
		img = cvQueryFrame(capture);
		cvCopy( img, dst, NULL);
		
		//  In this version, getConnectedRegions thresholds and modifies dst
		getConnectedRegions(dst, threshold);
		
		#ifdef DISPLAY_OUTPUT
			//  Show the image in the window
			cvShowImage("mainWin", dst );
		#endif

		//  Calculate FPS and output
		time(&t_end);
		++counter;
		double sec = difftime(t_end, t_start);
		double fps = counter/sec;
		printf("FPS: %lf, Threshold: %u\n",fps,threshold);

		//  Wait 10 ms for a key to be pressed
		c = cvWaitKey(10);

		// escape key terminates program
		if(c == 27)
			break;
		if(c == 'a')
			threshold++;
		if(c == 's')
			threshold--;

		#ifdef RECORD_OUTPUT
			//  Output to video file
			cvWriteFrame(writer,dst);
		#endif
	}

	#ifdef RECORD_OUTPUT
		//  Release video writer
		cvReleaseVideoWriter(&writer);
	#endif

	//  Release capture 
	cvReleaseCapture(&capture);

	return 0;
}



#else



int _tmain(int argc, _TCHAR* argv[])
{
	printf("Capturing video...\n");

	int c;
	int i;

	int step = 1920;		//  Number of bytes in a row of image data
	int channels = 3;		//  RGB
	int isColor = 1;		//  
	int fps = 30;			//  Frames per second
	int depth = 8;			//  8 bits per channel per pixel

	int stepVideoOn = 1;
	int stepVideoCount = 0;

	uchar* data;			//  Type cast for image data
	int threshold = 75;
	
	//  Capture video from file
	CvCapture* capture = cvCaptureFromAVI("Nick_Far.avi");
	int numFrames = (int) cvGetCaptureProperty(capture,  CV_CAP_PROP_FRAME_COUNT);	
	
	#ifdef DISPLAY_OUTPUT
		//  Create a window to display the images
		cvNamedWindow("mainWin", CV_WINDOW_AUTOSIZE);
		//  Position the window
		cvMoveWindow("mainWin", 5, 5);
	#endif

	#ifdef RECORD_OUTPUT
		//  Setup output
		CvVideoWriter *writer = cvCreateVideoWriter("out.avi", CV_FOURCC('P','I','M','1'), fps, cvSize(640,480),isColor);
	#endif

	int frameW = cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH);
	int frameH = cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT);
	//  Allocate memory for an image
	IplImage *img;
	//  OpenCV reference says we shouldn't modify the output of cvQueryFrame
	//    so this is used to store a copy.
	IplImage *dst  = cvCreateImage(cvSize(frameW, frameH), depth, channels);

	//  Record FPS in camerea mode
	for(i = 0; i < numFrames; ++i)
	{
		//  Retrieve the captured frame
		img = cvQueryFrame(capture);
		cvCopy( img, dst, NULL);
		
		//  In this version, getConnectedRegions thresholds and modifies dst
		getConnectedRegions(dst, threshold);
		
		#ifdef DISPLAY_OUTPUT
			//  Show the image in the window
			cvShowImage("mainWin", dst );
			cvWaitKey(10);
		#endif

		#ifdef VIDEO_STEP_THROUGH
			if(stepVideoOn == 1)
			{
				if(stepVideoCount == 0)
				{
					printf("Current Frame: %u\n", i+1);
					c = getch();
					if(c == 't')				//  Skip 10 frames
						stepVideoCount = 10;
					else if(c == 'f')			//  Skip 50 frames
						stepVideoCount = 50;
					else if(c == 'e')			//  Skip to end
						stepVideoOn = 0;
				}
				else
					--stepVideoCount;
			}
		#endif
		
		#ifdef RECORD_OUTPUT
			//  Output to video file
			cvWriteFrame(writer,dst);
		#endif
	}

	#ifdef RECORD_OUTPUT
		//  Release video writer
		cvReleaseVideoWriter(&writer);
	#endif

	//  Release capture 
	cvReleaseCapture(&capture);

	return 0;
}
#endif