#pragma once
//gui for opencv
//@author : <Thomas Tsai, d04922009@csie.ntu.edu.tw>
#define	FRAME_WIDTH		(640)
#define	FRAME_HEIGHT	(480)
#define	IMAGE_FPS	(30)

struct my_gui {
	string color_win_name;
	string depth_win_name;
	string std_win_name;
	string mean_win_name;
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
void hw4(my_gui &myGui, Mat &depthMat);