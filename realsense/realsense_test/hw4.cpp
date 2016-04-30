//@authur <Thomas Tsai , d04922009@csie.ntu.edu.tw>
//hw4.cpp
#define	OPENCV_SUPPORTED	1	//OPENCV ONLY

#include <windows.h>
//opencv
#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
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
		myGui.rois[myGui.frames]= roi.clone();
		char prg_str[80];
		sprintf_s(prg_str,"%d/%d", myGui.frames+1, myGui.roi_no);
		cv::putText(roi, prg_str, Point(10,40), FONT_HERSHEY_DUPLEX, 1, cv::Scalar(255, 255, 255));
		//cv::imshow(myGui.crop_win_name, myGui.rois[myGui.frames]);
		cv::imshow(myGui.crop_win_name, roi);
		cout << '.';
		myGui.frames++;
		//printf("insert : %d/%d\n", myGui.frames, myGui.roi_no);
		
	}
	//cout << endl;
	if ( (myGui.rois!=NULL) && (myGui.frames == myGui.roi_no) && myGui.action) {//process

		printf("%s:done!\n",__func__);
		myGui.action = 0;//done

		Mat roi_std(myGui.rect.height, myGui.rect.width, CV_32F);
		Mat roi_mean(myGui.rect.height, myGui.rect.width, CV_32F);
		for(int i = 0; i < myGui.rect.height;i++)
			for (int j = 0; j < myGui.rect.width;j++) {
				//unsigned short *pixel_vec= new unsigned short[myGui.roi_no];
				Mat pixel_vec(myGui.roi_no, 1, CV_32F);
				for (int k = 0; k < myGui.roi_no; k++) {
					Mat mat = myGui.rois[k];
					pixel_vec.at<float>(k) = mat.at<unsigned short>(i, j);
					//printf("%d:%f\n",k, pixel_vec.at<float>(k));
				}
				
				Mat tmp_m, tmp_sd;
				double m = 0, sd = 0;

				//m = mean(pixel_vec)[0];
				//cout << "Mean: " << m << endl;

				meanStdDev(pixel_vec, tmp_m, tmp_sd);
				m = tmp_m.at<double>(0, 0);
				sd = tmp_sd.at<double>(0, 0);
				//cout << "Mean: " << m << " , StdDev: " << sd << endl;
				roi_std.at<float>(i, j) = sd;
				roi_mean.at<float>(i, j) = m;
			}
		//void minMaxLoc(InputArray src, double* minVal, double* maxVal = 0, Point* minLoc = 0, Point* maxLoc = 0, InputArray mask = noArray())
		double std_minVal, std_maxVal, mean_minVal, mean_maxVal;
		Point std_minLoc, std_maxLoc, mean_minLoc, mean_maxLoc;
		cv::minMaxLoc(roi_std, &std_minVal, &std_maxVal, &std_minLoc, &std_maxLoc);
		cv::minMaxLoc(roi_mean, &mean_minVal, &mean_maxVal, &mean_minLoc, &mean_maxLoc);
		printf("std_minVal=%.3f, std_maxVal=%.3f,std_minLoc=(x=%d,y=%d),std_maxLoc=(x=%d,y=%d)\n",
			std_minVal, std_maxVal, std_minLoc.x, std_minLoc.y, std_maxLoc.x, std_maxLoc.y);
		printf("mean_minVal=%.3f, mean_maxVal=%.3f,mean_minLoc=(x=%d,y=%d),mean_maxLoc=(x=%d,y=%d)\n",
			mean_minVal, mean_maxVal, mean_minLoc.x, mean_minLoc.y, mean_maxLoc.x, mean_maxLoc.y);

		cv::imshow(myGui.std_win_name, roi_std);
		cv::imshow(myGui.mean_win_name, roi_mean);
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
