#pragma once

/** Thomas Tsai : d04922009@ntu.edu.tw
http://ccw1986.blogspot.tw/2015/11/realsenseopencv-translate-image-format.html
http://stackoverflow.com/questions/32609341/convert-a-pxcimage-into-an-opencv-mat
*/
enum ImageFormat {
	STREAM_TYPE_COLOR = 0,
	STREAM_TYPE_DEPTH = 1,
	STREAM_TYPE_IR = 2
};
void ConvertPXCImageToOpenCVMat(PXCImage *inImg, cv::Mat *outImg, ImageFormat imgFormat= STREAM_TYPE_COLOR);
