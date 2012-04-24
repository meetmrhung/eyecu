#include "stdafx.h"
#include <stdio.h>
#include <highgui.h>
#include <time.h>
#include <conio.h>
#include <Windows.h>

#include "eyeFeatureExtraction.h"
#include "eFEParam.h"

//  Initialize program parameters
//  * Indicates value is set based on other parameters
//    (computation done in storageInit()
param p = {
	"Nick_Capstone.avi",//  inFile Nick
	//"Nick_Far.avi",	//  inFile Armeen
	"out.avi",	//  outFile

	0,					//  imgWidth
	0,					//  imgHeight

	170,				//  iStart
	270,				//  iFinish
	270,				//  jStart
	420,				//  jFinish

	0.6,				//  aspectMin
	2.0,				//  aspectMax

	27.91,				// lengthRegion
	1.4,				// legthMaxRatio
	0.6,				// lengthMinRatio

	516,				//  refSize
	0.6,				//  refSizeMinRatio
	1.8,				//  refSizeMaxRatio
	0,					//  refSizeMin*
	0,					//  refSizeMax*

	0,					//  procRegioniSize*
	0,					//  procRegionjSize*
	0,					//  totalPixels (procRegioniSize * procRegionjSize * MAX_TOTAL_REGIONS)

	{352,214},			//  refCentroid
	45,					//  initThreshold

	0.7,				// numStdVertical
	1.5,				// numStdHorizontal

	8,					//  minxChangeL
	8,					//  minxChangeR
	14,					//  minyChangeU
	5,					//  minyChangeD
	100,				//  maxcentroidChange

	5,					//	maxNumFrames
	10,					// minBlinks

	4,					//  maxAdaptations
	{8,4,2,1},			//  magThreshChange

	0,					//  nFrames
	0					//  startFrame
};

point dirArray[5] = {{0,0},{1,0},{-1,0},{0,1},{0,-1}};

point* cRPointList;					//  cRPointList[I2D(k,i)] = ith point of region k
point* removedPoints;				//  Keeps track of points removed in removeAberrations
int removedPointCount;				//  Keeps track of number of points removed in removeAberrations
unsigned char* cRBinary;			//  cRBinary[I3D(k,x,y)] = 1 if (x,y) is in region k
int* cRMap;							//  cRMap[I3D(k,x,y)] = index of point (x,y) in region k in crPointList
int* cRSizes0;						//  Region sizes before clean up
int* cRSizes1;						//  Region sizes after clean up
unsigned char* processedPixels;		//  Used in flood fill algorithm to keep track of already filled pixels
unsigned char* gSImg;				//  Greyscale version of entire image gSImg[2DFULL(x,y)]
int* candidateRegionIndices;		//  Stores index of regions that meet aspect ratio test
int candidateRegionCount;			//  Number of regions that meet aspect ratio test
double* cRAspectRatio;				//  Array storing aspect ratios
int cRCount;						//  Total number of connected regions meeting size requirement
int maxRegionSize = 0;				//	Max region size found in getConnectedRegions
int doCalibration = 0;				//  Stores pupil size and centroid as reference if set to 1
point centroid;
int consecDirFrame;					//  Counts number of consecutive frames user is looking in particular direction
enum resultType procResult;			//  Stores processing result (see definition)
int cursorCommand = 2;				//	Stores the cursor command (related to procResult). Originally assume that user is looking in the middle
int prevResultType;					//  stores the processing result of the previous frame. Used to assure consecutive number of frames in particular direction. 
double maxLengthConnected;			//  Maximum length of the connected region allowed to pass as the pupil
double minLengthConnected;			//  Minimum length of the connected region allowed to pass as the pupil
int cursorSpeed = 5;				
point prevCentroid;					// Keeps track of previous centroid
int numBlinks = 0;


void computeParameters(int width, int height)
{
	p.imgWidth = width;
	p.imgHeight = height;
	p.refSizeMin = p.refSize * p.refSizeMinRatio;
	p.refSizeMax = p.refSize * p.refSizeMaxRatio;
	p.procRegioniSize = p.iFinish - p.iStart + 1;
	p.procRegionjSize = p.jFinish - p.jStart + 1;

	p.totalPixels = MAX_TOTAL_REGIONS * p.procRegioniSize * p.procRegionjSize;
}
void storageInit()
{
	cRPointList = (point*)malloc(p.totalPixels * sizeof(point));
	removedPoints = (point*)malloc(p.totalPixels * sizeof(point));
	cRBinary = (unsigned char*)malloc(p.totalPixels * sizeof(unsigned char));
	cRMap = (int*)malloc(p.totalPixels * sizeof(int));
	cRSizes0 = (int*)malloc(MAX_TOTAL_REGIONS * sizeof(int));
	cRSizes1 = (int*)malloc(MAX_TOTAL_REGIONS * sizeof(int));
	cRAspectRatio = (double*)malloc(MAX_TOTAL_REGIONS * sizeof(double));
	candidateRegionIndices = (int*)malloc(MAX_TOTAL_REGIONS * sizeof(int));
	processedPixels = (unsigned char*) malloc( (p.imgWidth ) * (p.imgHeight) * sizeof(unsigned char) );
	gSImg = (unsigned char*) malloc( (p.imgWidth) * (p.imgHeight) * sizeof(unsigned char) );
	if(  cRPointList == 0 || cRBinary == 0 || cRMap == 0 || cRSizes0 == 0 || cRSizes1 == 0 || processedPixels == 0 || gSImg == 0 || removedPoints == 0 || cRAspectRatio == 0 || candidateRegionIndices == 0)
		printf("Error allocating memory\n");
	return;

}

void storageDestroy()
{
	free(cRPointList);
	free(removedPoints);
	free(cRBinary);
	free(cRMap);
	free(cRSizes0);
	free(cRSizes1);
	free(cRAspectRatio);
	free(candidateRegionIndices);
	free(processedPixels);
	free(gSImg);
	return;
}

#ifdef DEBUG_MAIN
int _tmain(int argc, _TCHAR* argv[])
{
	int i;
	point pt = {1,2};
	pointStackElement* stackHead = (pointStackElement*)malloc(sizeof(pointStackElement));
	stackHead->nextElement = 0;
	stackHead->elementData = pt;

	for(i = 0; i < 5; i++)
	{
		++(pt.x); ++(pt.y);
		pointStackPush(&stackHead, pt);
	}

	pointStackPrint(stackHead);

	printf("Pop top two\n");
	pointStackPop(&stackHead);pointStackPop(&stackHead);
	pointStackPrint(stackHead);

	printf("Pop remaining\n");
	while(stackHead != 0)
		pointStackPop(&stackHead);
	pointStackPrint(stackHead);

	printf("Allocating storage\n");
	_getch();
	storageInit();
	int loc = I3D( (p.maxTotalRegions-1), (p.iFinish-p.iStart), (p.jFinish-p.jStart) );
	cRBinary[loc] = 25;
	cRMap[loc] = 25;
	printf("Deallocating storage\n");
	_getch();
	storageDestroy();

	_getch();
	return(1);
}
#endif


#ifdef CAPTURE_CAMERA
int _tmain(int argc, _TCHAR* argv[])
{
	printf("Capturing camera...\n");

	int c;
	int i;
	int counter;

	int step = 1920;		//  Number of bytes in a row of image data
	int channels = 3;		//  RGB
	int isColor = 1;		//  
	int fps = 30;			//  Frames per second
	int depth = 8;			//  8 bits per channel per pixel

	uchar* data;			//  Type cast for image data
	int cameraIndex = 0;
	int updateValues = 1;
	if(argc > 1)
	{
		cameraIndex = atoi((char*)argv[1]);
		printf("Using camera %u\n", cameraIndex);
	}

	time_t t_start,t_end;
	double sec, fps_measure;

	//  Capture from video device #1
	CvCapture* capture = cvCaptureFromCAM(cameraIndex);

	if(capture == 0)
	{
		printf("Couldn't open camera.\n");
		_getch();
		return 1;
	}

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
		CvVideoWriter *writer = cvCreateVideoWriter(p.outFile, CV_FOURCC('P','I','M','1'), fps, cvSize(640,480),isColor);
	#endif

	int frameW = cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH);
	int frameH = cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT);
	
	//  Allocate memory for an image
	IplImage *img;
	
	//  OpenCV reference says we shouldn't modify the output of cvQueryFrame
	//  so this is used to store a copy.
	IplImage *dst  = cvCreateImage(cvSize(frameW, frameH), depth, channels);
	

	//  Record FPS in camerea mode
	time(&t_start);
	counter = 0;
	computeParameters(frameW, frameH);
	storageInit();
	
	
	while(1)
	{
		img = cvQueryFrame(capture);
		cvCopy( img, dst, NULL);
		Calibration(dst, updateValues);
		#ifdef DISPLAY_OUTPUT
			//  Show the image in the window
			cvShowImage("mainWin", dst );
		#endif
		//  Calculate FPS and output
		time(&t_end);
		++counter;
		double sec = difftime(t_end, t_start);
		fps_measure = counter/sec;
		printf("FPS: %.2lf, T: %u, S: %.2lf, C: (%u,%u)\n",fps_measure,p.initThreshold, p.refSize);
		c = cvWaitKey(30);
		if(c == 'g')
			break;
		
		switch(c)
		{
		case 'r':
			p.initThreshold++;
			break;
		case 'f':
			p.initThreshold--;
			break;
		case 'a':
			p.jStart -= 20;
			break;
		case 'A':
			p.jStart += 20;
			break;
		case 's':
			p.iFinish += 20;
			break;
		case 'S':
			p.iFinish -= 20;
			break;
		case 'd':
			p.jFinish += 20;
			break;
		case 'D':
			p.jFinish -= 20;
			break;
		case 'w':
			p.iStart -= 20;
			break;
		case 'W':
			p.iStart += 20;
			break;
		}

	}

	time(&t_start);
	counter = 0;
	while(1)
	{
		//  Retrieve the captured frame
		img = cvQueryFrame(capture);
		cvCopy( img, dst, NULL);
	
		
		//  In this version, getConnectedRegions thresholds and modifies dst
		processFrame(dst);
		
		#ifdef DISPLAY_OUTPUT
			//  Show the image in the window
			cvShowImage("mainWin", dst );
		#endif

		//  Calculate FPS and output
		time(&t_end);
		++counter;
		sec = difftime(t_end, t_start);
		fps_measure = counter/sec;
		printf("FPS: %.2lf, T: %u, S: %.2lf, C: (%u,%u)\n",fps_measure,p.initThreshold, p.refSize, p.refCentroid.x, p.refCentroid.y);
		printf("MinL: %u, MinR: %u, MinD: %u, MinU: %u\n", p.minxChangeL, p.minxChangeR, p.minyChangeD, p.minyChangeU);
		printf("Speed: %u\n", cursorSpeed);

		//  Wait 10 ms for a key to be pressed
		c = cvWaitKey(10);

		// escape key terminates program
		if(c == 27)
			break;
		switch(c)
		{
		case 'r':
			p.initThreshold++;
			break;
		case 'f':
			p.initThreshold--;
			break;
		case 't':
			p.refSize += 20;
			break;
		case 'g':
			p.refSize -= 20;
			break;
		case 'c':
			doCalibration = 1;
			break;
		case 'a':
			p.jStart -= 20;
			break;
		case 'A':
			p.jStart += 20;
			break;
		case 's':
			p.iFinish += 20;
			break;
		case 'S':
			p.iFinish -= 20;
			break;
		case 'd':
			p.jFinish += 20;
			break;
		case 'D':
			p.jFinish -= 20;
			break;
		case 'w':
			p.iStart -= 20;
			break;
		case 'W':
			p.iStart += 20;
			break;
		case 'j':
			p.minxChangeL += 1;
			break;
		case 'J':
			p.minxChangeL -= 1;
			break;
		case 'k':
			p.minyChangeD += 1;
			break;
		case 'K':
			p.minyChangeD -= 1;
			break;
		case 'i':
			p.minyChangeU += 1;
			break;
		case 'I':
			p.minyChangeU -= 1;
			break;
		case 'l':
			p.minxChangeR += 1;
			break;
		case 'L':
			p.minxChangeR -= 1;
			break;
		case 'y':
			cursorSpeed++;
			break;
		case 'h':
			cursorSpeed--;
			break;
		}
		if(c != -1)
		{
			storageDestroy();
			computeParameters(p.imgWidth, p.imgHeight);
			storageInit();
		}

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

	storageDestroy();
	return 0;
}



#endif

#ifdef CAPTURE_VIDEO

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

	int threshold = p.initThreshold;
	


	#ifdef GET_PARAMETERS_FROM_PYTHON
		//  Memory mapping
		HANDLE hMapFile;
		LPCTSTR pBuf;
		TCHAR buffName[] = TEXT("Local\\GazeTrackerFileMapping");
	
		hMapFile = OpenFileMapping(
                   FILE_MAP_ALL_ACCESS,   // read/write access
                   FALSE,                 // do not inherit the name
                   buffName);  
		if (hMapFile == NULL)
		{
			_tprintf(TEXT("Could not create file mapping object (%d). Please ensure that the Python GUI is running first.\n"),
			GetLastError());
			_getch();
			return(1);
		}
		pBuf = (LPTSTR) MapViewOfFile(hMapFile,   // handle to map object
							FILE_MAP_ALL_ACCESS, // read/write permission
							0,
							0,
							256);

		if (pBuf == NULL)
		{
			_tprintf(TEXT("Could not create file mapping object (%d). Please ensure that the Python GUI is running first.\n"), GetLastError());
			CloseHandle(hMapFile);
			_getch();
			return(1);
		}
		
		int* fileParam = (int*)pBuf;
		*fileParam = p.initThreshold;
	#endif

	//  End memory mapping
	//  Capture video from file
	CvCapture* capture = cvCaptureFromAVI(p.inFile);
	int numFrames = (int) cvGetCaptureProperty(capture,  CV_CAP_PROP_FRAME_COUNT);	
	
	#ifdef DISPLAY_OUTPUT
		//  Create a window to display the images
		cvNamedWindow("mainWin", CV_WINDOW_AUTOSIZE);
		//  Position the window
		cvMoveWindow("mainWin", 5, 5);
	#endif

	#ifdef RECORD_OUTPUT
		//  Setup output
		CvVideoWriter *writer = cvCreateVideoWriter(p.outFile, CV_FOURCC('P','I','M','1'), fps, cvSize(640,480),isColor);
	#endif

	int frameW = (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH);
	int frameH = (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT);
	//  Allocate memory for an image
	IplImage *img;
	IplImage** dst = (IplImage**)malloc(numFrames * sizeof(IplImage*));
	IplImage* tempImg = cvCreateImage(cvSize(frameW, frameH), depth, channels);
	//  OpenCV reference says we shouldn't modify the output of cvQueryFrame
	//  so this is used to store a copy.
	printf("Reading %u frames...", numFrames);
	for(i = 0; i < numFrames; ++i)
	{
		img = cvQueryFrame(capture);
		dst[i]  = cvCreateImage(cvSize(frameW, frameH), depth, channels);
		cvCopy(img, dst[i], NULL);
	}
	printf("Done\n");
	
	computeParameters(frameW, frameH);
	storageInit();

#ifdef CALIBRATION_ACTIVE
	printf("Adjust the boundaries to pinpoint the pupil. Then adjust threshold until pupil region is approximately filled\n");
	printf("Press G when finished\n");	
	int updateValues = 1;
	while(1)
	{
		cvCopy(dst[0], tempImg, NULL);
			
		Calibration(tempImg, updateValues);
		#ifdef DISPLAY_OUTPUT
			//  Show the image in the window
			cvShowImage("mainWin", tempImg );
			c = cvWaitKey(100);
		#endif
		//c = _getch();
		if(c == 'g')
		{
			if(updateValues == 1)
				updateValues = 0;
			else
				break;
		}
	
		// escape key terminates program
		switch(c)
		{
		case 'r':
			p.initThreshold++;
			break;
		case 'f':
			p.initThreshold--;
			break;
		case 'a':
			p.jStart -= 10;
			break;
		case 'A':
			p.jStart += 10;
			break;
		case 's':
			p.iFinish += 10;
			break;
		case 'S':
			p.iFinish -= 10;
			break;
		case 'd':
			p.jFinish += 10;
			break;
		case 'D':
			p.jFinish -= 10;
			break;
		case 'w':
			p.iStart -= 10;
			break;
		case 'W':
			p.iStart += 10;
			break;
		case 27:
			exit(1);
			break;
			
		}
		printf("T: %u, S: %.2lf, C: (%u,%u)\n",p.initThreshold, p.refSize, p.refCentroid.x, p.refCentroid.y);
		printf("MinL: %u, MinR: %u, MinD: %u, MinU: %u\n", p.minxChangeL, p.minxChangeR, p.minyChangeD, p.minyChangeU);
		prevCentroid = p.refCentroid;

		#ifdef GET_PARAMETERS_FROM_PYTHON			
			p.initThreshold = fileParam[0];
			p.iStart = fileParam[1];
			p.iFinish = fileParam[2];
			p.jStart = fileParam[3];
			p.jFinish = fileParam[4];
		#endif	
		storageDestroy();
		computeParameters(p.imgWidth, p.imgHeight);
		storageInit();

	}

	#ifdef GET_PARAMETERS_FROM_PYTHON
		UnmapViewOfFile(pBuf);
		CloseHandle(hMapFile);
	#endif
	
	#endif
	clock_t t_start = clock();
	for(i = 0; i < numFrames; ++i)
	{
		processFrame(dst[i]);

		#ifdef DISPLAY_OUTPUT
			//  Show the image in the window
			cvShowImage("mainWin", dst[i] );
			cvWaitKey(10);
		#endif

		#ifdef VIDEO_STEP_THROUGH
			if(stepVideoOn == 1)
			{
				if(stepVideoCount == 0)
				{
					printf("Current Frame: %u\n", i+1);
					while(1)
					{
						c = cvWaitKey(100);
						if(c != -1)
							break;
					}
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
	}
	printf("Time elapsed: %f\n", ((double)clock() - t_start) / CLOCKS_PER_SEC);

	#ifdef RECORD_OUTPUT
		for(i = 0; i < numFrames; ++i)
		{
			//  Output to video file
			cvWriteFrame(writer,dst[i]);
		}
		//  Release video writer
		cvReleaseVideoWriter(&writer);
	#endif

	//  Release capture and images
	cvReleaseCapture(&capture);
	for(i = 0; i < numFrames; ++i)
		cvReleaseImage(&(dst[i]));
	cvReleaseImage(&tempImg);
	storageDestroy();

	#ifdef DISPLAY_OUTPUT
		while(1)
		{
			c = cvWaitKey(100);
			if(c != -1)
				break;
		}
	#else
		_getch();
	#endif
	return 0;
}
#endif