#ifndef _ZQ_PLAY_SHAPE_AND_SCENE_CUT_GUI_H_
#define _ZQ_PLAY_SHAPE_AND_SCENE_CUT_GUI_H_
#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "ZQ_CvCapture_FFMPEG.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "ZQ_ShapeAndSceneCut.h"

namespace ZQ
{
	class ZQ_PlayShapeAndSceneCutGUI
	{
	private:
		static const char* winName;
		static cv::Mat background_img;
		static IplImage* background_img_;
		static cv::Mat draw_img;
		static IplImage* draw_img_;
		static ZQ_SceneCutList scene_cut_list;
		static int scene_id;
		static int shape_id;
		static int start_frame_id;
		static int end_frame_id;
		static int cur_frame_id;
		static bool updated_flag;
		static bool is_paused;
		static bool draw_marker;
		static bool has_marker;
		static bool draw_info;
		static ZQ_SceneCut::Shape shape;
	public:
		static bool Run(const char* video_file, const char* config_file);
		static bool Run_fold(const char* fold, const char* config_file);
		static bool Run_(const char* video_file, const char* config_file);

	private:
		static void _remove_invalid_scene_cuts(ZQ_SceneCutList& scene_cut_list);
	
		static void _load_first_scene_cut(cv::VideoCapture& capture);
		static void _load_first_scene_cut(ZQ_CvCapture_FFMPEG& capture);
		static void _load_first_scene_cut(const char* fold);
		static void _load_next_scene_cut(cv::VideoCapture& capture);
		static void _load_next_scene_cut(ZQ_CvCapture_FFMPEG& capture);
		static void _load_next_scene_cut(const char* fold);
		static void _load_previous_scene_cut(cv::VideoCapture& capture);
		static void _load_previous_scene_cut(ZQ_CvCapture_FFMPEG& capture);
		static void _load_previous_scene_cut(const char* fold);
		static void _load_first_shape(cv::VideoCapture& capture);
		static void _load_first_shape(ZQ_CvCapture_FFMPEG& capture);
		static void _load_first_shape(const char* fold);
		static void _load_next_shape(cv::VideoCapture& capture);
		static void _load_next_shape(ZQ_CvCapture_FFMPEG& capture);
		static void _load_next_shape(const char* fold);
		static void _load_previous_shape(cv::VideoCapture& capture);
		static void _load_previous_shape(ZQ_CvCapture_FFMPEG& capture);
		static void _load_previous_shape(const char* fold);
		static void _load_cur_shape_frame(cv::VideoCapture& capture);
		static void _load_cur_shape_frame(ZQ_CvCapture_FFMPEG& capture);
		static void _load_cur_shape_frame(const char* fold);
		static void _load_next_frame(cv::VideoCapture& capture);
		static void _load_next_frame(ZQ_CvCapture_FFMPEG& capture);
		static void _load_next_frame(const char* fold);

		static void _mouseHandler(int event, int x, int y, int flags, void* param);

		static void _draw();
		static void _draw_();
		static void _draw_marker(cv::Mat& draw_img);
		static void _draw_marker(IplImage* draw_img);
		static void _draw_info(cv::Mat& draw_img);
		static void _draw_info(IplImage* draw_img);

		static void _copyTo(const ZQ_Image_FFMPEG& frame, IplImage* dst);
	};
}

#endif