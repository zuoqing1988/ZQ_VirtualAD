#ifndef _ZQ_DETECT_SHAPE_AND_SCENE_CUT_H_
#define _ZQ_DETECT_SHAPE_AND_SCENE_CUT_H_
#pragma once

#include "ZQ_DetectShapeAndSceneCutOptions.h"
#include "ZQ_TrackingBoard.h"
#include "ZQ_ShapeAndSceneCut.h"
#include "ZQ_VirtualAdKeyIO_CV2.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/nonfree/nonfree.hpp>
#include "ZQ_DoubleImage.h"
#include <iostream>
#include <time.h>

namespace ZQ
{
	class ZQ_DetectShapeAndSceneCut
	{
	public:
		static bool Go(const ZQ_DetectShapeAndSceneCutOptions& opt)
		{
			cv::VideoCapture capture(opt.video_file);
			cv::Mat foremask_im;
			ZQ_DImage<bool> foremask;
			bool*& foremask_data = foremask.data();
			
			/******* prepare video and foremask begin ********/
			if (!capture.isOpened())
			{
				if (opt.display_running_info)
					printf("failed to open video: %s\n", opt.video_file);
				return false;
			}
			long totalFrameNumber = capture.get(CV_CAP_PROP_FRAME_COUNT);
			int width = capture.get(CV_CAP_PROP_FRAME_WIDTH);
			int height = capture.get(CV_CAP_PROP_FRAME_HEIGHT);
			if (opt.has_foremask_file)
			{
				foremask_im = cv::imread(opt.foremask_file, 0);
				if (!foremask_im.empty())
				{
					if (foremask_im.rows != height || foremask_im.cols != width)
					{
						if (opt.display_running_info)
							printf("foremask's dimensions don't match with video\n");
						return false;
					}
					else
					{
						foremask.allocate(width, height, 1);
						for (int i = 0; i < width*height; i++)
							foremask_data[i] = foremask_im.data[i] > 127;
					}
				}
			}
			/******* prepare video and foremask end ********/

			const int frameToStart = 0;// frame_pos 0-based
			int cur_frame = 0;
			int skip = __max(1, opt.detect_shape_skip);
			//capture.set(CV_CAP_PROP_POS_FRAMES, frameToStart);
			
			cv::Mat frame, gray, last_frame, scene_first_frame;

			int total_count = 0;
			int total_scene_num = 0;
			int fr_count = 0;
			int cur_scene_id = 1;
			int cur_scene_fr_num = 0;
			int last_scene_fr_num = 0;
			ZQ_DImage<int> scene_shape_num(totalFrameNumber, 1);
			int*& scene_shape_num_data = scene_shape_num.data();
			ZQ_DImage<int> scene_id(totalFrameNumber, 1);
			std::vector<int> static_scene_ids;
			int*& scene_id_data = scene_id.data();
			
			
			std::vector<ZQ_SceneCut::Shape> all_shapes;
			std::vector<ZQ_SceneCut::Shape> cur_frame_shapes;
			
			bool static_scene = true;
			clock_t t1 = clock();
			while (true)
			{
				if (!capture.read(frame))
					break;
				cvtColor(frame, gray, CV_BGR2GRAY);
				/******  detect shape begin  ******/
				if (cur_frame % skip == 0)
				{
					GetContours(gray, foremask_data, cur_frame_shapes, opt);
					int cur_count = cur_frame_shapes.size();
					total_count += cur_count;
					fr_count += cur_count != 0;
					clock_t t2 = clock();
					scene_shape_num_data[cur_frame] = cur_count;
					if (opt.display_running_info)
						printf("fr=[%d], cur_count=[%d], total_count=[%d] in [%d] frames, time=%5.f secs\n", cur_frame, cur_count, total_count, fr_count, 0.001*(t2 - t1));
					if (cur_count > 0)
					{
						for (int i = 0; i < cur_frame_shapes.size(); i++)
							cur_frame_shapes[i].frame_id = cur_frame;
						if (opt.export_thumbnails)
						{
							if (!_export_thumbnails(frame, cur_frame_shapes, cur_frame + frameToStart, opt))
								return false;
						}
						all_shapes.insert(all_shapes.end(), cur_frame_shapes.begin(), cur_frame_shapes.end());
					}
				}
				/******  detect shape end  ******/

				/******  scene cut begin  ******/

				bool scene_id_changed = false;
				
				if (cur_frame > 0)
				{
					if (!_is_same_scene(gray, last_frame, opt))
					{
						cur_scene_id++;
						scene_id_changed = true;
						last_scene_fr_num = cur_scene_fr_num;
						cur_scene_fr_num = 0;
					}
					else
					{
						cur_scene_fr_num++;
					}
				}
				else
				{
					gray.copyTo(scene_first_frame);
				}

				scene_id_data[cur_frame] = cur_scene_id;
				if (opt.display_running_info)
				{
					if (scene_id_changed)
					{
						if (opt.display_running_info)
							printf("scene=[%d]\n", cur_scene_id);
					}	
				}
				///
				if (scene_id_changed)
				{
					if (static_scene)
					{
						if (last_scene_fr_num > 1)
						{
							double ratio = _count_same_pixel_ratio(last_frame, scene_first_frame);
							if (ratio < opt.static_pixels_ratio)
								static_scene = false;
						}
						else
						{
							static_scene = false;
						}
						if (static_scene)
						{
							static_scene_ids.push_back(cur_scene_id - 1);
							printf("detect static scene !!!!!!!!!\n");
						}
					}
					static_scene = true;
					gray.copyTo(scene_first_frame);
				}
				else 
				{
					if (static_scene)
					{
						if (cur_frame == 0)
						{
							static_scene = true;
						}
						else
						{
							double ratio = _count_same_pixel_ratio(gray, last_frame);
							if (ratio < opt.static_pixels_ratio)
								static_scene = false;
						}
					}
				}

				gray.copyTo(last_frame);
				/******  scene cut end  ******/
				cur_frame++;
			}
			if (static_scene)
				static_scene_ids.push_back(cur_scene_id);
			total_scene_num = cur_scene_id;

			/******************************************/

			ZQ_SceneCutList scene_cut_list;
			_get_scene_cut_list(all_shapes, scene_shape_num, scene_id,static_scene_ids, total_scene_num, totalFrameNumber, scene_cut_list);
			all_shapes.clear();
			all_shapes.shrink_to_fit();

			if (opt.remove_repeat_shape)
			{
				for (int i = 0; i < scene_cut_list.scene_cuts.size(); i++)
					Remove_repeat_shape(scene_cut_list.scene_cuts[i], capture, opt);
			}
			
			if (!scene_cut_list.ExportToFile(opt.scene_cut_config_file))
			{
				printf("warning: failed to export scene cut config: %s\n", opt.scene_cut_config_file);
			}

			



			/******* export scene cuts begin ********/
			if (!_export_scene_cuts(capture, scene_cut_list, totalFrameNumber, width, height, opt))
			{
				return false;
			}
			/******* export scene cuts end ********/
			return true;
		}

		static void GetContours(const cv::Mat& gray, const bool* foremask, std::vector<ZQ_SceneCut::Shape>& shapes, const ZQ_DetectShapeAndSceneCutOptions& opt)
		{
			int width = gray.cols;
			int height = gray.rows;
			bool has_foremask = foremask != 0;
			std::vector<std::vector<cv::Point>> contours;
			std::vector<std::vector<cv::Point>> tmp_contours;
			clock_t t1 = clock();
			_find_contour_canny(gray, contours, opt.canny_thresh, opt.canny_aperture_size);
			_find_contour_binary_thresh(gray, tmp_contours, opt.binary_Nlevels);
			contours.insert(contours.end(), tmp_contours.begin(), tmp_contours.end());
			clock_t t2 = clock();
			//printf("find contour cost: %.3f\n", 0.001*(t2 - t1));
			
			_filter_contours_foremask(contours, foremask, width, height);

			int shape_num = contours.size();
			shapes.clear();
			shapes.resize(shape_num);
			

			int min_edge_num = 4;
			int max_edge_num = 4;
			double min_area = opt.shape_min_area;
			double max_area = 0.25*width*height;
			double angle_ratio = opt.shape_angle_ratio;
			std::vector<float> areas;
			_filter_contours_edgenum_area_and_angle(contours, shapes, min_edge_num, max_edge_num, min_area, max_area, angle_ratio);

			double repeat_dis_thresh = opt.repeat_shape_dis_thresh;
			_filter_repeat_contours(contours, shapes, repeat_dis_thresh);

			

			double surf_hessian_thresh = 500;
			int surf_feature_num_thresh = opt.shape_feature_num_thresh;
			_filter_contours_surf_feature_num(contours, shapes, gray, surf_hessian_thresh, surf_feature_num_thresh);

			for (int i = 0; i < contours.size(); i++)
			{
				//shapes[i].poly = contours[i];
				shapes[i].poly.swap(contours[i]);
			}
		}

		static bool Remove_repeat_shape(ZQ_SceneCut& scene_cut, cv::VideoCapture& capture, const ZQ_DetectShapeAndSceneCutOptions& opt)
		{
			if (!capture.isOpened())
				return false;

			std::vector<ZQ_SceneCut::Shape>& shapes = scene_cut.GetShapes();
			if (shapes.size() <= 1)
				return true;

			if (opt.display_running_info)
				printf("begin to remove repeat shape!\n");
			std::vector<ZQ_SceneCut::Shape>::iterator it = shapes.begin();

			const float repeat_dis_thresh = 300;

			++it;
			for (;;)
			{
				std::vector<ZQ_SceneCut::Shape>::iterator it2 = shapes.begin();
				bool repeat_flag = false;
				for (; it2 != it; ++it2)
				{
					if (_is_same_shape((*it), (*it2), repeat_dis_thresh, capture,opt))
					{
						if (opt.display_running_info)
							printf("repeat detected!\n");
						repeat_flag = true;
						break;
					}
					else
					{
						
					}
				}
				if (repeat_flag)
				{
					it = shapes.erase(it);
				}
				else
				{
					++it;
				}
				if (it == shapes.end())
					break;
			}
			if (opt.display_running_info)
				printf("remove repeat shape done: [%d] remained!\n", shapes.size());
			return true;
		}

	private:

		static void _get_scene_cut_list(const std::vector<ZQ_SceneCut::Shape>& shapes, const ZQ_DImage<int>& scene_shape_num, const ZQ_DImage<int>& scene_id, const std::vector<int>& static_scene_ids, 
			int total_scene_num, int total_frame_num, ZQ_SceneCutList& scene_cut_list)
		{
			const int*& scene_shape_num_data = scene_shape_num.data();
			const int*& scene_id_data = scene_id.data();
			ZQ_DImage<bool> export_scene_id_flag(total_scene_num + 1, 1);
			bool*& export_scene_id_flag_data = export_scene_id_flag.data();
			for (int i = 0; i < total_frame_num; i++)
			{
				if (scene_shape_num_data[i] > 0)
					export_scene_id_flag_data[scene_id_data[i]] = true;
			}
			for (int i = 0; i < static_scene_ids.size(); i++)
			{
				export_scene_id_flag_data[static_scene_ids[i]] = true;
			}
			int start_frame_id = 0, end_frame_id = 0;
			std::vector<int> frame_ids;
			while (true)
			{
				bool has_start = false;
				int cur_scene_id = 0;
				for (; start_frame_id < total_frame_num; start_frame_id++)
				{
					cur_scene_id = scene_id_data[start_frame_id];
					if (export_scene_id_flag_data[cur_scene_id])
					{
						has_start = true;
						break;
					}
				}
				if (!has_start)
					break;
				for (end_frame_id = start_frame_id; end_frame_id < total_frame_num; end_frame_id++)
				{
					if (scene_id_data[end_frame_id] != cur_scene_id)
					{
						break;
					}
				}
				frame_ids.push_back(start_frame_id);
				frame_ids.push_back(end_frame_id);
				frame_ids.push_back(cur_scene_id);
				start_frame_id = end_frame_id;
				if (end_frame_id >= total_frame_num)
					break;
			}

			int scene_cut_num = frame_ids.size() / 3;
			scene_cut_list.scene_cuts.resize(scene_cut_num);
			for (int i = 0; i < scene_cut_num; i ++)
			{
				scene_cut_list.scene_cuts[i].start_frame_id = frame_ids[i * 3];
				scene_cut_list.scene_cuts[i].end_frame_id = frame_ids[i * 3 + 1] - 1;
				int cur_scene_id = frame_ids[i * 3 + 2];
				bool flag = false;
				for (int j = 0; j < static_scene_ids.size(); j++)
				{
					if (static_scene_ids[j] == cur_scene_id)
					{
						flag = true;
						break;
					}
				}
				scene_cut_list.scene_cuts[i].is_static_scene = flag;
			}

			for (int i = 0; i < shapes.size(); i++)
			{
				int fr_id = shapes[i].frame_id;
				for (int j = 0; j < scene_cut_num; j++)
				{
					int st_id = scene_cut_list.scene_cuts[j].start_frame_id;
					int end_id = scene_cut_list.scene_cuts[j].end_frame_id;
					if (fr_id >= st_id && fr_id <= end_id)
					{
						scene_cut_list.scene_cuts[j].shapes.push_back(shapes[i]);
						break;
					}
				}
			}
		}

		static bool _export_scene_cuts(cv::VideoCapture& capture, const ZQ_SceneCutList& scene_cut_list, int total_frame_num, int width, int height, const ZQ_DetectShapeAndSceneCutOptions& opt)
		{
			char filename[2000];
			
			cv::Mat frame;

			if (opt.export_scene_cut_videos)
			{
				for (int i = 0; i < scene_cut_list.scene_cuts.size(); i ++)
				{
					int start_fr_id = scene_cut_list.scene_cuts[i].start_frame_id;
					int end_fr_id = scene_cut_list.scene_cuts[i].end_frame_id;
					sprintf(filename, "%s\\%d_%d.avi", opt.scene_cut_video_fold, start_fr_id, end_fr_id);
					cv::VideoWriter writer(filename, CV_FOURCC('D', 'I', 'V', 'X'), 25.0, cv::Size(width, height));
					capture.set(CV_CAP_PROP_POS_FRAMES, start_fr_id);
					if (opt.display_running_info)
						printf("begin to write video %s\n", filename);

					while (start_fr_id <= end_fr_id)
					{
						if (!capture.read(frame))
							break;
						writer.write(frame);
						start_fr_id++;
					}
				}
			}

			return true;
		}

		static bool _export_thumbnails(const cv::Mat& frame, const std::vector<ZQ_SceneCut::Shape>& shapes, int frame_id, const ZQ_DetectShapeAndSceneCutOptions& opt)
		{
			char filename[2000];
			cv::Mat contour_img(frame);
			std::vector<std::vector<cv::Point>> contours;
			for (int i = 0; i < shapes.size(); i++)
				contours.push_back(shapes[i].poly);
			for (int i = 0; i < contours.size(); i++)
			{
				drawContours(contour_img, contours, i, cv::Scalar(0, 255, 0), 2);
				for (int j = 0; j < contours[i].size(); j++)
					drawMarker(contour_img, contours[i][j], cv::Scalar(0, 0, 255), 0, 5);
			}
			cv::resize(contour_img, contour_img, cv::Size(contour_img.cols*opt.thumbnails_scale, contour_img.rows*opt.thumbnails_scale));
			sprintf(filename, "%s\\%d.jpg", opt.thumbnails_fold, frame_id);
			return cv::imwrite(filename, contour_img);
		}

		static bool _export_shape_polygon(const std::vector<std::vector<cv::Point>>& contours, int frame_id, const ZQ_DetectShapeAndSceneCutOptions& opt)
		{
			char filename[2000];
			for (int i = 0; i < contours.size(); i++)
			{
				sprintf(filename, "%s\\%d_%d.txt", opt.shape_polygon_fold, frame_id, i);

				FILE* out = fopen(filename, "w");
				if (out == 0)
				{
					if (opt.display_running_info)
						printf("failed to create file %s\n", filename);
					return false;
				}
				int n = contours[i].size();
				fprintf(out, "%d\n", n);

				for (int j = 0; j < n; j++)
				{
					fprintf(out, "%d %d\n", contours[i][j].x, contours[i][j].y);
				}

				fclose(out);
			}
			return true;
		}

		static bool _is_same_scene(const cv::Mat& cur_frame, const cv::Mat& last_frame, const ZQ_DetectShapeAndSceneCutOptions& opt)
		{
			//float thresh_dis_avg_L2 = 10;
			//float thresh_dis_hist = 0.7;
			double dis_avg_L2 = _get_dis_avg_L2(cur_frame, last_frame);
			double dis_hist = _get_dis_hist(cur_frame, last_frame);
			if (dis_avg_L2 > opt.dis_avg_L2_thresh && dis_hist < opt.dis_hist_thresh)
			{
				return false;
			}
			return true;
		}

		static double _angle(const cv::Point& pt0, const cv::Point& pt1, const cv::Point& pt2)
		{
			double dx1 = pt1.x - pt0.x;
			double dy1 = pt1.y - pt0.y;
			double dx2 = pt2.x - pt0.x;
			double dy2 = pt2.y - pt0.y;
			return (dx1*dx2 + dy1*dy2) / sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
		}

		static void _find_contour_canny(const cv::Mat& gray, std::vector<std::vector<cv::Point>>& contours, int canny_thresh = 300, int apertureSize = 5)
		{
			contours.clear();
			std::vector<cv::Vec4i> hierarchy;
			cv::Mat edges;
			cv::Canny(gray, edges, -1, canny_thresh, apertureSize);
			cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
			cv::dilate(edges, edges, kernel);
			cv::erode(edges, edges, kernel);
			cv::findContours(edges, contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
		}

		static void _find_contour_binary_thresh(const cv::Mat& gray, std::vector<std::vector<cv::Point>>& contours, int N)
		{
			cv::Mat pyr, gray_up;
			int width = gray.cols;
			int height = gray.rows;
			cv::pyrDown(gray, pyr);
			cv::pyrUp(pyr, gray_up);

			contours.clear();
			std::vector<std::vector<cv::Point>> tmp_contours;
			std::vector<cv::Vec4i> hierarchy;
			cv::Mat binary;
			for (int l = 0; l < N; l++)
			{
				cv::threshold(gray_up, binary, (l + 1) * 255 / (N + 1), 255, CV_THRESH_BINARY);
				tmp_contours.clear();
				cv::findContours(binary, tmp_contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
				contours.insert(contours.end(), tmp_contours.begin(), tmp_contours.end());
			}
		}

		static int _get_surf_feature_num_in_contour(const cv::Mat& frame, const std::vector<cv::Point>& contour, double surf_hessian_thresh)
		{
			int count = 0;
			cv::SurfFeatureDetector feature;
			feature.hessianThreshold = surf_hessian_thresh;

			std::vector<cv::KeyPoint> keypoints;
			std::vector<cv::KeyPoint> out_keypoints;
			cv::Rect _rect = cv::boundingRect(contour);
			int bound_size = 32;
			int dx = _rect.x >= bound_size ? bound_size : _rect.x;
			int dy = _rect.y >= bound_size ? bound_size : _rect.y;
			_rect.width += dx;
			_rect.height += dy;
			_rect.x -= dx;
			_rect.y -= dy;
			_rect.width += __min(bound_size, frame.cols - 1 - _rect.x - _rect.width);
			_rect.height += __min(bound_size, frame.rows - 1 - _rect.y - _rect.height);

			cv::Mat imgROI = frame(_rect);
			feature.detect(imgROI, keypoints);
			for (int i = 0; i < keypoints.size(); i++)
			{
				cv::Point pt(keypoints[i].pt.x + _rect.x, keypoints[i].pt.y + _rect.y);
				if (cv::pointPolygonTest(contour, pt, false) >= 0)
				{
					count++;
				}
			}

			return count;
		}

		static void _filter_contours_foremask(std::vector<std::vector<cv::Point>>& contours, std::vector<ZQ_SceneCut::Shape>& shape_headers, const bool* foremask, int width, int height)
		{
			if (foremask != 0)
			{
				std::vector<ZQ_SceneCut::Shape>::iterator s_it = shape_headers.begin();
				std::vector<std::vector<cv::Point>>::iterator c_it = contours.begin();
				for (; c_it != contours.end(); )
				{
					bool check_succ = true;
					for (int j = 0; j < (*c_it).size(); j++)
					{
						int ix = (*c_it)[j].x;
						int iy = (*c_it)[j].y;
						if (foremask[iy*width + ix])
						{
							check_succ = false;
							break;
						}
					}

					if (!check_succ)
					{
						c_it = contours.erase(c_it);
						s_it = shape_headers.erase(s_it);
					}
					else
					{
						++c_it;
						++s_it;
					}
				}
			}
		}

		static void _filter_contours_foremask(std::vector<std::vector<cv::Point>>& contours, const bool* foremask, int width, int height)
		{
			if (foremask != 0)
			{
				for (std::vector<std::vector<cv::Point>>::iterator it = contours.begin(); it != contours.end();)
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

		static void _filter_contours_edgenum_area_and_angle(std::vector<std::vector<cv::Point>>& contours, std::vector<ZQ_SceneCut::Shape>& shape_headers, int min_edge_num, int max_edge_num,
			double min_area, double max_area, double angle_ratio)
		{
			std::vector<std::vector<cv::Point>> out_contours;
			std::vector<cv::Point> result;
			std::vector<std::vector<cv::Point>>::iterator c_it = contours.begin();
			std::vector<ZQ_SceneCut::Shape>::iterator s_it = shape_headers.begin();
			for (; c_it != contours.end();)
			{
				bool check_succ = true;

				double area = 0;
				int contour_size = 0;

				if (check_succ)
				{
					area = contourArea(*c_it);
					check_succ = area > min_area && area < max_area;
				}

				if (check_succ)
				{
					double arc_len = cv::arcLength((*c_it), true);
					double eps = __min(arc_len*0.02, 20);
					cv::approxPolyDP((*c_it), result, eps, 1);
					*c_it = result;
					contour_size = result.size();
					check_succ = contour_size >= min_edge_num && contour_size <= max_edge_num;
				}
				
				if (check_succ)
				{
					area = contourArea(result);
					(*s_it).area_size = area;
					check_succ = area > min_area && area < max_area;
				}

				if (check_succ)
				{
					double m_pi = atan(1.0) * 4;
					double avg_angle = m_pi*(contour_size - 2) / contour_size;

					for (int j = 0; j < contour_size; j++)
					{
						double tmp = _angle(result[(j + 1) % contour_size], result[j], result[(j + 2) % contour_size]);
						double tmp_angle = acos(tmp);
						if (tmp_angle < (1.0 - angle_ratio)*avg_angle || tmp_angle >(1.0 + angle_ratio)*avg_angle)
						{
							check_succ = false;
							break;
						}
					}
				}

				if (!check_succ)
				{
					c_it = contours.erase(c_it);
					s_it = shape_headers.erase(s_it);
				}
				else
				{
					++c_it;
					++s_it;
				}
			}
		}

		static void _filter_contours_edgenum_area_and_angle(std::vector<std::vector<cv::Point>>& contours, int min_edge_num, int max_edge_num,
			double min_area, double max_area, double angle_ratio)
		{
			std::vector<std::vector<cv::Point>> out_contours;
			std::vector<cv::Point> result;
			std::vector<std::vector<cv::Point>>::iterator c_it = contours.begin();
			for (; c_it != contours.end();)
			{
				bool check_succ = true;

				double arc_len = cv::arcLength((*c_it), true);
				double eps = __min(arc_len*0.02, 20);
				cv::approxPolyDP((*c_it), result, eps, 1);
				*c_it = result;
				int contour_size = result.size();
				check_succ = contour_size >= min_edge_num && contour_size <= max_edge_num;
				double area = 0;
				if (check_succ)
				{
					area = contourArea(result);
					check_succ = area > min_area && area < max_area;
				}

				if (check_succ)
				{
					double m_pi = atan(1.0) * 4;
					double avg_angle = m_pi*(contour_size - 2) / contour_size;

					for (int j = 0; j < contour_size; j++)
					{
						double tmp = _angle(result[(j + 1) % contour_size], result[j], result[(j + 2) % contour_size]);
						double tmp_angle = acos(tmp);
						if (tmp_angle < (1.0 - angle_ratio)*avg_angle || tmp_angle >(1.0 + angle_ratio)*avg_angle)
						{
							check_succ = false;
							break;
						}
					}
				}

				if (!check_succ)
				{
					c_it = contours.erase(c_it);
				}
				else
				{
					++c_it;
				}
			}
		}

		static void _filter_repeat_contours(std::vector<std::vector<cv::Point>>& contours, std::vector<ZQ_SceneCut::Shape>& shape_headers, double repeat_dis_thresh)
		{
			if (contours.size() <= 1)
				return;
			std::vector<std::vector<cv::Point>>::iterator it = contours.begin();
			std::vector<ZQ_SceneCut::Shape>::iterator s_it = shape_headers.begin();

			++it;
			++s_it;
			for (;;)
			{
				std::vector<std::vector<cv::Point>>::iterator it2 = contours.begin();
				bool repeat_flag = false;
				for (; it2 != it; ++it2)
				{
					if (_is_same_contour((*it), (*it2), repeat_dis_thresh))
					{
						repeat_flag = true;
						break;
					}
				}
				if (repeat_flag)
				{
					it = contours.erase(it);
					s_it = shape_headers.erase(s_it);
				}
				else
				{
					++it;
					++s_it;
				}
				if (it == contours.end())
					break;
			}
		}

		static void _filter_repeat_contours(std::vector<std::vector<cv::Point>>& contours, double repeat_dis_thresh)
		{
			if (contours.size() <= 1)
				return;
			std::vector<std::vector<cv::Point>>::iterator it = contours.begin();
			
			++it;
			for (;;)
			{
				std::vector<std::vector<cv::Point>>::iterator it2 = contours.begin();
				bool repeat_flag = false;
				for (; it2 != it; ++it2)
				{
					if (_is_same_contour((*it), (*it2), repeat_dis_thresh))
					{
						repeat_flag = true;
						break;
					}
				}
				if (repeat_flag)
				{
					it = contours.erase(it);
				}
				else
				{
					++it;
				}
				if (it == contours.end())
					break;
			}
		}

		static void _filter_contours_surf_feature_num(std::vector<std::vector<cv::Point>>& contours, std::vector<ZQ_SceneCut::Shape>& shape_headers, const cv::Mat& frame, double surf_hessian_thresh, int surf_feature_num_thresh)
		{
			std::vector<std::vector<cv::Point>>::iterator c_it = contours.begin();
			std::vector<ZQ_SceneCut::Shape>::iterator s_it = shape_headers.begin();
			for (; c_it != contours.end();)
			{
				int surf_pt_num = _get_surf_feature_num_in_contour(frame, (*c_it), surf_hessian_thresh);
				(*s_it).feature_num = surf_pt_num;
				bool check_succ = surf_pt_num >= surf_feature_num_thresh;
				if (!check_succ)
				{
					c_it = contours.erase(c_it);
					s_it = shape_headers.erase(s_it);
				}
				else
				{
					++c_it;
					++s_it;
				}
			}
		}

		static void _filter_contours_surf_feature_num(std::vector<std::vector<cv::Point>>& contours, const cv::Mat& frame, double surf_hessian_thresh, int surf_feature_num_thresh)
		{
			std::vector<std::vector<cv::Point>>::iterator c_it = contours.begin();
			for (; c_it != contours.end();)
			{
				int surf_pt_num = _get_surf_feature_num_in_contour(frame, (*c_it), surf_hessian_thresh);
				bool check_succ = surf_pt_num >= surf_feature_num_thresh;
				if (!check_succ)
				{
					c_it = contours.erase(c_it);
				}
				else
				{
					++c_it;
				}
			}
		}
	public:
		static bool _is_same_contour(const std::vector<cv::Point>& contour1, const std::vector<cv::Point>& contour2, double dis_thresh)
		{
			double dis_thresh2 = dis_thresh*dis_thresh;
			bool all_same_flag = true;
			for (int i = 0; i < contour1.size(); i++)
			{
				cv::Point pt1 = contour1[i];
				bool same_flag = false;
				for (int j = 0; j < contour2.size(); j++)
				{
					cv::Point pt2 = contour2[j];
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
	private:
		static double _get_dis_avg_L2(const cv::Mat& frame1, const cv::Mat& frame2)
		{
			int width = frame1.cols;
			int height = frame1.rows;
			double result = 0;
			for (int h = 0; h < height; h+=2)
			{
				//const unsigned char* data1 = frame1.ptr<unsigned char>(h);
				//const unsigned char* data2 = frame2.ptr<unsigned char>(h);
				
				const unsigned char* data1 = (const unsigned char*)frame1.data + h*frame1.step;
				const unsigned char* data2 = (const unsigned char*)frame2.data + h*frame2.step;
				for (int w = 0; w < width; w+=2)
				{
					double diff_data = (double)data1[w] - data2[w];
					result += diff_data*diff_data;
				}
			}

			result /= (width*height*0.25 + 1e-10);
			result = sqrt(result);
			return result;
		}
		

		static double _get_dis_hist(const cv::Mat& frame1, const cv::Mat& frame2)
		{
			int channels[1] = { 0 };
			int histSize[] = { 256 };
			float h_ranges[] = { 0, 256 };
			const float* ranges[] = { h_ranges };

			cv::MatND hist_base1;
			clock_t t1 = clock();
			cv::calcHist(&frame1, 1, channels, cv::Mat(), hist_base1, 1, histSize, ranges, true, false);
			cv::normalize(hist_base1, hist_base1, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());
			clock_t t2 = clock();

			cv::MatND hist_base2;
			cv::calcHist(&frame2, 1, channels, cv::Mat(), hist_base2, 1, histSize, ranges, true, false);
			cv::normalize(hist_base2, hist_base2, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());
			clock_t t3 = clock();
			
			double base_base = cv::compareHist(hist_base1, hist_base2, CV_COMP_CORREL);

			return base_base;
		}

		static double _count_same_pixel_ratio(const cv::Mat& frame1, const cv::Mat& frame2)
		{
			int width = frame1.cols;
			int height = frame1.rows;
			/*ZQ_DImage<int> gx(width, height);
			ZQ_DImage<int> gy(width, height);
			ZQ_DImage<bool> flag(width, height);
			int*& gx_data = gx.data();
			int*& gy_data = gy.data();
			bool*& flag_data = flag.data();
			for (int h = 0; h < height; h++)
			{
				const unsigned char* data1 = (const unsigned char*)frame1.data + h*frame1.step;
				for (int w = 0; w < width-1; w++)
				{
					gx_data[h*width + w] = (int)data1[w + 1] - data1[w];
				}
			}
			for (int w = 0; w < width; w++)
			{
				for (int h = 0; h < height-1; h++)
				{
					const unsigned char* data1 = (const unsigned char*)frame1.data + h*frame1.step;
					const unsigned char* data2 = (const unsigned char*)frame1.data + (h+1)*frame1.step;
					gy_data[h*width + w] = (int)data2[w] - data1[w];
				}
			}

			for (int i = 0; i < height*width; i++)
			{
				flag_data[i] = abs(gx_data[i]) > 2 || abs(gy_data[i]) > 2;
			}*/

			int step = frame1.step;
			double result = 0;
			int count = 0;
			for (int h = 0; h < height-1; h+=2)
			{
				const unsigned char* data1 = (const unsigned char*)frame1.data + h*step;
				const unsigned char* data2 = (const unsigned char*)frame2.data + h*step;
				for (int w = 0; w < width-1; w+=2)
				{
					int gx = (int)data1[w + 1] - data1[w];
					int gy = (int)data1[w + step] - data1[w];
					if (abs(gx) > 2 || abs(gy) > 2)
					{
						result += abs((int)data1[w] - data2[w]) <= 2;
						count++;
					}
				}
			}

			result /= (count + 1e-10);
			return result;
		}

		static bool _is_same_shape(const ZQ_SceneCut::Shape& shape1, const ZQ_SceneCut::Shape& shape2, float repeat_dis_thresh, cv::VideoCapture& capture, const ZQ_DetectShapeAndSceneCutOptions& opt)
		{
			if (!capture.isOpened())
				return false;
			long totalFrameNumber = capture.get(CV_CAP_PROP_FRAME_COUNT);
			int width = capture.get(CV_CAP_PROP_FRAME_WIDTH);
			int height = capture.get(CV_CAP_PROP_FRAME_HEIGHT);
			if (shape1.frame_id < 0 || shape1.frame_id >= totalFrameNumber || shape2.frame_id < 0 || shape2.frame_id >= totalFrameNumber)
				return false;
			if (shape1.frame_id == shape2.frame_id)
			{
				return _is_same_contour(shape1.poly, shape2.poly, repeat_dis_thresh);
			}

			cv::Mat frame1, frame2;
			capture.set(CV_CAP_PROP_POS_FRAMES, shape1.frame_id);
			if (!capture.read(frame1))
				return false;
			capture.set(CV_CAP_PROP_POS_FRAMES, shape2.frame_id);
			if (!capture.read(frame2))
				return false;


			int dim = 0;
			int num1 = 0, num2 = 0;
			float* coords1 = 0;
			float* vals1 = 0;
			float* coords2 = 0;
			float* vals2 = 0;

			_get_surf_feature(frame1, shape1, num1, dim, coords1, vals1, opt);
			_get_surf_feature(frame2, shape2, num2, dim, coords2, vals2, opt);

			ZQ_TrackingBoard<float> track(width, height, 1);
			ZQ_TrackingBoardOptions tr_opt;
			tr_opt.ncores = opt.ncores;
			tr_opt.dis1_to_dis2_ratio = 0.6;
			tr_opt.feature_dis_angle = atan(1.0)*0.4;
			std::vector<ZQ_Vec2D> tmp_poly, out_poly;
			double H[9], invH[9];
			if (!track.HandleFirstFrame(num1, dim, vals1, coords1, tmp_poly, tr_opt))
			{
				if (coords1) delete[]coords1;
				if (coords2) delete[]coords2;
				if (vals1) delete[]vals1;
				if (vals2) delete[]vals2;
				return false;
			}

			if (!track.TrackNextFrame(num2, dim, vals2, coords2, H, invH, out_poly, tr_opt))
			{
				if (coords1) delete[]coords1;
				if (coords2) delete[]coords2;
				if (vals1) delete[]vals1;
				if (vals2) delete[]vals2;
				return false;
			}
			if (coords1) delete[]coords1;
			if (coords2) delete[]coords2;
			if (vals1) delete[]vals1;
			if (vals2) delete[]vals2;


			tmp_poly.resize(shape1.poly.size());
			for (int i = 0; i < shape1.poly.size(); i++)
			{
				tmp_poly[i].x = shape1.poly[i].x;
				tmp_poly[i].y = shape1.poly[i].y;

			}

			if (!ZQ_TrackingBoard<float>::Project_polygon_to_dst(H, tmp_poly, out_poly))
				return false;

			std::vector<cv::Point> contour(shape1.poly.size());
			for (int i = 0; i < shape1.poly.size(); i++)
			{
				contour[i].x = out_poly[i].x;
				contour[i].y = out_poly[i].y;
			}

			return _is_same_contour(contour, shape2.poly, repeat_dis_thresh);
		}

		static void _get_surf_feature(const cv::Mat& frame, const ZQ_SceneCut::Shape& shape, int& num, int& dim, float*& coords, float*& vals, const ZQ_DetectShapeAndSceneCutOptions& opt)
		{
			cv::SurfFeatureDetector feature;
			feature.hessianThreshold = 200;
			cv::SurfDescriptorExtractor descript;
			cv::Mat description;

			std::vector<cv::KeyPoint> keypoints;
			cv::Rect _rect = cv::boundingRect(shape.poly);
			int bound_size = 32;
			int dx = _rect.x >= bound_size ? bound_size : _rect.x;
			int dy = _rect.y >= bound_size ? bound_size : _rect.y;
			_rect.width += dx;
			_rect.height += dy;
			_rect.x -= dx;
			_rect.y -= dy;
			_rect.width += __min(bound_size, frame.cols - 1 - _rect.x - _rect.width);
			_rect.height += __min(bound_size, frame.rows - 1 - _rect.y - _rect.height);

			cv::Mat imgROI = frame(_rect);
			feature.detect(imgROI, keypoints);
			std::vector<cv::KeyPoint> out_keypoints;
			ZQ_VirtualAdKeyIO_CV2::Filter_repeat_key(keypoints, out_keypoints, frame.rows, frame.cols);
			descript.compute(frame, out_keypoints, description);

			ZQ_VirtualAdKeyIO_CV2::ConvertSiftSurfKey(out_keypoints, description, num, dim, coords, vals);
		}
	};
}

#endif