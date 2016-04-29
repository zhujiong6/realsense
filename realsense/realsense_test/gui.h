#pragma once
struct my_gui {
	string color_win_name;
	string depth_win_name;
	Mat color_image;
	Mat depth_image;
	string win_name;
	Mat image;
};


void onMouse(int event, int x, int y, int flag, void* param);