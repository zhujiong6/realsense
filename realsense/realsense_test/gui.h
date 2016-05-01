#pragma once
//gui for opencv
//@author : <Thomas Tsai, d04922009@csie.ntu.edu.tw>
#define	FRAME_WIDTH		(640)
#define	FRAME_HEIGHT	(480)
#define	IMAGE_FPS	(30)

#define	MAX_REC_FRAMES	(300)

#define	DEFAULT_BIT_DEPTH	(8)
#define	MAX_GREY_LEVEL	((1<<DEFAULT_BIT_DEPTH) -1)
#define	HIST_WIN_WIDTH 	(256)
#define	HIST_WIN_HEIGHT	(256)


struct my_gui {
	string color_win_name;
	string depth_win_name;
	string std_win_name;
	string mean_win_name;
	string qtstd_win_name;//quantization
	string qtmean_win_name;//quantization
	string qthe_std_win_name;//Histogram equalization of quantization
	string qthe_mean_win_name;//Histogram equalization of quantization

	string crop_win_name;
	Mat color_image;
	Mat depth_image;
	string win_name;		//window name
	Mat image;				//the source image
	Rect rect;				//the target ROI rectangle (x,y,width,height)
	int action;
	int frames;
	int roi_no;		//how many rois to be stored
	//vector<Mat *> depth_imgs;	//pointer list to the stored images
	cv::Mat *rois;	//the roi list
};

//callback of mouse event
void onMouse(int event, int x, int y, int flag, void* param);
void myGui_init(my_gui &myGui);

//http://stackoverflow.com/questions/3071665/getting-a-directory-name-from-a-filename
//split path string to substring folder and file
void SplitFilename(const string& str, string &folder, string &file);

/** @brief Draw the histograms
* input
* hist_table[] : histogram table
* h_size : level of histogram table, ie, 256 grey levels
*/
void draw_hist(unsigned *hist_table, int h_size, const string &win_name,
	int wx = 300, int wy = 300, int flag = CV_WINDOW_AUTOSIZE, Scalar color = Scalar(255, 0, 0)
);

/*
int h_size : usually 256 for 8 bit
win_name : the window name
Mat &image: the image to show its histogram
*/
void draw_hist(Mat &image, const string &win_name, int h_size=(MAX_GREY_LEVEL+1), int flag = CV_WINDOW_NORMAL,
	Scalar color = Scalar(255, 0, 0));

void draw_hist2(Mat& image, const string &win_name, int h_size = (MAX_GREY_LEVEL + 1));

/** @brief  output a format text by opencv
*
*/
void cvPrintf(IplImage* img, const char *text, CvPoint TextPosition, CvFont Font1 = cvFont(1.0, 1.0),
	CvScalar Color = CV_RGB(255, 255, 255));

/** @brief show the image by opencv gui
* buf[] : image buffer
* width, height : dimension of the buffer
* x,y : window position to show
* wname : window name
* depth : color depth
* channel : 1 single channel or 3, RGB
*/
IplImage* cvDisplay(uint8_t *buf, int width, int height, int x, int y, string wname,
	int flag = CV_WINDOW_AUTOSIZE, int depth = IPL_DEPTH_8U, int channel = 1);

IplImage* cvDisplay(IplImage** pimg, uint8_t *buf, int width, int height, int x, int y,
	string wname, int flag = CV_WINDOW_AUTOSIZE, int depth = IPL_DEPTH_8U,
	int channel = 1);


void hw4(my_gui &myGui, Mat &depthMat);
