#include "ZQ_DetectShapeAndSceneCut.h"
#include "ZQ_ExtractSiftSurf.h"
#include "ZQ_PlayTrackingGUI.h"
#include "ZQ_ImageMattingGUI.h"
#include "ZQ_PlayShapeAndSceneCutGUI.h"
#include "ZQ_ConvertVideoToJPEG.h"
#include "ZQ_ConvertVideoToVideo.h"
#define ZQ_LINK_OPENCV_VERSION_2413
//#define ZQ_LINK_OPENCV_STATIC
#include "ZQ_Link_OpenCV_Lib.h"

using namespace ZQ;
using namespace cv;
using namespace std;

int main_detect(int argc, const char** argv);
int main_extract_single(int argc, const char** argv);
int main_extract_sequence(int argc, const char** argv);
int main_play_tracking(int argc, const char** argv);
int main_matting(int argc, const char** argv);
int main_play_shape_and_scene_cut(int argc, const char** argv);
int main_play_shape_and_scene_cut_fold(int argc, const char** argv);
int main_convert_video_to_jpg(int argc, const char** argv);
int main_convert_video_to_video(int argc, const char** argv);

int main(int argc, const char** argv)
{
	if (argc < 2)
	{
		printf(".exe detect [opts]\n");
		printf(".exe extract_single [opts]\n");
		printf(".exe extract_sequence [opts]\n");
		printf(".exe play_tracking [opts]\n");
		printf(".exe matting [opts]\n");
		printf(".exe play_shape_and_scene_cut [opts]\n");
		printf(".exe play_shape_and_scene_cut_fold [opts]\n");
		printf(".exe convert_video_to_jpg [opts]\n");
		printf(".exe convert_video_to_video [opts]\n");
		return -1;
	}

	if (_strcmpi(argv[1], "detect") == 0)
	{
		return main_detect(argc - 1, argv + 1);
	}
	else if (_strcmpi(argv[1], "extract_single") == 0)
	{
		return main_extract_single(argc - 1, argv + 1);
	}
	else if (_strcmpi(argv[1], "extract_sequence") == 0)
	{
		return main_extract_sequence(argc - 1, argv + 1);
	}
	else if (_strcmpi(argv[1], "play_tracking") == 0)
	{
		return main_play_tracking(argc - 1, argv + 1);
	}
	else if (_strcmpi(argv[1], "matting") == 0)
	{
		return main_matting(argc - 1, argv + 1);
	}
	else if (_strcmpi(argv[1], "play_shape_and_scene_cut") == 0)
	{
		return main_play_shape_and_scene_cut(argc - 1, argv + 1);
	}
	else if (_strcmpi(argv[1], "play_shape_and_scene_cut_fold") == 0)
	{
		return main_play_shape_and_scene_cut_fold(argc - 1, argv + 1);
	}
	else if (_strcmpi(argv[1], "convert_video_to_jpg") == 0)
	{
		return main_convert_video_to_jpg(argc - 1, argv + 1);
	}
	else if (_strcmpi(argv[1], "convert_video_to_video") == 0)
	{
		return main_convert_video_to_video(argc - 1, argv + 1);
	}
	else
	{
		printf("unknown para: %s\n", argv[1]);
		return -1;
	}
	return 0;
}

int main_detect(int argc, const char** argv)
{
	if (argc != 2)
	{
		printf(".exe detect config\n");
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
	printf("done!\n");
	return 0;
}

int main_extract_single(int argc, const char** argv)
{
	if (argc < 3)
	{
		printf(".exe extract_single imagefile keyfile [opts]\n");
		return -1;
	}
	
	clock_t t1 = clock();
	const char* imagefile = argv[1];
	const char* keyfile = argv[2];
	cv::Mat image = cv::imread(imagefile, 1);
	if (image.empty())
	{
		printf("failed to load image %s\n", imagefile);
		return -1;
	}

	ZQ_ExtractSiftSurfOptions opt;
	if (!opt.HandleArgs(argc - 3, argv + 3))
		return -1;


	int num = 0, dim = 0;
	float* coords = 0;
	float* vals = 0;
	if (!ZQ_ExtractSiftSurf::ExtractSiftSurfFromImage(image, num, dim, coords, vals, opt))
	{
		printf("failed to extract keys\n");
		return -1;
	}

	if (!ZQ_VirtualAdKeyIO::SaveSiftSurfKey(keyfile, num, dim, coords, vals))
	{
		printf("failed to save keyfile %s\n", keyfile);
		if (coords) delete[]coords;
		if (vals) delete[]vals;
		return -1;
	}
	if (coords) delete[]coords;
	if (vals) delete[]vals;
	printf("done!\n");
	return 0;
}

int main_extract_sequence(int argc, const char** argv)
{
	ZQ_ExtractSiftSurfOptions opt;
	if (!opt.HandleArgs(argc - 1, argv + 1))
	{
		return -1;
	}

	if (!ZQ_ExtractSiftSurf::ExtractSiftSurfFromSequence(opt))
	{
		return -1;
	}

	printf("done!\n");
	return 0;
}


int main_play_tracking(int argc, const char** argv)
{
	if (argc != 2)
	{
		printf(".exe play_tracking config\n");
		return -1;
	}
		
	if (!ZQ_PlayTrackingGUI::LoadConfig(argv[1]))
		return -1;
	if (!ZQ_PlayTrackingGUI::Run())
		return -1;
	return 0;
}


int main_matting(int argc, const char** argv)
{
	ZQ_ImageMattingOptions opt;
	if (!opt.HandleArgs(argc - 1, argv + 1))
		return -1;

	if (!ZQ_ImageMattingGUI::Load(opt))
	{
		printf("failed to load options\n");
		return -1;
	}
	if (!ZQ_ImageMattingGUI::Run())
	{
		printf("failed to run!\n");
		return -1;
	}
	return 0;
}


int main_play_shape_and_scene_cut(int argc, const char** argv)
{
	if (argc != 3)
	{
		printf(".exe play_shape_and_scene_cut videofile config\n");
		return -1;
	}
	
	if (!ZQ_PlayShapeAndSceneCutGUI::Run(argv[1], argv[2]))
		return -1;
	return 0;
}

int main_play_shape_and_scene_cut_fold(int argc, const char** argv)
{
	if (argc != 3)
	{
		printf(".exe play_shape_and_scene_cut_fold fold config\n");
		return -1;
	}

	if (!ZQ_PlayShapeAndSceneCutGUI::Run_fold(argv[1], argv[2]))
		return -1;
	return 0;
}

int main_convert_video_to_jpg(int argc, const char** argv)
{
	
	if (argc == 4)
	{
		if (!ZQ_ConvertVideoToJPEG::Convert(argv[1], argv[2], atoi(argv[3])))
			return -1;
		return 0;
	}
	if (argc == 5)
	{
		if (!ZQ_ConvertVideoToJPEG::ConvertMultiThread(argv[1], argv[2],atoi(argv[3]),atoi(argv[4])))
			return -1;
		return 0;
	}
	else
	{
		printf(".exe convert_video_to_jpg video_file out_fold quality [ncores]\n");
		return -1;
	}
}


int main_convert_video_to_video(int argc, const char** argv)
{

	if (argc == 3)
	{
		if (!ZQ_ConvertVideoToVideo::Convert(argv[1], argv[2]))
			return -1;
		return 0;
	}
	else
	{
		printf(".exe convert_video_to_video in_file out_file\n");
		return -1;
	}
}