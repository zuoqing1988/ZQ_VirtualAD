#ifndef _ZQ_IMAGE_MATTING_GUI_H_
#define _ZQ_IMAGE_MATTING_GUI_H_
#pragma once 

#define _USE_UMFPACK 1
#include "ZQ_ImageMattingOptions.h"
#include "ZQ_DoubleImage.h"
#include "ZQ_BinaryImageProcessing.h"
#include "ZQ_ClosedFormImageMatting.h"

#include "ZQ_ImageIO.h"
#include "ZQ_VirtualAdRender.h"

namespace ZQ
{
	class ZQ_ImageMattingGUI
	{
		enum EraserType{ERASER_TYPE_FORE, ERASER_TYPE_BACK, ERASER_TYPE_UNKNOWN};
	private:
		static const char* winName;
		static const char* render_winName;
		static ZQ_DImage<double> ori_image;
		static bool has_ori_trimap;
		static ZQ_DImage<double> ori_trimap;
		static ZQ_DImage<double> trimap;
		static ZQ_DImage<float> tex;
		static ZQ_DImage<float> tex_alpha;
		static ZQ_DImage<double> fore;
		static ZQ_DImage<double> back;
		static ZQ_DImage<double> alpha;
		static ZQ_ImageMattingOptions opt;
		static IplImage* background_img;
		static IplImage* zoom_background_img;
		static IplImage* draw_img;
		static int eraser_half_size;
		static int eraser_max_half_size;
		static int eraser_min_half_size;
		static int eraser_pos_x;
		static int eraser_pos_y;
		static bool eraser_has_last_pos;
		static int eraser_last_pos_x;
		static int eraser_last_pos_y;
		static bool erase_mode;
		static EraserType eraser_type;
		static bool has_marker;
		static ZQ_VirtualAdRender::MarkerMode marker_mode;
		static double marker[8];
		static bool draw_marker;
		static bool updated_flag;
		static int zoom_scale;
		static bool zoom_mode;
		static int zoom_center_x;
		static int zoom_center_y;
		static int cur_mouse_pos_x;
		static int cur_mouse_pos_y;
	public:
		static bool Load(const ZQ_ImageMattingOptions& opt);
		static bool Run();

	private:
		static void _mouseHandler(int event, int x, int y, int flags, void* param);

		static void _draw();
		static void _drawTrimap(IplImage* img);
		static void _drawErazer(IplImage* img);
		static void _drawMarker(IplImage* img);

		static void _erase();

		static void _go();
		static void _saveAsBack();
		static void _renderAndShow();

		static void _auto_matting();
	};
}
#endif