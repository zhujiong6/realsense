//@authur <Thomas Tsai , d04922009@csie.ntu.edu.tw>
//hw4.cpp
#define	OPENCV_SUPPORTED	1	//OPENCV ONLY

#include <windows.h>
//opencv
#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
//realsense sdk
#include "pxcsensemanager.h"
#include "pxcsession.h"
#include "pxcmetadata.h"
#include "util_cmdline.h"
#include "util_render.h"

#include <iostream>
#include <conio.h>

#include "realsense2cvmat.h"

//! [namespace]
using namespace cv;
using namespace std;
//! [namespace]
#include "gui.h"

void hw4(my_gui &myGui, Mat &depthMat)
{
	if (myGui.action && (myGui.frames < myGui.roi_no)) {
		Mat roi(depthMat, myGui.rect); // using a rectangle ROI
		myGui.rois[myGui.frames++]= roi.clone();
	
		printf("insert : %d/%d\n", myGui.frames, myGui.roi_no);
	}

	if ( (myGui.rois!=NULL) && (myGui.frames == myGui.roi_no) && myGui.action) {//process

		printf("%s:done!\n",__func__);
		myGui.action = 0;//done

		for(int i = 0; i < myGui.rect.height;i++)
			for(int j= 0; j < myGui.rect.width;j++)
				for (int k= 0;  k < myGui.roi_no; k++) {
					Mat mat = myGui.rois[k];
					unsigned short us=mat.at<unsigned short>(i, j);
				}

		//free rois buffer
		printf("free myGui.rois[k] and myGui.rois\n");
		delete[]myGui.rois;
		myGui.rois = NULL;

		//http://answers.opencv.org/question/15917/how-to-access-data-from-a-cvmat/
		//Mat::at<type>(i,j,k)
		//http://answers.opencv.org/question/34487/accessing-slices-of-a-3d-matrix/
		/*
		//iterate all vectors and release them
		Mat plane;
		NAryMatIterator it(myGui.rois, &plane, 1);
		// iterate through the matrix. on each iteration
		// it.planes[*] (of type Mat) will be set to the current plane.
		for (int p = 0; p < it.nplanes; p++, ++it)
		{
			//it.planes[0]
			//it.planes[0])[0];
		}*/

	}
}
