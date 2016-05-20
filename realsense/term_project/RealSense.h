#pragma once
#include <iostream>
#include <conio.h>
#include <iostream>
#include <string>

#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "pxcsensemanager.h"
#include "pxcmetadata.h"
#include "util_cmdline.h"
#include "util_render.h"
#include "pxcprojection.h"
#include "VECTOR3D.h"

class DepthElement
{
public:
	int index;  // UShortElement ���� Image �� Index
	ushort depthVal;


	DepthElement() {}
	DepthElement(int index, ushort depthVal) { this->index = index;  this->depthVal = depthVal; }


	~DepthElement() {}
};


class RealSense
{
public:
	enum VIDEO_STREAM_MODE { COLOR_MODE, DEPTH_MODE, COLOR_DEPTH_MODE };
	enum VIDEO_STREAM_TYPE { STREAM_COLOR, STREAM_DEPTH };


	std::vector<cv::Mat> mColorImgs;
	std::vector<PXCImage*> mAllTheTimesPXCColorImgs, mAllTheTimesPXCDepthImgs;


	RealSense();
	~RealSense();
	pxcStatus init(int mirror = 1);

	void startVideoStream(VIDEO_STREAM_MODE mVideoStreamMode = COLOR_DEPTH_MODE);  // �}�� Video Stream
	void stopVideoStream();  // ���� Video Stream

	void showVideoSteams(VIDEO_STREAM_MODE mVideoStreamMode = COLOR_DEPTH_MODE);  // ���� Video Stream
	bool captureImgs(int imgCapTimes, VIDEO_STREAM_MODE mVideoStreamMode = COLOR_DEPTH_MODE);  // ���� (imgCapTimes ��)
	void getColorImgColorAligned3DCoords(cv::Mat &mColorImg, std::vector<POINT3D> &mColorAligned3DCoords); // ���o 1 �� Color Image �M Color-Aligned 3D Coordinate Image
	void PXCImg2OpenCVImg(PXCImage *mPXCImg, cv::Mat &mOpenCVImg, VIDEO_STREAM_TYPE mVideoStreamType);  // �N 1 �i PXCImage �榡�� Image �ন OpenCV �榡�� Image



private:
	PXCSession::ImplVersion mRSversion;///the version of real sense
	PXCSenseManager *mPXCSenseManager;
	int depthValMax;
	PXCCapture::DeviceInfo mDeviceInfo;
	int imgWidth, imgHeight, videoStreamFPS;
	PXCProjection *mPXCProjection;


	void getDepthMaxVal();

	PXCCapture::Sample* getVideoFrame();  // ���o 1 �i Video Frame (�]�A: 1 �i PXCImage �榡�� Color Image �M 1 �i PXCImage �榡�� Depth Image)
	PXCImage* OpenCVImg2PXCImg(cv::Mat mOpenCVImg, VIDEO_STREAM_TYPE mVideoStreamType);  // �N 1 �i OpenCV �榡�� Image �ন PXCImage �榡�� Image

	// �N���l�� Depth Image �ন�m�⪩�i���ܪ� Image
	void getColoredDepthImg2Show(cv::Mat mDepthImg, cv::Mat &mColoredDepthImg);
	void calculateHistogram(cv::Mat mDepthImg, int histogramSize, std::vector<float> &histograms);
	void getGrayedDepthImg2Show(cv::Mat mDepthImg, cv::Mat &mGrayedDepthImg2Show);

	// ���o PXCImage �� Copy
	PXCImage* getPXCImgColorCopy(PXCImage *mPXCImgColor);
	PXCImage* getPXCImgDepthCopy(PXCImage *mPXCImgDepth);


	// ���o Color Image �� Depth Image:  (�� Depth Image ��������)
	// (1) ���Ҧ� Color Images �����Ĥ@�i�@���ҭn�� Color Image
	// (2) �N�C������ PXCImage �榡�� Depth Image �ন OpenCV �榡�� Depth Image
	// (3) ���Ҧ� Depth Images �� Median
	// (4) �N���X�� Median Depth Image ���榡�� PXCImage �ন OpenCV
	// (5) �N Median Depth Image �ন Median-Color-Aligned Depth Image
	// (6) �N Median-Color-Aligned Depth Image �ন Color-Aligend 3D Coordinates
	void getAllTheTimesCVDepthImgs(std::vector<cv::Mat> &mAllTheTimesCVDepthImgs);
	void getMedianDepthImg(std::vector<cv::Mat> mAllTheTimesCVDepthImgs, cv::Mat &mMedianCVDepthImg);
	void getMedianUShortElement(std::vector<DepthElement> mDepthElements, DepthElement &mMedianDepthElement);
	void convertPXCDepthImg2ColorAlignedDepthImg(PXCImage* mPXCDepthImg, cv::Mat &mColorAlignedDepthImg);
	ushort getDepthValBilinearInter(cv::Mat mDepthImg, float u, float v);
	void convertColorAlignedDepthImg2ColorAligned3DCoords(cv::Mat mColorAlignedDepthImg, std::vector<POINT3D> &mColorAligned3DCoords);
	void convertCoordSysLeft2Right(std::vector<POINT3D> &m3DCoords);
};
