#ifndef _ZQ_SELECT_POLYGON_GUI_H_
#define _ZQ_SELECT_POLYGON_GUI_H_
#pragma once


#include "ZQ_Vec2D.h"
#include <vector>
#include <opencv/cv.h>
#include <opencv/highgui.h>

class ZQ_SelectPolygonGUI
{
public:
	enum CONST_VAL{ MAX_NUM_PTS = 50 };
	static bool Select(const IplImage* img, std::vector<ZQ::ZQ_Vec2D>& poly, int max_npts = MAX_NUM_PTS, float show_scale = 1, const char* win_name = 0);

private:
	static const char* winName;
	static float coord_x[MAX_NUM_PTS];
	static float coord_y[MAX_NUM_PTS];
	static int max_num_pts;
	static int cur_coord_num;
	static float scale;
	static IplImage* background_img;
	static IplImage* zoom_background_img;
	static IplImage* draw_img;
	static int zoom_scale;
	static bool zoom_mode;
	static int zoom_center_x;
	static int zoom_center_y;
	static int cur_mouse_pos_x;
	static int cur_mouse_pos_y;
	static bool updated_flag;


private:
	static void _mouseHandler(int event, int x, int y, int flags, void* param);

	static void _drawPolygon(IplImage* img);

	static void _draw();
};

#endif