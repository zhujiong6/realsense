#pragma once
enum ImageFormat {
	STREAM_TYPE_COLOR = 0,
	STREAM_TYPE_DEPTH = 1,
	STREAM_TYPE_IR = 2
};
void ConvertPXCImageToOpenCVMat(PXCImage *inImg, cv::Mat *outImg, ImageFormat imgFormat= STREAM_TYPE_COLOR);
