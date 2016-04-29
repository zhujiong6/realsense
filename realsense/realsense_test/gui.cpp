//gui.cpp
//@author : <Thomas Tsai, d04922009@csie.ntu.edu.tw>

#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <iostream>
#include <string>

//! [namespace]
using namespace cv;
//! [namespace]

using namespace std;
#include "gui.h"

//string win_name = "Display window";

void onMouse(int event, int x, int y, int flag, void* param) {
	if (param == NULL) {
		cout << __func__ << ": null parameters" << endl;
		return;
	}
	my_gui &gui = *(my_gui *)param;
	//static int action = 0;
	//static int frames = 0;
	printf("(x,y)=( %d, %d) :", x, y);
	printf("Event : %d, ", event);
	printf("flags : %d\n", flag);
	//printf("The param is : %d\n", param);
	static cv::Point VertexOne, VertexThree, preV3, preV1;
#if 0
	if (event == CV_EVENT_LBUTTONDOWN || event == CV_EVENT_RBUTTONDOWN) {//upper left point
		VertexOne = Point(x, y);
	}
	if (event == CV_EVENT_LBUTTONUP || event == CV_EVENT_RBUTTONUP) {//lower right point
		VertexThree = Point(x, y);
	}
	if ((flag == CV_EVENT_FLAG_LBUTTON) || (flag == CV_EVENT_FLAG_RBUTTON)) {//mouse drag
		preV3 = VertexThree;
		VertexThree = Point(x, y);
		gui.action = 0;
	}

#else
	if (event == CV_EVENT_LBUTTONDOWN) {//upper left point
		preV1 = VertexOne;
		VertexOne = Point(x, y);
		gui.action = 0;
	}
	if (event == CV_EVENT_RBUTTONDOWN) {//lower right point
		preV3 = VertexThree;
		VertexThree = Point(x, y);
		gui.action = 0;
	}

#endif

	if (gui.image.rows) {
		string win_name = gui.win_name;
		//Mat &Imagex = gui.
		//cvReleaseImage(&Image);
		//Image = cvCloneImage(Imagex);//cvCopy(Imagex, Image, 0);
		Mat Image = gui.image.clone();
		int thickness = 1;
		int lineType = 8;
		int shift = 0;
		
		if ((event == CV_EVENT_LBUTTONDBLCLK)) {
			VertexOne = preV1;
			gui.rect.x = preV1.x;
			gui.rect.y = preV1.y;
			gui.rect.width = VertexThree.x - preV1.x + 1;
			gui.rect.height = VertexThree.y - preV1.y + 1;
		}
		else {
			gui.rect.x = VertexOne.x;
			gui.rect.y = VertexOne.y;
			gui.rect.width = VertexThree.x - VertexOne.x + 1;
			gui.rect.height = VertexThree.y - VertexOne.y + 1;
		}
		//Rectangle(Image, VertexOne, VertexThree, Color, Thickness, CV_AA, Shift);
		if ( (event == CV_EVENT_LBUTTONDBLCLK )|| (event == CV_EVENT_MBUTTONDOWN) ) {//middle mouse button down to action
			gui.frames = 0;//reset it
			gui.action = 1;
			cout << "action" << endl;
			if (gui.rois) {
				//for all rois[i], delete rois[i]
				delete [] gui.rois;
				gui.rois = NULL;
			}
			
			//const int ROWS = 2, COLS = 3, PLANES = 4;
			//int dims[3] = { ROWS, COLS, PLANES };
			//int sizes[] = { gui.rect.height, gui.rect.width, gui.roi_no };
			//gui.rois = new cv::Mat(3, sizes, CV_16UC1, 0);
			gui.rois = new cv::Mat[gui.roi_no];

			cv::rectangle(Image, preV1, VertexThree, Scalar(255, 255, 255), thickness, lineType, shift);
		}
		else
			cv::rectangle(Image, VertexOne, VertexThree, Scalar(200, 200, 200), thickness, lineType, shift);
		cv::imshow(win_name, Image);
	}
	printf("VertexOne( %d, %d) ", VertexOne.x, VertexOne.y);
	printf("VertexThree( %d, %d)\n", VertexThree.x, VertexThree.y);
}