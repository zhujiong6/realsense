/*******************************************************************************

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2012-2013 Intel Corporation. All Rights Reserved.

*******************************************************************************/

#include <Windows.h>
#include <WindowsX.h>
#include <vector>
#include <sstream>

#include "pxchandmodule.h"
#include "pxcsensemanager.h"
#include "pxccapture.h"
#include "pxcvideomodule.h"
#include "pxchandconfiguration.h"
#include "pxchanddata.h"
#include "pxccursorconfiguration.h"
#include "pxccursordata.h"

#include "timer.h"
#include "resource1.h"


#define CURSOR_FACTOR_X 60
#define CURSOR_FACTOR_Y_UP 120
#define CURSOR_FACTOR_Y_DOWN 40

extern volatile bool g_stop;
extern volatile bool g_activeapp;
extern bool showNormalizedSkeleton;
extern bool showExtremityPoint;
extern bool noRender;


int gestureIndex = 0;

volatile bool g_connected=false;
extern pxcCHAR g_file[1024];
extern PXCSession *g_session;

//gesture combobox 
int GetSelectedGesture(HWND hwndDlg,pxcCHAR *gestureName); 
void EnableCMBItem(HWND hwndDlg, pxcBool enable);
void SetCMBGesturePos(HWND hwndDlg); 
void ResetCMBGesture(HWND hwndDlg);
bool IsCMBGestureInit(); 
void SetIsCMBGestureInit(bool isInit);
void AddCMBItem(HWND hwndDlg, pxcCHAR *line);
void SetGestureLeftStatus(HWND hwndDlg, pxcCHAR *line);
void SetGestureRightStatus(HWND hwndDlg, pxcCHAR *line);
void SetStatus(HWND hwndDlg, pxcCHAR *line);
pxcCHAR* GetCheckedDevice(HWND);
pxcCHAR* GetCheckedModule(HWND);
PXCCapture::DeviceInfo* GetCheckedDeviceInfo(HWND hwndDlg);
PXCCapture::DeviceInfo* GetDeviceFromFile(WCHAR *file);
bool GetContourState(HWND);
bool GetLabelmapState(HWND);
bool GetDepthState(HWND);
bool GetAlertState(HWND);
void ReleaseGlobalBitmap();
void DrawBitmap(HWND,PXCImage*);
void DrawCursorBitmap(HWND hwndDlg,PXCImage::ImageInfo);

void DrawGesture(HWND hwndDlg,PXCHandData::GestureData gestureData,int bodySide);
void DrawContour(HWND hwndDlg,pxcI32 accSize, PXCPointI32* point, int blobNumber);
void SetHandsMask(PXCImage*,pxcI32);
void ClearBuffer(PXCImage::ImageInfo);
void DrawJoints(HWND hwndDlg, PXCHandData::JointData nodes[2][PXCHandData::NUMBER_OF_JOINTS],
				PXCHandData::ExtremityData extremitiesPointsNodes[2][PXCHandData::NUMBER_OF_EXTREMITIES], 
				int handId);

void DrawCursor(HWND hwndDlg, 
				const std::vector<PXCPoint3DF32> cursorPoints[2],
				const std::vector<PXCPoint3DF32> adaptivePoints[2],
				int cursorClick[2], int handId);

void UpdatePanel(HWND);
bool GetPlaybackState(HWND hwndDlg);
bool GetRecordState(HWND hwndDlg);
int GetFramesToRecord(HWND hwndDlg);
void setFramesRecordBox(HWND hwndDlg,int frameNumber);
void SetFPSStatus(HWND hwndDlg,pxcCHAR *line);

void setMaxRangeValue(float value);
 
void SetInfoBox(HWND hwndDlg, pxcCHAR *line);



//vector containing depth image - for synchronization purposes
std::vector<PXCImage*> m_depthImages;
std::vector<PXCPoint3DF32> m_cursorPoints[2] = {};
std::vector<PXCPoint3DF32> m_adaptivePoints[2] = {};
int m_cursorClick[2] = {};

PXCImage::ImageInfo info;

const int NUMBER_OF_FRAMES_TO_DELAY = 3;

PXCImage* m_cursorImage = NULL;




pxcF32 GetXPosition(pxcF32 cursorXPos)
{
	if (cursorXPos < CURSOR_FACTOR_X)
	{
		return 0.0;
	}
	if (cursorXPos >= info.width - CURSOR_FACTOR_X)
	{
		return (float)info.width - 1.0f;
	}
	return ((cursorXPos - CURSOR_FACTOR_X) / (info.width - 2 * CURSOR_FACTOR_X)) * info.width;
}

pxcF32 GetYPosition(pxcF32 cursorYPos)
{
	if (cursorYPos < CURSOR_FACTOR_Y_UP)
	{
		return 0.0;
	}
	if (cursorYPos >= info.height - CURSOR_FACTOR_Y_DOWN)
	{
		return (float)info.height - 1.0f;
	}
	return ((cursorYPos - CURSOR_FACTOR_Y_UP) / (info.height - CURSOR_FACTOR_Y_UP - CURSOR_FACTOR_Y_DOWN)) * info.height;
}


static void DisplayFPS(HWND hwndDlg,pxcI32 fps)
{
	pxcCHAR line[256];
	swprintf_s<sizeof(line)/sizeof(line[0])>(line,L"FPS=%d", fps);
	SetFPSStatus(hwndDlg,line);
}

static void releaseVectorImages()
{
	for (std::vector<PXCImage *>::iterator it = m_depthImages.begin() ; it != m_depthImages.end(); ++it)
	{
		(*it)->Release();
	}

	while(!m_depthImages.empty())
	{
		m_depthImages.pop_back();
	}
}

/* Checking if sensor device connect or not */
static bool DisplayDeviceConnection(HWND hwndDlg, bool state) {
    if (state) {
        if (!g_connected) SetStatus(hwndDlg,L"Device Reconnected");
		g_connected = true;
    } else {
        if (g_connected) SetStatus(hwndDlg, L"Device Disconnected");
		g_connected = false;
    }
    return g_connected;
}

/* Displaying Depth/Mask Images - for depth image only we use a delay of NUMBER_OF_FRAMES_TO_DELAY to sync image with tracking */
static void DisplayPicture(HWND hwndDlg, PXCImage *depth, PXCHandData *handData) {
	if(!depth) return;
	PXCImage* image = depth;
	info=image->QueryInfo();
	
	ClearBuffer(info);

	
		//Mask Image
		if (GetLabelmapState(hwndDlg)) 
		{
			std::vector<std::vector<PXCPointI32*>> points;
			std::vector<std::vector<int>> pointsAccualSize;

			pxcStatus status = PXC_STATUS_NO_ERROR;

			pxcUID handID;
			pxcI32 numOfHands = handData->QueryNumberOfHands();

			points.resize(numOfHands);
			pointsAccualSize.resize(numOfHands);

			info.format=PXCImage::PIXEL_FORMAT_Y8;
			
			PXCImage::ImageData bdata; 
			if(numOfHands>0)
			{
				for(int i=0 ;i<numOfHands;i++)
				{
					handData->QueryHandId(PXCHandData::AccessOrderType::ACCESS_ORDER_BY_ID,i,handID);
					PXCHandData::IHand* hand;
				
					if (handData->QueryHandDataById(handID,hand)==PXC_STATUS_NO_ERROR)
					{
						hand->QuerySegmentationImage(image);
					
						SetHandsMask(image,i+1);

						if(GetContourState(hwndDlg) ==true)
						{

							int numberOfContours = hand->QueryNumberOfContours();
							points[i].resize(numberOfContours);
							std::vector<int> blobPointsSize  = pointsAccualSize[i];
							pointsAccualSize[i].resize(numberOfContours);

							if(numberOfContours>0)
							{
								for(int j=0;j<numberOfContours;++j)
								{
									PXCHandData::IContour* contour;
									if(hand->QueryContour(j,contour)==pxcStatus::PXC_STATUS_NO_ERROR)
									{
										pointsAccualSize[i].at(j) = 0;
										int contourSize = contour->QuerySize();					
										points[i].at(j)= 0;
										if(contourSize>0)
										{
											points[i].at(j)= new PXCPointI32[contourSize];			
											pxcStatus results = contour->QueryPoints(contourSize,points[i].at(j));
											if(results != PXC_STATUS_NO_ERROR) continue;
											pointsAccualSize[i].at(j) = contourSize;				
										}
									}

								}
							}
						}
					}
					else
					{	
						image=g_session->CreateImage(&info);
						image->AcquireAccess(PXCImage::ACCESS_WRITE,&bdata);
						memset(bdata.planes[0],0,bdata.pitches[0]*info.height);			
					}	
				}
			}
			else
			{
				image=g_session->CreateImage(&info);
				image->AcquireAccess(PXCImage::ACCESS_WRITE,&bdata); 
				memset(bdata.planes[0],0,bdata.pitches[0]*info.height);				
			}

			DrawBitmap(hwndDlg,image);

			if (status ==pxcStatus::PXC_STATUS_NO_ERROR)
			{
				for(int i=0; i<numOfHands; i++)
				{
					std::vector<PXCPointI32*> blobPoints  = points[i];
					std::vector<int> blobPointsSize  = pointsAccualSize[i];

					for (int j = 0; j < blobPointsSize.size(); j++)
					{
						DrawContour(hwndDlg,blobPointsSize[j],blobPoints[j],i);
					}
				}
			}

			//clear counter points
			size_t pointsLen = points.size();
			for(int k=0;k<pointsLen;++k)
			{
				for(int h=0;h<points[k].size();++h)
				{
					if(points[k].at(h)!=NULL)
					{
						delete [] points[k].at(h);
						points[k].at(h)=NULL;
					}
				}
				points[k].clear();
			}
		}

		//Depth image
		else
		{	
			//collecting 3 images inside a vector and displaying the oldest image
			PXCImage::ImageInfo iinfo;
			memset(&iinfo,0,sizeof(iinfo));
			iinfo=image->QueryInfo();	
			PXCImage *clonedImage;
			clonedImage=g_session->CreateImage(&iinfo);
			clonedImage->CopyImage(image);
			m_depthImages.push_back(clonedImage);
			if(m_depthImages.size() == NUMBER_OF_FRAMES_TO_DELAY)
			{
				DrawBitmap(hwndDlg,m_depthImages.front());			
				m_depthImages.front()->Release();
				m_depthImages.erase(m_depthImages.begin());				
			}
		}
	
}

static void DisplayCursorPicture(HWND hwndDlg, PXCImage *depth, PXCCursorData *cursorData) {
	if(!depth) return;
	PXCImage* image = depth;
	info=image->QueryInfo();

	ClearBuffer(info);
	DrawCursorBitmap(hwndDlg,info);	

}


static std::string ConvertWStringToString( const std::wstring& str )
{
	size_t numConverted, finalCount;

	// what size of buffer (in bytes) do we need to allocate for conversion?
	wcstombs_s(&numConverted, NULL, 0, str.c_str(), 1024);
	numConverted+=2; // for null termination
	char *pBuffer = new char[numConverted];

	// do the actual conversion
	wcstombs_s(&finalCount, pBuffer, numConverted, str.c_str(), 1024);
	std::string strValue = std::string(pBuffer);
	delete[] pBuffer;
	return strValue;


}

void string2wchar_t(wchar_t* wchar,const std::string &str)
{
	int index = 0;
	while(index < (int)str.size())
	{
		wchar[index] = (wchar_t)str[index];
		++index;
	}
	wchar[index] = 0;	
}


/* Displaying current frame hand gestures */
static void DisplayGesture(HWND hwndDlg, PXCHandData *handAnalysis,int frameNumber) {

	static pxcCHAR arr[1000];

	int numOfGesture = handAnalysis->QueryFiredGesturesNumber();
	
	if( numOfGesture > 0 )
	{
		std::ostringstream s;
		s << "Frame " << frameNumber <<") " ;
		std::string gestureStatus(s.str());
	
		//Iterate fired gestures
		for(int i = 0; i < numOfGesture ; i++)
		{
			//Get fired gesture data
			PXCHandData::GestureData gestureData;
			if(handAnalysis->QueryFiredGestureData(i,gestureData) == PXC_STATUS_NO_ERROR)
			{
				//Get hand data related to fired gesture
				PXCHandData::IHand* handData;
				if(handAnalysis->QueryHandDataById(gestureData.handId,handData) == PXC_STATUS_NO_ERROR)
				{
					std::wstring str(gestureData.name);	

					if(handData->QueryBodySide() == PXCHandData::BodySideType::BODY_SIDE_LEFT)
					{
						gestureStatus += ",Left Hand Gesture: ";
						gestureStatus += std::string(str.begin(),str.end());
						gestureStatus +="\n";
						string2wchar_t(arr,gestureStatus);	
					}
					else if(handData->QueryBodySide() == PXCHandData::BodySideType::BODY_SIDE_RIGHT)
					{
						gestureStatus += "  Right Hand Gesture: ";
						gestureStatus += std::string(str.begin(),str.end());
						gestureStatus +="\n";
						string2wchar_t(arr,gestureStatus);	
					}
				}
			}
		}
		SetInfoBox(hwndDlg,arr);
	}
	
}

/* Displaying current frame cursor gestures */
static void DisplayCursorGestures(HWND hwndDlg, PXCCursorData *cursorAnalysis,int frameNumber) {

	static pxcCHAR arr[1000];

	int numOfGesture = cursorAnalysis->QueryFiredGesturesNumber();

	if(numOfGesture>0)
	{
		std::ostringstream s;
		s << "Frame " << frameNumber <<") " ;
		std::string gestureStatus(s.str());
		
		//Iterate fired gestures
		for(int i = 0; i < numOfGesture ; i++)
		{
			//Get fired gesture data
			PXCCursorData::GestureData gestureData;
			if(cursorAnalysis->QueryFiredGestureData(i,gestureData) == PXC_STATUS_NO_ERROR)
			{
				//Get hand data related to fired gesture
				PXCCursorData::ICursor* cursorData;
				if(cursorAnalysis->QueryCursorDataById(gestureData.handId,cursorData) == PXC_STATUS_NO_ERROR)
				{
					if(cursorData->QueryBodySide() == PXCHandData::BodySideType::BODY_SIDE_LEFT)
					{
						gestureStatus += " Left Hand Gesture: ";					
					}
					else if(cursorData->QueryBodySide() == PXCHandData::BodySideType::BODY_SIDE_RIGHT)
					{
						gestureStatus += "  Right Hand Gesture: ";						
					}

					std::string gestureName = "";
					switch (gestureData.label)
					{
					case PXCCursorData::CURSOR_CLICK:
						gestureName = "cursor_click";
						break;
					case PXCCursorData::CURSOR_CLOCKWISE_CIRCLE:
						gestureName = "cursor_clockwise_circle";
						break;
					case PXCCursorData::CURSOR_COUNTER_CLOCKWISE_CIRCLE:
						gestureName = "cursor_counter_clockwise_circle";
						break;
					case PXCCursorData::CURSOR_HAND_OPENING:
						gestureName = "cursor_hand_opening";
						break;
					case PXCCursorData::CURSOR_HAND_CLOSING:
						gestureName = "cursor_hand_closing";
						break;
					default:
						break;
					}

					

					gestureStatus += gestureName;
					gestureStatus +="\n";
					string2wchar_t(arr,gestureStatus);	
				}
			}
		}
		SetInfoBox(hwndDlg,arr);
	}

}


/* Displaying current frames hand joints */
static void DisplayJoints(HWND hwndDlg, PXCHandData *handAnalyzer,pxcI64 timeStamp = 0) {
	
	PXCHandData::JointData nodes[2][PXCHandData::NUMBER_OF_JOINTS]={};
	PXCHandData::ExtremityData extremitiesPointsNodes[2][PXCHandData::NUMBER_OF_EXTREMITIES]={};



	//Iterate hands

	pxcI32 numOfHands = handAnalyzer->QueryNumberOfHands();


	for(pxcI32 i = 0 ; i < numOfHands ; i++)
	{
		//Get hand by time of appearance
		PXCHandData::IHand* handData;
		if(handAnalyzer->QueryHandData(PXCHandData::AccessOrderType::ACCESS_ORDER_BY_TIME,i,handData) == PXC_STATUS_NO_ERROR)
		{
			PXCHandData::JointData jointData;

			//Iterate Joints
			for(int j = 0; j < PXCHandData::NUMBER_OF_JOINTS ; j++)
			{
				if(showNormalizedSkeleton == false)
				{
					handData->QueryTrackedJoint((PXCHandData::JointType)j,jointData);
				}
				else
				{
					handData->QueryNormalizedJoint((PXCHandData::JointType)j,jointData);
				}
				nodes[i][j] = jointData;
			}

			if(showExtremityPoint == true)
			{
				for(int j = 0; j < PXCHandData::NUMBER_OF_EXTREMITIES ; j++)
				{
					handData->QueryExtremityPoint((PXCHandData::ExtremityType)j,extremitiesPointsNodes[i][j]);					
				}
			}
		}	

		
	}
	DrawJoints(hwndDlg,nodes,extremitiesPointsNodes, 0);
}

/* Displaying current frames cursor point */
static void DisplayCursorPoint(HWND hwndDlg, PXCCursorData *cursorAnalysis,pxcI64 timeStamp = 0) {
	
	m_cursorClick[0] = max(0, m_cursorClick[0] - 1);
	m_cursorClick[1] = max(0, m_cursorClick[1] - 1);

	//Iterate hands

	pxcI32 numOfHands = cursorAnalysis->QueryNumberOfCursors();
	if(numOfHands==0)
	{
		m_cursorPoints[0].clear();
		m_cursorPoints[1].clear();
		m_adaptivePoints[0].clear();
		m_adaptivePoints[1].clear();
	}

	if(numOfHands==1)
	{
		m_cursorPoints[1].clear();
		m_adaptivePoints[1].clear();
	}

	for(pxcI32 i = 0 ; i < numOfHands ; i++)
	{
		//Get hand by time of appearance
		PXCCursorData::ICursor* cursorData;
		if(cursorAnalysis->QueryCursorData(PXCCursorData::AccessOrderType::ACCESS_ORDER_NEAR_TO_FAR,i,cursorData) == PXC_STATUS_NO_ERROR)
		{
			// cursor point
			PXCPoint3DF32 imagePoint = cursorData->QueryCursorImagePoint();
			imagePoint.x = GetXPosition(imagePoint.x);
			imagePoint.y = GetYPosition(imagePoint.y);

			if (m_cursorPoints[i].size() > 50)
				m_cursorPoints[i].erase(m_cursorPoints[i].begin());
			m_cursorPoints[i].push_back(imagePoint);

			// adaptive point
			PXCPoint3DF32 adaptivePoint = cursorData->QueryAdaptivePoint();
			adaptivePoint.x *= 640;
			adaptivePoint.y = adaptivePoint.y * 480;

			adaptivePoint.x = GetXPosition(adaptivePoint.x);
			adaptivePoint.y = GetYPosition(adaptivePoint.y);

			if (m_adaptivePoints[i].size() > 50)
				m_adaptivePoints[i].erase(m_adaptivePoints[i].begin());
			m_adaptivePoints[i].push_back(adaptivePoint);

			// click gesture
			PXCCursorData::GestureData gestureData;
			if (cursorAnalysis->IsGestureFiredByHand(PXCCursorData::CURSOR_CLICK, cursorData->QueryUniqueId(), gestureData))
			{
				m_cursorClick[i] = 7;
			}
		}	
	}
	DrawCursor(hwndDlg,m_cursorPoints, m_adaptivePoints, m_cursorClick, 0);
}


/* Display current frames hands alerts */
void DisplayAlerts(HWND hwndDlg, PXCHandData *handAnalyzer, int frameNumber) {
	
	static pxcCHAR arr[1000];
	//iterate Alerts 
	PXCHandData::AlertData alertData;
	bool isFired = false;

	std::ostringstream s;
	s << "Frame " << frameNumber <<")  Alert: " ;
	std::string sAlerts(s.str());

	for(int i = 0 ; i <handAnalyzer->QueryFiredAlertsNumber()  ; ++i)
	{
		pxcStatus sts = handAnalyzer->QueryFiredAlertData(i,alertData);
		if(sts==PXC_STATUS_NO_ERROR)
		{
			//Displaying last alert - see AlertData::AlertType for all available alerts
			switch(alertData.label)
			{
			case PXCHandData::ALERT_HAND_DETECTED:
				{
					
					sAlerts+="Hand Detected, ";
					isFired = true;
					break;
				}
			case PXCHandData::ALERT_HAND_NOT_DETECTED:
				{
					
					sAlerts+="Hand Not Detected, ";
					isFired = true;
					break;
				}
			case PXCHandData::ALERT_HAND_CALIBRATED:
				{
					
					sAlerts+="Hand Calibrated, ";
					isFired = true;
					break;
				}
			case PXCHandData::ALERT_HAND_NOT_CALIBRATED:
				{
					
					sAlerts+="Hand Not Calibrated, ";
					isFired = true;
					break;
				}
			case PXCHandData::ALERT_HAND_INSIDE_BORDERS:
				{
					
					sAlerts+="Hand Inside Borders, ";
					isFired = true;
					break;
				}
			case PXCHandData::ALERT_HAND_OUT_OF_BORDERS:
				{
					
					sAlerts+="Hand Out Of Borders, ";
					isFired = true;
					break;
				}
			/*case PXCHandData::ALERT_CURSOR_DETECTED:
				{

					sAlerts+="Cursor detected, ";
					isFired = true;
					break;
				}
			case PXCHandData::ALERT_CURSOR_NOT_DETECTED:
				{

					sAlerts+="Cursor not detected, ";
					isFired = true;
					break;
				}
			case PXCHandData::ALERT_CURSOR_INSIDE_BORDERS:
				{

					sAlerts+="Cursor inside Borders, ";
					isFired = true;
					break;
				}
			case PXCHandData::ALERT_CURSOR_OUT_OF_BORDERS:
				{

					sAlerts+="Cursor Out Of Borders, ";
					isFired = true;
					break;
				}*/

			}
		}
	}
	if(isFired == true)
	{
		sAlerts+="\n";
		string2wchar_t(arr,sAlerts);
		SetInfoBox(hwndDlg,arr);
	}
	
}

/* Display current frames cursor alerts */
void DisplayCursorAlerts(HWND hwndDlg, PXCCursorData *cursorAnalyzer, int frameNumber) {
	
	static pxcCHAR arr[1000];
	//iterate Alerts 
	PXCCursorData::AlertData alertData;
	bool isFired = false;

	std::ostringstream s;
	s << "Frame " << frameNumber <<")  Alert: " ;
	std::string sAlerts(s.str());

	for(int i = 0 ; i <cursorAnalyzer->QueryFiredAlertsNumber()  ; ++i)
	{
		pxcStatus sts = cursorAnalyzer->QueryFiredAlertData(i,alertData);
		if(sts==PXC_STATUS_NO_ERROR)
		{
			//Displaying last alert - see AlertData::AlertType for all available alerts
			switch(alertData.label)
			{
			 case PXCCursorData::CURSOR_DETECTED:
				{

					sAlerts+="Cursor detected, ";
					isFired = true;
					break;
				}
			case PXCCursorData::CURSOR_NOT_DETECTED:
				{

					sAlerts+="Cursor not detected, ";
					isFired = true;
					break;
				}
			case PXCCursorData::CURSOR_INSIDE_BORDERS:
				{

					sAlerts+="Cursor inside Borders, ";
					isFired = true;
					break;
				}
			case PXCCursorData::CURSOR_OUT_OF_BORDERS:
				{

					sAlerts+="Cursor Out Of Borders, ";
					isFired = true;
					break;
				}

			}
		}
	}
	if(isFired == true)
	{
		sAlerts+="\n";
		string2wchar_t(arr,sAlerts);
		SetInfoBox(hwndDlg,arr);
	}
	
}


/* Using PXCSenseManager to handle data */
void SimplePipeline(HWND hwndDlg) {

	gestureIndex = 0;
	SetInfoBox(hwndDlg,NULL);
	bool liveCamera = false;

	PXCSenseManager *pp = g_session->CreateSenseManager();
	if(!pp)
	{
		SetStatus(hwndDlg,L"Failed to create SenseManager");
		return;
	}

	PXCCapture::DeviceInfo* checkedDeviceInfo = GetCheckedDeviceInfo(hwndDlg);

	/* Set Mode & Source */
	if (GetRecordState(hwndDlg)) {
		pp->QueryCaptureManager()->SetFileName(g_file,true);
		pp->QueryCaptureManager()->FilterByDeviceInfo(GetCheckedDevice(hwndDlg),0,0);
	} else if (GetPlaybackState(hwndDlg)) {
		checkedDeviceInfo = GetDeviceFromFile(g_file);
		pp->QueryCaptureManager()->SetFileName(g_file,false);
	} else {
		pp->QueryCaptureManager()->FilterByDeviceInfo(GetCheckedDevice(hwndDlg),0,0);
		liveCamera = true;
	}

	if(!checkedDeviceInfo)
	{
		SetStatus(hwndDlg,L"Device Failure");	
		return;
	}

	bool sts=true;
	/* Set Module */
    
	PXCHandModule *handAnalyzer = NULL;
	PXCHandCursorModule *handCursorAnalyzer = NULL;
	

	if (Button_GetState(GetDlgItem(hwndDlg,IDC_CURSOR_MODE))&BST_CHECKED)
	{
		if(checkedDeviceInfo->model != PXCCapture::DEVICE_MODEL_SR300)
		{
			if(GetPlaybackState(hwndDlg))
			{	
				SetStatus(hwndDlg,L"Cursor mode is unsupported for chosen playback file");
			}
			else
			{
				SetStatus(hwndDlg,L"Cursor mode is unsupported for chosen device");
			}
		
			return;
		}

		pxcStatus status = pp->EnableHandCursor(0);
	
		if(status != pxcStatus::PXC_STATUS_NO_ERROR)
		{
			SetStatus(hwndDlg,L"Failed to pair the hand cursor module with I/O");
			return;
		}		
		handCursorAnalyzer=pp->QueryHandCursor();
		if(handCursorAnalyzer == NULL)
		{
			SetStatus(hwndDlg,L"Failed to pair the hand cursor module with I/O");
			return;
		}

	
	}else
	{
		pxcStatus status = pp->EnableHand(0);
		
		if(status != pxcStatus::PXC_STATUS_NO_ERROR)
		{
			SetStatus(hwndDlg,L"Failed to pair the hand module with I/O");
			return;
		}		
		handAnalyzer=pp->QueryHand();
		if(handAnalyzer == NULL)
		{
			SetStatus(hwndDlg,L"Failed to pair the hand module with I/O");
			return;
		}
	}

	
	/* Init */
	FPSTimer timer;
	SetStatus(hwndDlg,L"Init Started");
	if (pp->Init() >= PXC_STATUS_NO_ERROR) 
	{

		// Mirror mode
		PXCCapture::Device *device=pp->QueryCaptureManager()->QueryDevice();

		setMaxRangeValue(device->QueryDepthSensorRange().max);

		PXCCursorData* outputCursorData = NULL;
		PXCCursorConfiguration* configCursor = NULL;
		PXCHandData* outputData = NULL;
		PXCHandConfiguration* config = NULL;


		if (Button_GetState(GetDlgItem(hwndDlg,IDC_CURSOR_MODE))&BST_CHECKED)
		{
			if(handCursorAnalyzer)
			{
				outputCursorData = handCursorAnalyzer->CreateOutput();
				configCursor = handCursorAnalyzer->CreateActiveConfiguration();
			} 
			else
			{
				return;
			}
			if(configCursor)
			{
				configCursor->EnableAllAlerts();
				configCursor->ApplyChanges();
			} else
				return;

			if(IsCMBGestureInit()==false)
			{		
				SetCMBGesturePos(hwndDlg);
			}

			AddCMBItem(hwndDlg,L"cursor_click");						
			AddCMBItem(hwndDlg,L"cursor_clockwise_circle");	
			AddCMBItem(hwndDlg,L"cursor_counterclockwise_circle");	
			AddCMBItem(hwndDlg,L"cursor_hand_opening");	
			AddCMBItem(hwndDlg,L"cursor_hand_closing");	
			EnableCMBItem(hwndDlg,true);


		}
		else
		{
			if(handAnalyzer)
			{
				outputData = handAnalyzer->CreateOutput();
				config = handAnalyzer->CreateActiveConfiguration();	
				config->EnableNormalizedJoints(showNormalizedSkeleton);
			}
			else
			{
				return;
			}

			PXCHandData::TrackingModeType trackingMode = PXCHandData::TRACKING_MODE_FULL_HAND;
			if (Button_GetState(GetDlgItem(hwndDlg,IDC_FULLHAND_MODE))&BST_CHECKED)
				trackingMode =  PXCHandData::TRACKING_MODE_FULL_HAND;

			config->SetTrackingMode(trackingMode);
			config->EnableAllAlerts();
			config->EnableSegmentationImage(true);		
			config->ApplyChanges();


			pxcI32 totalNumOfGestures = config->QueryGesturesTotalNumber();		
			if (totalNumOfGestures > 0)
			{
				if(IsCMBGestureInit()==false)
				{		
					SetCMBGesturePos(hwndDlg);
				}

				for (int i = 0; i < totalNumOfGestures; i++)
				{
					pxcCHAR* gestureName= new pxcCHAR[PXCHandData::MAX_NAME_SIZE];
					if (config->QueryGestureNameByIndex(i,PXCHandData::MAX_NAME_SIZE, gestureName) ==	pxcStatus::PXC_STATUS_NO_ERROR)
					{						
						AddCMBItem(hwndDlg,gestureName);						
					}
					delete[] gestureName;
					gestureName = NULL;
				}	
				EnableCMBItem(hwndDlg,true);
			}
		}
		
		
		
		
			
		SetStatus(hwndDlg,L"Streaming");
		g_connected=true;
        bool activeapp=true;
		int frameCounter = 0;
		int frameNumber = 0;
		pxcCHAR gestureName[50];

		while (!g_stop) 
		{

			if (Button_GetState(GetDlgItem(hwndDlg,IDC_CURSOR_MODE))&BST_CHECKED)
			{
				if(configCursor!=NULL)
				{
				int index = GetSelectedGesture(hwndDlg,gestureName);
				if(index>0 && gestureIndex!=index)
				{
					gestureIndex = index;
					
						configCursor->DisableAllGestures();	

						if(std::wcscmp(gestureName,L"cursor_click")==0)
						{
							configCursor->EnableGesture(PXCCursorData::GestureType::CURSOR_CLICK);	
						}
						if(std::wcscmp(gestureName,L"cursor_clockwise_circle")==0)
						{
							configCursor->EnableGesture(PXCCursorData::GestureType::CURSOR_CLOCKWISE_CIRCLE);	
						}
						if(std::wcscmp(gestureName,L"cursor_counterclockwise_circle")==0)
						{
							configCursor->EnableGesture(PXCCursorData::GestureType::CURSOR_COUNTER_CLOCKWISE_CIRCLE);	
						}
						if(std::wcscmp(gestureName,L"cursor_hand_opening")==0)
						{
							configCursor->EnableGesture(PXCCursorData::GestureType::CURSOR_HAND_OPENING);	
						}
						if(std::wcscmp(gestureName,L"cursor_hand_closing")==0)
						{
							configCursor->EnableGesture(PXCCursorData::GestureType::CURSOR_HAND_CLOSING);	
						}

				
								
						configCursor->ApplyChanges();
					
				}
				else if(index==0 && gestureIndex!=index)
				{
					gestureIndex = index;
					configCursor->DisableAllGestures();		
					configCursor->ApplyChanges();
					
				}
				}

			}else
			{
				if(config!=NULL)
				{
					int index = GetSelectedGesture(hwndDlg,gestureName);
					if(index>0 && gestureIndex!=index)
					{
						gestureIndex = index;
						config->DisableAllGestures();				
						config->EnableGesture(gestureName,true);				
						config->ApplyChanges();
					}else if(index==0 && gestureIndex!=index)
					{
						gestureIndex = index;
						config->DisableAllGestures();		
						config->ApplyChanges();
					}
				}
			
			}

			
			
			pxcStatus sts=pp->AcquireFrame(true);
			
			if (DisplayDeviceConnection(hwndDlg,pp->IsConnected())) 
			{
				if (sts < PXC_STATUS_NO_ERROR) 
					break;

				frameCounter++;
				const PXCCapture::Sample *sample;
				if (Button_GetState(GetDlgItem(hwndDlg,IDC_CURSOR_MODE))&BST_CHECKED)
				{
					sample = pp->QueryHandCursorSample();
				}else
				{
					sample = pp->QueryHandSample();
				}
				if (sample && sample->depth && !noRender)
				{
					frameNumber = liveCamera ? frameCounter : pp->QueryCaptureManager()->QueryFrameIndex(); 
					if (Button_GetState(GetDlgItem(hwndDlg,IDC_CURSOR_MODE))&BST_CHECKED)
					{
						if(outputCursorData)
						{						
							outputCursorData->Update();
							DisplayCursorPicture(hwndDlg,sample->depth,outputCursorData);
							DisplayCursorPoint(hwndDlg,outputCursorData);
							DisplayCursorGestures(hwndDlg,outputCursorData,frameNumber);
							DisplayCursorAlerts(hwndDlg,outputCursorData,frameNumber);
						}else
							return;
					}
					else
					{
						if(outputData)
						{
							outputData->Update();
							DisplayPicture(hwndDlg,sample->depth,outputData);
							DisplayJoints(hwndDlg,outputData);
							DisplayGesture(hwndDlg,outputData,frameNumber);
							DisplayAlerts(hwndDlg,outputData,frameNumber);
						}
						else
							return;
					}
		
					UpdatePanel(hwndDlg);   
					timer.Tick(hwndDlg);
				}
             
			}
			pp->ReleaseFrame();
		}

		if (Button_GetState(GetDlgItem(hwndDlg,IDC_CURSOR_MODE))&BST_CHECKED)
		{
			if(configCursor)
				configCursor->Release();
			if(outputCursorData)
				outputCursorData->Release();
		}
		else
		{
			if(config)
				config->Release();
			if(outputData)
				outputData->Release();
		}
		
	} 
	else 
	{
		SetStatus(hwndDlg,L"Init Failed");
		sts=false;
	}

	if(m_cursorImage)
	{
		m_cursorImage->Release();
		m_cursorImage = NULL;
	}

	releaseVectorImages();
	if(pp)
	{
		pp->Close();
		pp->Release();
	}
	if (sts) SetStatus(hwndDlg,L"Stopped");
}


