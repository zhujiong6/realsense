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

void myGui_init(my_gui &myGui)
{
	//////////////////////////////////////////////
	// my gui init
	//////////////////////////////////////////////

	memset(&myGui, 0, sizeof(myGui));
	myGui.frames = 0;
	myGui.roi_no = MAX_REC_FRAMES;

	myGui.rect = cv::Rect(0, 0, 0, 0);
	myGui.depth_win_name = "OpenCV Window Depth";
	myGui.win_name = "OpenCV Window Depth";
	namedWindow(myGui.depth_win_name, WINDOW_AUTOSIZE);
	setMouseCallback(myGui.depth_win_name, onMouse, &myGui);//setup callback
	myGui.color_win_name = "OpenCV Window Color";
	namedWindow(myGui.color_win_name, WINDOW_AUTOSIZE/*WINDOW_KEEPRATIO*/);

	myGui.std_win_name = "std window";
	namedWindow(myGui.std_win_name, WINDOW_NORMAL);

	myGui.mean_win_name = "mean window";
	namedWindow(myGui.mean_win_name, WINDOW_NORMAL);

	myGui.crop_win_name = "cropped roi window";
	namedWindow(myGui.crop_win_name, WINDOW_NORMAL);

	myGui.std_win_name = "std of roi";
	namedWindow(myGui.std_win_name, WINDOW_NORMAL);
	myGui.mean_win_name = "mean of roi";
	namedWindow(myGui.mean_win_name, WINDOW_NORMAL);
	myGui.qtstd_win_name = "std quantization of roi";//quantization
	namedWindow(myGui.qtstd_win_name, WINDOW_NORMAL);
	myGui.qtmean_win_name = "mean quantization of roi";//quantization
	namedWindow(myGui.qtmean_win_name, WINDOW_NORMAL);
	myGui.qthe_std_win_name = "HE of std quantization roi";//Histogram equalization of quantization
	namedWindow(myGui.qthe_std_win_name, WINDOW_NORMAL);
	myGui.qthe_mean_win_name = "HE of mean quantization roi";//Histogram equalization of quantization
	namedWindow(myGui.qthe_mean_win_name, WINDOW_NORMAL);
	////////////////////////////////////////////////////////
}

/*
// IplImage to Mat
IplImage *img;
cv::Mat mat(img, 0);
// Mat to IplImage
cv::Mat mat;
IplImage *img = IplImage(mat);
*/

//http://stackoverflow.com/questions/3071665/getting-a-directory-name-from-a-filename
void SplitFilename(const string& str, string &folder, string &file)
{
	size_t found;
	cout << "Splitting: " << str << endl;
	found = str.find_last_of("/\\");
	folder = str.substr(0, found);
	file = str.substr(found + 1);
	cout << " folder: " << str.substr(0, found) << endl;
	cout << " file: " << str.substr(found + 1) << endl;
}

/** @brief Draw the histograms
* input
* hist_table[] : histogram table
* h_size : level of histogram table, ie, 256 grey levels
* int wx, int wy : window location
*/
//draw_hist(hist_tableL, (MAX_GREY_LEVEL + 1), wname_Lhist, WIN_GAP_X + SCR_X_OFFSET,
//	WIN_GAP_Y * 2 + SCR_Y_OFFSET, cvFlag);
void draw_hist(unsigned *hist_table, int h_size, const string &win_name, int wx, int wy,
	int flag, Scalar color)
{
	float ht[MAX_GREY_LEVEL];
	for (int i = 0; i < h_size; i++)
		ht[i] = hist_table[i];
	//create a openCV matrix from an array : 1xh_size, floating point array
	Mat b_hist = Mat(1, h_size, CV_32FC1, ht);
	//calcHist( &bgr_planes[0], 1, 0, Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate );

	int hist_w = HIST_WIN_WIDTH, hist_h = HIST_WIN_HEIGHT;
	int bin_w = cvRound((double)hist_w / h_size);

	Mat histImage(hist_h, hist_w, CV_8UC3, Scalar(0, 0, 0));
	// Normalize the result to [ 0, histImage.rows ]
	normalize(b_hist, b_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());
	for (int i = 1; i < h_size; i++)
		line(histImage, Point(bin_w*(i - 1), hist_h - cvRound(b_hist.at<float>(i - 1))),
			Point(bin_w*(i), hist_h - cvRound(b_hist.at<float>(i))),
			color/*Scalar( 255, 0, 0)*/, 2, 8, 0);
	//string win_name("Histogram: ");
	//win_name = win_name + t_name;
	cv::namedWindow(win_name, flag);
	cv::moveWindow(win_name, wx, wy);
	cv::imshow(win_name, histImage);
}

/*
int h_size : usually 256 for 8 bit
win_name : the window name
Mat &image: the image to show its histogram
*/
void draw_hist(Mat &image, const string &win_name, int h_size, int flag, Scalar color)
{
	// Initialize parameters
	int histSize = h_size;    // bin size
	float range[] = { 0, h_size-1 };
	const float *ranges[] = { range };

	// Calculate histogram
	MatND hist;
	calcHist(&image, 1, 0, Mat(), hist, 1, &histSize, ranges, true, false);

	// Show the calculated histogram in command window
	double total;
	total = image.rows * image.cols;
	for (int h = 0; h < histSize; h++)
	{
		float binVal = hist.at<float>(h);
		cout << " " << binVal;
	}

	// Plot the histogram
	//int hist_w = 512; int hist_h = 400;
	int hist_w = HIST_WIN_WIDTH, hist_h = HIST_WIN_HEIGHT;
	int bin_w = cvRound((double)hist_w / histSize);

	Mat histImage(hist_h, hist_w, CV_8UC1, Scalar(0, 0, 0));
	normalize(hist, hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());

	for (int i = 1; i < histSize; i++)
	{
		line(histImage, Point(bin_w*(i - 1), hist_h - cvRound(hist.at<float>(i - 1))),
			Point(bin_w*(i), hist_h - cvRound(hist.at<float>(i))),
			color/*Scalar(255, 0, 0)*/, 2, 8, 0);
	}

	cv::namedWindow(win_name, flag);
	cv::imshow(win_name, histImage);

}

/** @brief  output a format text by opencv
*
*/
void cvPrintf(IplImage* img, const char *text, CvPoint TextPosition, CvFont Font1,
	CvScalar Color)
{
	//char text[] = "Funny text inside the box";
	//int fontFace = FONT_HERSHEY_SCRIPT_SIMPLEX;
	//double fontScale = 2;

	// center the text
	//Point textOrg((img.cols - textSize.width)/2,
	//			(img.rows + textSize.height)/2);

	//double Scale=2.0;
	//int Thickness=2;
	//CvScalar Color=CV_RGB(255,0,0);
	//CvPoint TextPosition=cvPoint(400,50);
	//CvFont Font1=cvFont(Scale,Thickness);
	// then put the text itself
	cvPutText(img, text, TextPosition, &Font1, Color);
	//cvPutText(img, text, TextPosition, &Font1, CV_RGB(0,255,0));
}

/** @brief show the image by opencv gui
* buf[] : image buffer
* width, height : dimension of the buffer
* x,y : window position to show
* wname : window name
* depth : color depth
* channel : 1 single channel or 3, RGB
*/
IplImage* cvDisplay(uint8_t *buf, int width, int height, int x, int y, string wname,
	int flag, int depth, int channel)
{
	//show image H, the histogram equlization of image D
	IplImage* img = cvCreateImageHeader(cvSize(width, height), depth, channel);
	cvSetData(img, buf, width);
	namedWindow(wname, flag);	// Create a window for display.
	moveWindow(wname, x, y);
	cvShowImage(wname.c_str(), img);     // Show our image inside it.
	return img;
}

IplImage* cvDisplay(IplImage** pimg, uint8_t *buf, int width, int height, int x, int y, string wname,
	int flag, int depth, int channel)
{
	printf(">>%s:pimg=%p, *pimg=%p\n", __func__, pimg, *pimg);
	if ((pimg != NULL) && (*pimg != NULL))
		cvReleaseImageHeader(pimg);

	//show image H, the histogram equlization of image D
	IplImage* img = cvCreateImageHeader(cvSize(width, height), depth, channel);
	cvSetData(img, buf, width);
	namedWindow(wname, flag);	// Create a window for display.
	moveWindow(wname, x, y);
	cvShowImage(wname.c_str(), img);     // Show our image inside it.
	printf("<<%s:img=%p\n", __func__, img);
	return img;
}
