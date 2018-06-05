#ifndef _ZQ_PLAY_SEQUENCE_H_
#define _ZQ_PLAY_SEQUENCE_H_
#pragma once

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "ZQ_PlaySequenceOptions.h"
#include "ZQ_PlaySequenceConfig.h"
#include <vector>
#include "ZQ_Vec2D.h"
#include "ZQ_VirtualAdvertisementUtils.h"
#include "ZQ_DoubleImage.h"

namespace ZQ
{
	class ZQ_PlaySequence
	{
	private:
		static const char* winName;
		static const char* match_winName;
		static IplImage* background_img;
		static IplImage* draw_img;
		static int cur_frame_id;
		static bool updated_flag;
		static bool is_paused;
		static bool draw_marker;
		static bool has_marker;
		static double marker[8];
		static bool has_polygon;
		static std::vector<ZQ_Vec2D> poly;
		static bool draw_info;
		static ZQ_DImage<bool> poly_keyframe_flag;
		static ZQ_DImage<bool> marker_keyframe_flag;
		static ZQ_PlaySequenceConfig config;
		static char config_filename[200];

	public:
		static bool LoadConfig(const char* file);
		static bool Run();

	private:
		static void _mouseHandler(int event, int x, int y, int flags, void* param);

		static bool _load_first_frame();
		static void _load_next_keyframe();
		static void _load_previous_frame();
		static void _load_next_frame();

		static void _draw();
		static void _draw_marker(IplImage* draw_img);
		static void _draw_polygon(IplImage* draw_img);
		static void _draw_info(IplImage* draw_img);

		static void _select_polygon();
		static void _select_marker();
		static void _save_config();

		static void _propagate_marker(int key_id);
		static void _propagate_marker_after_polygon(int poly_key_id);
		static void _remove_marker_keyframe(int key_id);
		static void _propagate_marker(int start_id, int end_id);
		

		static void _propagate_polygon(int key_id);
		static void _propagate_polygon(int start_id, int end_id);

	};
}
#endif