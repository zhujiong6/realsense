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

void showHistogram(Mat& img)
{
	int bins = 256;             // number of bins
	int nc = img.channels();    // number of channels

	vector<Mat> hist(nc);       // histogram arrays

								// Initalize histogram arrays
	for (int i = 0; i < hist.size(); i++)
		hist[i] = Mat::zeros(1, bins, CV_32SC1);

	// Calculate the histogram of the image
	for (int i = 0; i < img.rows; i++)
	{
		for (int j = 0; j < img.cols; j++)
		{
			for (int k = 0; k < nc; k++)
			{
				uchar val = nc == 1 ? img.at<uchar>(i, j) : img.at<Vec3b>(i, j)[k];
				hist[k].at<int>(val) += 1;
			}
		}
	}

	// For each histogram arrays, obtain the maximum (peak) value
	// Needed to normalize the display later
	int hmax[3] = { 0,0,0 };
	for (int i = 0; i < nc; i++)
	{
		for (int j = 0; j < bins - 1; j++)
			hmax[i] = hist[i].at<int>(j) > hmax[i] ? hist[i].at<int>(j) : hmax[i];
	}

	const char* wname[3] = { "blue", "green", "red" };
	Scalar colors[3] = { Scalar(255,0,0), Scalar(0,255,0), Scalar(0,0,255) };

	vector<Mat> canvas(nc);

	// Display each histogram in a canvas
	for (int i = 0; i < nc; i++)
	{
		canvas[i] = Mat::ones(125, bins, CV_8UC3);

		for (int j = 0, rows = canvas[i].rows; j < bins - 1; j++)
		{
			line(
				canvas[i],
				Point(j, rows),
				Point(j, rows - (hist[i].at<int>(j) * rows / hmax[i])),
				nc == 1 ? Scalar(200, 200, 200) : colors[i],
				1, 8, 0
			);
		}

		imshow(nc == 1 ? "value" : wname[i], canvas[i]);
	}
}


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
	string imageName("lena.jpg"/*"HappyFish.jpg"*/); // by default
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
	showHistogram(image);
													//! [imshow]
	imshow("Display window", image);                // Show our image inside it.
													//! [imshow]

													//! [wait]
	waitKey(0); // Wait for a keystroke in the window
				//! [wait]
	return 0;
}
