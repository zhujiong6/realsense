//gui.cpp
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

	printf("( %d, %d) :", x, y);
	printf("Event : %d, ", event);
	printf("flags : %d\n", flag);
	//printf("The param is : %d\n", param);
	static cv::Point VertexOne, VertexThree;
#if 0
	if (event == CV_EVENT_LBUTTONDOWN || event == CV_EVENT_RBUTTONDOWN) {//upper left point
		VertexOne = Point(x, y);
	}
	if (event == CV_EVENT_LBUTTONUP || event == CV_EVENT_RBUTTONUP) {//lower right point
		VertexThree = Point(x, y);
	}
#else
	if (event == CV_EVENT_LBUTTONDOWN) {//upper left point
		VertexOne = Point(x, y);
	}
	if (event == CV_EVENT_RBUTTONDOWN) {//lower right point
		VertexThree = Point(x, y);
	}

#endif
	if (flag == CV_EVENT_FLAG_LBUTTON || flag == CV_EVENT_FLAG_RBUTTON) {//mouse drag
		VertexThree = Point(x, y);
	}
	my_gui &gui= *(my_gui *)param;
	string win_name = gui.win_name;
	//Mat &Imagex = gui.
	//cvReleaseImage(&Image);
	//Image = cvCloneImage(Imagex);//cvCopy(Imagex, Image, 0);
	Mat Image = gui.image.clone();
	int thickness = 1;
	int lineType = 8;
	int shift = 0;
	//Rectangle(Image, VertexOne, VertexThree, Color, Thickness, CV_AA, Shift);
	if (event == CV_EVENT_MBUTTONDOWN) {//middle mouse button down to action
		cout << "action" << endl;
		cv::rectangle(Image, VertexOne, VertexThree, Scalar(0, 0, 255), thickness, lineType, shift);
	}
	else
		cv::rectangle(Image, VertexOne, VertexThree, Scalar(0, 255, 255), thickness, lineType, shift);
	cv::imshow(win_name, Image);

	printf("VertexOne( %d, %d) ", VertexOne.x, VertexOne.y);
	printf("VertexThree( %d, %d)\n", VertexThree.x, VertexThree.y);
}