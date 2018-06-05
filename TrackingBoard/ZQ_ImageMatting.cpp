#include "ZQ_ImageMatting.h"
#include "ZQ_VirtualAdvertisementUtils.h"
#include <time.h>
using namespace ZQ;

const char* ZQ_ImageMatting::winName = "Image Matting @ Zuo Qing";
const char* ZQ_ImageMatting::render_winName = "render";
ZQ_DImage<double> ZQ_ImageMatting::ori_image;
ZQ_DImage<double> ZQ_ImageMatting::ori_trimap;
ZQ_DImage<double> ZQ_ImageMatting::trimap;
ZQ_DImage<float> ZQ_ImageMatting::tex;
ZQ_DImage<float> ZQ_ImageMatting::tex_alpha;
ZQ_DImage<double> ZQ_ImageMatting::fore;
ZQ_DImage<double> ZQ_ImageMatting::back;
ZQ_DImage<double> ZQ_ImageMatting::alpha;
ZQ_ImageMattingOptions ZQ_ImageMatting::opt;
IplImage* ZQ_ImageMatting::background_img = 0;
IplImage* ZQ_ImageMatting::zoom_background_img = 0;
IplImage* ZQ_ImageMatting::draw_img = 0;
int ZQ_ImageMatting::eraser_half_size = 5;
int ZQ_ImageMatting::eraser_max_half_size = 50;
int ZQ_ImageMatting::eraser_min_half_size = 1;
int ZQ_ImageMatting::eraser_pos_x = 0;
int ZQ_ImageMatting::eraser_pos_y = 0;
bool ZQ_ImageMatting::eraser_has_last_pos = false;
int ZQ_ImageMatting::eraser_last_pos_x = 0;
int ZQ_ImageMatting::eraser_last_pos_y = 0;
bool ZQ_ImageMatting::erase_mode = false;
ZQ_ImageMatting::EraserType ZQ_ImageMatting::eraser_type = ZQ_ImageMatting::ERASER_TYPE_UNKNOWN;
bool ZQ_ImageMatting::has_marker = false;
double ZQ_ImageMatting::marker[8] = { 0 };
ZQ_VirtualAdvertisementRender::MarkerMode ZQ_ImageMatting::marker_mode = ZQ_VirtualAdvertisementRender::MODE_ABCD;
bool ZQ_ImageMatting::draw_marker = false;
bool ZQ_ImageMatting::updated_flag = false;
int ZQ_ImageMatting::zoom_scale = 5;
bool ZQ_ImageMatting::zoom_mode = false;
int ZQ_ImageMatting::zoom_center_x = 0;
int ZQ_ImageMatting::zoom_center_y = 0;
int ZQ_ImageMatting::cur_mouse_pos_x = 0;
int ZQ_ImageMatting::cur_mouse_pos_y = 0;

bool ZQ_ImageMatting::Load(const ZQ_ImageMattingOptions& option)
{
	if (!ZQ_ImageIO::loadImage(ori_image, option.in_image_file,1))
	{
		printf("failed to load %s\n", option.in_image_file);
		return false;
	}
	if (!ZQ_ImageIO::loadImage(ori_trimap, option.in_trimap_file,0))
	{
		printf("failed to load %s\n", option.in_trimap_file);
		return false;
	}
	if (!LoadMarker(option.in_marker_file, marker))
	{
		has_marker = false;
	}
	else
		has_marker = true;

	
	switch (option.render_mode)
	{
	case ZQ_ImageMattingOptions::RENDER_NOTHING:
		break;
	case ZQ_ImageMattingOptions::RENDER_ONLY_TEX: case ZQ_ImageMattingOptions::RENDER_FBA_TEX:
		if (!has_marker)
		{
			printf("failed to load marker %s!\n", option.in_marker_file);
			return false;
		}
		
		if (!ZQ_ImageIO::loadImage(tex, option.in_tex_file, 1))
		{
			printf("failed to load %s\n", option.in_tex_file);
			return false;
		}

		if (!ZQ_VirtualAdvertisementRender::GetMarkerMode(option.marker_mode, marker_mode))
		{
			printf("unknown marker_mode %s\n", option.marker_mode);
			return false;
		}
		break;
	case ZQ_ImageMattingOptions::RENDER_TEX_WITH_ALPHA: case ZQ_ImageMattingOptions::RENDER_FBA_TEX_WITH_ALPHA:
		if (!has_marker)
		{
			printf("failed to load marker %s!\n", option.in_marker_file);
			return false;
		}
		
		if (!ZQ_ImageIO::loadImage(tex, option.in_tex_file, 1))
		{
			printf("failed to load %s\n", option.in_tex_file);
			return false;
		}
		if (!ZQ_ImageIO::loadImage(tex_alpha, option.in_tex_alpha_file, 0))
		{
			printf("failed to load %s\n", option.in_tex_alpha_file);
			return false;
		}
		if (!ZQ_VirtualAdvertisementRender::GetMarkerMode(option.marker_mode, marker_mode))
		{
			printf("unknown marker_mode %s\n", option.marker_mode);
			return false;
		}
		break;
	}
	
	int width = ori_image.width();
	int height = ori_image.height();
	int nChannels = ori_image.nchannels();
	if (!ori_trimap.matchDimension(width, height, 1))
	{
		printf("dimension dont match!\n");
		return false;
	}
	
	double*& ori_image_data = ori_image.data();
	cvReleaseImage(&background_img);
	background_img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);

	for (int h = 0; h < height; h++)
	{
		for (int w = 0; w < width; w++)
		{
			if (nChannels == 1)
			{
				CvScalar sca;
				for (int c = 0; c < 3; c++)
					sca.val[c] = ori_image_data[h*width + w] * 255;
				cvSet2D(background_img, h, w, sca);
			}
			else if (nChannels == 3)
			{
				CvScalar sca;
				for (int c = 0; c < 3; c++)
					sca.val[c] = ori_image_data[(h*width + w)*3+c] * 255;
				cvSet2D(background_img, h, w, sca);
			}
		}
	}
	cvReleaseImage(&zoom_background_img);
	zoom_background_img = cvCreateImage(cvSize(width*zoom_scale, height*zoom_scale), IPL_DEPTH_8U, 3);
	cvResize(background_img, zoom_background_img, CV_INTER_NN);

	trimap = ori_trimap;
	opt = option;

	
	return true;
}

bool ZQ_ImageMatting::Run()
{
	draw_img = cvCloneImage(background_img);
	cvNamedWindow(winName);
	cvShowImage(winName, draw_img);
	cvSetMouseCallback(winName, _mouseHandler, 0);

	if (has_marker && opt.auto_matting)
	{
		_auto_matting();
		_renderAndShow();
		erase_mode = true;
		_draw();
	}

	
	while (true)
	{
		//cvReleaseImage(&draw_img);
		//draw_img = cvCloneImage(background_img);
		updated_flag = false;
		int c = cvWaitKey(30);
		c = (char)c;
		if (c == 27)
		{
			break;
		}
		else if (c == 'r')
		{
			trimap = ori_trimap;
			//_draw();
			updated_flag = true;
		}
		else if (c == 'a')
		{
			cvDestroyWindow(render_winName);
			_auto_matting();
			_renderAndShow();
		}
		else if (c == 's')
		{
			cvDestroyWindow(render_winName);
			_saveAsBack();
			_renderAndShow();
		}
		else if (c == 'e')
		{
			erase_mode = !erase_mode;
			//_draw();
			updated_flag = true;
		}
		else if (c == 'l')
		{
			draw_marker = !draw_marker;
			//_draw();
			updated_flag = true;
		}
		else if (c == 'f')
		{
			eraser_type = ERASER_TYPE_FORE;
			//_draw();
			updated_flag = true;
		}
		else if (c == 'b')
		{
			eraser_type = ERASER_TYPE_BACK;
			//_draw();
			updated_flag = true;
		}
		else if (c == 'o')
		{
			eraser_half_size--;
			eraser_half_size = __max(eraser_half_size, eraser_min_half_size);
			//_draw();
			updated_flag = true;
		}
		else if (c == 'p')
		{
			eraser_half_size++;
			eraser_half_size = __min(eraser_half_size, eraser_max_half_size);
			//_draw();
			updated_flag = true;
		}
		else if (c == 'u')
		{
			eraser_type = ERASER_TYPE_UNKNOWN;
			//_draw();
			updated_flag = true;
		}
		else if (c == 'g')
		{
			cvDestroyWindow(render_winName);
			_go();
			_renderAndShow();
		}
		else if (c == 'z')
		{
			if (!zoom_mode)
			{
				zoom_mode = true;
				zoom_center_x = cur_mouse_pos_x;
				zoom_center_y = cur_mouse_pos_y;
				updated_flag = true;
			}
			else
			{
				zoom_mode = false;
				updated_flag = true;
			}
		}
		if (updated_flag)
		{
			if (!zoom_mode)
			{
				cvReleaseImage(&draw_img);
				draw_img = cvCloneImage(background_img);
			}
			else
			{
				int width = ori_image.width();
				int height = ori_image.height();
				CvRect rect = cvRect(zoom_center_x*zoom_scale - zoom_center_x, zoom_center_y*zoom_scale - zoom_center_y, width, height);
				cvSetImageROI(zoom_background_img, rect);
				cvCopy(zoom_background_img, draw_img);
				cvResetImageROI(zoom_background_img);
			}
			
			_draw();
		}
		cvNamedWindow(winName, 1);
		cvShowImage(winName, draw_img);
	}
	return true;
}

void ZQ_ImageMatting::_draw()
{
	//cvReleaseImage(&draw_img);
	//draw_img = cvCloneImage(background_img);
	
	if (draw_marker)
	{
		_drawMarker(draw_img);
	}
	if (erase_mode)
	{
		_drawTrimap(draw_img);
		_drawErazer(draw_img);
	}
	//cvShowImage(winName, draw_img);
}

void ZQ_ImageMatting::_drawTrimap(IplImage* img)
{
	double fore_color[4] = { 0.6, 0.6, 0.6, 0.3 };
	double back_color[4] = { 0.6, 0.0, 0.0, 0.3 };
	int width = trimap.width();
	int height = trimap.height();
	double*& trimap_data = trimap.data();
	if (!zoom_mode)
	{
		for (int h = 0; h < height; h++)
		{
			for (int w = 0; w < width; w++)
			{
				if (trimap_data[h*width + w] == 1)
				{
					CvScalar sca = cvGet2D(img, h, w);
					for (int c = 0; c < 3; c++)
					{
						sca.val[c] = fore_color[c] * 255 + (1 - fore_color[3]) * sca.val[c];
					}
					cvSet2D(img, h, w, sca);
				}
				else if (trimap_data[h*width + w] == 0)
				{
					CvScalar sca = cvGet2D(img, h, w);
					for (int c = 0; c < 3; c++)
					{
						sca.val[c] = back_color[c] * 255 + (1 - back_color[3]) * sca.val[c];
					}
					cvSet2D(img, h, w, sca);
				}
			}
		}
	}
	else
	{
		for (int h = 0; h < height; h++)
		{
			for (int w = 0; w < width; w++)
			{
				int real_h = (double)(h - zoom_center_y) / zoom_scale + zoom_center_y + 0.5;
				int real_w = (double)(w - zoom_center_x) / zoom_scale + zoom_center_x + 0.5;
				real_h = __min(height - 1, __max(0, real_h));
				real_w = __min(width - 1, __max(0, real_w));
				if (trimap_data[real_h*width + real_w] == 1)
				{
					CvScalar sca = cvGet2D(img, h, w);
					for (int c = 0; c < 3; c++)
					{
						sca.val[c] = fore_color[c] * 255 + (1 - fore_color[3]) * sca.val[c];
					}
					cvSet2D(img, h, w, sca);
				}
				else if (trimap_data[real_h*width + real_w] == 0)
				{
					CvScalar sca = cvGet2D(img, h, w);
					for (int c = 0; c < 3; c++)
					{
						sca.val[c] = back_color[c] * 255 + (1 - back_color[3]) * sca.val[c];
					}
					cvSet2D(img, h, w, sca);
				}
			}
		}
	}
}

void ZQ_ImageMatting::_drawErazer(IplImage* img)
{
	if (!zoom_mode)
	{
		CvPoint pt1 = cvPoint(eraser_pos_x - eraser_half_size, eraser_pos_y - eraser_half_size);
		CvPoint pt2 = cvPoint(eraser_pos_x - eraser_half_size, eraser_pos_y + eraser_half_size);
		CvPoint pt3 = cvPoint(eraser_pos_x + eraser_half_size, eraser_pos_y + eraser_half_size);
		CvPoint pt4 = cvPoint(eraser_pos_x + eraser_half_size, eraser_pos_y - eraser_half_size);
		CvScalar eraser_color;
		if (eraser_type == ERASER_TYPE_FORE)
		{
			eraser_color = cvScalar(255, 255, 255);
		}
		else if (eraser_type == ERASER_TYPE_BACK)
		{
			eraser_color = cvScalar(0, 0, 0);
		}
		else
			eraser_color = cvScalar(120, 120, 120);
		cvLine(img, pt1, pt2, eraser_color);
		cvLine(img, pt2, pt3, eraser_color);
		cvLine(img, pt3, pt4, eraser_color);
		cvLine(img, pt4, pt1, eraser_color);
	}
	else
	{
		int half_size = eraser_half_size * zoom_scale;
		int pos_x = (eraser_pos_x - zoom_center_x)*zoom_scale + zoom_center_x;
		int pos_y = (eraser_pos_y - zoom_center_y)*zoom_scale + zoom_center_y;
		CvPoint pt1 = cvPoint(pos_x - half_size, pos_y - half_size);
		CvPoint pt2 = cvPoint(pos_x - half_size, pos_y + half_size);
		CvPoint pt3 = cvPoint(pos_x + half_size, pos_y + half_size);
		CvPoint pt4 = cvPoint(pos_x + half_size, pos_y - half_size);
		CvScalar eraser_color;
		if (eraser_type == ERASER_TYPE_FORE)
		{
			eraser_color = cvScalar(255, 255, 255);
		}
		else if (eraser_type == ERASER_TYPE_BACK)
		{
			eraser_color = cvScalar(0, 0, 0);
		}
		else
			eraser_color = cvScalar(120, 120, 120);
		cvLine(img, pt1, pt2, eraser_color);
		cvLine(img, pt2, pt3, eraser_color);
		cvLine(img, pt3, pt4, eraser_color);
		cvLine(img, pt4, pt1, eraser_color);
	}
}

void ZQ_ImageMatting::_drawMarker(IplImage* img)
{
	if (!zoom_mode)
	{
		if (has_marker)
		{
			CvPoint pt1 = cvPoint(marker[0], marker[1]);
			CvPoint pt2 = cvPoint(marker[2], marker[3]);
			CvPoint pt3 = cvPoint(marker[4], marker[5]);
			CvPoint pt4 = cvPoint(marker[6], marker[7]);
			CvScalar marker_color = cvScalar(0, 255, 0);
			cvLine(img, pt1, pt2, marker_color);
			cvLine(img, pt2, pt3, marker_color);
			cvLine(img, pt3, pt4, marker_color);
			cvLine(img, pt4, pt1, marker_color);
		}
	}
	else
	{
		if (has_marker)
		{
			CvPoint pt1 = cvPoint((marker[0] - zoom_center_x)*zoom_scale + zoom_center_x, (marker[1] - zoom_center_y)*zoom_scale + zoom_center_y);
			CvPoint pt2 = cvPoint((marker[2] - zoom_center_x)*zoom_scale + zoom_center_x, (marker[3] - zoom_center_y)*zoom_scale + zoom_center_y);
			CvPoint pt3 = cvPoint((marker[4] - zoom_center_x)*zoom_scale + zoom_center_x, (marker[5] - zoom_center_y)*zoom_scale + zoom_center_y);
			CvPoint pt4 = cvPoint((marker[6] - zoom_center_x)*zoom_scale + zoom_center_x, (marker[7] - zoom_center_y)*zoom_scale + zoom_center_y);
			CvScalar marker_color = cvScalar(0, 255, 0);
			cvLine(img, pt1, pt2, marker_color);
			cvLine(img, pt2, pt3, marker_color);
			cvLine(img, pt3, pt4, marker_color);
			cvLine(img, pt4, pt1, marker_color);
		}
	}
	
}

void ZQ_ImageMatting::_mouseHandler(int event, int x, int y, int flags, void* param)
{
	cur_mouse_pos_x = x;
	cur_mouse_pos_y = y;
	if (event == CV_EVENT_LBUTTONDOWN)
	{
		if (erase_mode)
		{
			if (!zoom_mode)
			{
				eraser_pos_x = x;
				eraser_pos_y = y;
			}
			else
			{
				eraser_pos_x = (x - zoom_center_x) / zoom_scale + zoom_center_x;
				eraser_pos_y = (y - zoom_center_y) / zoom_scale + zoom_center_y;
			}
			_erase();
			//_draw();
			updated_flag = true;
			eraser_last_pos_x = eraser_pos_x;
			eraser_last_pos_y = eraser_pos_y;
			eraser_has_last_pos = true;
		}
	}
	else if (event == CV_EVENT_LBUTTONUP)
	{
		eraser_has_last_pos = false;
	}
	else if (event == CV_EVENT_MOUSEMOVE)
	{
		if (flags & CV_EVENT_FLAG_LBUTTON)
		{
			if (erase_mode)
			{
				if (!zoom_mode)
				{
					eraser_pos_x = x;
					eraser_pos_y = y;
				}
				else
				{
					eraser_pos_x = (x - zoom_center_x) / zoom_scale + zoom_center_x;
					eraser_pos_y = (y - zoom_center_y) / zoom_scale + zoom_center_y;
				}
				_erase();
				//_draw();
				updated_flag = true;
				eraser_last_pos_x = eraser_pos_x;
				eraser_last_pos_y = eraser_pos_y;
				eraser_has_last_pos = true;
			}
		}
		else
		{
			if (erase_mode)
			{
				if (!zoom_mode)
				{
					eraser_pos_x = x;
					eraser_pos_y = y;
				}
				else
				{
					eraser_pos_x = (x - zoom_center_x) / zoom_scale + zoom_center_x;
					eraser_pos_y = (y - zoom_center_y) / zoom_scale + zoom_center_y;
				}
				//_draw();
				updated_flag = true;

			}
		}

	}
	else if (event == CV_EVENT_RBUTTONDOWN)
	{
	}

}

void ZQ_ImageMatting::_erase()
{
	int width = trimap.width();
	int height = trimap.height();
	double*& trimap_data = trimap.data();
	double val = 0;
	if (eraser_type == ERASER_TYPE_FORE)
		val = 1;
	else if (eraser_type == ERASER_TYPE_BACK)
		val = 0;
	else
	{
		val = 0.5;
	}


	for (int h = eraser_pos_y - eraser_half_size; h <= eraser_pos_y + eraser_half_size; h++)
	{
		for (int w = eraser_pos_x - eraser_half_size; w <= eraser_pos_x + eraser_half_size; w++)
		{
			if (h >= 0 && h < height && w >= 0 && w < width)
				trimap_data[h*width + w] = val;
		}
	}

	if (eraser_has_last_pos && eraser_last_pos_x >= 0 && eraser_last_pos_x < width && eraser_last_pos_y >= 0 && eraser_last_pos_y < height
		&& eraser_pos_x >= 0 && eraser_pos_x < width && eraser_pos_y >= 0 && eraser_pos_y < height)
	{
		std::vector<ZQ_Vec2D> poly, pixels;
		if (eraser_pos_y >= eraser_last_pos_y)
		{
			if (eraser_pos_x >= eraser_last_pos_x)
			{
				poly.push_back(ZQ_Vec2D(eraser_pos_x + eraser_half_size, eraser_pos_y - eraser_half_size));
				poly.push_back(ZQ_Vec2D(eraser_pos_x + eraser_half_size, eraser_pos_y + eraser_half_size));
				poly.push_back(ZQ_Vec2D(eraser_pos_x - eraser_half_size, eraser_pos_y + eraser_half_size));
				poly.push_back(ZQ_Vec2D(eraser_last_pos_x - eraser_half_size, eraser_last_pos_y + eraser_half_size));
				poly.push_back(ZQ_Vec2D(eraser_last_pos_x - eraser_half_size, eraser_last_pos_y - eraser_half_size));
				poly.push_back(ZQ_Vec2D(eraser_last_pos_x + eraser_half_size, eraser_last_pos_y - eraser_half_size));
			}
			else
			{
				poly.push_back(ZQ_Vec2D(eraser_pos_x + eraser_half_size, eraser_pos_y + eraser_half_size));
				poly.push_back(ZQ_Vec2D(eraser_pos_x - eraser_half_size, eraser_pos_y + eraser_half_size));
				poly.push_back(ZQ_Vec2D(eraser_pos_x - eraser_half_size, eraser_pos_y - eraser_half_size));
				poly.push_back(ZQ_Vec2D(eraser_last_pos_x - eraser_half_size, eraser_last_pos_y - eraser_half_size));
				poly.push_back(ZQ_Vec2D(eraser_last_pos_x + eraser_half_size, eraser_last_pos_y - eraser_half_size));
				poly.push_back(ZQ_Vec2D(eraser_last_pos_x + eraser_half_size, eraser_last_pos_y + eraser_half_size));
			}
		}
		else
		{
			if (eraser_pos_x >= eraser_last_pos_x)
			{
				poly.push_back(ZQ_Vec2D(eraser_pos_x - eraser_half_size, eraser_pos_y - eraser_half_size));
				poly.push_back(ZQ_Vec2D(eraser_pos_x + eraser_half_size, eraser_pos_y - eraser_half_size));
				poly.push_back(ZQ_Vec2D(eraser_pos_x + eraser_half_size, eraser_pos_y + eraser_half_size));
				poly.push_back(ZQ_Vec2D(eraser_last_pos_x + eraser_half_size, eraser_last_pos_y + eraser_half_size));
				poly.push_back(ZQ_Vec2D(eraser_last_pos_x - eraser_half_size, eraser_last_pos_y + eraser_half_size));
				poly.push_back(ZQ_Vec2D(eraser_last_pos_x - eraser_half_size, eraser_last_pos_y - eraser_half_size));
			}
			else
			{
				poly.push_back(ZQ_Vec2D(eraser_pos_x - eraser_half_size, eraser_pos_y + eraser_half_size));
				poly.push_back(ZQ_Vec2D(eraser_pos_x - eraser_half_size, eraser_pos_y - eraser_half_size));
				poly.push_back(ZQ_Vec2D(eraser_pos_x + eraser_half_size, eraser_pos_y - eraser_half_size));
				poly.push_back(ZQ_Vec2D(eraser_last_pos_x + eraser_half_size, eraser_last_pos_y - eraser_half_size));
				poly.push_back(ZQ_Vec2D(eraser_last_pos_x + eraser_half_size, eraser_last_pos_y + eraser_half_size));
				poly.push_back(ZQ_Vec2D(eraser_last_pos_x - eraser_half_size, eraser_last_pos_y + eraser_half_size));
			}
		}
		
		ZQ_ScanLinePolygonFill::ScanLinePolygonFillWithClip(poly, width, height, pixels);

		for (int p = 0; p < pixels.size(); p++)
		{
			int x = pixels[p].x;
			int y = pixels[p].y;
			if (x >= 0 && x < width && y >= 0 && y < height)
				trimap_data[y*width + x] = val;
		}
	}
}

void ZQ_ImageMatting::_go()
{
	clock_t t1 = clock();
	double eps = 1e-7;
	int win_size = opt.win_size;
	int width = ori_image.width();
	int height = ori_image.height();
	ZQ_DImage<bool> consts_map(width,height);
	double*& trimap_data = trimap.data();
	bool*& consts_map_data = consts_map.data();
	for (int i = 0; i < width*height; i++)
	{
		consts_map_data[i] = trimap_data[i] == 0 || trimap_data[i] == 1;
	}
	if (!ZQ_ClosedFormImageMatting::SolveAlpha(ori_image, consts_map, trimap, alpha, eps, win_size, true))
	{
		printf("failed to solve alpha!\n");
		return ;
	}
	
	clock_t t2 = clock();
	printf("solve alpha cost: %.3f seconds\n", 0.001*(t2 - t1));

	int max_iter = 50;
	bool flag = ZQ_ClosedFormImageMatting::SolveForeBack_ori_paper(ori_image, alpha, fore, back, max_iter, true);
	
	clock_t t3 = clock();
	printf("solve fore&back cost: %.3f seconds\n", 0.001*(t3 - t2));
	if(!flag)
	{
		printf("failed to solve fore and back!\n");
		return ;
	}

	if (opt.has_out_alpha)
	{
		if (!ZQ_ImageIO::saveImage(alpha, opt.out_alpha_file))
		{
			printf("failed to save alpha_file %s\n", opt.out_alpha_file);
		}
	}

	if (opt.has_out_fore)
	{
		if (!ZQ_ImageIO::saveImage(fore, opt.out_fore_file))
		{
			printf("failed to save fore_file %s\n", opt.out_fore_file);
		}
	}
	
	if (opt.has_out_back)
	{
		if (!ZQ_ImageIO::saveImage(back, opt.out_back_file))
		{
			printf("failed to save back_file %s\n", opt.out_back_file);
		}
	}
	
	printf("done\n");
	clock_t t4 = clock();
	printf("save fore&back&alpha cost: %.3f seconds\n", 0.001*(t4 - t3));
}

void ZQ_ImageMatting::_saveAsBack()
{
	int width = ori_image.width();
	int height = ori_image.height();
	int nChannels = ori_image.nchannels();
	back = ori_image;
	fore.allocate(width, height, nChannels);
	alpha.allocate(width,height);

	if (opt.has_out_alpha)
	{
		if (!ZQ_ImageIO::saveImage(alpha, opt.out_alpha_file))
		{
			printf("failed to save alpha_file %s\n", opt.out_alpha_file);
		}
	}
	
	if (opt.has_out_fore)
	{
		if (!ZQ_ImageIO::saveImage(fore, opt.out_fore_file))
		{
			printf("failed to save fore_file %s\n", opt.out_fore_file);
		}
	}
	
	if (opt.has_out_back)
	{
		if (!ZQ_ImageIO::saveImage(back, opt.out_back_file))
		{
			printf("failed to save back_file %s\n", opt.out_back_file);
		}
	}
	
	printf("done\n");
}

void ZQ_ImageMatting::_renderAndShow()
{
	ZQ_DImage<float> im_f;
	ZQ_DImage<float> fore_f;
	ZQ_DImage<float> back_f;
	ZQ_DImage<float> alpha_f;
	ZQ_DImage<float> out_im;
	if (opt.render_mode == ZQ_ImageMattingOptions::RENDER_ONLY_TEX
		|| opt.render_mode == ZQ_ImageMattingOptions::RENDER_TEX_WITH_ALPHA
		|| opt.render_mode == ZQ_ImageMattingOptions::RENDER_FBA_TEX
		|| opt.render_mode == ZQ_ImageMattingOptions::RENDER_FBA_TEX_WITH_ALPHA)
	{
		int width = ori_image.width();
		int height = ori_image.height();
		int nChannels = ori_image.nchannels();
		im_f.allocate(width, height, nChannels);
		fore_f.allocate(width, height, nChannels);
		back_f.allocate(width, height, nChannels);
		alpha_f.allocate(width, height, 1);
		double*& ori_im_data = ori_image.data();
		double*& ori_fore_data = fore.data();
		double*& ori_back_data = back.data();
		double*& ori_alpha_data = alpha.data();
		float*& im_data = im_f.data();
		float*& fore_data = fore_f.data();
		float*& back_data = back_f.data();
		float*& alpha_data = alpha_f.data();
		for (int i = 0; i < width*height; i++)
		{
			for (int c = 0; c < nChannels; c++)
			{
				im_data[i*nChannels + c] = ori_im_data[i*nChannels + c];
				fore_data[i*nChannels + c] = ori_fore_data[i*nChannels + c];
				back_data[i*nChannels + c] = ori_back_data[i*nChannels + c];
			}
			alpha_data[i] = ori_alpha_data[i];
		}

		switch (opt.render_mode)
		{
		case ZQ_ImageMattingOptions::RENDER_ONLY_TEX: case ZQ_ImageMattingOptions::RENDER_FBA_TEX:
			if (!ZQ_VirtualAdvertisementRender::Render_fba(out_im, fore_f, back_f, alpha_f, tex, marker, marker_mode))
			{
				printf("failed to run Render_fba!\n");
				return;
			}
			ZQ_ImageIO::Show(render_winName, out_im);
			if (opt.has_out_render)
			{
				if (!ZQ_ImageIO::saveImage(out_im, opt.out_render_file))
				{
					printf("failed to save %s\n", opt.out_render_file);
					return;
				}
			}
			
			break;
		case ZQ_ImageMattingOptions::RENDER_TEX_WITH_ALPHA:case ZQ_ImageMattingOptions::RENDER_FBA_TEX_WITH_ALPHA:
			if (!ZQ_VirtualAdvertisementRender::Render_fba_with_alpha(out_im, fore_f, back_f, alpha_f, tex, tex_alpha, marker, marker_mode))
			{
				printf("failed to run Render_fba_with_alpha!\n");
				return;
			}
			ZQ_ImageIO::Show(render_winName, out_im);
			if (opt.has_out_render)
			{
				if (!ZQ_ImageIO::saveImage(out_im, opt.out_render_file))
				{
					printf("failed to save %s\n", opt.out_render_file);
					return;
				}
			}
			break;
		}
	}
	
}

void ZQ_ImageMatting::_auto_matting()
{
	clock_t t1 = clock();
	int width = ori_trimap.width();
	int height = ori_trimap.height();
	double*& ori_trimap_data = ori_trimap.data();
	double*& trimap_data = trimap.data();
	ZQ_DImage<bool> fore_map(width, height);
	ZQ_DImage<bool> back_map(width, height);
	ZQ_DImage<bool> tmp(width, height);
	bool*& fore_map_data = fore_map.data();
	bool*& back_map_data = back_map.data();
	bool*& tmp_data = tmp.data();
	for (int i = 0; i < width*height; i++)
	{
		if (ori_trimap_data[i] == 1)
			fore_map_data[i] = true;
		else if (ori_trimap_data[i] == 0)
			back_map_data[i] = true;
	}
	bool pfilter2D[9] = { 1, 1, 1, 1, 1, 1, 1, 1, 1 };
	for (int i = 0; i < opt.auto_matting_fore_erode_size; i++)
	{
		ZQ_BinaryImageProcessing::Erode(fore_map_data, tmp_data, width, height, pfilter2D, 1, 1);
		fore_map.swap(tmp);
	}
	for (int i = 0; i < opt.auto_matting_back_erode_size; i++)
	{
		ZQ_BinaryImageProcessing::Erode(back_map_data, tmp_data, width, height, pfilter2D, 1, 1);
		back_map.swap(tmp);
	}

	std::vector<ZQ_Vec2D> poly(4);
	std::vector<ZQ_Vec2D> pixels;
	pixels.reserve(width*height);
	for (int i = 0; i < 4; i++)
	{
		poly[i].x = marker[i * 2];
		poly[i].y = marker[i * 2 + 1];
	}
	ZQ_ScanLinePolygonFill::ScanLinePolygonFillWithClip(poly, width, height, pixels);
	ZQ_DImage<bool> board_mask(width, height);
	bool*& board_mask_data = board_mask.data();
	for (int i = 0; i < pixels.size(); i++)
	{
		int y = pixels[i].y;
		int x = pixels[i].x;
		if (x >= 0 && x < width && y >= 0 && y < height)
			board_mask_data[y*width + x] = true;
	}

	for (int i = 0; i < opt.auto_matting_board_dilate_size; i++)
	{
		ZQ_BinaryImageProcessing::Dilate(board_mask_data, tmp_data, width, height, pfilter2D, 1, 1);
		board_mask.swap(tmp);
	}

	bool has_unknown = false;
	for (int i = 0; i < width*height; i++)
	{
		if (board_mask_data[i] && !fore_map_data[i] && !back_map_data[i])
		{
			trimap_data[i] = 0.5;
			has_unknown = true;
		}
		else
			trimap_data[i] = ori_trimap_data[i];
	}
	clock_t t2 = clock();
	printf("auto select trimap unknown area: %.3f seconds\n", 0.001*(t2 - t1));

	if (has_unknown)
	{
		_go();
	}
	else
	{
		_saveAsBack();
	}
	
}