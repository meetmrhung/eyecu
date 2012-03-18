#include "stdafx.h"
#include <highgui.h>
#include <conio.h>

#include "eyeFeatureExtraction.h"

#include "eFEParam.h"

extern param p;
extern point* cRPointList;
extern unsigned char* cRBinary;
extern int* cRMap;
extern int* cRSizes0;
extern int* cRSizes1;
extern int cRCount;
extern enum resultType procResult;
extern int maxRegionSize;
extern unsigned char* processedPixels;
extern unsigned char* gSImg;
extern int* candidateRegionIndices;
extern int candidateRegionCount;
extern double* cRAspectRatio;
extern int doCalibration;

#define RGB2GS(X,Y) 0.1140*data[(X)*step+(Y)*channels+0]+0.5870*data[(X)*step+(Y)*channels+1]+0.2989*data[(X)*step+(Y)*channels+2]

#define I2DFULL(X,Y) ((X)*width + (Y))

void processFrame(IplImage *img)
{
	point centroid = {0,0};
	int numAdapt = 0;
	int threshold = p.initThreshold;
	procResult = isBlink; 
	int unityIndex = -1;
	fillGreyScale(img);

	while (numAdapt < p.maxAdaptations && procResult != isPupil)
		{
		
			getConnectedRegions(threshold);
			#ifdef DEBUG_OUTPUT
				printf("Max region size: %u\n", maxRegionSize);
			#endif

			if(cRCount == 0)
			{
				if( maxRegionSize > p.refSizeMax )
					threshold -= p.magThreshChange[numAdapt];
				else if(maxRegionSize < p.refSizeMin)
					threshold += p.magThreshChange[numAdapt];
			}
			
			else
			{
				//  Compute aspectRatio finds the index of connected region that has closest to unity aspect ratio
				unityIndex = findUnityRatio();
				if( unityIndex == -1 )
				{
					if( maxRegionSize > p.refSize )
						threshold -= p.magThreshChange[numAdapt];
					else
						threshold += p.magThreshChange[numAdapt];
				}
				else
				{
					procResult = isPupil;
				}
			}
			++numAdapt;
		}
	if (procResult == isPupil)
	{
		centroid = computeCentroid(unityIndex);
	}
	else
	{
		centroid.x = 0; centroid.y = 0;
	}

	#ifdef CALIBRATION_ACTIVE
		if(doCalibration == 1 && procResult == isPupil)
		{
			p.refCentroid = centroid;
			p.refSize = cRSizes0[unityIndex];
			doCalibration = 0;
		}
	#endif

	#ifdef DEBUG_OUTPUT
		printf("Centroid: (%u, %u)\n\n", centroid.x, centroid.y);
	#endif
	addProcessingOverlay(img, unityIndex, centroid);
}

void fillGreyScale(IplImage *img)
{
	int width = img->width;
	int height = img->height;
	int channels = img->nChannels;
	int step = img->widthStep;
	int i,j;
	uchar* data = (uchar *)img->imageData;

	for(i = p.iStart; i <= p.iFinish; ++i)
	{
		for(j = p.jStart; j <= p.jFinish; ++j)
		{
			gSImg[I2DFULL(i,j)] = (unsigned char)(RGB2GS(i,j));
		}
	}
}

void getConnectedRegions(int threshold)
{
	int width = p.imgWidth;
	int height = p.imgHeight;
	int iStart = p.iStart;
	int iFinish = p.iFinish;
	int jStart = p.jStart;
	int jFinish = p.jFinish;
	int totalPixels = p.totalPixels;

	double refSizeMin = p.refSizeMin;
	double refSizeMax = p.refSizeMax;

	int i,j;
	int coord;
	int currentPixel;
	point pt;
	int currentRegion = 0;
	pointStackElement* stackHead = 0;

	int maxSizeOverall = 0;
	int maxSizeProper = 0;
	
	memset(processedPixels, 0,width*height*sizeof( unsigned char));
	memset(cRBinary, 0, totalPixels * sizeof(unsigned char));
	memset(cRMap, 0, totalPixels * sizeof(int));

	#ifdef DEBUG_OUTPUT
		printf("Getting connected regions...\n");
		printf("Threshold: %u\n", threshold);
	#endif

	for(i = iStart; i <= iFinish; ++i)
		for(j = jStart; j <= jFinish; ++j)
		{
			{
				if( gSImg[I2DFULL(i,j)] < threshold && processedPixels[I2DFULL(i,j)] == 0 )
				{
					currentPixel = 0;
					pt.x = i; pt.y = j;
					pointStackPush(&stackHead, i, j);
					processedPixels[I2DFULL(i,j)] = 1;

					//  Do flood fill algorithm
					while(stackHead != 0)
					{
						pt = pointStackPop(&stackHead);		
						coord = I3D(currentRegion, pt.x-iStart, pt.y-jStart);
						cRBinary[coord] = 1;
						cRMap[coord] = currentPixel;
						cRPointList[I2D(currentRegion,currentPixel)] = pt;
						currentPixel++;

						if( ( pt.x + 1 <= iFinish ) && ( processedPixels[I2DFULL(pt.x + 1, pt.y)] == 0 ) && ( gSImg[I2DFULL(pt.x + 1, pt.y)] < threshold ))
						{
							pointStackPush(&stackHead, pt.x + 1, pt.y);
							processedPixels[I2DFULL(pt.x + 1, pt.y)] = 1;
						}

						if( ( pt.x - 1 >= iStart ) && ( processedPixels[I2DFULL(pt.x - 1, pt.y)] ==  0 ) && ( gSImg[I2DFULL(pt.x - 1, pt.y)] < threshold ))
						{
							pointStackPush(&stackHead, pt.x - 1, pt.y);
							processedPixels[I2DFULL(pt.x - 1, pt.y)] = 1;
						}

						if( ( pt.y + 1 <= jFinish ) && ( processedPixels[I2DFULL(pt.x, pt.y + 1)] == 0 ) && ( gSImg[I2DFULL(pt.x, pt.y + 1)] < threshold ))
						{
							pointStackPush(&stackHead, pt.x, pt.y + 1);
							processedPixels[I2DFULL(pt.x, pt.y + 1)] = 1;
						}

						if( ( pt.y  - 1 >= jStart ) && ( processedPixels[I2DFULL(pt.x, pt.y - 1)] == 0 ) && ( gSImg[I2DFULL(pt.x, pt.y - 1)] < threshold ))
						{
							pointStackPush(&stackHead, pt.x, pt.y - 1);
							processedPixels[I2DFULL(pt.x, pt.y - 1)] = 1;
						}
					}  //  End flood fill
					if( currentPixel <= refSizeMax && currentPixel >= refSizeMin )
					{
						cRSizes0[currentRegion++] = currentPixel;
						if( currentPixel > maxSizeProper) 
							maxSizeProper = currentPixel;
					}
					if( currentPixel > maxSizeOverall)
						maxSizeOverall = currentPixel;
					if( currentRegion == MAX_TOTAL_REGIONS)
						printf("Maxed number of connected regions reached.\n");
				}
				else
					processedPixels[I2DFULL(i,j)] = 1;
			}
		}

		cRCount = currentRegion;
		candidateRegionCount = currentRegion;
		if( cRCount != 0 )
			maxRegionSize = maxSizeProper;
		else
			maxRegionSize = maxSizeOverall;
		#ifdef DEBUG_OUTPUT
			printf("Connected regions found: %u\n", cRCount);
		#endif
}


// Goal: Compute the aspect ratio of the connected regions and find the ones closest to 1.
int findUnityRatio()
{
	int i,j,k;
	int lengthx, lengthy = 0;	
	int xcount, ycount = 0;								// horizontal and vertical length of the image
	double dummyratio = 0;									//dummy aspect ratio
	int unityIndex = -1;										// Output index value 

	candidateRegionCount = 0;
	for (k = 0; k < cRCount; ++k)
	{
		xcount = 0;
		ycount = 0;
		for (i = 0; i < p.procRegioniSize; ++i)
		{
			lengthx = 0;
			for (j = 0; j < p.procRegionjSize; ++j)
			{
				lengthx += (int)cRBinary[I3D(k,i,j)];
			}
			if (lengthx > xcount)
			{
				xcount = lengthx;
			}
		}
	
		for (j = 0; j < p.procRegionjSize; ++j)
		{
			lengthy = 0;
			for (i = 0; i < p.procRegioniSize; ++i)
			{
				lengthy += (int)cRBinary[I3D(k,i,j)];
			}
			if (lengthy > ycount)
			{
				ycount = lengthy;
			}
		}

		cRAspectRatio[k] = (double) (ycount)/(xcount);
		#ifdef DEBUG_OUTPUT
			printf("Region: %u\n", k);
			printf("Size: %u\n", cRSizes0[k]);
			printf("LengthX: %u, LengthY: %u\n", xcount, ycount);
			printf("Aspect ratio for region %u: %lf\n", k, cRAspectRatio[k]);
		#endif
		if (cRAspectRatio[k] < p.aspectMax && cRAspectRatio[k] > p.aspectMin)
		{
			candidateRegionIndices[candidateRegionCount++] = k;
			if (abs(dummyratio-1) > abs(cRAspectRatio[k]-1))
			{
				dummyratio = cRAspectRatio[k];
				unityIndex = k;
			}
			procResult = isPupil;
		}
	}
	
	return unityIndex;
}

// Goal: Find the centroid of the connected region
point computeCentroid(int unityIndex)
{
	int i;
	int sumx = 0;
	int sumy = 0;
	point centroid;

	for (i = 0; i < cRSizes0[unityIndex]; ++i)
	{
		sumx += cRPointList[I2D(unityIndex,i)].x;
		sumy += cRPointList[I2D(unityIndex,i)].y;
	}
	centroid.x = (int) sumx/cRSizes0[unityIndex];
	centroid.y = (int) sumy/cRSizes0[unityIndex];
	return centroid;
}

// Compares finds reference centroid with the current pupil centroid to determine a direction. 
void compareCentroids(point centroid)
{
	int xdist = centroid.x - p.refCentroid.x;
	int ydist = centroid.y - p.refCentroid.y;
	if (abs(xdist) > abs(ydist))
	{
		if (xdist > 0 && xdist > p.minxChangeL)
		{
			 procResult = isLeft;
		}
		else if (xdist < 0 && abs(xdist) > p.minxChangeR)
		{
		 	 procResult = isRight;
		}
		else
		{
			procResult = isMiddle;
		}
	}
	else
	{
		if (ydist > 0 && ydist > p.minyChangeD)
		{
			procResult = isDown;
		}
		else if (ydist < 0 && abs(ydist) > p.minyChangeU)
		{
			procResult = isUp;
		}
		else
		{
			procResult = isMiddle;
		}
	}
		
}

void removeAberrations(int unityIndex)
{
	int i,j;
	double* rowCount = (double*)malloc(p.procRegioniSize * sizeof(double));
	double* colCount = (double*)malloc(p.procRegionjSize * sizeof(double));;
	double sumTotal = 0;
	int sum = 0;
	int index;
	double avgRowCount;
	double stdRowCount;
	double avgColCount;
	double stdColCount;


	// Horizontal Scan

	// Compute the average pixel count in a horizontal scan
	for (i = p.iStart; i <= p.iFinish; ++i)
	{
		sum = 0;
		for (j = p.jStart; j <= p.jFinish; ++j)
		{
			sum += cRBinary[I3D(unityIndex,j,i)];
		}
		rowCount[i-p.iStart] = sum;
		sumTotal += sum;
	}
	avgRowCount = sumTotal/(p.iFinish-p.iStart+1);
		
	
	// Compute the standard deviation of pixel count in horizontal scan
	sumTotal = 0;
	for (i = 0; i<= p.iFinish-p.iStart; ++i)
	{
		sumTotal += (rowCount[i]-avgRowCount)*(rowCount[i]-avgRowCount);
	}
	stdRowCount = sumTotal/(p.iFinish-p.iStart+1);
	
	for (i = 0; i<= p.iFinish-p.iStart; ++i)
	{
		if (rowCount[i] < avgRowCount - 2*stdRowCount)
		{
			for (j = p.jStart; j <= p.jFinish; ++j)
			{
				index = cRMap[I3D(unityIndex,j,i+p.iStart)];
				cRPointList[I2D(unityIndex,index)].x = 0;	
				cRPointList[I2D(unityIndex,index)].y = 0;
				cRSizes0[unityIndex] += -1;
			}

		}
	}
	
	// Vertical scan analysis
	// Compute the average pixel count in a vertical scan
	sumTotal = 0;
	for (j = p.jStart; j <= p.jFinish; ++j)
	{
		sum = 0;
		for (i = p.iStart; i <= p.iFinish; ++i)
		{
			sum += cRBinary[I3D(unityIndex,j,i)];
		}
		colCount[j-p.jStart] = sum;
		sumTotal += sum;
	}
	avgColCount = sumTotal/(p.jFinish-p.jStart+1);
	
	
	// Compute the standard deviation of pixel count in vertical scan
	sumTotal = 0;
	for (j = 0; j<= p.jFinish-p.jStart; ++j)
	{
		sumTotal += pow(colCount[j]-avgColCount,2);
	}
	stdColCount = sumTotal/(p.jFinish-p.jStart+1);
	
	for (j = 0; j<= p.jFinish-p.jStart; ++j)
	{
		if (colCount[j] < avgColCount - 2*stdColCount)
		{
			for (i = p.iStart; i <= p.iFinish; ++i)
			{
				index = cRMap[I3D(unityIndex,j+p.jStart,i)];
				cRPointList[I2D(unityIndex,index)].x = 0;	
				cRPointList[I2D(unityIndex,index)].y = 0;
				cRSizes0[unityIndex] += -1;	
			}
		}
	}

	free(rowCount); free(colCount);
}


void addProcessingOverlay(IplImage *img, int unityIndex, point centroid)
{
	int i,j;
	int channels = img->nChannels;
	int step = img->widthStep;
	uchar* data = (uchar *)img->imageData;
	int x, y, ind;

		//  Draw processing region
	#ifdef SHOW_PROCESSING_REGION
		for(i=p.iStart; i < p.iFinish; ++i)
		{
			data[i*step+p.jStart*channels+0]=0;
			data[i*step+p.jStart*channels+1]=0;
			data[i*step+p.jStart*channels+2]=255;
		}
		for(i = p.iStart; i < p.iFinish; ++i)
		{
			data[i*step+p.jFinish*channels+0]=0;
			data[i*step+p.jFinish*channels+1]=0;
			data[i*step+p.jFinish*channels+2]=255;
		}
		for(j = p.jStart; j < p.jFinish; ++j)
		{
			data[p.iStart*step+j*channels+0]=0;
			data[p.iStart*step+j*channels+1]=0;
			data[p.iStart*step+j*channels+2]=255;
		}
		for(j = p.jStart; j < p.jFinish; ++j)
		{
			data[p.iFinish*step+j*channels+0]=0;
			data[p.iFinish*step+j*channels+1]=0;
			data[p.iFinish*step+j*channels+2]=255;
		}
	#endif
	
	#ifdef SHOW_CONNECTED_REGIONS
		for(i = 0; i < cRCount; ++i)
		{
			for(j = 0; j < cRSizes0[i]; ++j)
			{
				ind = I2D(i,j);
				x = cRPointList[I2D(i,j)].x; y = cRPointList[I2D(i,j)].y;
				data[x*step + y*channels + 0] = 0;
				data[x*step + y*channels + 1] = 0;
				data[x*step + y*channels + 2] = 255;
			}
		}
	#endif

	#ifdef SHOW_CANDIDATE_REGIONS
		for(i = 0; i < candidateRegionCount; ++i)
		{
			for(j = 0; j < cRSizes0[candidateRegionIndices[i]]; ++j)
			{
				ind = I2D(i,j);
				x = cRPointList[I2D(candidateRegionIndices[i],j)].x; y = cRPointList[I2D(candidateRegionIndices[i],j)].y;
				data[x*step + y*channels + 0] = 255;
				data[x*step + y*channels + 1] = 0;
				data[x*step + y*channels + 2] = 0;
			}
		}
	#endif

	// Colors the connected region denoted as the pupil yellow 
	#ifdef SHOW_FINAL_REGION
		if(unityIndex != -1)
		{
			for(i = 0; i < cRSizes0[unityIndex]; ++i)
			{
				x = cRPointList[I2D(unityIndex,i)].x; y = cRPointList[I2D(unityIndex,i)].y;
				data[x*step + y*channels + 0] = 0;
				data[x*step + y*channels + 1] = 255;
				data[x*step + y*channels + 2] = 0;
			}
		}
	#endif

	#ifdef SHOW_CENTROID_LOCATION
		// Draw vertical line
	
		//for (i = p.iStart; i <= p.iFinish; ++i)			//  Draw inside processing region
		for (i = 0; i < p.imgHeight; ++i)
		{
		
			data[i*step + centroid.y*channels + 0] = 255;
			data[i*step + centroid.y*channels + 1] = 0;
			data[i*step + centroid.y*channels + 2] = 255;

		}
	
		// Draw horizontal line

		//for (j = p.jStart; j <= p.jFinish; ++j)			//  Draw inside processing region
		for (j = 0; j < p.imgWidth; ++j)
		{
			data[centroid.x*step + j*channels + 0] = 255;
			data[centroid.x*step + j*channels + 1] = 0;
			data[centroid.x*step + j*channels + 2] = 255;

		}
	#endif

	
	return;
}
