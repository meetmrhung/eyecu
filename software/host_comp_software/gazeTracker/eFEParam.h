#ifndef eFEParam_
#define eFEParam_

//#define DEBUG_MAIN									//  Exits directly after compile
//#define DEBUG_OUTPUT

//#define CALIBRATION_ACTIVE					//  Allow calibration
//#define CAPTURE_CAMERA						//  Capture from camera if defined, else from video file
//#define DISPLAY_OUTPUT						//  Shows video output in a window

#define CAPTURE_VIDEO
#define RECORD_OUTPUT							//  Records output to p.outFile
//#define VIDEO_STEP_THROUGH						//  Requires a key press to advance to next frame in video

//#define MOVE_CURSOR

//#define GET_PARAMETERS_FROM_PYTHON

#define SHOW_PROCESSING_REGION					//  Draw processing region
#define SHOW_THRESHOLD_PIXELS					//  Color thresholded pixels
#define SHOW_CONNECTED_REGIONS					//  Color connected regions
#define SHOW_CANDIDATE_REGIONS					//  Color regions that meet size and aspect ratio requirements
#define SHOW_FINAL_REGION						//  Color final pupil region
#define SHOW_PIXELS_REMOVED						//  Color the the part of the final region that was removed in removeAberration
#define SHOW_CENTROID_LOCATION					//  Draw cross over pupil centroid

#define MAX_TOTAL_REGIONS 10					//  Total number of connected regions to store

//Macros
#define I2D(K,I) ((K) * p.procRegioniSize * p.procRegionjSize + (I))
#define I3D(K,X,Y) ((K) * p.procRegioniSize * p.procRegionjSize + (X) * p.procRegionjSize + (Y))

struct point
{
	int x,y;
};

struct param
{
	//  Paths for video input/output
	char		inFile[500];
	char		outFile[500];

	int			imgWidth;
	int			imgHeight;

	//  Processing region
	int			iStart;
	int			iFinish;
	int			jStart;
	int			jFinish;

	//  Acceptable aspect ratio
	double		aspectMin;
	double		aspectMax;
	
	// Acceptable length of the connected region (Note we assume the same constraints for x and y direction).
	double		lengthRegion;
	double		lengthMaxRatio;
	double		lengthMinRatio;

	//  Acceptable pupil dimensions
	double		refSize;
	double		refSizeMinRatio;
	double		refSizeMaxRatio;
	double		refSizeMin;
	double		refSizeMax;

	int			procRegioniSize;
	int			procRegionjSize;
	int			totalPixels;

	//  Reference centroid and initial threshold
	point		refCentroid;
	int			initThreshold;
	
	// Number of standard deviations away from the mean in removeAberrations for vertical and horizontal scan.
	double		numStdVertical;
	double		numStdHorizontal;

	//	How much the eye moves before moving the cursor
	int			minxChangeL;
	int			minxChangeR;
	int			minyChangeU;
	int			minyChangeD;

	// Number of frames with a particular direction before moving the cursor in that direction
	int			maxNumFrames;	

	// Maximum number of threshold adaptations and the change in magnitude of threshold:
	int			maxAdaptations;
	int			magThreshChange[4];
	
	//  Define which video frames are processed
	int			nFrames;
	int			startFrame;
};

// Results from processing	
enum resultType
{			
	isMiddle,			
	isRight,			
	isLeft,				
	isDown,				
	isUp,
	isBlink,			
	isPupil
};
#endif