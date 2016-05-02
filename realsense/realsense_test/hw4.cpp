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
#include <fstream>

#include "realsense2cvmat.h"

//! [namespace]
using namespace cv;
using namespace std;
//! [namespace]
#include "gui.h"

/* (d)
convert float matrix to 8bit unsigned char matrix
src : 32bit float matrix
*/
void quantization(Mat &src, Mat &dst)
{
	//void minMaxLoc(InputArray src, double* minVal, double* maxVal = 0, Point* minLoc = 0, Point* maxLoc = 0, InputArray mask = noArray())
	double minVal, maxVal;
	Point minLoc, maxLoc;
	cv::minMaxLoc(src, &minVal, &maxVal, &minLoc, &maxLoc);
	
	printf("%s:minVal=%.3f, maxVal=%.3f,minLoc=(x=%d,y=%d),maxLoc=(x=%d,y=%d)\n",__func__,
		minVal, maxVal, minLoc.x, minLoc.y, maxLoc.x, maxLoc.y);

	dst.create(src.rows,src.cols,CV_8UC1);
	for(int i = 0; i < src.rows; i ++)
		for (int j = 0;j < src.cols;j++) {
			dst.at <unsigned char>(i,j)= (unsigned char) ((src.at<float>(i,j) - minVal) * 255 / (maxVal - minVal) );
		}
}

void hw4(my_gui &myGui, Mat &depthMat)
{

	ofstream fout;

	if (myGui.action && (myGui.frames < myGui.roi_no)) {
		Mat roi(depthMat, myGui.rect); // using a rectangle ROI
		myGui.rois[myGui.frames]= roi.clone();
		char prg_str[80];
		sprintf_s(prg_str,"%d/%d", myGui.frames+1, myGui.roi_no);
		cv::putText(roi, prg_str, Point(10,15), FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
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
				roi_std.at<float>(i, j) = (float)sd;
				roi_mean.at<float>(i, j) = (float)m;
			}
		//void minMaxLoc(InputArray src, double* minVal, double* maxVal = 0, Point* minLoc = 0, Point* maxLoc = 0, InputArray mask = noArray())
		double std_minVal, std_maxVal, mean_minVal, mean_maxVal;
		Point std_minLoc, std_maxLoc, mean_minLoc, mean_maxLoc;
		cv::minMaxLoc(roi_std, &std_minVal, &std_maxVal, &std_minLoc, &std_maxLoc);
		//cv::minMaxLoc(roi_mean, &mean_minVal, &mean_maxVal, &mean_minLoc, &mean_maxLoc);
		printf("std_minVal=%.3f, std_maxVal=%.3f,std_minLoc=(x=%d,y=%d),std_maxLoc=(x=%d,y=%d)\n",
			std_minVal, std_maxVal, std_minLoc.x, std_minLoc.y, std_maxLoc.x, std_maxLoc.y);
		FILE *fp;
		fp = fopen("std_MinMaxValue.txt", "w");
		if (fp != NULL)
			fprintf(fp, "std_minVal=%.3f, std_maxVal=%.3f,std_minLoc=(x=%d,y=%d),std_maxLoc=(x=%d,y=%d)\n",
				std_minVal, std_maxVal, std_minLoc.x, std_minLoc.y, std_maxLoc.x, std_maxLoc.y);
		fclose(fp);
		//printf("mean_minVal=%.3f, mean_maxVal=%.3f,mean_minLoc=(x=%d,y=%d),mean_maxLoc=(x=%d,y=%d)\n",
		//	mean_minVal, mean_maxVal, mean_minLoc.x, mean_minLoc.y, mean_maxLoc.x, mean_maxLoc.y);

		//(b)
		fout.open("depth_std.txt", ios::out | ios::trunc);
		fout << roi_std;
		//fout << "roi_mean:\n" << endl << roi_mean << endl;
		fout.close();

		//(a)
		fout.open("cropped_Image.txt", ios::out | ios::trunc);
		fout << myGui.rois[0];
		fout.close();
		imwrite("cropped_Image.bmp", myGui.rois[0]);

		cv::imshow(myGui.std_win_name, roi_std);
		cv::imshow(myGui.mean_win_name, roi_mean);
		
		////////////////////////////////////////////////////
		//(c) mean and std of roi_std
		Mat tmp_m, tmp_sd;
		double m = 0, sd = 0;
		meanStdDev(roi_std, tmp_m, tmp_sd);
		m = tmp_m.at<double>(0, 0);
		sd = tmp_sd.at<double>(0, 0);
		cout << "(c) Mean: " << m << " , StdDev: " << sd << endl;
		fout.open("std_mean&variance.txt", ios::out | ios::trunc);
		fout << "Mean and std of of the 2D array of standard deviation:" << endl;
		fout<< "(c) Mean: " << m << " , StdDev: " << sd << endl;
		fout.close();
		//////////////////////////////////////////////
		// (d)
		Mat std_8bit, mean_8bit;
		printf("quantize roi\n");
		quantization(roi_std, std_8bit);
		cv::imshow(myGui.qtstd_win_name, std_8bit);
		imwrite("std_8bit.bmp", std_8bit);
		fout.open("std_8bit.txt", ios::out | ios::trunc);
		fout << std_8bit << endl;
		fout.close();

		//draw_hist(hist_tableL, (MAX_GREY_LEVEL + 1), wname_Lhist, WIN_GAP_X + SCR_X_OFFSET,
		//	WIN_GAP_Y * 2 + SCR_Y_OFFSET, cvFlag);
		//(f)
		string win_name = "roi std :histogram";
		//draw_hist2(std_8bit, win_name);
		imwrite("histogram_std_8bit.bmp",draw_hist(std_8bit, win_name));
		

		////////////////////////////////////////////////////
		//(c) mean and std of roi_std
		//Mat tmp_m, tmp_sd;
		//double m = 0, sd = 0;
		meanStdDev(std_8bit, tmp_m, tmp_sd);
		m = tmp_m.at<double>(0, 0);
		sd = tmp_sd.at<double>(0, 0);
		cout << "(c) quantization Mean: " << m << " , StdDev: " << sd << endl;
		fout.open("std_mean&variance.txt", ios::out | ios::app);
		fout << "Mean and std of quantilized 8-bit standard deviation image:" << endl;
		fout << "(c) Mean: " << m << " , StdDev: " << sd << endl;
		fout.close();
		//////////////////////////////////////////////

		//quantization(roi_mean, mean_8bit);
		//cv::imshow(myGui.qtmean_win_name, mean_8bit);

		/////////////////////////////////////////////////
		//(e)
		/////////////////////////////////////////////////
		Mat std_8bit_he, mean_8bit_he;
		cv::equalizeHist(std_8bit, std_8bit_he);
		cv::imshow(myGui.qthe_std_win_name, std_8bit_he);
		imwrite("std_8bit_equlized.bmp", std_8bit_he);
		fout.open("std_8bit_equlized.txt", ios::out|ios::trunc);
		fout << std_8bit_he << endl;
		fout.close();
		//(f)
		//draw_hist2(std_8bit_he, win_name);
		win_name = "roi std : histogram EQ";
		imwrite("histogram_std_8bit_equlized.bmp", draw_hist(std_8bit_he, win_name));

		//draw_hist(hist_tableL, (MAX_GREY_LEVEL + 1), wname_Lhist, WIN_GAP_X + SCR_X_OFFSET,
		//	WIN_GAP_Y * 2 + SCR_Y_OFFSET, cvFlag);
		//cv::equalizeHist(mean_8bit, mean_8bit_he);
		//cv::imshow(myGui.qthe_mean_win_name, mean_8bit_he);

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
