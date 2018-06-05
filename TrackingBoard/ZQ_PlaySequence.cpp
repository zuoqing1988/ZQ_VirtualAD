#include "ZQ_PlaySequence.h"
#include <stdio.h>
#include "ZQ_SelectPolygon.h"
#include "ZQ_TrackingBoard.h"
#include <time.h>

using namespace ZQ;

const char* ZQ_PlaySequence::winName = "Play Sequence @ Zuo Qing";
const char* ZQ_PlaySequence::match_winName = "match";
IplImage* ZQ_PlaySequence::background_img = 0;
IplImage* ZQ_PlaySequence::draw_img = 0;
int ZQ_PlaySequence::cur_frame_id = 0;
bool ZQ_PlaySequence::updated_flag = false;
bool ZQ_PlaySequence::is_paused = true;
bool ZQ_PlaySequence::draw_marker = true;
bool ZQ_PlaySequence::has_marker = false;
bool ZQ_PlaySequence::has_polygon = false;
std::vector<ZQ_Vec2D> ZQ_PlaySequence::poly;
double ZQ_PlaySequence::marker[8] = { 0 };
bool ZQ_PlaySequence::draw_info = true;
ZQ_PlaySequenceConfig ZQ_PlaySequence::config;
ZQ_DImage<bool> ZQ_PlaySequence::poly_keyframe_flag;
ZQ_DImage<bool> ZQ_PlaySequence::marker_keyframe_flag;
char ZQ_PlaySequence::config_filename[200];

bool ZQ_PlaySequence::LoadConfig(const char* file)
{
	if (!config.LoadFromFile(file))
	{
		return false;
	}
	if (config.frame_num <= 0)
		return false;

	if (!_load_first_frame())
	{
		return false;
	}

	poly_keyframe_flag.allocate(config.frame_num, 1);
	marker_keyframe_flag.allocate(config.frame_num, 1);
	for (int i = 0; i < config.poly_keyframes.size(); i++)
	{
		int id = config.poly_keyframes[i] - config.base_id;
		if (id >= 0 && id < config.frame_num)
			poly_keyframe_flag.data()[id] = true;
	}
	for (int i = 0; i < config.marker_keyframes.size(); i++)
	{
		int id = config.marker_keyframes[i] - config.base_id;
		if (id >= 0 && id < config.frame_num)
			marker_keyframe_flag.data()[id] = true;
	}
	strcpy(config_filename, file);

	return true;
}


bool ZQ_PlaySequence::Run()
{
	draw_img = cvCloneImage(background_img);
	_draw();
	cvNamedWindow(winName);
	cvShowImage(winName, draw_img);
	cvSetMouseCallback(winName, _mouseHandler, 0);

	while (true)
	{
		updated_flag = false;
		int c = cvWaitKey(30);
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
			_load_previous_frame();
			updated_flag = true;
		}
		else if (c == 'd')
		{
			_load_next_frame();
			updated_flag = true;
		}
		else if (c == 'i')
		{
			draw_info = !draw_info;
			updated_flag = true;
		}
		else if (c == 'e')
		{
			_select_polygon();
			updated_flag = true;
		}
		else if (c == 'm')
		{
			_select_marker();
			updated_flag = true;
		}
		else if (c == 's')
		{
			_save_config();
		}
		else if (c == 'k')
		{
			_remove_marker_keyframe(cur_frame_id);
			updated_flag = true;
		}
		else if (c == 9)//TAB
		{
			_load_next_keyframe();
			updated_flag = true;
		}

		if (!is_paused)
		{
			_load_next_frame();
			updated_flag = true;
		}
		if (updated_flag)
		{
			/*if (!zoom_mode)
			{*/
				cvReleaseImage(&draw_img);
				draw_img = cvCloneImage(background_img);
			/*}
			else
			{
				int width = ori_image.width();
				int height = ori_image.height();
				CvRect rect = cvRect(zoom_center_x*zoom_scale - zoom_center_x, zoom_center_y*zoom_scale - zoom_center_y, width, height);
				cvSetImageROI(zoom_background_img, rect);
				cvCopy(zoom_background_img, draw_img);
				cvResetImageROI(zoom_background_img);
			}*/

			_draw();
		}
		cvNamedWindow(winName, 1);
		cvShowImage(winName, draw_img);
	}
	return true;
}

void ZQ_PlaySequence::_mouseHandler(int event, int x, int y, int flags, void* param)
{

}

bool ZQ_PlaySequence::_load_first_frame()
{
	if (config.frame_num <= 0)
		return false;
	int tmp_frame_id = 0;
	char filename[200];
	sprintf(filename, "%s\\%s\\%d.%s", config.work_fold, config.image_fold, config.base_id + cur_frame_id, config.image_suffix);
	IplImage* tmp_img = cvLoadImage(filename, 1);
	if (tmp_img != 0)
	{
		cvReleaseImage(&background_img);
		background_img = tmp_img;
		cur_frame_id = tmp_frame_id;

		sprintf(filename, "%s\\%s\\%d.txt", config.work_fold, config.marker_fold, config.base_id + cur_frame_id);
		has_marker = LoadMarker(filename, marker);
		sprintf(filename, "%s\\%s\\%d.txt", config.work_fold, config.poly_fold, config.base_id + cur_frame_id);
		has_polygon = LoadPolygonConfig(filename, poly);
	
		if(!has_polygon)
		{
			sprintf(filename, "%s\\%s\\%d_fw.txt", config.work_fold, config.poly_fold, config.base_id + cur_frame_id);
			has_polygon = LoadPolygonConfig(filename, poly);
			if (!has_polygon)
			{
				sprintf(filename, "%s\\%s\\%d_bw.txt", config.work_fold, config.poly_fold, config.base_id + cur_frame_id);
				has_polygon = LoadPolygonConfig(filename, poly);
			}
		}
		return true;
	}
	else
	{
		return false;
	}
}


void ZQ_PlaySequence::_load_next_keyframe()
{
	if (config.frame_num <= 0)
		return;

	bool has_find = false;
	int find_keyframe_id;
	for (int i = cur_frame_id + 1; i < cur_frame_id + config.frame_num; i++)
	{
		int id = i%config.frame_num;
		if (poly_keyframe_flag.data()[id] || marker_keyframe_flag.data()[id])
		{
			has_find = true;
			find_keyframe_id = id;
			break;
		}
	}

	if (!has_find)
		return;

	int tmp_frame_id = find_keyframe_id;
	char filename[200];
	sprintf(filename, "%s\\%s\\%d.%s", config.work_fold, config.image_fold, config.base_id + tmp_frame_id, config.image_suffix);
	IplImage* tmp_img = cvLoadImage(filename, 1);
	if (tmp_img != 0)
	{
		cvReleaseImage(&background_img);
		background_img = tmp_img;
		cur_frame_id = tmp_frame_id;

		sprintf(filename, "%s\\%s\\%d.txt", config.work_fold, config.marker_fold, config.base_id + cur_frame_id);
		has_marker = LoadMarker(filename, marker);
		if (!has_marker)
			marker_keyframe_flag.data()[cur_frame_id] = false;
		sprintf(filename, "%s\\%s\\%d.txt", config.work_fold, config.poly_fold, config.base_id + cur_frame_id);
		has_polygon = LoadPolygonConfig(filename, poly);
		if (!has_polygon)
			poly_keyframe_flag.data()[cur_frame_id] = false;
	}
}


void ZQ_PlaySequence::_load_previous_frame()
{
	if (config.frame_num > 1)
	{
		int tmp_frame_id = cur_frame_id - 1 + config.frame_num;
		tmp_frame_id %= config.frame_num;
		char filename[200];
		sprintf(filename, "%s\\%s\\%d.%s", config.work_fold, config.image_fold, config.base_id + tmp_frame_id,config.image_suffix);
		IplImage* tmp_img = cvLoadImage(filename, 1);
		if (tmp_img != 0)
		{
			cvReleaseImage(&background_img);
			background_img = tmp_img;
			cur_frame_id = tmp_frame_id;
			sprintf(filename, "%s\\%s\\%d.txt", config.work_fold, config.marker_fold, config.base_id + cur_frame_id);
			has_marker = LoadMarker(filename, marker);
			sprintf(filename, "%s\\%s\\%d.txt", config.work_fold, config.poly_fold, config.base_id + cur_frame_id);
			has_polygon = LoadPolygonConfig(filename, poly);
			if (!has_polygon)
			{
				sprintf(filename, "%s\\%s\\%d_fw.txt", config.work_fold, config.poly_fold, config.base_id + cur_frame_id);
				has_polygon = LoadPolygonConfig(filename, poly);
				if (!has_polygon)
				{
					sprintf(filename, "%s\\%s\\%d_bw.txt", config.work_fold, config.poly_fold, config.base_id + cur_frame_id);
					has_polygon = LoadPolygonConfig(filename, poly);
				}
			}
		}
	}
}


void ZQ_PlaySequence::_load_next_frame()
{
	if (config.frame_num > 1)
	{
		int tmp_frame_id = cur_frame_id + 1;
		tmp_frame_id %= config.frame_num;
		char filename[200];
		sprintf(filename, "%s\\%s\\%d.%s", config.work_fold, config.image_fold, config.base_id + tmp_frame_id, config.image_suffix);
		IplImage* tmp_img = cvLoadImage(filename, 1);
		if (tmp_img != 0)
		{
			cvReleaseImage(&background_img);
			background_img = tmp_img;
			cur_frame_id = tmp_frame_id;
			sprintf(filename, "%s\\%s\\%d.txt", config.work_fold, config.marker_fold, config.base_id + cur_frame_id);
			has_marker = LoadMarker(filename, marker);
			sprintf(filename, "%s\\%s\\%d.txt", config.work_fold, config.poly_fold, config.base_id + cur_frame_id);
			has_polygon = LoadPolygonConfig(filename, poly);
			if (!has_polygon)
			{
				sprintf(filename, "%s\\%s\\%d_fw.txt", config.work_fold, config.poly_fold, config.base_id + cur_frame_id);
				has_polygon = LoadPolygonConfig(filename, poly);
				if (!has_polygon)
				{
					sprintf(filename, "%s\\%s\\%d_bw.txt", config.work_fold, config.poly_fold, config.base_id + cur_frame_id);
					has_polygon = LoadPolygonConfig(filename, poly);
				}
			}
		}
	}
}

void ZQ_PlaySequence::_draw()
{
	if (draw_marker)
	{
		if (has_polygon)
			_draw_polygon(draw_img);
		if (has_marker)
			_draw_marker(draw_img);
	}
	if (draw_info)
	{
		_draw_info(draw_img);
	}
}

void ZQ_PlaySequence::_draw_marker(IplImage* draw_img)
{
	CvScalar marker_color = cvScalar(0, 0, 255);
	for (int i = 0; i < 4; i++)
	{
		int id1 = i;
		int id2 = (i + 1) % 4;
		CvPoint pt1 = cvPoint(marker[id1 * 2 + 0], marker[id1 * 2 + 1]);
		CvPoint pt2 = cvPoint(marker[id2 * 2 + 0], marker[id2 * 2 + 1]);
		cvLine(draw_img, pt1, pt2, marker_color);
	}
}

void ZQ_PlaySequence::_draw_polygon(IplImage* draw_img)
{
	CvScalar poly_color = cvScalar(0, 255, 0);
	int size = poly.size();
	if (size >= 2)
	{
		for (int i = 0; i < size; i++)
		{
			CvPoint pt1 = cvPoint(poly[i].x, poly[i].y);
			CvPoint pt2 = cvPoint(poly[(i + 1) % size].x, poly[(i + 1) % size].y);
			cvLine(draw_img, pt1, pt2, poly_color);
		}
	}
}

void ZQ_PlaySequence::_draw_info(IplImage* draw_img)
{
	CvFont font;
	cvInitFont(&font, CV_FONT_HERSHEY_COMPLEX, 1.0, 1.0, 0, 1, 8);
	char filename[200];
	sprintf(filename, "%d.%s", config.base_id + cur_frame_id, config.image_suffix);
	cvPutText(draw_img, filename, cvPoint(50,50), &font, cvScalar(0, 0, 255));
	sprintf(filename, "kfr_poly  : %s", poly_keyframe_flag.data()[cur_frame_id] ? "YES" : "NO");
	cvPutText(draw_img, filename, cvPoint(50, 100), &font, cvScalar(0, 0, 255));
	sprintf(filename, "kfr_marker: %s", marker_keyframe_flag.data()[cur_frame_id] ? "YES" : "NO");
	cvPutText(draw_img, filename, cvPoint(50, 150), &font, cvScalar(0, 0, 255));
}

void ZQ_PlaySequence::_select_polygon()
{
	std::vector<ZQ_Vec2D> tmp_poly;
	ZQ_SelectPolygon::Select(background_img, tmp_poly, 50, 1, "select polygon");
	if (tmp_poly.size() >= 3)
	{
		char filename[200];
		sprintf(filename, "%s\\%s\\%d.txt", config.work_fold, config.poly_fold, config.base_id + cur_frame_id);
		if (SavePolygonConfig(filename, tmp_poly))
		{
			poly = tmp_poly;
			poly_keyframe_flag.data()[cur_frame_id] = true;
			_propagate_polygon(cur_frame_id);
			_propagate_marker_after_polygon(cur_frame_id);
		}
	}
}


void ZQ_PlaySequence::_select_marker()
{
	std::vector<ZQ_Vec2D> tmp_poly;
	ZQ_SelectPolygon::Select(background_img, tmp_poly, 4, 1, "select marker");
	if (tmp_poly.size() == 4)
	{
		double tmp_marker[8] = {
			tmp_poly[0].x, tmp_poly[0].y,
			tmp_poly[1].x, tmp_poly[1].y,
			tmp_poly[2].x, tmp_poly[2].y,
			tmp_poly[3].x, tmp_poly[3].y
		};
		char filename[200];
		sprintf(filename, "%s\\%s\\%d.txt", config.work_fold, config.marker_fold, config.base_id + cur_frame_id);
		if (SaveMarker(filename,tmp_marker))
		{
			memcpy(marker, tmp_marker,sizeof(double)*8);
			marker_keyframe_flag.data()[cur_frame_id] = true;

			_propagate_marker(cur_frame_id);
		}
	}
}

void ZQ_PlaySequence::_save_config()
{
	config.marker_keyframes.clear();
	config.poly_keyframes.clear();
	for (int i = 0; i < config.frame_num; i++)
	{
		if (poly_keyframe_flag.data()[i])
		{
			config.poly_keyframes.push_back(config.base_id + i);
		}
	}
	for (int i = 0; i < config.frame_num; i++)
	{
		if (marker_keyframe_flag.data()[i])
		{
			config.marker_keyframes.push_back(config.base_id + i);
		}
	}
	bool flag = config.WriteToFiLe(config_filename);
	if (flag)
	{
		printf("sucess to save config: %s\n", config_filename);
	}
	else
	{
		printf("failed to save config: %s\n", config_filename);
	}
}

void ZQ_PlaySequence::_propagate_marker(int key_id)
{
	if (key_id < 0 || key_id >= config.frame_num)
		return;
	int previous_key_id = 0;
	int next_key_id = config.frame_num - 1;
	for (int i = key_id - 1; i >= 0; i--)
	{
		if (marker_keyframe_flag.data()[i])
		{
			previous_key_id = i;
			break;
		}
	}
	for (int i = key_id + 1; i < config.frame_num; i++)
	{
		if (marker_keyframe_flag.data()[i])
		{
			next_key_id = i;
			break;
		}
	}

	_propagate_marker(previous_key_id, key_id);
	_propagate_marker(key_id, next_key_id);
}

void ZQ_PlaySequence::_remove_marker_keyframe(int key_id)
{
	if (key_id < 0 || key_id >= config.frame_num)
		return;
	int previous_key_id = 0;
	int next_key_id = config.frame_num - 1;
	for (int i = key_id - 1; i >= 0; i--)
	{
		if (marker_keyframe_flag.data()[i])
		{
			previous_key_id = i;
			break;
		}
	}
	for (int i = key_id + 1; i < config.frame_num; i++)
	{
		if (marker_keyframe_flag.data()[i])
		{
			next_key_id = i;
			break;
		}
	}

	marker_keyframe_flag.data()[key_id] = false;
	_propagate_marker(previous_key_id, next_key_id);
}

void ZQ_PlaySequence::_propagate_marker_after_polygon(int poly_key_id)
{
	if (poly_key_id < 0 || poly_key_id >= config.frame_num)
		return;
	int first_key_id = 0;
	int last_key_id = config.frame_num - 1;
	for (int i = poly_key_id; i >= 0; i--)
	{
		if (marker_keyframe_flag.data()[i])
		{
			first_key_id = i;
			break;
		}
	}
	for (int i = poly_key_id + config.max_propagate_polygon_num + 1; i < config.frame_num; i++)
	{
		if (marker_keyframe_flag.data()[i])
		{
			last_key_id = i;
			break;
		}
	}

	while (true)
	{
		if (first_key_id >= last_key_id)
			break;
		int k;
		for (k = first_key_id + 1; k <= last_key_id; k++)
		{
			if (marker_keyframe_flag.data()[k])
			{
				break;
			}
		}
		_propagate_marker(first_key_id, k);
		first_key_id = k;
	}
}

void ZQ_PlaySequence::_propagate_marker(int start_id, int end_id)
{
	if (start_id < 0 || end_id >= config.frame_num || start_id >= end_id)
		return;

	int N = end_id - start_id + 1;
	ZQ_DImage<bool> keyframe_mask(1, N);
	bool*& keyframe_mask_data = keyframe_mask.data();

	if (marker_keyframe_flag.data()[start_id])
		keyframe_mask_data[0] = true;
	if (marker_keyframe_flag.data()[end_id])
		keyframe_mask_data[N - 1] = true;

	if (!keyframe_mask_data[0] && !keyframe_mask_data[N - 1])
		return;

	char filename[200];
	ZQ_DImage<double> Hmat(9 * (N - 1), 1);
	ZQ_DImage<double> invHmat(9 * (N - 1), 1);
	double*& H = Hmat.data();
	double*& invH = invHmat.data();
	for (int i = 0; i < N - 1; i++)
	{
		sprintf(filename, "%s\\%s\\%d_%d.txt", config.work_fold, config.Hconfig_fold, config.base_id+start_id + i, config.base_id+start_id + i + 1);
		if (!Load_H_invH(filename, H + i * 9, invH + i * 9))
		{
			printf("failed to load %s\n", filename);
			return;
		}
		//double tmp_I[9];
		//ZQ_MathBase::MatrixMul(H + i * 9, invH + i * 9, 3, 3, 3, tmp_I);
		//printf("hell0\n");
	}

	ZQ_DImage<double> marker(N, 8);
	double*& marker_data = marker.data();
	ZQ_DImage<double> marker_fw(N, 8);
	double*& marker_fw_data = marker_fw.data();
	ZQ_DImage<double> marker_bw(N, 8);
	double*& marker_bw_data = marker_bw.data();

	if (keyframe_mask_data[0])
	{
		sprintf(filename, "%s\\%s\\%d.txt", config.work_fold, config.marker_fold, config.base_id+start_id);
		if (!LoadMarker(filename, marker_data))
		{
			printf("failed to load %s\n", filename);
			return;
		}
		memcpy(marker_fw_data, marker_data, sizeof(double)* 8);
		memcpy(marker_bw_data, marker_data, sizeof(double)* 8);
	}

	if (keyframe_mask_data[N - 1])
	{
		sprintf(filename, "%s\\%s\\%d.txt", config.work_fold, config.marker_fold, config.base_id+end_id);
		if (!LoadMarker(filename, marker_data + 8 * (N - 1)))
		{
			printf("failed to load %s\n", filename);
			return;
		}
		memcpy(marker_fw_data + 8 * (N - 1), marker_data + 8 * (N - 1), sizeof(double)* 8);
		memcpy(marker_bw_data + 8 * (N - 1), marker_data + 8 * (N - 1), sizeof(double)* 8);
	}

	if (keyframe_mask_data[0])
	{
		int end_ff = keyframe_mask_data[N - 1] ? N - 2 : N - 1;
		for (int ff = 1; ff <= end_ff; ff++)
		{
			for (int p = 0; p < 4; p++)
			{
				if (!ZQ_TrackingBoard<double>::Project_with_H(H + (ff - 1) * 9, marker_fw_data + (ff - 1) * 8 + p * 2, marker_fw_data + ff * 8 + p * 2))
				{
					printf("failed to project !\n");
					return;
				}
			}
		}
	}

	if (keyframe_mask_data[N - 1])
	{
		int start_ff = keyframe_mask_data[0] ? 1 : 0;
		for (int ff = N - 2; ff >= start_ff; ff--)
		{
			for (int p = 0; p < 4; p++)
			{
				if (!ZQ_TrackingBoard<double>::Project_with_H(invH + ff * 9, marker_bw_data + (ff + 1) * 8 + p * 2, marker_bw_data + ff * 8 + p * 2))
				{
					printf("failed to project !\n");
					return;
				}
			}
		}
	}


	for (int i = 0; i < N; i++)
	{
		if (keyframe_mask_data[i])
		{
		}
		else if (keyframe_mask_data[0] && keyframe_mask_data[N - 1])
		{
			double fw_d = i;
			double bw_d = N - 1 - i;
			double fw_w = bw_d / (fw_d + bw_d);
			double bw_w = 1.0 - fw_w;
			for (int p = 0; p < 8; p++)
				marker_data[i * 8 + p] = marker_fw_data[i * 8 + p] * fw_w + marker_bw_data[i * 8 + p] * bw_w;
		}
		else if (keyframe_mask_data[0])
		{
			memcpy(marker_data + i * 8, marker_fw_data + i * 8, sizeof(double)* 8);
		}
		else if (keyframe_mask_data[N - 1])
		{
			memcpy(marker_data + i * 8, marker_bw_data + i * 8, sizeof(double)* 8);
		}
		sprintf(filename, "%s\\%s\\%d.txt", config.work_fold, config.marker_fold, config.base_id+i + start_id);
		if (!keyframe_mask_data[i])
		{
			if (!SaveMarker(filename, marker_data + i * 8))
			{
				printf("failed to save %s\n", filename);
			}
		}
	}
}


void ZQ_PlaySequence::_propagate_polygon(int key_id)
{
	if (key_id < 0 || key_id >= config.frame_num)
		return;

	int next_key_id = config.frame_num - 1;
	for (int i = key_id + 1; i < config.frame_num; i++)
	{
		if (marker_keyframe_flag.data()[i])
		{
			next_key_id = i;
			break;
		}
	}
	next_key_id = __min(next_key_id, key_id + config.max_propagate_polygon_num);
	_propagate_polygon(key_id, next_key_id);
}


void ZQ_PlaySequence::_propagate_polygon(int start_id, int end_id)
{
	if (start_id < 0 || end_id >= config.frame_num || start_id >= end_id)
		return;

	int N = end_id - start_id + 1;
	
	int width = background_img->width;
	int height = background_img->height;

	typedef float T;

	ZQ_TrackingBoard<T> track(width, height, 1);
	std::vector<ZQ_Vec2D> polygon;
	char filename[200];

	int num, dim;
	T* coords = 0;
	T* vals = 0;

	ZQ_TrackingBoardOptions opt;
	opt.ncores = config.ncores;
	opt.dis1_to_dis2_ratio = config.match_dis1_to_dis2_ratio;
	opt.feature_dis_angle = config.match_feature_dis_angle_degree / 180.0 * atan(1.0) * 4;
	opt.display_running_info = true;

	for (int fr = 0; fr < N; fr++)
	{
		int fr_id = fr + config.base_id + start_id;
		if (draw_info)
			printf("frame[%d]...\n", fr_id);
		sprintf(filename, "%s\\%s\\%d.%s", config.work_fold, config.key_fold, fr_id, config.key_suffix);
		double load_save_time = 0;
		double track_time = 0;
		if (fr == 0)
		{
			clock_t t3 = clock();
			
			if (!LoadSiftKey(filename, num, dim, coords, vals))
			{
				printf("failed to load %s\n", filename);
				return;
			}
			sprintf(filename, "%s\\%s\\%d.txt", config.work_fold, config.poly_fold, fr_id);
			if (!LoadPolygonConfig(filename, polygon))
			{
				delete[]coords;
				delete[]vals;
				printf("failed to load %s\n", filename);
				return;
			}
			clock_t t4 = clock();
			load_save_time = 0.001*(t4 - t3);

			if (!track.HandleFirstFrame(num, dim, vals, coords, polygon, opt))
			{
				delete[]coords;
				delete[]vals;
				printf("fail to track %d\n", fr_id);
				return ;
			}
		}
		else
		{
			delete[]coords;
			delete[]vals;
			num = 0;
			coords = 0;
			vals = 0;
			clock_t t3 = clock();
			if (!LoadSiftKey(filename, num, dim, coords, vals))
			{
			
				printf("failed to load %s\n", filename);
				return ;
			}
			clock_t t4 = clock();
			load_save_time = 0.001*(t4 - t3);
			clock_t t5 = clock();
			double H[9], invH[9];

			if (!track.TrackNextFrame(num, dim, vals, coords, H, invH, polygon, opt))
			{
				printf("fail to track %d\n", fr_id);
				delete[]vals;
				delete[]coords;
				return ;
			}
			clock_t t6 = clock();
			track_time = 0.001*(t6 - t5);

			sprintf(filename, "%s\\%s\\%d_%d.txt", config.work_fold, config.Hconfig_fold, fr_id - 1, fr_id);
			if (!Save_H_invH(filename, H, invH))
			{
				printf("failed to save %s\n", filename);
				return ;
			}

			sprintf(filename, "%s\\%s\\%d.txt", config.work_fold, config.poly_fold, fr_id);
			if (!SavePolygonConfig(filename, polygon))
			{
				printf("failed to save %s\n", filename);
				return ;
			}
			
		}

		if (draw_info)
			printf("load&save = %.3f seconds, track = %.3f seconds\n", load_save_time, track_time);
	}


	delete[]coords;
	delete[]vals;
	return ;
}