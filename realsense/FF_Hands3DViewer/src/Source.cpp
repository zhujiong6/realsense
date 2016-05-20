#include <stdio.h>
#include <iostream>

#include "RssdkHandler.h"

ModelViewController::HandsModel* handModel;
ModelViewController::IView* openGLView;
ModelViewController::HandsController* controller;
RssdkHandler* rssdkHandler;


void releaseAll();

int main(int argc, char** argv)
{
	bool isFullHand = false;
	if(argc == 2)
	{
		if (strcmp(argv[1],"-full")==0)
		{
			isFullHand = true;
		}
	}

	pxcStatus status = PXC_STATUS_ALLOC_FAILED;

	// Create hand model
	handModel = new ModelViewController::HandsModel();
	if(!handModel)
	{
		std::printf("Failed at Initialization\n");
		releaseAll();
		return -1;
	}

	// Create Openglview which implements IView (allows creations of different views)
	openGLView = new ModelViewController::OpenGLView(isFullHand);
	if(!openGLView)
	{
		std::printf("Failed at Initialization\n");
		releaseAll();
		return -1;
	}

	// When using sequence, change useSequence to true and apply sequencePath with sequence path
	bool useSequence = false;
	pxcCHAR* sequencePath = L"Insert Sequence Path Here";
	

	// Bind controller with model and view and start playing
	controller = new ModelViewController::HandsController(handModel,openGLView);
	if(!controller)
	{
		std::printf("Failed at Initialization\n");
		releaseAll();
		return -1;
	}

	rssdkHandler = new RssdkHandler(controller,handModel,openGLView);

	if(useSequence)
	{
		if(rssdkHandler->Init(isFullHand,sequencePath) == PXC_STATUS_NO_ERROR)
		{
			rssdkHandler->Start();
		}

		else
		{
			std::printf("Failed at Initialization\n");
			releaseAll();
			return -1;
		}
	}

	else
	{
		if(rssdkHandler->Init(isFullHand) == PXC_STATUS_NO_ERROR)
		{
			rssdkHandler->Start();
		}

		else
		{
			std::printf("Failed at Initialization\n");
			releaseAll();
			return -1;
		}
	}

	releaseAll();
    return 0;
}

void releaseAll()
{
	//delete all pointers
	if(handModel)
		delete handModel;
	if(openGLView)
		delete openGLView;
	if(controller)
		delete controller;
	if(rssdkHandler)
		delete rssdkHandler;	
}


