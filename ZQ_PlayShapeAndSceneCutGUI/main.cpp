#include "ZQ_PlayShapeAndSceneCutGUI.h"
#include <stdio.h>
#define ZQ_LINK_OPENCV_VERSION_2413
#include "ZQ_Link_OpenCV_Lib.h"

int main(int argc, const char** argv)
{
	if (argc != 3)
	{
		printf(".exe videofile config\n");
		return -1;
	}

	if (!ZQ::ZQ_PlayShapeAndSceneCutGUI::Run_(argv[1], argv[2]))
		return -1;
	return 0;
}
