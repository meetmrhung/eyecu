//#define DEBUG								//  Exits directly after compile

#define CAPTURE_CAMERA						//  Capture from camera if defined, else from video file

#define SHOW_PROCESSING_REGION				//  Draw processing region
#define SHOW_THRESHOLD_PIXELS				//  Color thresholded pixels
#define SHOW_CONNECTED_REGIONS				//  Color connected regions
#define SHOW_FINAL_REGION					//  Color final pupil region
#define SHOW_CENTROID_LOCATION				//  Draw cross over pupil centroid

#include "pointStack.h"

struct param
{
	//  Paths for video input/output
	char		inFile[500];
	char		outFile[500];

	//  Processing region
	int			iStart;
	int			iFinish;
	int			jStart;
	int			jFinish;

	//  Acceptable aspect ratio
	float		aspectMin;
	float		aspectMax;

	//  Acceptable pupil dimensions
	double		refSize;
	double		refSizeMinRatio;
	double		refSizeMaxRatio;
	double		refSizeMin;
	double		refSizeMax;

	//  Reference centroid and initial threshold
	point		refCentroid;
	float		initThreshold;

	//  Define which video frames are processed
	int			nFrames;
	int			startFrame;
};