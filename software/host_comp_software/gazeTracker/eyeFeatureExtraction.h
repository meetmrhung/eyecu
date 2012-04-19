#ifndef eyeFeatureExtraction_
#define eyeFeatureExtraction_
#include "eFEParam.h"

//  Current function: modifies img by indicated threshold values
//  Goal: Create cR, cRSizes, cRCount
void processFrame(IplImage *img);

// Goal: Has the user select the boundaries for calibration.
void Calibration(IplImage *img, int updateValues);

//  Fill 2D array with greyscale values
void fillGreyScale(IplImage *img);

//  Fill connected region structures
void getConnectedRegions(int threshold);

// Compute Aspect Ratio and find selected region that has ratio equaling to 1
int findUnityRatio();

//remove abberations from the image
void removeAberrations(int unityIndex);

// Draw lines to indicate the pupil centroid, and color the chosen region yellow
void addProcessingOverlay(IplImage *img, int unityIndex, point centroid);

// Finds the centroid of the region selected as the pupil
point computeCentroid(int unityIndex);

// Compares the reference centroid and the computed centroid to determine direction
void generateCursorCommand(point centroid);

#endif