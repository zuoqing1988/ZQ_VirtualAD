#include "ZQ_DetectShapeAndSceneCut.h"
#define ZQ_LINK_OPENCV_VERSION_2413
#include "ZQ_Link_OpenCV_Lib.h"

using namespace ZQ;

int main(int argc, const char** argv)
{
	if (argc != 2)
	{
		printf(".exe config\n");
		return -1;
	}
	ZQ_DetectShapeAndSceneCutOptions opt;
	if (!opt.LoadFromFile(argv[1]))
	{
		printf("failed to load config file %s\n", argv[1]);
		return -1;
	}
	
	if (!ZQ_DetectShapeAndSceneCut::Go(opt))
	{
		printf("failed to run!\n");
		return -1;
	}
	return 0;
}

