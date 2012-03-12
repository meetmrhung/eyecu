#include "stdafx.h"
#include <highgui.h>

#include "eyeFeatureExtraction.h"

#include "eFEParam.h"

extern param p;

void getConnectedRegions(IplImage *img, int threshold)
{
	int i,j;
	float grey;
	int channels = img->nChannels;
	int step = img->widthStep;
	uchar* data = (uchar *)img->imageData;


	//  Draw processing region
#ifdef SHOW_PROCESSING_REGION
	for(i=p.iStart; i < p.iFinish; ++i)
		data[i*step+p.jStart*channels+2]=255;
	for(i = p.iStart; i < p.iFinish; ++i)
		data[i*step+p.jFinish*channels+2]=255;
	for(j = p.jStart; j < p.jFinish; ++j)
		data[p.iStart*step+j*channels+2]=255;
	for(j = p.jStart; j < p.jFinish; ++j)
		data[p.iFinish*step+j*channels+2]=255;
#endif

	//  Color thresholded values green
	for(i=p.iStart; i<p.iFinish; ++i)
		for(j=p.jStart;j<p.jFinish; ++j)
		{
			{
				grey = 0.1140*data[i*step+j*channels+0]+0.5870*data[i*step+j*channels+1]+0.2989*data[i*step+j*channels+2];
				if(grey < threshold)
				{
					data[i*step+j*channels+0] = 0;
					data[i*step+j*channels+1] = 255;
					data[i*step+j*channels+2] = 0;
				}
			}
		}
	return;
}