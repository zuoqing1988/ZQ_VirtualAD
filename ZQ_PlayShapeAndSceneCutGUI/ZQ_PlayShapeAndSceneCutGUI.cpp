#include "ZQ_PlayShapeAndSceneCutGUI.h"
#include <math.h>

using namespace ZQ;

const char* ZQ_PlayShapeAndSceneCutGUI::winName = "Play Shape and SceneCut @ Zuo Qing";

IplImage* ZQ_PlayShapeAndSceneCutGUI::background_img_ = 0;
IplImage* ZQ_PlayShapeAndSceneCutGUI::draw_img_ = 0;
ZQ_SceneCutList ZQ_PlayShapeAndSceneCutGUI::scene_cut_list;
int ZQ_PlayShapeAndSceneCutGUI::scene_id = 0;
int ZQ_PlayShapeAndSceneCutGUI::shape_id = 0;
int ZQ_PlayShapeAndSceneCutGUI::start_frame_id = 0;
int ZQ_PlayShapeAndSceneCutGUI::end_frame_id = 0;
int ZQ_PlayShapeAndSceneCutGUI::cur_frame_id = 0;
bool ZQ_PlayShapeAndSceneCutGUI::updated_flag = false;
bool ZQ_PlayShapeAndSceneCutGUI::is_paused = true;
bool ZQ_PlayShapeAndSceneCutGUI::draw_marker = true;
bool ZQ_PlayShapeAndSceneCutGUI::has_marker = false;
bool ZQ_PlayShapeAndSceneCutGUI::draw_info = true;
ZQ_SceneCut::Shape ZQ_PlayShapeAndSceneCutGUI::shape;

bool ZQ_PlayShapeAndSceneCutGUI::Run_(const char* video_file, const char* config_file)
{
	if (!scene_cut_list.ImportFile(config_file))
	{
		printf("failed to load %s\n", config_file);
		return false;
	}
	CvCapture* capture = cvCaptureFromAVI(video_file);
	//CvCapture* capture = cvCreateFileCapture(video_file);
	if (capture == 0)
	{
		printf("failed to open video: %s\n", video_file);
		return false;
	}

	scene_cut_list.SortDec_FeatNum_AreaSize();
	_remove_invalid_scene_cuts(scene_cut_list);
	if (scene_cut_list.scene_cuts.size() == 0)
	{
		printf("no valid scene cut exists\n");
		return false;
	}

	long totalFrameNumber = cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_COUNT);
	int width = cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH);
	int height = cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT);

	///////////////////////////////
	_load_first_scene_cut(capture);
	background_img_ = cvQueryFrame(capture);
	draw_img_ = cvCloneImage(background_img_);
	_draw_();

	cvNamedWindow(winName);
	cvShowImage(winName, draw_img_);
	cvSetMouseCallback(winName, _mouseHandler, 0);

	while (true)
	{
		updated_flag = false;
		int c = cvWaitKey(20);
		c = (char)c;
		if (c == 27)
		{
			break;
		}
		else if (c == 'p')
		{
			is_paused = !is_paused;
		}
		else if (c == 'l')
		{
			draw_marker = !draw_marker;
			updated_flag = true;
		}
		else if (c == 'a')
		{
			_load_previous_shape(capture);
			updated_flag = true;
		}
		else if (c == 'd')
		{
			_load_next_shape(capture);
			updated_flag = true;
		}
		else if (c == 'i')
		{
			draw_info = !draw_info;
			updated_flag = true;
		}
		else if (c == 'k')
		{
			//ZQ_DetectShapeAndSceneCutOptions opt;
			//opt.display_running_info = true;
			//ZQ_DetectShapeAndSceneCut::Remove_repeat_shape(scene_cut_list.scene_cuts[scene_id], capture, opt);
			_load_first_shape(capture);
			updated_flag = true;
		}
		else if (c == 9)//TAB
		{
			_load_next_scene_cut(capture);
			updated_flag = true;
		}

		if (!is_paused)
		{
			_load_next_frame(capture);
			updated_flag = true;
		}
		if (updated_flag)
		{
			cvCopy(background_img_, draw_img_);
			_draw_();
		}
		cvShowImage(winName, draw_img_);
	}

	cvReleaseCapture(&capture);
	cvReleaseImage(&draw_img_);
	return true;
}

void ZQ_PlayShapeAndSceneCutGUI::_remove_invalid_scene_cuts(ZQ_SceneCutList& scene_cut_list)
{
	std::vector<ZQ_SceneCut>::iterator it = scene_cut_list.scene_cuts.begin();
	for (; it != scene_cut_list.scene_cuts.end();)
	{
		int tmp_ed_fr_id = (*it).GetEndFrameID();
		int tmp_st_fr_id = (*it).GetStartFrameID();
		int tmp_fr_num = tmp_ed_fr_id - tmp_st_fr_id + 1;
		bool succ = (*it).GetShapeNum() > 0 || ((*it).IsStaticScene() && tmp_fr_num >= 10);

		if (succ)
		{
			++it;
		}
		else
		{
			it = scene_cut_list.scene_cuts.erase(it);
		}
	}
}


void ZQ_PlayShapeAndSceneCutGUI::_load_first_scene_cut(CvCapture* capture)
{
	scene_id = 0;
	start_frame_id = scene_cut_list.scene_cuts[0].GetStartFrameID();
	end_frame_id = scene_cut_list.scene_cuts[0].GetEndFrameID();
	_load_first_shape(capture);
}

void ZQ_PlayShapeAndSceneCutGUI::_load_next_scene_cut(CvCapture* capture)
{
	int scene_num = scene_cut_list.scene_cuts.size();
	scene_id++;
	if (scene_id >= scene_num)
		scene_id = 0;
	start_frame_id = scene_cut_list.scene_cuts[scene_id].GetStartFrameID();
	end_frame_id = scene_cut_list.scene_cuts[scene_id].GetEndFrameID();
	_load_first_shape(capture);
}

void ZQ_PlayShapeAndSceneCutGUI::_load_previous_scene_cut(CvCapture* capture)
{
	int scene_num = scene_cut_list.scene_cuts.size();
	scene_id--;
	if (scene_id < 0)
		scene_id = scene_num - 1;
	start_frame_id = scene_cut_list.scene_cuts[scene_id].GetStartFrameID();
	end_frame_id = scene_cut_list.scene_cuts[scene_id].GetEndFrameID();
	_load_first_shape(capture);
}


void ZQ_PlayShapeAndSceneCutGUI::_load_first_shape(CvCapture* capture)
{
	int shape_num = scene_cut_list.scene_cuts[scene_id].GetShapeNum();
	if (shape_num > 0)
	{
		shape_id = 0;
		shape = scene_cut_list.scene_cuts[scene_id].GetShape(shape_id);
		_load_cur_shape_frame(capture);
	}
	else
	{
		shape_id = -1;
		shape.frame_id = -1;
		cur_frame_id = start_frame_id - 1;
		_load_next_frame(capture);
	}
}


void ZQ_PlayShapeAndSceneCutGUI::_load_next_shape(CvCapture* capture)
{
	int shape_num = scene_cut_list.scene_cuts[scene_id].GetShapeNum();
	if (shape_num > 0)
	{
		shape_id++;
		if (shape_id >= shape_num)
			shape_id = 0;
		shape = scene_cut_list.scene_cuts[scene_id].GetShape(shape_id);
		_load_cur_shape_frame(capture);
	}
}



void ZQ_PlayShapeAndSceneCutGUI::_load_previous_shape(CvCapture* capture)
{
	int shape_num = scene_cut_list.scene_cuts[scene_id].GetShapeNum();
	if (shape_num > 0)
	{
		shape_id--;
		if (shape_id < 0)
			shape_id = shape_num - 1;
		shape = scene_cut_list.scene_cuts[scene_id].GetShape(shape_id);
		_load_cur_shape_frame(capture);
	}
}



void ZQ_PlayShapeAndSceneCutGUI::_load_cur_shape_frame(CvCapture* capture)
{
	cur_frame_id = shape.frame_id;
	cvSetCaptureProperty(capture, CV_CAP_PROP_POS_FRAMES, cur_frame_id);
	background_img_ = cvQueryFrame(capture);
	has_marker = true;
}


void ZQ_PlayShapeAndSceneCutGUI::_load_next_frame(CvCapture* capture)
{
	cur_frame_id++;
	if (cur_frame_id > end_frame_id)
		cur_frame_id = start_frame_id;
	has_marker = cur_frame_id == shape.frame_id;
	
	int cv_fr_pos = cvGetCaptureProperty(capture, CV_CAP_PROP_POS_FRAMES);
	if (cv_fr_pos != cur_frame_id)
	{
		cvSetCaptureProperty(capture, CV_CAP_PROP_POS_FRAMES, cur_frame_id);
	}
	background_img_ = cvQueryFrame(capture);
}

void ZQ_PlayShapeAndSceneCutGUI::_mouseHandler(int event, int x, int y, int flags, void* param)
{

}

void ZQ_PlayShapeAndSceneCutGUI::_draw_()
{
	if (has_marker && draw_marker)
	{
		_draw_marker(draw_img_);
	}
	if (draw_info)
	{
		_draw_info(draw_img_);
	}
}


void ZQ_PlayShapeAndSceneCutGUI::_draw_marker(IplImage* draw_img)
{
	CvScalar poly_color = cvScalar(0, 255, 0);
	int size = shape.poly.size();
	if (size >= 2)
	{
		for (int i = 0; i < size; i++)
		{
			CvPoint pt1 = cvPoint(shape.poly[i].x, shape.poly[i].y);
			CvPoint pt2 = cvPoint(shape.poly[(i + 1) % size].x, shape.poly[(i + 1) % size].y);
			cvLine(draw_img, pt1, pt2, poly_color, 2);
		}
	}
}


void ZQ_PlayShapeAndSceneCutGUI::_draw_info(IplImage* draw_img)
{
	CvFont _font;
	cvInitFont(&_font, CV_FONT_HERSHEY_COMPLEX, 1, 1);
	char text[200];
	sprintf(text, "fr: %d", cur_frame_id);
	cvPutText(draw_img, text, cvPoint(50, 50), &_font, cvScalar(0, 0, 255));
	sprintf(text, "scene_cut: [%d,%d]", start_frame_id, end_frame_id);
	cvPutText(draw_img, text, cvPoint(50, 100), &_font, cvScalar(0, 0, 255));
	sprintf(text, "scene_id: %d / %d", scene_id + 1, scene_cut_list.scene_cuts.size());
	cvPutText(draw_img, text, cvPoint(50, 150), &_font, cvScalar(0, 0, 255));
	sprintf(text, "shape_id: %d / %d", shape_id + 1, scene_cut_list.scene_cuts[scene_id].GetShapeNum());
	cvPutText(draw_img, text, cvPoint(50, 200), &_font, cvScalar(0, 0, 255));
	sprintf(text, "shape_fr: %d", shape.frame_id);
	cvPutText(draw_img, text, cvPoint(50, 250), &_font, cvScalar(0, 0, 255));
	sprintf(text, "static: %s", scene_cut_list.scene_cuts[scene_id].IsStaticScene() ? "true" : "false");
	cvPutText(draw_img, text, cvPoint(50, 300), &_font, cvScalar(0, 0, 255));
}