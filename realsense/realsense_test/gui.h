#pragma once
//gui for opencv
//@author : <Thomas Tsai, d04922009@csie.ntu.edu.tw>

struct my_gui {
	string color_win_name;
	string depth_win_name;
	Mat color_image;
	Mat depth_image;
	string win_name;		//window name
	Mat image;				//the source image
	Rect rect;				//the target ROI rectangle (x,y,width,height)
	int action;
	int frames;
	int imglist_size;		//how many images to be stored
	vector<Mat *> depth_imgs;	//pointer list to the stored images
};

//callback of mouse event
void onMouse(int event, int x, int y, int flag, void* param);