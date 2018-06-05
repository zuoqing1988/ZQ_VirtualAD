#include "ZQ_SelectPolygonGUI.h"

const char* ZQ_SelectPolygonGUI::winName = "Select Polygon @ Zuo Qing";
float ZQ_SelectPolygonGUI::coord_x[ZQ_SelectPolygonGUI::MAX_NUM_PTS] = { 0 };
float ZQ_SelectPolygonGUI::coord_y[ZQ_SelectPolygonGUI::MAX_NUM_PTS] = { 0 };
int ZQ_SelectPolygonGUI::max_num_pts = ZQ_SelectPolygonGUI::MAX_NUM_PTS;
int ZQ_SelectPolygonGUI::cur_coord_num = 0;
float ZQ_SelectPolygonGUI::scale = 1.0;
IplImage* ZQ_SelectPolygonGUI::background_img = 0;
IplImage* ZQ_SelectPolygonGUI::zoom_background_img = 0;
IplImage* ZQ_SelectPolygonGUI::draw_img = 0;
int ZQ_SelectPolygonGUI::zoom_scale = 5;
bool ZQ_SelectPolygonGUI::zoom_mode = false;
int ZQ_SelectPolygonGUI::zoom_center_x = 0;
int ZQ_SelectPolygonGUI::zoom_center_y = 0;
int ZQ_SelectPolygonGUI::cur_mouse_pos_x = 0;
int ZQ_SelectPolygonGUI::cur_mouse_pos_y = 0;
bool ZQ_SelectPolygonGUI::updated_flag = false;

bool ZQ_SelectPolygonGUI::Select(const IplImage* img, std::vector<ZQ::ZQ_Vec2D>& poly, int max_npts, float show_scale, const char* win_name)
{
	if (img == 0)
		return false;
	poly.clear();
	cvReleaseImage(&background_img);
	cvReleaseImage(&zoom_background_img);
	cvReleaseImage(&draw_img);
	background_img = 0;
	zoom_background_img = 0;
	draw_img = 0;

	max_num_pts = __min(max_npts, MAX_NUM_PTS);
	cur_coord_num = 0;

	scale = __max(0.1, __min(2.0, show_scale));
	int width = img->width;
	int height = img->height;
	int nChannels = img->nChannels;
	int new_width = width*scale;
	int new_height = height*scale;
	if (scale == 1)
		background_img = cvCloneImage(img);
	else
	{
		background_img = cvCreateImage(cvSize(new_width, new_height), IPL_DEPTH_8U, nChannels);
		cvResize(img, background_img);
	}

	zoom_background_img = cvCreateImage(cvSize(new_width*zoom_scale, new_height*zoom_scale), IPL_DEPTH_8U, nChannels);
	cvResize(background_img, zoom_background_img, CV_INTER_NN);

	draw_img = cvCloneImage(background_img);

	
	if (!zoom_mode)
	{
		cvCopy(background_img, draw_img);
	}
	else
	{
		CvRect rect = cvRect(zoom_center_x*zoom_scale - zoom_center_x, zoom_center_y*zoom_scale - zoom_center_y, new_width, new_height);
		cvSetImageROI(zoom_background_img, rect);
		cvCopy(zoom_background_img, draw_img);
		cvResetImageROI(zoom_background_img);
	}
	_draw();

	

	const char* real_win_name = win_name == 0 ? winName : win_name;
	cvNamedWindow(real_win_name);
	cvShowImage(real_win_name, draw_img);
	cvSetMouseCallback(real_win_name, _mouseHandler,0);
	while (true)
	{
		updated_flag = false;
		int c = cvWaitKey(30);
		c = (char)c;
		if (c == 27)
		{
			break;
		}
		else if (c == 'r')
		{
			cur_coord_num = 0;
			updated_flag = true;
		}
		else if (c == 'z')
		{
			zoom_mode = !zoom_mode;
			if (zoom_mode)
			{
				zoom_center_x = cur_mouse_pos_x;
				zoom_center_y = cur_mouse_pos_y;
			}
			updated_flag = true;
		}
		else if (c == 's')
		{
			break;
		}
		
		if (updated_flag)
		{
			if (!zoom_mode)
			{
				cvCopy(background_img, draw_img);
			}
			else
			{
				CvRect rect = cvRect(zoom_center_x*zoom_scale - zoom_center_x, zoom_center_y*zoom_scale - zoom_center_y, new_width, new_height);
				cvSetImageROI(zoom_background_img, rect);
				cvCopy(zoom_background_img, draw_img);
				cvResetImageROI(zoom_background_img);
			}
			_draw();
			
		}
		cvNamedWindow(real_win_name, 1);
		cvShowImage(real_win_name, draw_img);
	}
	cvDestroyWindow(real_win_name);
	for (int i = 0; i < cur_coord_num; i++)
	{
		poly.push_back(ZQ::ZQ_Vec2D(coord_x[i] / scale, coord_y[i] / scale));
	}
	return true;
}


void ZQ_SelectPolygonGUI::_mouseHandler(int event, int x, int y, int flags, void* param)
{
	cur_mouse_pos_x = x;
	cur_mouse_pos_y = y;
	if (event == CV_EVENT_LBUTTONDOWN)
	{
		if (cur_coord_num < max_num_pts)
		{
			if (!zoom_mode)
			{
				coord_x[cur_coord_num] = x;
				coord_y[cur_coord_num] = y;
			}
			else
			{
				coord_x[cur_coord_num] = (float)(x - zoom_center_x) / zoom_scale + zoom_center_x;
				coord_y[cur_coord_num] = (float)(y - zoom_center_y) / zoom_scale + zoom_center_y;
			}
			
			cur_coord_num++;
			updated_flag = true;
		}
	}
	else if (event == CV_EVENT_RBUTTONDOWN)
	{
		if (cur_coord_num > 0)
		{
			cur_coord_num--;
			updated_flag = true;
		}
	}
}

void ZQ_SelectPolygonGUI::_drawPolygon(IplImage* img)
{
	CvScalar color_pts = cvScalar(0, 0, 255);
	//CvScalar color_rect = cvScalar(0, 0, 200);
	CvScalar color_line1 = cvScalar(255, 0, 0);
	CvScalar color_line2 = cvScalar(200, 0, 0);


	for (int i = 0; i < cur_coord_num; i++)
	{
		float x, y;
		if (zoom_mode)
		{
			x = (coord_x[i] - zoom_center_x)*zoom_scale + zoom_center_x;
			y = (coord_y[i] - zoom_center_y)*zoom_scale + zoom_center_y;
		}
		else
		{
			x = coord_x[i];
			y = coord_y[i];
		}
		cvCircle(img, cvPoint(x, y), 1, color_pts, 2);
	}

	int poly_pt_num = cur_coord_num;
	if (poly_pt_num >= 2)
	{
		for (int i = 0; i < cur_coord_num - 1; i++)
		{
			float x1, y1, x2, y2;
			if (zoom_mode)
			{
				x1 = (coord_x[i] - zoom_center_x)*zoom_scale + zoom_center_x;
				y1 = (coord_y[i] - zoom_center_y)*zoom_scale + zoom_center_y;
				x2 = (coord_x[i + 1] - zoom_center_x)*zoom_scale + zoom_center_x;
				y2 = (coord_y[i + 1] - zoom_center_y)*zoom_scale + zoom_center_y;
			}
			else
			{
				x1 = coord_x[i];
				y1 = coord_y[i];
				x2 = coord_x[i + 1];
				y2 = coord_y[i + 1];
			}
			CvPoint pt1 = cvPoint(x1, y1);
			CvPoint pt2 = cvPoint(x2, y2);
			cvLine(img, pt1, pt2, color_line1, 2);
		}
	}
	
	if (poly_pt_num >= 3)
	{
		float x1, y1, x2, y2;
		if (zoom_mode)
		{
			x1 = (coord_x[cur_coord_num - 1] - zoom_center_x)*zoom_scale + zoom_center_x;
			y1 = (coord_y[cur_coord_num - 1] - zoom_center_y)*zoom_scale + zoom_center_y;
			x2 = (coord_x[0] - zoom_center_x)*zoom_scale + zoom_center_x;
			y2 = (coord_y[0] - zoom_center_y)*zoom_scale + zoom_center_y;
		}
		else
		{
			x1 = coord_x[cur_coord_num - 1];
			y1 = coord_y[cur_coord_num - 1];
			x2 = coord_x[0];
			y2 = coord_y[0];
		}
		CvPoint pt1 = cvPoint(x1, y1);
		CvPoint pt2 = cvPoint(x2, y2);
		cvLine(img, pt1, pt2, color_line2, 1);
	}

}

void ZQ_SelectPolygonGUI::_draw()
{
	_drawPolygon(draw_img);
}
