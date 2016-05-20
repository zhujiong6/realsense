#include "RealSense.h"
#include <windows.h>


// �Ω� C++ ���� Quick Sort �������禡 (�p -> �j) (�ھ� Depth Value)
int compareDepthElement(const void * a, const void * b) { return ((int)((*(DepthElement*)a).depthVal) - (int)((*(DepthElement*)b).depthVal)); }


RealSense::RealSense() {}


RealSense::~RealSense() {}

pxcStatus RealSense::init(int mirror=1)
{
	/* 1. Creates an instance of the PXCSenseManager */
	PXCSenseManager *pp = PXCSenseManager::CreateInstance();
	if (!pp) {
		wprintf_s(L"Unable to create the SenseManager\n");
		return 3;
	}

	PXCSession::ImplVersion mRSversion = pp->QuerySession()->QueryVersion();
	std::cout << "SDK Version:" << version.major << "." << version.minor << std::endl;

	/* Collects command line arguments */
	UtilCmdLine cmdl(pp->QuerySession());
	if (!cmdl.Parse(L"-listio-nframes-sdname-csize-dsize-isize-lsize-rsize-file-record-noRender-mirror", argc, argv)) return 3;

	/* Sets file recording or playback */
	PXCCaptureManager *cm = pp->QueryCaptureManager();
	cm->SetFileName(cmdl.m_recordedFile, cmdl.m_bRecord);
	if (cmdl.m_sdname) cm->FilterByDeviceInfo(cmdl.m_sdname, 0, 0);

#if	OPENCV_SUPPORTED
	PXCImage *colorIm, *depthIm, *irIm, *rightIm, *leftIm;
#else
	// Create stream renders
	UtilRender renderc(L"Color"), renderd(L"Depth"), renderi(L"IR"), renderr(L"Right"), renderl(L"Left");
#endif
	pxcStatus sts;

	//////////////////////////////////////////////
	// my gui init
	//////////////////////////////////////////////
	my_gui myGui;
	myGui_init(myGui);
	////////////////////////////////////////////////////////
	do {
		//2. enable realsense camera streams
		/* Apply command line arguments */
		pxcBool revert = false;
		if (cmdl.m_csize.size()>0) {
			pp->EnableStream(PXCCapture::STREAM_TYPE_COLOR, cmdl.m_csize.front().first.width, cmdl.m_csize.front().first.height, (pxcF32)cmdl.m_csize.front().second);
		}
		if (cmdl.m_dsize.size()>0) {
			pp->EnableStream(PXCCapture::STREAM_TYPE_DEPTH, cmdl.m_dsize.front().first.width, cmdl.m_dsize.front().first.height, (pxcF32)cmdl.m_dsize.front().second);
		}
		if (cmdl.m_isize.size() > 0) {
			pp->EnableStream(PXCCapture::STREAM_TYPE_IR, cmdl.m_isize.front().first.width, cmdl.m_isize.front().first.height, (pxcF32)cmdl.m_isize.front().second);
		}
		if (cmdl.m_rsize.size() > 0) {
			pp->EnableStream(PXCCapture::STREAM_TYPE_RIGHT, cmdl.m_rsize.front().first.width, cmdl.m_rsize.front().first.height, (pxcF32)cmdl.m_rsize.front().second);
		}
		if (cmdl.m_lsize.size() > 0) {
			pp->EnableStream(PXCCapture::STREAM_TYPE_LEFT, cmdl.m_lsize.front().first.width, cmdl.m_lsize.front().first.height, (pxcF32)cmdl.m_lsize.front().second);
		}
		if (cmdl.m_csize.size() == 0 && cmdl.m_dsize.size() == 0 && cmdl.m_isize.size() == 0 && cmdl.m_rsize.size() == 0 && cmdl.m_lsize.size() == 0) {
#if 1
			pp->EnableStream(PXCCapture::STREAM_TYPE_DEPTH, FRAME_WIDTH, FRAME_HEIGHT, (pxcF32)IMAGE_FPS);
			pp->EnableStream(PXCCapture::STREAM_TYPE_COLOR, FRAME_WIDTH, FRAME_HEIGHT, (pxcF32)IMAGE_FPS);
#else
			PXCVideoModule::DataDesc desc = {};
			if (cm->QueryCapture()) {
				cm->QueryCapture()->QueryDeviceInfo(0, &desc.deviceInfo);
			}
			else {
				desc.deviceInfo.streams = PXCCapture::STREAM_TYPE_COLOR | PXCCapture::STREAM_TYPE_DEPTH;
				revert = true;
			}
			pp->EnableStreams(&desc);
#endif
		}
		// 3. Set the coordinate system
		PXCSession *session = pp->QuerySession();
		session->SetCoordinateSystem(PXCSession::COORDINATE_SYSTEM_FRONT_DEFAULT /*COORDINATE_SYSTEM_REAR_OPENCV*/);

		/* 4. Initializes the pipeline */
		sts = pp->Init();
		if (sts<PXC_STATUS_NO_ERROR) {
			if (revert) {
				/* Enable a single stream */
				pp->Close();
				pp->EnableStream(PXCCapture::STREAM_TYPE_DEPTH);
				sts = pp->Init();
				if (sts<PXC_STATUS_NO_ERROR) {
					pp->Close();
					pp->EnableStream(PXCCapture::STREAM_TYPE_COLOR);
					sts = pp->Init();
				}
			}
			if (sts<PXC_STATUS_NO_ERROR) {
				wprintf_s(L"Failed to locate any video stream(s)\n");
				pp->Release();
				return sts;
			}
		}

		/* Reset all properties */
		PXCCapture::Device *device = pp->QueryCaptureManager()->QueryDevice();
		device->ResetProperties(PXCCapture::STREAM_TYPE_ANY);

		/* Set mirror mode */
		if (cmdl.m_bMirror) {
			device->SetMirrorMode(PXCCapture::Device::MirrorMode::MIRROR_MODE_HORIZONTAL);
		}
		else {
			device->SetMirrorMode(PXCCapture::Device::MirrorMode::MIRROR_MODE_DISABLED);
		}

}


void RealSense::startVideoStream(VIDEO_STREAM_MODE mVideoStreamMode/* = COLOR_DEPTH_MODE*/)
{
	// �Ыؤ@�� PXCSenseManager Instance
	mPXCSenseManager = PXCSenseManager::CreateInstance();
	while (mPXCSenseManager == NULL)
	{
		std::cout << "Create the PXCSenseManager Failed !!" << std::endl;
		mPXCSenseManager = PXCSenseManager::CreateInstance();
	}


	// �]�w World Coordinate System ����
	mPXCSenseManager->QuerySession()->SetCoordinateSystem(PXCSession::COORDINATE_SYSTEM_FRONT_DEFAULT);


	// �]�w�n�}�ҭ��@�� Video Stream �ά����Ѽ�
	imgWidth = 640;
	imgHeight = 480;
	videoStreamFPS = 30;
	if (mVideoStreamMode == COLOR_MODE || mVideoStreamMode == COLOR_DEPTH_MODE)
		mPXCSenseManager->EnableStream(PXCCapture::STREAM_TYPE_COLOR, imgWidth, imgHeight, videoStreamFPS);

	if (mVideoStreamMode == DEPTH_MODE || mVideoStreamMode == COLOR_DEPTH_MODE)
		mPXCSenseManager->EnableStream(PXCCapture::STREAM_TYPE_DEPTH, imgWidth, imgHeight, videoStreamFPS);


	// �}�� Video Stream
	if(mPXCSenseManager->Init() != PXC_STATUS_NO_ERROR)
		std::cout << "Init the PXCSenseManager Failed !!" << std::endl;


	// �� Depth Vdieo Stream �������F��
	if (mVideoStreamMode == DEPTH_MODE || mVideoStreamMode == COLOR_DEPTH_MODE)
	{
		getDepthMaxVal();  // �p�� Depth Maximum Value
		mPXCProjection = mPXCSenseManager->QueryCaptureManager()->QueryDevice()->CreateProjection();  // �Ыؤ@�� Projection Insatnce
	}
}


void RealSense::getDepthMaxVal()
{
	if (mPXCSenseManager != NULL)
	{
		// �p�� RealSense Depth Value Maximum
		PXCRangeF32 mDepthSensorRange = mPXCSenseManager->QueryCaptureManager()->QueryDevice()->QueryDepthSensorRange();
		std::cout << "Depth Sensor Range(mm): (min, Max):\t(" << mDepthSensorRange.min << ", " << mDepthSensorRange.max << ")" << std::endl;
		depthValMax = mDepthSensorRange.max - mDepthSensorRange.min;
	}
}


void RealSense::stopVideoStream()
{
	mPXCSenseManager->Close();  // ���� Video Stream
	mPXCSenseManager->Release();  // ���� PXCSenseManager Instance
}




void RealSense::showVideoSteams(VIDEO_STREAM_MODE mVideoStreamMode/* = COLOR_DEPTH_MODE*/)
{
	// �}�� Video Stream
	startVideoStream(mVideoStreamMode);


	while (true)
	{
		PXCCapture::Sample *sample;
		cv::Mat mColorImg, mDepthImg, mColoredDepthImg2Show;


		// ���o 1 �i Video Frame (�]�A: 1 �i PXCImage �榡�� Color Image �M 1 �i PXCImage �榡�� Depth Image)
		sample = getVideoFrame();


		if (mVideoStreamMode == COLOR_MODE || mVideoStreamMode == COLOR_DEPTH_MODE)
		{
			PXCImg2OpenCVImg(sample->color, mColorImg, STREAM_COLOR);  // �N PXCImage �榡�� Color Image �ন OpenCV �榡�� Color Image
			cv::imshow("Color Image", mColorImg);  // �N Color Image �Q�� OpenCV ���ܥX��
		}


		if (mVideoStreamMode == DEPTH_MODE || mVideoStreamMode == COLOR_DEPTH_MODE)
		{
			PXCImg2OpenCVImg(sample->depth, mDepthImg, STREAM_DEPTH);  // �N PXCImage �榡�� Depth Image �ন OpenCV �榡�� Depth Image
			getColoredDepthImg2Show(mDepthImg, mColoredDepthImg2Show);  // �N���l�� Depth Image �ন�m�⪩�i���ܪ� Image
			//getGrayedDepthImg2Show(mDepthImg, mColoredDepthImg2Show);  // �N���l�� Depth Image �ন�Ƕ����i���ܪ� Image
			cv::imshow("Depth Image", mColoredDepthImg2Show);  // �N Depth Image �Q�� OpenCV ���ܥX��
		}


		mPXCSenseManager->ReleaseFrame();  // ���� Video Frame


		// �Y�ϥΪ̫��U "q" ��, "ENTER" �� �άO "SPACE" ��, �N�������� Color Image �M Depth Image
		int keyIndex = cv::waitKey(40);
		if (keyIndex == 'q' || keyIndex == 0xD || keyIndex == 0x20)
			break;
	}


	// ���� Video Stream
	stopVideoStream();
}


PXCCapture::Sample* RealSense::getVideoFrame()
{
	// ���o 1 �i Video Frame
	mPXCSenseManager->AcquireFrame();
	return mPXCSenseManager->QuerySample();  // �^�Ǩ��o�� Video Frame (Sample �榡)
}


void RealSense::PXCImg2OpenCVImg(PXCImage *mPXCImg, cv::Mat &mOpenCVImg, VIDEO_STREAM_TYPE mVideoStreamType)
{
	PXCImage::ImageData data;
	int width = mPXCImg->QueryInfo().width;
	int height = mPXCImg->QueryInfo().height;
	int mOpenCVImgType;


	if (mVideoStreamType == STREAM_COLOR)
	{
		mPXCImg->AcquireAccess(PXCImage::ACCESS_READ, PXCImage::PIXEL_FORMAT_RGB24, &data);
		mOpenCVImgType = CV_8UC3;
	}
	else if (mVideoStreamType == STREAM_DEPTH)
	{
		mPXCImg->AcquireAccess(PXCImage::ACCESS_READ, PXCImage::PIXEL_FORMAT_DEPTH, &data);
		mOpenCVImgType = CV_16UC1;
	}


	mOpenCVImg = cv::Mat(cv::Size(width, height), mOpenCVImgType, data.planes[0]);


	mPXCImg->ReleaseAccess(&data);
}


PXCImage* RealSense::OpenCVImg2PXCImg(cv::Mat mOpenCVImg, VIDEO_STREAM_TYPE mVideoStreamType)
{
	PXCImage *mPXCImg = NULL;
	PXCImage::ImageData data;
	int width = mOpenCVImg.cols;
	int height = mOpenCVImg.rows;


	PXCImage::ImageInfo mImgInfo;
	memset(&mImgInfo, 0, sizeof(mImgInfo));
	mImgInfo.width = width;
	mImgInfo.height = height;


	startVideoStream(COLOR_DEPTH_MODE);
	if (mVideoStreamType == STREAM_COLOR)
	{
		uchar *colorValues = new uchar[width*height * 3];
		for (int i = 0; i < mOpenCVImg.rows; i++)
		{
			for (int j = 0; j < mOpenCVImg.cols; j++)
			{
				int index = mOpenCVImg.cols*i + j;
				colorValues[index + 0] = mOpenCVImg.at<cv::Vec3b>(i, j)[2];
				colorValues[index + 1] = mOpenCVImg.at<cv::Vec3b>(i, j)[1];
				colorValues[index + 2] = mOpenCVImg.at<cv::Vec3b>(i, j)[0];
			}
		}


		mImgInfo.format = PXCImage::PIXEL_FORMAT_RGB24;
		mPXCImg = mPXCSenseManager->QuerySession()->CreateImage(&mImgInfo);


		mPXCImg->AcquireAccess(PXCImage::ACCESS_WRITE, PXCImage::PIXEL_FORMAT_RGB24, &data);
		memcpy(data.planes[0], colorValues, sizeof(uchar)*width*height * 3);
		delete[] colorValues;
	}
	else if (mVideoStreamType == STREAM_DEPTH)
	{
		ushort *depthValues = new ushort[width*height];
		for (int i = 0; i < mOpenCVImg.rows; i++)
		{
			for (int j = 0; j < mOpenCVImg.cols; j++)
			{
				int index = mOpenCVImg.cols*i + j;
				depthValues[index] = mOpenCVImg.at<ushort>(i, j);
			}
		}


		mImgInfo.format = PXCImage::PIXEL_FORMAT_DEPTH;
		mPXCImg = mPXCSenseManager->QuerySession()->CreateImage(&mImgInfo);


		mPXCImg->AcquireAccess(PXCImage::ACCESS_WRITE, PXCImage::PIXEL_FORMAT_DEPTH, &data);
		memcpy(data.planes[0], depthValues, sizeof(ushort)*width*height);
		delete[] depthValues;
	}


	mPXCImg->ReleaseAccess(&data);
	stopVideoStream();
	return mPXCImg;
}


void RealSense::getColoredDepthImg2Show(cv::Mat mDepthImg, cv::Mat &mColoredDepthImg)
{
	cv::Mat mColoredDepthImgTemp = cv::Mat(mDepthImg.rows, mDepthImg.cols, CV_8UC3);
	std::vector<float> histograms;


	calculateHistogram(mDepthImg, depthValMax, histograms);


	for (int i = 0; i < mDepthImg.rows; i++)
	{
		for (int j = 0; j < mDepthImg.cols; j++)
		{
			mColoredDepthImgTemp.at<cv::Vec3b>(i, j)[2] = histograms[mDepthImg.at<ushort>(i, j)];
			mColoredDepthImgTemp.at<cv::Vec3b>(i, j)[1] = histograms[mDepthImg.at<ushort>(i, j)];
			mColoredDepthImgTemp.at<cv::Vec3b>(i, j)[0] = 0;
		}
	}


	mColoredDepthImg = mColoredDepthImgTemp;
}


void RealSense::calculateHistogram(cv::Mat mDepthImg, int histogramSize, std::vector<float> &histograms)
{
	unsigned int numPts = 0;
	histograms.resize(histogramSize, 0.0f);


	for (int i = 0, idx = 0; i < mDepthImg.rows; i++)
	{
		for (int j = 0; j < mDepthImg.cols; j++, idx++)
		{
			if (mDepthImg.at<ushort>(i, j) != 0)
			{
				histograms[mDepthImg.at<ushort>(i, j)]++;
				numPts++;
			}
		}
	}


	for (int idx = 1; idx < histogramSize; idx++)
		histograms[idx] += histograms[idx - 1];


	if (numPts)
		for (int idx = 1; idx < histogramSize; idx++)
			histograms[idx] = (256.0f*(1.0f - (histograms[idx] / (float)numPts)));
}


void RealSense::getGrayedDepthImg2Show(cv::Mat mDepthImg, cv::Mat &mGrayedDepthImg2Show)
{
	cv::Mat mColoredDepthImg_temp = cv::Mat(mDepthImg.rows, mDepthImg.cols, CV_8UC1);


	for (int i = 0; i < mDepthImg.rows; i++)
		for (int j = 0; j < mDepthImg.cols; j++)
			mColoredDepthImg_temp.at<uchar>(i, j) = (uchar)((float)mDepthImg.at<ushort>(i, j)* (255.0f / (float)depthValMax));


	mGrayedDepthImg2Show = mColoredDepthImg_temp;
}


bool RealSense::captureImgs(int imgCapTimes, VIDEO_STREAM_MODE mVideoStreamMode/* = COLOR_DEPTH_MODE*/)
{
	if (mVideoStreamMode == COLOR_MODE || mVideoStreamMode == COLOR_DEPTH_MODE)
	{
		for (int i = 0; i < mAllTheTimesPXCColorImgs.size(); i++)
			mAllTheTimesPXCColorImgs[i]->Release();
		mAllTheTimesPXCColorImgs.clear();
	}

	if (mVideoStreamMode == DEPTH_MODE || mVideoStreamMode == COLOR_DEPTH_MODE)
	{
		for (int i = 0; i < mAllTheTimesPXCDepthImgs.size(); i++)
			mAllTheTimesPXCDepthImgs[i]->Release();
		mAllTheTimesPXCDepthImgs.clear();
	}


	// �}�� Video Stream
	startVideoStream(mVideoStreamMode);


	// if open color video stream, delay for color camera adjusting exposure automatically.
	if (mVideoStreamMode == COLOR_MODE || mVideoStreamMode == COLOR_DEPTH_MODE)
		Sleep(350);


	// ���� (imgCapTimes ��)
	for (int i = 0; i < imgCapTimes; i++)
	{
		std::cout << "Capture the " << i << "th Image." << std::endl;
		PXCCapture::Sample *sample;


		// ���o 1 �i Video Frame (�]�A: 1 �i PXCImage �榡�� Color Image �M 1 �i PXCImage �榡�� Depth Image)
		sample = getVideoFrame();
		if (sample == NULL ||
				((mVideoStreamMode == COLOR_MODE || mVideoStreamMode == COLOR_DEPTH_MODE) && !sample->color) ||
				((mVideoStreamMode == DEPTH_MODE || mVideoStreamMode == COLOR_DEPTH_MODE) && !sample->depth))
		{
			i--;
			continue;
		}
		else
		{
			if (mVideoStreamMode == COLOR_MODE || mVideoStreamMode == COLOR_DEPTH_MODE)
				mAllTheTimesPXCColorImgs.push_back(getPXCImgColorCopy(sample->color));


			if (mVideoStreamMode == DEPTH_MODE || mVideoStreamMode == COLOR_DEPTH_MODE)
				mAllTheTimesPXCDepthImgs.push_back(getPXCImgDepthCopy(sample->depth));
		}


		mPXCSenseManager->ReleaseFrame();
	}


	// ���� Video Stream
	stopVideoStream();
	return true;
}


PXCImage* RealSense::getPXCImgColorCopy(PXCImage *mPXCImgColor)
{
	// �]�w PXCImage �����T(Memory, width, height, format)
	PXCImage::ImageInfo iinfo;
	memset(&iinfo, 0, sizeof(iinfo));
	iinfo.width = imgWidth;
	iinfo.height = imgHeight;
	iinfo.format = PXCImage::PIXEL_FORMAT_RGB32;


	// �Ы� 1 �� PXCImage
	PXCImage *mPXCImgColorCopy = mPXCSenseManager->QuerySession()->CreateImage(&iinfo);


	// �ƻs PXCImage
	mPXCImgColorCopy->CopyImage(mPXCImgColor);


	return mPXCImgColorCopy;  // �^�� PXCImageCopy
}


PXCImage* RealSense::getPXCImgDepthCopy(PXCImage *mPXCImgDepth)
{
	// �]�w PXCImage �����T(Memory, width, height, format)
	PXCImage::ImageInfo iinfo;
	memset(&iinfo, 0, sizeof(iinfo));
	iinfo.width = imgWidth;
	iinfo.height = imgHeight;
	iinfo.format = PXCImage::PIXEL_FORMAT_DEPTH;


	// �Ы� 1 �� PXCImage
	PXCImage *mPXCImgDepthCopy = mPXCSenseManager->QuerySession()->CreateImage(&iinfo);


	// �ƻs PXCImage
	mPXCImgDepthCopy->CopyImage(mPXCImgDepth);


	return mPXCImgDepthCopy;  // �^�� PXCImageCopy
}




void RealSense::getColorImgColorAligned3DCoords(cv::Mat &mColorImg, std::vector<POINT3D> &mColorAligned3DCoords)
{
	std::vector<cv::Mat> mAllTheTimesCVDepthImgs;
	cv::Mat mMedianCVDepthImg;
	PXCImage *mDepthImg;
	cv::Mat mColorAlignedDepthImg;


	PXCImg2OpenCVImg(mAllTheTimesPXCColorImgs.front(), mColorImg, STREAM_COLOR);
	getAllTheTimesCVDepthImgs(mAllTheTimesCVDepthImgs);
	getMedianDepthImg(mAllTheTimesCVDepthImgs, mMedianCVDepthImg);
	mDepthImg = OpenCVImg2PXCImg(mMedianCVDepthImg, STREAM_DEPTH);
	convertPXCDepthImg2ColorAlignedDepthImg(mDepthImg, mColorAlignedDepthImg);
	convertColorAlignedDepthImg2ColorAligned3DCoords(mColorAlignedDepthImg, mColorAligned3DCoords);
	convertCoordSysLeft2Right(mColorAligned3DCoords);  // �N Color-Aligend 3D Coordinates �� �����y�Шt -> �k���y�Шt
}


void RealSense::getAllTheTimesCVDepthImgs(std::vector<cv::Mat> &mAllTheTimesCVDepthImgs)
{
	mAllTheTimesCVDepthImgs.clear();


	for (int i = 0; i < mAllTheTimesPXCDepthImgs.size(); i++)
	{
		cv::Mat mCVDepthImg;
		PXCImg2OpenCVImg(mAllTheTimesPXCDepthImgs[i], mCVDepthImg, STREAM_DEPTH);
		mAllTheTimesCVDepthImgs.push_back(mCVDepthImg);
	}
}


void RealSense::getMedianDepthImg(std::vector<cv::Mat> mAllTheTimesCVDepthImgs, cv::Mat &mMedianCVDepthImg)
{
	mMedianCVDepthImg = cv::Mat(mAllTheTimesCVDepthImgs.front().rows, mAllTheTimesCVDepthImgs.front().cols, CV_16UC1);


	// �� Depth Image �W���C���I��������
	for (int i = 0; i < mAllTheTimesCVDepthImgs.front().rows; i++)
	{
		for (int j = 0; j < mAllTheTimesCVDepthImgs.front().cols; j++)
		{
			std::vector<DepthElement> mDepthElements;


			// �ǳƥΨӨ������ƪ� Array
			for (int k = 0; k < mAllTheTimesCVDepthImgs.size(); k++)
			{
				if (mAllTheTimesCVDepthImgs[k].at<ushort>(i, j) > 0.0f)  // Depth Value �n > 0.0f �~��
				{
					DepthElement mDepthElement = DepthElement(k, mAllTheTimesCVDepthImgs[k].at<ushort>(i, j));
					mDepthElements.push_back(mDepthElement);
				}
			}


			if (mDepthElements.size() == 0)
			{
				mMedianCVDepthImg.at<ushort>(i, j) = 0;
				mMedianCVDepthImg.at<ushort>(i, j) = 0;
				mMedianCVDepthImg.at<ushort>(i, j) = 0;
			}
			else
			{
				DepthElement mMedianDepthElement;
				getMedianUShortElement(mDepthElements, mMedianDepthElement);  // �������� UShortElement
				mMedianCVDepthImg.at<ushort>(i, j) = mMedianDepthElement.depthVal;
			}
		}
	}
}


void RealSense::getMedianUShortElement(std::vector<DepthElement> mDepthElements, DepthElement &mMedianDepthElement)
{
	mMedianDepthElement = DepthElement();
	std::qsort(&mDepthElements.front(), mDepthElements.size(), sizeof(DepthElement), compareDepthElement);  // Quick Sort �Ƨ�( Value �� �p -> �j)


	// ��������
	if (mDepthElements.size() % 2 == 1)  // �������Ƭ��_�Ʈ�
	{
		int medianIndex = (mDepthElements.size() - 1) / 2;
		mMedianDepthElement = mDepthElements[medianIndex];
	}
	else  // �������Ƭ����Ʈ�
	{
		int medianIndex = (mDepthElements.size() / 2) - 1;
		ushort medianVal = (ushort)(0.5f*((float)mDepthElements[medianIndex].depthVal + (float)mDepthElements[medianIndex + 1].depthVal));

		mMedianDepthElement.index = mDepthElements[medianIndex].index;
		mMedianDepthElement.depthVal = medianVal;
	}
}


void RealSense::convertPXCDepthImg2ColorAlignedDepthImg(PXCImage* mPXCDepthImg, cv::Mat &mColorAlignedDepthImg)
{
	cv::Mat mDepthImg;
	mColorAlignedDepthImg = cv::Mat(imgHeight, imgWidth, CV_32FC3);
	std::vector<PXCPointF32> mInverseUVMap;


	PXCImg2OpenCVImg(mPXCDepthImg, mDepthImg, STREAM_DEPTH);


	// �z�L RealSense SDK �� QueryInvUVMap(), ���o Inverse UV Map
	mInverseUVMap.resize(imgWidth*imgHeight, PXCPointF32());
	mPXCProjection->QueryInvUVMap(mPXCDepthImg, &mInverseUVMap.front());


	for (int i = 0; i < mColorAlignedDepthImg.rows; i++)
	{
		for (int j = 0; j < mColorAlignedDepthImg.cols; j++)
		{
			int index = mColorAlignedDepthImg.cols*i + j;


			if (mInverseUVMap[index].x < 0.0f && mInverseUVMap[index].y < 0.0f)
			{
				mColorAlignedDepthImg.at<cv::Vec3f>(i, j)[0] = -1.0f;
				mColorAlignedDepthImg.at<cv::Vec3f>(i, j)[1] = -1.0f;
				mColorAlignedDepthImg.at<cv::Vec3f>(i, j)[2] = -1.0f;
			}
			else
			{
				float depthImg_u = mInverseUVMap[index].x*(float)imgWidth;
				float depthImg_v = mInverseUVMap[index].y*(float)imgHeight;
				mColorAlignedDepthImg.at<cv::Vec3f>(i, j)[0] = depthImg_u;
				mColorAlignedDepthImg.at<cv::Vec3f>(i, j)[1] = depthImg_v;
				mColorAlignedDepthImg.at<cv::Vec3f>(i, j)[2] = (float)getDepthValBilinearInter(mDepthImg, depthImg_u, depthImg_v);  // �z�L Bilinear Interpolation ���o�����T�� Depth Value
			}
		}
	}
}


ushort RealSense::getDepthValBilinearInter(cv::Mat mDepthImg, float u, float v)
{
	float u0 = (int)u, u1 = (int)u + 1, v0 = (int)v, v1 = (int)v + 1;
	float depthVal_u0_v0 = (float)mDepthImg.at<ushort>(v0, u0);  float depthVal_u0_v1 = (float)mDepthImg.at<ushort>(v1, u0);
	float depthVal_u1_v0 = (float)mDepthImg.at<ushort>(v0, u1);  float depthVal_u1_v1 = (float)mDepthImg.at<ushort>(v1, u1);

	float depthValue;


	if (depthVal_u0_v0 == 0 || depthVal_u0_v1 == 0 || depthVal_u1_v0 == 0 || depthVal_u1_v1 == 1)
		depthValue = depthVal_u0_v0;
	else
	{
		depthValue = (depthVal_u0_v0*(u1 - u)*(v1 - v) +
									depthVal_u0_v1*(u1 - u)*(v - v0) +
									depthVal_u1_v0*(u - u0)*(v1 - v) +
									depthVal_u1_v1*(u - u0)*(v - v0)) / ((u1 - u0)*(v1 - v0));
	}


	return (ushort)depthValue;
}


void RealSense::convertColorAlignedDepthImg2ColorAligned3DCoords(cv::Mat mColorAlignedDepthImg, std::vector<POINT3D> &mColorAligned3DCoords)
{
	mColorAligned3DCoords.clear();
	std::vector<PXCPoint3DF32> depthImgPts;


	// �ǳ� RealSense SDK �� ProjectDepthToCamera() �ݭn�� Data
	for (int i = 0; i < mColorAlignedDepthImg.rows; i++)
	{
		for (int j = 0; j < mColorAlignedDepthImg.cols; j++)
		{
			PXCPoint3DF32 depthImgPt = { (float)j, (float)i, mColorAlignedDepthImg.at<cv::Vec3f>(i, j)[2] };
			//PXCPoint3DF32 depthImgPt = { (float)i, (float)j, mColorAlignedDepthImg.at<cv::Vec3f>(i, j)[2] };
			depthImgPts.push_back(depthImgPt);
		}
	}


	// �z�L RealSense SDK �� ProjectDepthToCamera(), ���o Camera Coordinate
	PXCPoint3DF32 invalidPt3D = { -1.0f, -1.0f, -1.0f };
	std::vector<PXCPoint3DF32> m3DCoords(depthImgPts.size(), invalidPt3D);
	mPXCProjection->ProjectColorToCamera(depthImgPts.size(), &depthImgPts.front(), &m3DCoords.front());


	for (int i = 0; i < m3DCoords.size(); i++)
		mColorAligned3DCoords.push_back(POINT3D(m3DCoords[i].x, m3DCoords[i].y, m3DCoords[i].z));
}


void RealSense::convertCoordSysLeft2Right(std::vector<POINT3D> &m3DCoords)
{
	for (int i = 0; i < m3DCoords.size(); i++)
		if (m3DCoords[i].x != -1.0f || m3DCoords[i].y != -1.0f || m3DCoords[i].z != -1.0f)
			m3DCoords[i].x = -m3DCoords[i].x;
}
