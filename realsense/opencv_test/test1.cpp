//! [includes]
//Thomas Tsai : d04922009@ntu.edu.tw
//How to install opencv 3.1 in visual studio 2015:
//https://www.youtube.com/watch?v=l4372qtZ4dc
//setup environment variables:
//OPENCV_BUILD=d:\opencv-3.1.0\build\x64\vc14\
//OPENCV_DIR=d:\opencv-3.1.0\
//VS2015 include: $(OPENCV_DIR)\build\include

#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <iostream>
#include <string>
//! [includes]

//! [namespace]
using namespace cv;
//! [namespace]

using namespace std;

string win_name = "Display window";

void onMouse(int event, int x, int y, int flag, void* param) {
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
	Mat &Imagex = *(Mat *)param;
	//cvReleaseImage(&Image);
	//Image = cvCloneImage(Imagex);//cvCopy(Imagex, Image, 0);
	Mat Image = Imagex.clone();
	int thickness = 1;
	int lineType = 8;
	int shift = 0;
	//Rectangle(Image, VertexOne, VertexThree, Color, Thickness, CV_AA, Shift);
	if (event == CV_EVENT_MBUTTONDOWN) {//middle mouse button down to action
		cout << "action" << endl;
		cv::rectangle(Image, VertexOne, VertexThree, Scalar(0,0,255), thickness, lineType, shift);
	}else
		cv::rectangle(Image, VertexOne, VertexThree, Scalar(0, 255, 255), thickness, lineType, shift);
	cv::imshow(win_name, Image);

	printf("VertexOne( %d, %d) ", VertexOne.x, VertexOne.y);
	printf("VertexThree( %d, %d)\n", VertexThree.x, VertexThree.y);
}

int main(int argc, char** argv)
{
	//! [load]
	string imageName("HappyFish.jpg"); // by default
	if (argc > 1)
	{
		imageName = argv[1];
	}
	//! [load]

	//! [mat]
	Mat image;
	//! [mat]

	//! [imread]
	image = imread(imageName.c_str(), IMREAD_COLOR); // Read the file
													 //! [imread]

	if (image.empty())                      // Check for invalid input
	{
		cout << "Could not open or find the image" << std::endl;
		return -1;
	}

	//! [window]
	namedWindow(win_name, WINDOW_AUTOSIZE); // Create a window for display.
													//! [window]
	setMouseCallback(win_name, onMouse, &image);//setup callback
	//C++: void rectangle(Mat& img, Point pt1, Point pt2, const Scalar& color, int thickness = 1, int lineType = 8, int shift = 0)
	//C++ : void rectangle(Mat& img, Rect rec, const Scalar& color, int thickness = 1, int lineType = 8, int shift = 0)
	//cv::Rect rec = Rect(10, 10, 50, 40);
	//cv::rectangle(image,rec, Scalar(0, 255, 255));
													//! [imshow]
	imshow("Display window", image);                // Show our image inside it.
													//! [imshow]

													//! [wait]
	waitKey(0); // Wait for a keystroke in the window
				//! [wait]
	return 0;
}
