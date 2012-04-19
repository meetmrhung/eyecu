#include "stdafx.h"
#include <highgui.h>
#include <conio.h>
#include <Windows.h>

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
extern int cursorCommand;
extern int maxRegionSize;
extern unsigned char* processedPixels;
extern unsigned char* gSImg;
extern int* candidateRegionIndices;
extern int candidateRegionCount;
extern double* cRAspectRatio;
extern int doCalibration;
extern point* removedPoints;
extern int removedPointCount;
extern point centroid;
extern int consecDirFrame;
extern int prevResultType;
extern double maxLengthConnected;
extern double minLengthConnected;
extern point dirArray[5];
extern int cursorSpeed;

#define RGB2GS(X,Y) 0.1140*data[(X)*step+(Y)*channels+0]+0.5870*data[(X)*step+(Y)*channels+1]+0.2989*data[(X)*step+(Y)*channels+2]

#define I2DFULL(X,Y) ((X)*width + (Y))

void processFrame(IplImage *img)
{
	point centroid = {0,0};
	int numAdapt = 0;
	int threshold = p.initThreshold;
	procResult = isBlink;			// Assume initially that user is blinking for each frame
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
					removedPointCount = 0;
					removeAberrations(unityIndex);
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
			p.lengthRegion = 2*sqrt((p.refSize)/(3.1415));
			p.initThreshold = threshold;
			doCalibration = 0;
		}
	#endif

	#ifdef DEBUG_OUTPUT
		printf("Reference Centroid: (%u, %u)\n", p.refCentroid.x, p.refCentroid.y);
		printf("Centroid: (%u, %u)\n", centroid.x, centroid.y);
	#endif	
	generateCursorCommand(centroid);

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
	int procRegionArea = p.procRegioniSize * p.procRegionjSize;
	
	double refSizeMin = p.refSizeMin;
	double refSizeMax = p.refSizeMax;

	int i,j;
	int coord;
	int currentPixel;
	point pt;
	int currentRegion = 0;
	point *pointStack;
	int stackIndex;	
	int maxSizeOverall = 0;
	int maxSizeProper = 0;
	
	pointStack = (point*)malloc(procRegionArea * sizeof(point));
	stackIndex = 0;

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
					pointStack[stackIndex++] = pt;
					processedPixels[I2DFULL(i,j)] = 1;

					//  Do flood fill algorithm
					while(stackIndex != 0)
					{
						pt = pointStack[--stackIndex];
						coord = I3D(currentRegion, pt.x-iStart, pt.y-jStart);
						cRBinary[coord] = 1;
						cRMap[coord] = currentPixel;
						cRPointList[I2D(currentRegion,currentPixel)] = pt;
						currentPixel++;

						if( ( pt.x + 1 <= iFinish ) && ( processedPixels[I2DFULL(pt.x + 1, pt.y)] == 0 ) && ( gSImg[I2DFULL(pt.x + 1, pt.y)] < threshold ))
						{
							pointStack[stackIndex].x = pt.x + 1;
							pointStack[stackIndex++].y = pt.y;
							processedPixels[I2DFULL(pt.x + 1, pt.y)] = 1;
						}

						if( ( pt.x - 1 >= iStart ) && ( processedPixels[I2DFULL(pt.x - 1, pt.y)] ==  0 ) && ( gSImg[I2DFULL(pt.x - 1, pt.y)] < threshold ))
						{
							pointStack[stackIndex].x = pt.x - 1;
							pointStack[stackIndex++].y = pt.y;
							processedPixels[I2DFULL(pt.x - 1, pt.y)] = 1;
						}

						if( ( pt.y + 1 <= jFinish ) && ( processedPixels[I2DFULL(pt.x, pt.y + 1)] == 0 ) && ( gSImg[I2DFULL(pt.x, pt.y + 1)] < threshold ))
						{
							pointStack[stackIndex].x = pt.x;
							pointStack[stackIndex++].y = pt.y + 1;
							processedPixels[I2DFULL(pt.x, pt.y + 1)] = 1;
						}

						if( ( pt.y  - 1 >= jStart ) && ( processedPixels[I2DFULL(pt.x, pt.y - 1)] == 0 ) && ( gSImg[I2DFULL(pt.x, pt.y - 1)] < threshold ))
						{
							pointStack[stackIndex].x = pt.x;
							pointStack[stackIndex++].y = pt.y - 1;
							processedPixels[I2DFULL(pt.x, pt.y - 1)] = 1;
						}
					}  //  End flood fill
					if( currentPixel <= refSizeMax && currentPixel >= refSizeMin )
					{
						cRSizes0[currentRegion++] = currentPixel;
						if( currentPixel > maxSizeProper) 
							maxSizeProper = currentPixel;
					}
					else
					{
						memset(cRBinary + currentRegion*procRegionArea, 0, sizeof(unsigned char)*procRegionArea);
						memset(cRMap + currentRegion*procRegionArea, 0, sizeof(int)*procRegionArea);
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
		free(pointStack);
}


// Goal: Compute the aspect ratio of the connected regions and find the ones closest to 1.
int findUnityRatio()
{
	int i,j,k;
	int lengthx, lengthy = 0;	
	int xcount, ycount = 0;								// horizontal and vertical length of the image
	double dummyratio = 0;									//dummy aspect ratio
	int unityIndex = -1;										// Output index value 
	maxLengthConnected = p.lengthRegion*p.lengthMaxRatio;
	minLengthConnected = p.lengthRegion*p.lengthMinRatio; 
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
		#ifdef CALIBRATION_ACTIVE
			if(doCalibration == 1)
			{
				candidateRegionIndices[candidateRegionCount++] = k;
				if (abs(dummyratio-1) > abs(cRAspectRatio[k]-1))
				{
					dummyratio = cRAspectRatio[k];
					unityIndex = k;
				}
				procResult = isPupil;
			}
			else
			{
				// Additional check to make sure that the x length and y length of the candidate region is acceptable
				if (ycount < maxLengthConnected && ycount > minLengthConnected && xcount < maxLengthConnected && xcount > minLengthConnected)
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
		#else
			if (ycount < maxLengthConnected && ycount > minLengthConnected && xcount < maxLengthConnected && xcount > minLengthConnected)
				{
					candidateRegionIndices[candidateRegionCount++] = k;
					if (abs(dummyratio-1) > abs(cRAspectRatio[k]-1))
					{
						dummyratio = cRAspectRatio[k];
						unityIndex = k;
					}
					procResult = isPupil;
				}
		#endif
		}

	}
	
	return unityIndex;
}

// Goal: Find the centroid of the connected region
point computeCentroid(int unityIndex)
{
	int i;
	int sumx = 0, sumy = 0;
	point centroid;

	for (i = 0; i < cRSizes0[unityIndex]; ++i)
	{
		sumx += cRPointList[I2D(unityIndex,i)].x;
		sumy += cRPointList[I2D(unityIndex,i)].y;
	}

	centroid.y = (int) sumx/(cRSizes0[unityIndex]-removedPointCount);
	centroid.x = (int) sumy/(cRSizes0[unityIndex]-removedPointCount);
	return centroid;
}

// Compares finds reference centroid with the current pupil centroid to determine a direction. Also checks to see how many consecutive frames a particular direction is outputed. 
void generateCursorCommand(point centroid)
{
	int xdist = centroid.x - p.refCentroid.x;
	int ydist = centroid.y - p.refCentroid.y;
	
	if (procResult !=  isBlink)
	{
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
		if (prevResultType == procResult)
		{
			consecDirFrame++;
		}
		else
		{
			consecDirFrame = 0;
		}

	// if consecutive frames are in the direction, generate cursor command. Note procResult = isBlink will be used for clicking
	if (consecDirFrame == p.maxNumFrames)
	{
		consecDirFrame = 0;
		cursorCommand = procResult;
	}
	#ifdef DEBUG_OUTPUT
			printf("The output direction for this frame is: %d\n", procResult);
			printf("The cursor command is: %d\n", cursorCommand);
			printf("\n");
    #endif	
	prevResultType = procResult;

	#ifdef MOVE_CURSOR
		printf("Proc result: %u\n", procResult);
		if(procResult != isBlink)
		{
			POINT mypoint;
			GetCursorPos(&mypoint);
			SetCursorPos(mypoint.x + cursorSpeed*dirArray[procResult].x, mypoint.y + cursorSpeed*dirArray[procResult].y);
		}
	#endif
	
}

// Goal: remove aberrations of the region that was found to be the pupil
void removeAberrations(int unityIndex)
{
	int i,j;
	double* rowCount = (double*)malloc(p.procRegioniSize * sizeof(double));
	double* colCount = (double*)malloc(p.procRegionjSize * sizeof(double));;
	int* rowValue = (int*)malloc(p.procRegioniSize * sizeof(int));
	int* colValue = (int*)malloc(p.procRegionjSize * sizeof(int));
	int  rcIndex [300][30];
	int  crIndex [300][30];
	double sumTotal = 0;
	int sum = 0;		
	int z = 0;
	int index;
	int numCol = 0;
	int numRow = 0;
	int rowIndex = 0 ;
	double avgRowCount;
	double stdRowCount;
	double avgColCount;
	double stdColCount;
	removedPointCount = 0;

	// Vertical Scan
	
	sumTotal = 0;
	int k = 0;
	for (j = 0; j < p.procRegionjSize; ++j)
	{
		sum = 0;
		numRow = 0;
		for (i =0; i < p.procRegioniSize; ++i)
		{
			if (cRBinary[I3D(unityIndex,i,j)] != 0)
			{
				sum += cRBinary[I3D(unityIndex,i,j)];
				crIndex[k][numRow] = i;
				++numRow;
			}
		}
		
		if (sum != 0)
		{
			colCount[k] = sum;
			colValue[k] =  j;
			sumTotal += sum;
			k++;
		}
		
	}
	avgColCount = sumTotal/(k);
	
	
	// Compute the standard deviation of pixel count in vertical scan
	sumTotal = 0;
	z = removedPointCount;
	for (j = 0; j < k; ++j)
	{
		sumTotal += (colCount[j]-avgColCount)*(colCount[j]-avgColCount);
	}
	stdColCount = sqrt(sumTotal/(k));
	for (j = 0; j < k; ++j)
	{
		if (colCount[j] < avgColCount - (p.numStdVertical)*stdColCount)
		{
			for (i = 0; i < colCount[j]; ++i)
			{
				index = cRMap[I3D(unityIndex,crIndex[j][i], colValue[j])];
       			removedPoints[z].x = cRPointList[I2D(unityIndex,index)].x;	
				removedPoints[z].y = cRPointList[I2D(unityIndex,index)].y;
				cRPointList[I2D(unityIndex,index)].x = 0;	
				cRPointList[I2D(unityIndex,index)].y = 0;
				removedPointCount++;
				z++;
			}
		}
	}

	// Horizontal Scan
	// Compute the average pixel count in a horizontal scan
	for (i = 0; i < p.procRegioniSize; ++i)
	{
		sum = 0;
		numCol = 0;
		for (j = 0; j < p.procRegionjSize; ++j)
		{	
			if (cRBinary[I3D(unityIndex,i,j)] != 0)
			{
				sum += cRBinary[I3D(unityIndex,i,j)];
				rcIndex[rowIndex][numCol] = j;
				++numCol;
			
			}
		}
		
		if (sum != 0)
		{
			rowCount[rowIndex] = sum;
			rowValue[rowIndex] =  i;
			sumTotal += sum;
			rowIndex++;
		}	
	}
	avgRowCount = sumTotal/(rowIndex);
		

	// Compute the standard deviation of pixel count in horizontal scan
	sumTotal = 0;
	z = 0;
	for (i = 0; i < rowIndex; ++i)
	{
		sumTotal += (rowCount[i]-avgRowCount)*(rowCount[i]-avgRowCount);
	}
	stdRowCount = sqrt(sumTotal/(rowIndex));
	for (i = 0; i < rowIndex ;++i)
	{
		if (rowCount[i] < avgRowCount - (p.numStdHorizontal)*stdRowCount)
		{
			for (j = 0; j < rowCount[i]; ++j)
			{
			
				index = cRMap[I3D(unityIndex,rowValue[i],rcIndex[i][j])];
       			removedPoints[removedPointCount].x = cRPointList[I2D(unityIndex,index)].x;	
				removedPoints[removedPointCount].y = cRPointList[I2D(unityIndex,index)].y;
				cRPointList[I2D(unityIndex,index)].x = 0;	
				cRPointList[I2D(unityIndex,index)].y = 0;
				removedPointCount++;
			}
		}
	}
	

	free(rowCount); free(rowValue);
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
				if (x != 0 && y != 0)
				{
					data[x*step + y*channels + 0] = 0;
					data[x*step + y*channels + 1] = 0;
					data[x*step + y*channels + 2] = 255;
				}
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
				if (x != 0 && y != 0)
				{
					data[x*step + y*channels + 0] = 255;
					data[x*step + y*channels + 1] = 0;
					data[x*step + y*channels + 2] = 0;
				}
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
	
	#ifdef SHOW_PIXELS_REMOVED
	  if(unityIndex != -1)	
	  {
		  for (j =0 ; j < removedPointCount; ++j)
			{
				x = removedPoints[j].x; y = removedPoints[j].y; 
				if (x != 0 && y != 0)
				{
					data[x*step + y*channels + 0] = 255;
					data[x*step + y*channels + 1] = 255;
					data[x*step + y*channels + 2] = 255;
				}
			}
	  }
	#endif
	#ifdef SHOW_CENTROID_LOCATION
		// Draw vertical line
	
		//for (i = p.iStart; i <= p.iFinish; ++i)			//  Draw inside processing region
		for (i = 0; i < p.imgHeight; ++i)
		{
		
			data[i*step + centroid.x*channels + 0] = 255;
			data[i*step + centroid.x*channels + 1] = 0;
			data[i*step + centroid.x*channels + 2] = 255;

		}
	
		// Draw horizontal line

		//for (j = p.jStart; j <= p.jFinish; ++j)			//  Draw inside processing region
		for (j = 0; j < p.imgWidth; ++j)
		{
			data[centroid.y*step + j*channels + 0] = 255;
			data[centroid.y*step + j*channels + 1] = 0;
			data[centroid.y*step + j*channels + 2] = 255;

		}
	#endif

	
	return;
}

void Calibration(IplImage *img, int updateValues)
{
	int width = img->width;
	int height = img->height;
	int channels = img->nChannels;
	int step = img->widthStep;
	uchar* data = (uchar *)img->imageData;
	int i,j;
	int refSize = 0;
	int sumx = 0;
	int sumy = 0;
	
	for(i=p.iStart; i < p.iFinish; ++i)
	{
		for (j = p.jStart; j < p.jFinish; ++j)
		{	
			if( (unsigned char)(RGB2GS(i,j)) < p.initThreshold)
				{
					data[i*step+j*channels+0]=0;
					data[i*step+j*channels+1]=0;
					data[i*step+j*channels+2]=255;
					refSize += 1;
					sumx += j;
					sumy += i;
				}
		}	
	}
	if(refSize == 0)
		refSize = 1;										//  Prevent division by zero
	if(updateValues == 1)
	{
		p.refSize = refSize;
		p.refCentroid.x = (int)(sumx/(p.refSize));
		p.refCentroid.y = (int)(sumy/(p.refSize));
		p.lengthRegion = 2*sqrt((p.refSize)/(3.1415));
	}

#ifdef CAPTURE_VIDEO
	#ifdef SHOW_CENTROID_LOCATION
		// Draw vertical line
	
		//for (i = p.iStart; i <= p.iFinish; ++i)			//  Draw inside processing region
		for (i = 0; i < p.imgHeight; ++i)
		{
		
			data[i*step + p.refCentroid.x*channels + 0] = 255;
			data[i*step + p.refCentroid.x*channels + 1] = 0;
			data[i*step + p.refCentroid.x*channels + 2] = 255;

		}
	
		// Draw horizontal line

		//for (j = p.jStart; j <= p.jFinish; ++j)			//  Draw inside processing region
		for (j = 0; j < p.imgWidth; ++j)
		{
			data[p.refCentroid.y*step + j*channels + 0] = 255;
			data[p.refCentroid.y*step + j*channels + 1] = 0;
			data[p.refCentroid.y*step + j*channels + 2] = 255;

		}
	#endif
#endif
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
	
}	