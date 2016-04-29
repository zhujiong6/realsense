/*******************************************************************************

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2011-2014 Intel Corporation. All Rights Reserved.

*******************************************************************************/
//Thomas Tsai : d04922009@csie.ntu.edu.tw
#define	OPENCV_SUPPORTED	1	//OPENCV ONLY

#include <windows.h>
//opencv
#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
//realsense sdk
#include "pxcsensemanager.h"
#include "pxcsession.h"
#include "pxcmetadata.h"
#include "util_cmdline.h"
#include "util_render.h"

#include <iostream>
#include <conio.h>

#include "realsense2cvmat.h"

//! [namespace]
using namespace cv;
using namespace std;
//! [namespace]
#include "gui.h"

#define	MAX_REC_FRAMES	(300)

int wmain(int argc, WCHAR* argv[]) {
    /* 1. Creates an instance of the PXCSenseManager */
    PXCSenseManager *pp = PXCSenseManager::CreateInstance();
    if (!pp) {
        wprintf_s(L"Unable to create the SenseManager\n");
        return 3;
    }
	
	PXCSession::ImplVersion version = pp->QuerySession()->QueryVersion();
	std::cout << "SDK Version:" << version.major << "." << version.minor << std::endl;


    /* Collects command line arguments */
    UtilCmdLine cmdl(pp->QuerySession());
    if (!cmdl.Parse(L"-listio-nframes-sdname-csize-dsize-isize-lsize-rsize-file-record-noRender-mirror",argc,argv)) return 3;

    /* Sets file recording or playback */
    PXCCaptureManager *cm=pp->QueryCaptureManager();
    cm->SetFileName(cmdl.m_recordedFile, cmdl.m_bRecord);
    if (cmdl.m_sdname) cm->FilterByDeviceInfo(cmdl.m_sdname,0,0);

	#if	OPENCV_SUPPORTED
		PXCImage *colorIm, *depthIm, *irIm, *rightIm, *leftIm;
	#else
		// Create stream renders
		UtilRender renderc(L"Color"), renderd(L"Depth"), renderi(L"IR"), renderr(L"Right"), renderl(L"Left");
	#endif
    pxcStatus sts;
	my_gui myGui;
	myGui.frames = 0;
	myGui.imglist_size = MAX_REC_FRAMES;
	myGui.rect = cv::Rect(0,0,0,0);
	myGui.depth_win_name = "OpenCV Window Depth";
	myGui.win_name = "OpenCV Window Depth";
	namedWindow(myGui.depth_win_name, WINDOW_AUTOSIZE);
	setMouseCallback(myGui.depth_win_name, onMouse, &myGui);//setup callback
	myGui.color_win_name = "OpenCV Window Color";
	namedWindow(myGui.color_win_name, WINDOW_AUTOSIZE/*WINDOW_KEEPRATIO*/);
	
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
            PXCVideoModule::DataDesc desc={};
            if (cm->QueryCapture()) {
                cm->QueryCapture()->QueryDeviceInfo(0, &desc.deviceInfo);
            } else {
				desc.deviceInfo.streams = PXCCapture::STREAM_TYPE_COLOR | PXCCapture::STREAM_TYPE_DEPTH;
                revert = true;
            }
            pp->EnableStreams(&desc);
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
        } else {
            device->SetMirrorMode(PXCCapture::Device::MirrorMode::MIRROR_MODE_DISABLED);
        }

        /* 6. Stream Data */
        for (int nframes=0;nframes<cmdl.m_nframes;nframes++) {
            /* Waits until new frame is available and locks it for application processing */
            sts=pp->AcquireFrame(false);	//6.a capture the frame

            if (sts<PXC_STATUS_NO_ERROR) {
                if (sts==PXC_STATUS_STREAM_CONFIG_CHANGED) {
                    wprintf_s(L"Stream configuration was changed, re-initilizing\n");
                    pp->Close();
                }
                break;
            }

            /* Render streams, unless -noRender is selected */
            if (cmdl.m_bNoRender == false) {
                const PXCCapture::Sample *sample = pp->QuerySample();// 6.b get the captured frame
                if (sample) {
					#if OPENCV_SUPPORTED
						if (sample->color) {
							colorIm = sample->color;
							cv::Mat colorMat;
							ConvertPXCImageToOpenCVMat(colorIm, &colorMat, STREAM_TYPE_COLOR);
							cv::imshow(myGui.color_win_name, colorMat);
						}
						if (sample->depth) {
							depthIm = sample->depth;
							cv::Mat depthMat;
							ConvertPXCImageToOpenCVMat(depthIm, &depthMat, STREAM_TYPE_DEPTH);
							myGui.image = depthMat.clone();
							cv::imshow(myGui.depth_win_name, depthMat);
							if (myGui.action && (myGui.frames++ < myGui.imglist_size)) {
								Mat roi(depthMat, myGui.rect); // using a rectangle ROI
								myGui.depth_imgs.insert(myGui.depth_imgs.begin(), &roi.clone());
								printf("insert : %d/%d\n", myGui.frames, myGui.imglist_size);
							}
						}
						if (sample->ir) {
							irIm = sample->ir;
							cv::Mat irMat;
							ConvertPXCImageToOpenCVMat(irIm, &irMat, STREAM_TYPE_IR);
							cv::imshow("OpenCV Window ir", irMat);
						}
						if (sample->left) {
							leftIm = sample->left;
							cv::Mat leftMat;
							ConvertPXCImageToOpenCVMat(leftIm, &leftMat);
							cv::imshow("OpenCV Window left", leftMat);
						}
						if (sample->right) {
							rightIm = sample->right;
							cv::Mat rightMat;
							ConvertPXCImageToOpenCVMat(rightIm, &rightMat);
							cv::imshow("OpenCV Window right", rightMat);
						}
					#else//windows render
						if (sample->depth && !renderd.RenderFrame(sample->depth)) break;
						if (sample->color && !renderc.RenderFrame(sample->color)) break;
						if (sample->ir    && !renderi.RenderFrame(sample->ir))    break;
						if (sample->right    && !renderr.RenderFrame(sample->right))    break;
						if (sample->left    && !renderl.RenderFrame(sample->left))    break;
					#endif // OPENCV_SUPPORTED
                }
            }
            /* 7. Releases lock so pipeline can process next frame */
            pp->ReleaseFrame();
			#if OPENCV_SUPPORTED
				int c=cv::waitKey(1);
				if (c == 27 || c == 'q' || c == 'Q') break; // ESC|q|Q for Exit
			#else
				if( _kbhit() ) { // Break loop
					int c = _getch() & 255;
					if( c == 27 || c == 'q' || c == 'Q') break; // ESC|q|Q for Exit
				}
			#endif
        }
    } while (sts == PXC_STATUS_STREAM_CONFIG_CHANGED);

    wprintf_s(L"Exiting\n");

    // 8.Clean Up
    pp->Release();
    return 0;
}
