
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/nonfree/nonfree.hpp>
#include <iostream>
#include <time.h>
#define ZQ_LINK_OPENCV_VERSION_2413
#include "ZQ_Link_OpenCV_Lib.h"

using namespace std;
using namespace cv;

// helper function:
// finds a cosine of angle between vectors
// from pt0->pt1 and from pt0->pt2
double angle(const Point& pt0, const Point& pt1, const Point& pt2);
int _get_contours(const Mat& frame, const bool* foremask, Mat& contour_img);
void _find_contour_canny(const Mat& gray, vector<vector<Point>>& contours, int canny_thresh = 300, int apertureSize = 5);
void _find_contour_binary_thresh(const Mat& gray, vector<vector<Point>>& contours, int N);
int _get_surf_feature_num_in_contour(const Mat& frame, const vector<Point>& contour, double surf_hessian_thresh);
bool _save_keyframe(const Mat& contour_img, int cur_frame, const char* out_fold);
void _filter_contours_foremask(vector<vector<Point>>& contours, const bool* foremask, int width, int height);
void _filter_contours_edgenum_area_and_angle(vector<vector<Point>>& contours, int min_edge_num, int max_edge_num, double min_area, double max_area, double angle_ratio);
void _filter_repeat_contours(vector<vector<Point>>& contours, double repeat_dis_thresh);
void _filter_contours_surf_feature_num(vector<vector<Point>>& contours, const Mat& frame, double surf_hessian_thresh, int surf_feature_num_thresh);
bool _is_same_contour(const vector<Point>& contour1, const vector<Point>& contour2, double dis_thresh);

double _get_dis_avg_L2(const Mat& frame, const Mat& last_frame);
double _get_dis_hist(const Mat& frame, const Mat& last_frame);

int main(int argc, const char **argv)
{
	const char* videofile = "C:\\Users\\ZQ\\Desktop\\人民检察官HDTV28.mp4";
	//const char* videofile = "C:\\Users\\ZQ\\Desktop\\我的新野蛮女友.HD1280超清韩语中字.mp4";
	//const char* videofile = "data1.mp4";
	const char* out_fold = "keyframe";
	const char* foremaskfile = "";
	bool show = false;
	// 读取视频流
	if (argc >= 2)
		videofile = argv[1];
	if (argc >= 3)
		out_fold = argv[2];
	if (argc >= 4)
		foremaskfile = argv[3];
	if (argc >= 5)
		show = atoi(argv[4]);
	VideoCapture capture(videofile);
	Mat foremask;
	foremask = imread(foremaskfile, 0);
	

	// 检测视频是否读取成功
	if (!capture.isOpened())
	{
		cout << "No Input Image" << endl;
		return -1;
	}
	long totalFrameNumber = capture.get(CV_CAP_PROP_FRAME_COUNT);
	int width = capture.get(CV_CAP_PROP_FRAME_WIDTH);
	int height = capture.get(CV_CAP_PROP_FRAME_HEIGHT);
	cout << "整个视频共" << totalFrameNumber << "帧" << endl;

	bool* foremask_data = 0;
	if (!foremask.empty())
	{
		if (foremask.rows != height || foremask.cols != width)
		{
			printf("foremask's dimensions don't match with video\n");
		}
		else
		{
			foremask_data = new bool[width*height];
			for (int i = 0; i < width*height; i++)
			{
				foremask_data[i] = foremask.data[i] > 127;
			}
		}
	}

	//设置开始帧()
	long frameToStart = 0;// frame_pos 0-based
	long cur_frame = frameToStart;
	int skip = 10;
	capture.set(CV_CAP_PROP_POS_FRAMES, frameToStart);
	cout << "从第" << frameToStart << "帧开始读" << endl;

	char outfile[200];
	// 获取图像帧率
	double rate = capture.get(CV_CAP_PROP_FPS);
	bool stop(false);
	Mat frame,last_frame, contour_img; // 当前视频帧
	
	int total_count = 0;
	int fr_count = 0;
	int cur_scene_id = 1;
	int* scene_rect_num = new int[totalFrameNumber];
	memset(scene_rect_num, 0, sizeof(int)*totalFrameNumber);
	int* scene_id = new int[totalFrameNumber];
	memset(scene_id, 0, sizeof(int)*totalFrameNumber);

	int total_scene_num = 0;
	double thresh_dis_avg_L2 = 10;
	double thresh_dis_hist = 0.7;

	int delay = 5; //1000 / rate;
	clock_t t1 = clock();
	// 遍历每一帧
	while (!stop)
	{
		// 尝试读取下一帧
		if (!capture.read(frame))
			break;

		/******  detect shape begin  ******/
		if (cur_frame % skip == 0)
		{
			frame.copyTo(contour_img);
			int cur_count = _get_contours(frame, foremask_data, contour_img);
			total_count += cur_count;
			fr_count += cur_count != 0;
			clock_t t2 = clock();
			scene_rect_num[cur_frame] = cur_count;
			printf("fr=[%6d], cur_count=[%2d], total_count=[%5d] in [%5d] frames, time=%5.f secs\n", cur_frame, cur_count, total_count, fr_count, 0.001*(t2 - t1));
			if (cur_count > 0)
				_save_keyframe(contour_img, cur_frame, out_fold);
			if (show)
			{
				imshow("Extracted Frame", contour_img);
				if (waitKey(delay) == 27)
					stop = true;
			}
		}
		/******  detect shape end  ******/
		
		/******  scene cut begin  ******/
		bool scene_id_changed = false;
		cvtColor(frame, frame, CV_BGR2GRAY);
		if (cur_frame > 0)
		{
			double dis_avg_L2 = _get_dis_avg_L2(frame, last_frame);
			double dis_hist = _get_dis_hist(frame, last_frame);
			if (dis_avg_L2 > thresh_dis_avg_L2 && dis_hist < thresh_dis_hist)
			{
				cur_scene_id++;
				scene_id_changed = true;

			}
			scene_id[cur_frame] = cur_scene_id;
		}
		else
		{
			scene_id[cur_frame] = 1;
		}
		if (scene_id_changed)
			printf("scene=[%5d]\n", cur_scene_id);
		frame.copyTo(last_frame);
		/******  scene cut end  ******/
		cur_frame++;
	}
	total_scene_num = cur_scene_id;

	/******* export scene cuts begin ********/
	bool* export_scene_id_flag = new bool[total_scene_num];
	memset(export_scene_id_flag, 0, sizeof(bool)*total_scene_num);
	for (int i = 0; i < totalFrameNumber; i++)
	{
		if (scene_rect_num[i] > 0)
			export_scene_id_flag[scene_id[i]] = true;
	}
	int start_frame_id = 0, end_frame_id = 0;
	sprintf(outfile, "%s\\scene_cut.txt", out_fold);
	FILE* out = fopen(outfile,"w");
	while (true)
	{
		bool has_start = false;
		int cur_scene_id;
		for (; start_frame_id < totalFrameNumber; start_frame_id++)
		{
			cur_scene_id = scene_id[start_frame_id];
			if (export_scene_id_flag[cur_scene_id])
			{
				has_start = true;
				break;
			}
		}
		if (!has_start)
			break;
		for (end_frame_id = start_frame_id; end_frame_id < totalFrameNumber; end_frame_id++)
		{
			if (scene_id[end_frame_id] != cur_scene_id)
			{
				break;
			}
		}
		if (out != 0)
			fprintf(out,"[%d,%d]\n", start_frame_id, end_frame_id - 1);
		else
		{
			printf("[%d,%d]\n", start_frame_id, end_frame_id - 1);
		}
		start_frame_id = end_frame_id + 1;
	}
	if (out != 0)
		fclose(out);
	delete[]export_scene_id_flag;
	/******* export scene cuts end ********/

	if (foremask_data)
	{
		delete[]foremask_data;
	}
	delete[]scene_id;
	delete[]scene_rect_num;
	return 0;
}

int _get_contours(const Mat& frame, const bool* foremask, Mat& contour_img)
{
	int width = frame.cols;
	int height = frame.rows;
	bool has_foremask = foremask != 0;
	int count = 0;
	Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
	Mat gray,  tmp_edges;
	vector<vector<Point>> contours,tmp_contours;
	vector<vector<Point>> result(1);
	vector<Vec4i> hierarchy;
	cvtColor(frame, gray, CV_BGR2GRAY);
	clock_t t1 = clock();
	_find_contour_canny(gray, contours, 400, 5);
	_find_contour_binary_thresh(gray, tmp_contours, 16);
	contours.insert(contours.end(), tmp_contours.begin(), tmp_contours.end());
	clock_t t2 = clock();
	//printf("find contour cost: %.3f\n", 0.001*(t2 - t1));

	_filter_contours_foremask(contours,foremask, width, height);
	
	int min_edge_num = 4;
	int max_edge_num = 4;
	double min_area = 2500;
	double max_area = 0.25*width*height;
	double angle_ratio = 0.25;
	_filter_contours_edgenum_area_and_angle(contours, min_edge_num, max_edge_num, min_area, max_area, angle_ratio);

	double repeat_dis_thresh = 10;
	_filter_repeat_contours(contours, repeat_dis_thresh);

	double surf_hessian_thresh = 500;
	int surf_feature_num_thresh = 25;
	_filter_contours_surf_feature_num(contours, frame, surf_hessian_thresh, surf_feature_num_thresh);
	
	for (int i = 0; i < contours.size(); i++)
	{
		drawContours(contour_img, contours, i, Scalar(0, 255, 0), 2);
		count++;
		for (int j = 0; j < contours[i].size(); j++)
			drawMarker(contour_img, contours[i][j], Scalar(0, 0, 255),0,5);
	}
	return count;
}

// helper function:
// finds a cosine of angle between vectors
// from pt0->pt1 and from pt0->pt2
double angle(const Point& pt0, const Point& pt1, const Point& pt2)
{
	double dx1 = pt1.x - pt0.x;
	double dy1 = pt1.y - pt0.y;
	double dx2 = pt2.x - pt0.x;
	double dy2 = pt2.y - pt0.y;
	return (dx1*dx2 + dy1*dy2) / sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

int _get_surf_feature_num_in_contour(const Mat& frame, const vector<Point>& contour, double surf_hessian_thresh)
{
	int count = 0;
	SurfFeatureDetector feature;
	feature.hessianThreshold = surf_hessian_thresh;

	vector<KeyPoint> keypoints;
	vector<KeyPoint> out_keypoints;
	Rect _rect = boundingRect(contour);
	int bound_size = 32;
	int dx = _rect.x >= bound_size ? bound_size : _rect.x;
	int dy = _rect.y >= bound_size ? bound_size : _rect.y;
	_rect.width += dx;
	_rect.height += dy;
	_rect.x -= dx;
	_rect.y -= dy;
	_rect.width += __min(bound_size, frame.cols-1 - _rect.x - _rect.width);
	_rect.height += __min(bound_size, frame.rows-1 - _rect.y - _rect.height);
	
	Mat imgROI = frame(_rect);
	feature.detect(imgROI, keypoints);
	for (int i = 0; i < keypoints.size(); i++)
	{
		Point pt(keypoints[i].pt.x + _rect.x, keypoints[i].pt.y + _rect.y);
		if (pointPolygonTest(contour, pt, false) >= 0)
		{
			count++;
		}
	}

	return count;
}

bool _save_keyframe(const Mat& contour_img, int cur_frame, const char* out_fold)
{
	if (out_fold == 0)
		return false;

	Mat _img;
	resize(contour_img, _img, Size(contour_img.cols*0.5, contour_img.rows*0.5));
	char file[200];
	sprintf(file, "%s\\%d.jpg", out_fold, cur_frame);
	return imwrite(string(file),_img);
}

void _find_contour_canny(const Mat& gray, vector<vector<Point>>& contours, int canny_thresh, int apertureSize)
{
	contours.clear();
	vector<Vec4i> hierarchy;
	Mat edges;
	Canny(gray, edges, -1, canny_thresh, apertureSize);
	Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
	dilate(edges, edges, kernel);
	erode(edges, edges, kernel);
	findContours(edges, contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
}

void _find_contour_binary_thresh(const Mat& gray, vector<vector<Point>>& contours, int N)
{
	Mat pyr,gray_up;
	int width = gray.cols;
	int height = gray.rows;
	cv::pyrDown(gray, pyr);
	cv::pyrUp(pyr, gray_up);

	contours.clear();
	vector<vector<Point>> tmp_contours;
	vector<Vec4i> hierarchy;
	Mat binary;
	for (int l = 0; l < N; l++)
	{
		threshold(gray_up, binary, (l + 1) * 255 / (N + 1), 255, CV_THRESH_BINARY);
		//cv::imshow("binary", binary);
		//cv::waitKey(0);
		tmp_contours.clear();
		findContours(binary, tmp_contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
		contours.insert(contours.end(), tmp_contours.begin(), tmp_contours.end());
	}
}

void _filter_contours_foremask(vector<vector<Point>>& contours, const bool* foremask, int width, int height)
{
	if (foremask != 0)
	{
		for (vector<vector<Point>>::iterator it = contours.begin(); it != contours.end();)
		{
			bool check_succ = true;
			for (int j = 0; j < (*it).size(); j++)
			{
				int ix = (*it)[j].x;
				int iy = (*it)[j].y;
				if (foremask[iy*width + ix])
				{
					check_succ = false;
					break;
				}
			}


			if (!check_succ)
				it = contours.erase(it);
			else
				++it;
		}
	}
}

void _filter_contours_edgenum_area_and_angle(vector<vector<Point>>& contours, int min_edge_num, int max_edge_num, double min_area, double max_area, double angle_ratio)
{
	vector<vector<Point>> out_contours;
	vector<Point> result;
	for (vector<vector<Point>>::iterator it = contours.begin(); it != contours.end();)
	{
		bool check_succ = true;

		double arc_len = cv::arcLength(*it, true);
		double eps = __min(arc_len*0.02, 20);
		cv::approxPolyDP(*it, result, eps, 1);
		int contour_size = result.size();
		check_succ = contour_size >= min_edge_num && contour_size <= max_edge_num;

		if (check_succ)
		{
			double area = contourArea(result);
			check_succ = area > min_area && area < max_area;
		}
		
		if (check_succ)
		{
			double m_pi = atan(1.0) * 4;
			double avg_angle = m_pi*(contour_size - 2) / contour_size;

			for (int j = 0; j < contour_size; j++)
			{
				double tmp = angle(result[(j + 1) % contour_size], result[j], result[(j + 2) % contour_size]);
				double tmp_angle = acos(tmp);
				if (tmp_angle < (1.0-angle_ratio)*avg_angle || tmp_angle > (1.0+angle_ratio)*avg_angle)
				{
					check_succ = false;
					break;
				}
			}
		}

		if (check_succ)
			out_contours.push_back(result);
		
		++it;
	}

	contours = out_contours;
}

void _filter_repeat_contours(vector<vector<Point>>& contours, double repeat_dis_thresh)
{
	if (contours.size() <= 1)
		return;
	vector<vector<Point>>::iterator it = contours.begin();
	
	++it;
	for (; ;)
	{
		vector<vector<Point>>::iterator it2 = contours.begin();
		bool repeat_flag = false;
		for (; it2 != it; ++it2)
		{
			if (_is_same_contour(*it, *it2, repeat_dis_thresh))
			{
				repeat_flag = true;
				break;
			}
		}
		if (repeat_flag)
			it = contours.erase(it);
		else
			++it;
		if (it == contours.end())
			break;
	}
}

void _filter_contours_surf_feature_num(vector<vector<Point>>& contours, const Mat& frame, double surf_hessian_thresh, int surf_feature_num_thresh)
{
	for (vector<vector<Point>>::iterator it = contours.begin(); it != contours.end();)
	{
		int surf_pt_num = _get_surf_feature_num_in_contour(frame, *it, surf_hessian_thresh);
		bool check_succ = surf_pt_num >= surf_feature_num_thresh;
		if (!check_succ)
			it = contours.erase(it);
		else
			++it;
	}
}

bool _is_same_contour(const vector<Point>& contour1, const vector<Point>& contour2, double dis_thresh)
{
	double dis_thresh2 = dis_thresh*dis_thresh;
	bool all_same_flag = true;
	for (int i = 0; i < contour1.size(); i++)
	{
		Point pt1 = contour1[i];
		bool same_flag = false;
		for (int j = 0; j < contour2.size(); j++)
		{
			Point pt2 = contour2[j];
			double cur_dis2 = (pt1.x - pt2.x)*(pt1.x - pt2.x) + (pt1.y - pt2.y)*(pt1.y - pt2.y);
			if (cur_dis2 < dis_thresh2)
			{
				same_flag = true;
				break;
			}
		}
		if (!same_flag)
		{
			all_same_flag = false;
			break;
		}
	}
	return all_same_flag;
}


double _get_dis_avg_L2(const Mat& frame1, const Mat& frame2)
{
	clock_t t1 = clock();
	int width = frame1.cols;
	int height = frame1.rows;
	Mat diff = frame1 - frame2;
	double result = 0;
	for (int h = 0; h < height; h++)
	{
		unsigned char* data = (unsigned char*)diff.data + h*diff.step;
		for (int w = 0; w < width; w++)
			result += (double)data[w] * data[w];
	}

	result /= (width*height + 1e-10);
	result = sqrt(result);
	clock_t t2 = clock();
	//	printf("dis_L2: %.3f, ", 0.001*(t2 - t1));
	return result;
}

double _get_dis_hist(const Mat& frame1, const Mat& frame2)
{
	int channels[1] = { 0 };
	int histSize[] = { 256 };
	float h_ranges[] = { 0, 256 };
	const float* ranges[] = { h_ranges };

	MatND hist_base1;
	clock_t t1 = clock();
	calcHist(&frame1, 1, channels, Mat(), hist_base1, 1, histSize, ranges, true, false);
	normalize(hist_base1, hist_base1, 0, 1, NORM_MINMAX, -1, Mat());
	clock_t t2 = clock();

	MatND hist_base2;
	calcHist(&frame2, 1, channels, Mat(), hist_base2, 1, histSize, ranges, true, false);
	normalize(hist_base2, hist_base2, 0, 1, NORM_MINMAX, -1, Mat());
	clock_t t3 = clock();
	//	printf("hist1: %.3f, hist2: %.3f \n", 0.001*(t2 - t1), 0.001*(t3 - t2));
	double base_base = compareHist(hist_base1, hist_base2, CV_COMP_CORREL);

	return base_base;
}