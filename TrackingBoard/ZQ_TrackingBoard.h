#ifndef _ZQ_TRACKING_BOARD_H_
#define _ZQ_TRACKING_BOARD_H_
#pragma once 
#include "ZQ_Vec2D.h"
#include <vector>
#include "ZQ_KDTree.h"
#include "ZQ_MathBase.h"
#include "ZQ_DoubleImage.h"
#include "ZQ_ScanLinePolygonFill.h"
#include "ZQ_TrackingBoardOptions.h"
#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <process.h>

namespace ZQ
{
	template<class T>
	class ZQ_TrackingBoard
	{
		enum CONST_VAL{MAX_REFERENCE_FRAME_NUM = 10};
		enum ClipMode{ CLIPMODE_BOTTOM, CLIPMODE_TOP, CLIPMODE_LEFT, CLIPMODE_RIGHT };
		class _FrameInfo
		{
		public:
			_FrameInfo()
			{
				key_num = 0;
				key_dim = 0;
				key_vals = 0;
				key_coords = 0;
			}
			~_FrameInfo()
			{
				Clear();
			}

			void Clear()
			{
				if (key_vals) delete[]key_vals; key_vals = 0;
				if (key_coords)	delete[]key_coords; key_coords = 0;
				key_num = 0;
				key_dim = 0;
			}

			void SetValue(int key_num, int key_dim, const T* key_vals, const T* key_coords)
			{
				Clear();
				this->key_num = key_num;
				this->key_dim = key_dim;
				if (key_num > 0)
				{
					this->key_vals = new T[key_num*key_dim];
					memcpy(this->key_vals, key_vals, sizeof(T)*key_dim*key_num);
					this->key_coords = new T[key_num*key_dim];
					memcpy(this->key_coords, key_coords, sizeof(T)* 2 * key_num);
				}
			}

			bool HomographyTransform(const double H[9])
			{
				T* tmp_in_key_coords = new T[key_num * 3];
				T* tmp_out_key_coords = new T[key_num * 3];
				for (int i = 0; i < key_num; i++)
				{
					tmp_in_key_coords[i * 3 + 0] = key_coords[i * 2 + 0];
					tmp_in_key_coords[i * 3 + 1] = key_coords[i * 2 + 2];
					tmp_in_key_coords[i * 3 + 2] = 1;
				}
				for (int i = 0; i < 3; i++)
				{
					for (int j = 0; j < key_num; j++)
					{
						tmp_out_key_coords[i*key_num + j] = 0;
						for (int k = 0; k < 3; k++)
							tmp_out_key_coords[i*key_num + j] += H[i * 3 + k] * tmp_in_key_coords[k*key_num + j];
					}
				}

				delete[]tmp_in_key_coords;
				for (int i = 0; i < key_num; i++)
				{
					double tmp_val = tmp_out_key_coords[i * 3 + 2];
					if (tmp_val == 0)
					{
						delete[]tmp_out_key_coords;
						return false;
					}
					tmp_out_key_coords[i * 3 + 0] /= tmp_val;
					tmp_out_key_coords[i * 3 + 1] /= tmp_val;
				}
				for (int i = 0; i < key_num; i++)
				{
					key_coords[i * 2 + 0] = tmp_out_key_coords[i * 3 + 0];
					key_coords[i * 2 + 1] = tmp_out_key_coords[i * 3 + 1];
				}
				delete[]tmp_out_key_coords;
				return true;
			}
			
			bool BruteForceSearch(const T* pt, int out_idx[2], double out_dis2[2]) const
			{
				if (key_coords == 0 || key_vals == 0 || key_num < 2 || key_dim <= 0)
					return false;

				int min_id1 = -1;
				double min_dis1 = 0;
				int min_id2 = -1;
				double min_dis2 = 0;
				for (int i = 0; i < key_num; i++)
				{
					double tmp_dis2 = _distance2(pt, key_vals+i*key_dim, key_dim);
					if (min_id1 == -1)
					{
						min_id1 = i;
						min_dis1 = tmp_dis2;
					}
					else
					{
						if (min_dis1 > tmp_dis2)
						{
							min_id1 = i;
							min_dis1 = tmp_dis2;
						}
					}
				}
				for (int i = 0; i < key_num; i++)
				{
					if (i == min_id1)
						continue;
					double tmp_dis2 = _distance2(pt, key_vals + i*key_dim, key_dim);
					if (min_id2 == -1)
					{
						min_id2 = i;
						min_dis2 = tmp_dis2;
					}
					else
					{
						if (min_dis2 > tmp_dis2)
						{
							min_id2 = i;
							min_dis2 = tmp_dis2;
						}
					}
				}
				out_idx[0] = min_id1;
				out_idx[1] = min_id2;
				out_dis2[0] = min_dis1;
				out_dis2[1] = min_dis2;
				return true;
			}
			
			const T* GetKeyCoordsPtr() const{ return key_coords; }
			const T* GetKeyValuesPtr() const { return key_vals; }
			const int GetKeyNum()const{ return key_num; }
			const int GetKeyDim()const{ return key_dim; }
		private:
			int key_num;
			int key_dim;
			T* key_vals;
			T* key_coords;

		private:
			static double _distance2(const T* pt1, const T* pt2, int key_dim)
			{
				double sum = 0;
				for (int i = 0; i < key_dim; i++)
				{
					double diff = pt1[i] - pt2[i];
					sum += diff*diff;
				}
				return sum;
			}
		};
		
		/*************************************************************************************************************/

	public:
		ZQ_TrackingBoard(int _width, int _height, int _refer_num) :width(_width), height(_height), cur_last_N_num(0)
		{
			REFERENCE_FRAME_NUM = __max(1, __min(MAX_REFERENCE_FRAME_NUM, _refer_num));
			scale = __max(0.125,__min(1.0, __max(480.0 / width, 270.0 / height)));
			memset(lastNframes, 0, sizeof(_FrameInfo*)*REFERENCE_FRAME_NUM);
		}
		~ZQ_TrackingBoard()
		{
			for (int i = 0; i < REFERENCE_FRAME_NUM; i++)
			{
				if (lastNframes[i])
					delete lastNframes[i];
				lastNframes[i] = 0;
			}
		}

	private:
		int width, height;
		std::vector<ZQ_Vec2D> polygon;
		double H[9];
		double invH[9];
		_FrameInfo* lastNframes[MAX_REFERENCE_FRAME_NUM];
		int REFERENCE_FRAME_NUM;
		int cur_last_N_num;
		double scale;
		std::vector<T> src_coords;
		std::vector<T> dst_coords;
		ZQ_DImage<bool> ransac_mask;
		
	public:
		
		bool HandleFirstFrame(int key_num, int key_dim, const T* key_vals, const T* key_coords, const std::vector<ZQ_Vec2D>& polygon, const ZQ_TrackingBoardOptions& opt)
		{
			if (key_num <= 0 || key_dim <= 0 || key_vals == 0 || key_coords == 0)
				return false;
			if (cur_last_N_num != 0)
			{
				for (int i = 0; i < cur_last_N_num; i++)
				{
					if (lastNframes[i])
						delete lastNframes[i];
					lastNframes[i] = 0;
				}
			}
			_FrameInfo* cur_frame = _assemble_frame_info(key_num, key_dim, key_vals, key_coords, polygon, opt.max_keep_key_num, width, height, scale);
			this->polygon = polygon;
			lastNframes[0] = cur_frame;
			cur_last_N_num = 1;
			return true;
		}

		bool TrackNextFrame(int key_num, int key_dim, const T* key_vals, const T* key_coords, double Hmat[9], double invHmat[9], std::vector<ZQ_Vec2D>& polygon,  const ZQ_TrackingBoardOptions& opt)
		{
			if (cur_last_N_num == 0)
				return false;
			if (key_num == 0 || key_dim <= 0 || key_vals == 0 || key_coords == 0)
				return false;

			_FrameInfo* cur_frame = _assemble_frame_info(key_num, key_dim, key_vals,key_coords);

			src_coords.clear();
			dst_coords.clear();
			ransac_mask.clear();
			clock_t t1 = clock();
			if (!_select_match_key(lastNframes[cur_last_N_num - 1], cur_frame, src_coords, dst_coords, opt))
			{
				delete cur_frame;
				return false;
			}
			clock_t t2 = clock();
			
			int match_num = src_coords.size() / 2;

			if (opt.display_running_info)
				printf("select match: %.3f, %d points matched\n", 0.001*(t2 - t1), match_num);

			ransac_mask.allocate(1, match_num);
			bool*& mask_data = ransac_mask.data();
			double reproj_err_thresh = opt.reproj_err_thresh; //suggest 1.0
			int ransac_iter = opt.ransac_iter;	//suggest 2000
			int rasac_inner_iter = opt.ransac_inner_iter; //suggest 1
			double confidence = opt.ransac_confidence; //suggest 0.995
			int levmar_iter = opt.levmar_iter;	//suggest 300
			double eps = opt.eps;	//suggest 1e-9

			clock_t t3 = clock();
			if (match_num == 0 || !_find_homography_RANSAC(match_num, &src_coords[0], &dst_coords[0], H, invH, mask_data, reproj_err_thresh, ransac_iter,rasac_inner_iter, confidence, levmar_iter, eps))
			{
				delete cur_frame;
				return false;
			}
			clock_t t4 = clock();
			if (opt.display_running_info)
				printf("ransac cost: %.3f\n", 0.001*(t4 - t3));
			
			memcpy(Hmat, H, sizeof(double)* 9);
			memcpy(invHmat, invH, sizeof(double)* 9);
			Project_polygon_to_dst(H, this->polygon, polygon);
			this->polygon = polygon;
			delete cur_frame;
			clock_t t5 = clock();
			cur_frame = _assemble_frame_info(key_num, key_dim, key_vals, key_coords, polygon, opt.max_keep_key_num, width, height,scale);
			if (cur_last_N_num < REFERENCE_FRAME_NUM-1)
			{
				for (int i = 0; i < cur_last_N_num; i++)
				{
					if (!lastNframes[i]->HomographyTransform(H))
					{
						delete cur_frame;
						return false;
					}
					
				}
				lastNframes[cur_last_N_num++] = cur_frame;
			}
			else
			{
				for (int i = 0; i < cur_last_N_num; i++)
				{
					if (!lastNframes[i]->HomographyTransform(H))
					{
						delete cur_frame;
						return false;
					}
				}
				delete lastNframes[0];
				for (int i = 0; i < cur_last_N_num-1; i++)
				{
					lastNframes[i] = lastNframes[i + 1];	
				}
				lastNframes[cur_last_N_num-1] = cur_frame;
			}
			clock_t t6 = clock();
			if (opt.display_running_info)
				printf("assemble cost : %.3f\n", 0.001*(t6 - t5));
			return true;
		}

		void GetMatchCoords(int& num, T*& src_coords, T*& dst_coords, bool*& mask) const
		{
			num = this->src_coords.size() / 2;
			if (num > 0)
			{
				if (src_coords)
					delete[]src_coords;
				if (dst_coords)
					delete[]dst_coords;
				if (mask)
					delete[]mask;
				src_coords = new T[num * 2];
				dst_coords = new T[num * 2];
				mask = new bool[num];
				memcpy(src_coords, &this->src_coords[0], sizeof(T)*num * 2);
				memcpy(dst_coords, &this->dst_coords[0], sizeof(T)*num * 2);
				memcpy(mask, this->ransac_mask.data(), sizeof(bool)*num);
			}
		}

		static bool Project_with_H(const double* H, const T* src_pt, T* dst_pt)
		{
			double src_pt_1[3] = { src_pt[0], src_pt[1], 1 };
			double dst_pt_1[3];
			ZQ_MathBase::MatrixMul(H, src_pt_1, 3, 3, 1, dst_pt_1);
			if (dst_pt_1[2] == 0)
				return false;
			dst_pt[0] = dst_pt_1[0] / dst_pt_1[2];
			dst_pt[1] = dst_pt_1[1] / dst_pt_1[2];
			return true;
		}

		static bool Project_polygon_to_dst(const double H[9], const std::vector<ZQ_Vec2D>& polygon, std::vector<ZQ_Vec2D>& dst_polygon)
		{
			int num = polygon.size();
			dst_polygon.resize(num);
			for (int i = 0; i < num; i++)
			{
				T src_pt[2] = { polygon[i].x, polygon[i].y };
				T dst_pt[2];
				if (!Project_with_H(H, src_pt, dst_pt))
					return false;
				dst_polygon[i].x = dst_pt[0];
				dst_polygon[i].y = dst_pt[1];
			}
			return true;
		}

		/**********************  for assemble frame info   *************************/
	private:
		static _FrameInfo* _assemble_frame_info(int key_num, int key_dim, const T* key_vals, const T* key_coords)
		{
			_FrameInfo* cur_frame = new _FrameInfo();
			cur_frame->SetValue(key_num, key_dim, key_vals, key_coords);
			return cur_frame;
		}

		static _FrameInfo* _assemble_frame_info(int key_num, int key_dim, const T* key_vals, const T* key_coords, const std::vector<ZQ_Vec2D>& polygon, int max_keep_key_num, int width, int height, double scale)
		{
			_FrameInfo* cur_frame = new _FrameInfo();

			int filtered_num = 0;
			T* filtered_coords = 0;
			T* filtered_vals = 0;
			if (!_filter_key(polygon, key_num, key_dim, key_coords, key_vals, filtered_num, filtered_coords, filtered_vals, max_keep_key_num, width, height, scale))
				cur_frame->SetValue(key_num, key_dim, key_vals, key_coords);
			else
			{
				cur_frame->SetValue(filtered_num, key_dim, filtered_vals, filtered_coords);
				delete[]filtered_coords;
				delete[]filtered_vals;
				filtered_coords = 0;
				filtered_vals = 0;
				filtered_num = 0;
			}
			return cur_frame;
		}
		
		/**********************  for filtering key   *************************/
	private:
		static bool _gen_image_mask(const std::vector<ZQ_Vec2D>& polygon, ZQ_DImage<bool>& image_mask, int width, int height, double scale)
		{
			std::vector<ZQ_Vec2D> cliped_poly;
			if (!ZQ_ScanLinePolygonFill::ClipPolygon(polygon, width, height, cliped_poly))
				return false;
			if (cliped_poly.size() < 3)
			{
				image_mask.allocate(width, height);
				return true;
			}
				
			if (scale == 1)
			{
				std::vector<ZQ_Vec2D> pixels;
				pixels.reserve(width*height);
				ZQ_ScanLinePolygonFill::ScanLinePolygonFill(cliped_poly, pixels);
				image_mask.allocate(width, height);
				bool*& image_mask_data = image_mask.data();
				for (int p = 0; p < pixels.size(); p++)
				{
					int y = pixels[p].y;
					int x = pixels[p].x;
					if (y < 0 || y > height - 1 || x < 0 || x > width - 1)
						continue;
					image_mask_data[y*width + x] = true;
				}
			}
			else
			{
				std::vector<ZQ_Vec2D> new_polygon = cliped_poly;
				for (int i = 0; i < new_polygon.size(); i++)
				{
					new_polygon[i].x *= scale;
					new_polygon[i].y *= scale;
				}
				int new_width = width*scale;
				int new_height = height*scale;

				std::vector<ZQ_Vec2D> pixels;
				pixels.reserve(new_width*new_height);
				ZQ_ScanLinePolygonFill::ScanLinePolygonFill(new_polygon, pixels);				
				image_mask.allocate(new_width, new_height);
				bool*& image_mask_data = image_mask.data();
				for (int p = 0; p < pixels.size(); p++)
				{
					int y = pixels[p].y;
					int x = pixels[p].x;
					if (y < 0 || y > new_height - 1 || x < 0 || x > new_width - 1)
						continue;
					image_mask_data[y*new_width + x] = true;
				}
			}
			return true;
		}

		static void _filter_key(const std::vector<ZQ_Vec2D>& poly, const int key_num, const T* coords, bool* keep_mask, int width, int height, double scale)
		{
			for (int i = 0; i < key_num; i++)
				keep_mask[i] = true;
			ZQ_DImage<bool> image_mask;
			if (!_gen_image_mask(poly, image_mask, width, height, scale))
				return;
			
			bool*& image_mask_data = image_mask.data();
			int mask_width = image_mask.width();
			for (int i = 0; i < key_num; i++)
			{
				int x = coords[i * 2]*scale + 0.5;
				int y = coords[i * 2 + 1]*scale + 0.5;
				if (x < 0 || x > width - 1 || y < 0 || y > height - 1)
				{
					keep_mask[i] = false;
					continue;
				}
				keep_mask[i] = image_mask_data[y*mask_width + x];
			}
		}

		static bool _filter_key(const std::vector<ZQ_Vec2D>& poly, const int key_num, const int key_dim, const T* coords, const T* vals, int& out_num, T*& out_coords, T*& out_vals, 
			int max_keep_key_num, int width, int height, double scale)
		{
			if (key_num <= 0)
				return false;
			bool* keep_mask = new bool[key_num];
			_filter_key(poly, key_num, coords, keep_mask, width, height, scale);
			out_num = 0;
			for (int i = 0; i < key_num; i++)
			{
				out_num += keep_mask[i];
			}
			int* keep_id = new int[out_num];
			int cur_id = 0;
			for (int i = 0; i < key_num; i++)
			{
				if (keep_mask[i])
				{
					keep_id[cur_id] = i;
					cur_id++;
				}
			}
			out_num = cur_id;
			if (out_num > max_keep_key_num)
			{
				for (int p = 0; p < max_keep_key_num; p++)
				{
					int cur_p = rand() % (out_num - p);
					if (cur_p != 0)
					{
						int tmp_p = keep_id[p];
						keep_id[p] = keep_id[cur_p + p];
						keep_id[cur_p + p] = tmp_p;
					}
				}
				out_num = max_keep_key_num;
			}
			if (out_coords)
				delete[]out_coords;
			out_coords = new T[out_num * 2];
			if (out_vals)
				delete[]out_vals;
			out_vals = new T[out_num*key_dim];
			for (int i = 0; i < out_num; i++)
			{
				int cur_p = keep_id[i];
				memcpy(out_coords + i * 2, coords + cur_p * 2, sizeof(T)* 2);
				memcpy(out_vals + i*key_dim, vals + cur_p*key_dim, sizeof(T)*key_dim);
			}
			delete[]keep_mask;
			delete[]keep_id;
			return true;
		}

		/****************************** for select match *******************************/
	private:	
		struct _select_match_search_arg
		{
			const _FrameInfo* last_frame;
			int key_dim;
			int start_id;
			int end_id;
			const T* dst_vals;
			int* out_idx;
			double* out_dis2;
			bool done_flag;
		};
		static void _select_match_search(void* para)
		{
			_select_match_search_arg* arg = (_select_match_search_arg*)para;
			int key_dim = arg->key_dim;
			for (int i = arg->start_id; i <= arg->end_id; i++)
			{
				const T* tmp_pt = arg->dst_vals + key_dim*i;
				arg->last_frame->BruteForceSearch(tmp_pt, arg->out_idx + i * 2, arg->out_dis2 + i * 2);
			}
			arg->done_flag = true;
		}

		static bool _select_match_key(const _FrameInfo* last_frame, const _FrameInfo* cur_frame, std::vector<T>& src_coords, std::vector<T>& dst_coords, const ZQ_TrackingBoardOptions& opt)
		{
			if (last_frame == 0 || cur_frame == 0 || cur_frame->GetKeyDim() != last_frame->GetKeyDim())
				return false;

			double ratio = opt.dis1_to_dis2_ratio; //suggest 0.6
			double angle = opt.feature_dis_angle;  //suggest 0.05*pi
			double real_fixed_radius = 2 * sin(angle*0.5);
			double real_fixed_radius2 = real_fixed_radius*real_fixed_radius;
			double fixed_radius = 2 * sin(angle*0.5) / sqrt(ratio);

			src_coords.clear();
			dst_coords.clear();
			
			std::vector<int> lastFramePtId;
			std::vector<int> curFramePtId;
			
			const T* tmp_src_vals = last_frame->GetKeyValuesPtr();
			const T* tmp_src_coords = last_frame->GetKeyCoordsPtr();
			const T* tmp_dst_vals = cur_frame->GetKeyValuesPtr();
			const T* tmp_dst_coords = cur_frame->GetKeyCoordsPtr();
			int dst_num = cur_frame->GetKeyNum();
			int key_dim = cur_frame->GetKeyDim();
			int* out_idx = new int[dst_num * 2];
			double* out_dis2 = new double[dst_num * 2];
			memset(out_idx, 0, sizeof(int)*dst_num * 2);
			memset(out_dis2, 0, sizeof(double)*dst_num * 2);
			if (opt.ncores <= 1)
			{
				for (int i = 0; i < dst_num; i++)
				{
					const T* tmp_pt = tmp_dst_vals + key_dim*i;
					last_frame->BruteForceSearch(tmp_pt, out_idx + i * 2, out_dis2 + i * 2);
				}
			}
			else
			{
				_select_match_search_arg* args = new _select_match_search_arg[opt.ncores];
				int each_num = (dst_num - 1) / opt.ncores + 1;
				for (int i = 0; i < opt.ncores; i++)
				{
					args[i].last_frame = last_frame;
					args[i].key_dim = key_dim;
					args[i].start_id = each_num*i;
					args[i].end_id = __min(dst_num-1, each_num*(i + 1)-1);
					args[i].dst_vals = tmp_dst_vals;
					args[i].out_idx = out_idx;
					args[i].out_dis2 = out_dis2;
					args[i].done_flag = false;
				}
				for (int i = 0; i < opt.ncores - 1; i++)
					_beginthread(_select_match_search, 0, args + i);
				_select_match_search(args + opt.ncores - 1);

				bool all_done = false;
				while (!all_done)
				{
					all_done = true;
					for (int i = 0; i < opt.ncores; i++)
					{
						if (!args[i].done_flag)
						{
							all_done = false;
							break;
						}
					}
					if (all_done)
						break;
				}
				delete[]args;
			}
			
			for (int i = 0; i < dst_num; i++)
			{
				if (out_dis2[i*2+0] < ratio*out_dis2[i*2+1] && out_dis2[i*2+0] < real_fixed_radius2)
				{
					src_coords.push_back(tmp_src_coords[out_idx[i*2+0] * 2]);
					src_coords.push_back(tmp_src_coords[out_idx[i*2+0] * 2 + 1]);
					dst_coords.push_back(tmp_dst_coords[i * 2]);
					dst_coords.push_back(tmp_dst_coords[i * 2 + 1]);
					lastFramePtId.push_back(out_idx[i*2+0]);
					curFramePtId.push_back(i);
				}
			}
			delete[]out_idx;
			delete[]out_dis2;
			return true;
		}

		/************************** for ransac **************************************/
	private: 
		static void _random_select_subset(int min_num, int n_pts, const T* src_pts, const T* dst_pts, T* sub_src_pts, T* sub_dst_pts)
		{
			int* idx = new int[n_pts];
			for (int i = 0; i < n_pts; i++)
			{
				idx[i] = i;
			}
			for (int pass = 0; pass < min_num; pass++)
			{
				int cur_id = rand() % (n_pts - pass) + pass;
				if (cur_id != pass)
				{
					int tmp = idx[pass];
					idx[pass] = idx[cur_id];
					idx[cur_id] = tmp;
				}
			}

			for (int i = 0; i < min_num; i++)
			{
				sub_src_pts[i * 2 + 0] = src_pts[idx[i] * 2 + 0];
				sub_src_pts[i * 2 + 1] = src_pts[idx[i] * 2 + 1];
				sub_dst_pts[i * 2 + 0] = dst_pts[idx[i] * 2 + 0];
				sub_dst_pts[i * 2 + 1] = dst_pts[idx[i] * 2 + 1];
			}
			delete[]idx;
		}

		static double _distance2_two_pts(const T* pt1, const T* pt2)
		{
			return (pt1[0] - pt2[0])*(pt1[0] - pt2[0]) + (pt1[1] - pt2[1])*(pt1[1] - pt2[1]);
		}

		static int _update_ransac_iters(double confidence, double outlier_ratio, int n_pts, int max_iters)
		{
			if (n_pts <= 0)
				return -1;

			double p = __min(1.0, __max(0.0, confidence));
			double ep = __min(1.0, __max(0.0, outlier_ratio));

			const double min_double_val = 1e-32;
			double num = __max(1.0 - p, min_double_val);
			double denom = 1.0 - pow(1.0 - ep, n_pts);
			if (denom < min_double_val)
				return 0;

			num = log(num);
			denom = log(denom);

			return denom >= 0 || -num >= max_iters*(-denom) ? max_iters : (num / denom + 0.5);
		}

		static bool _computeH(double H[9], int num, const T* src_pts, const T* dst_pts, int levmar_iter, double eps)
		{
			if (num < 4)
				return false;

			CvMat* src_mat = cvCreateMat(num, 2, CV_64FC1);
			CvMat* dst_mat = cvCreateMat(num, 2, CV_64FC1);
			CvMat* H_mat = cvCreateMat(3, 3, CV_64FC1);
			for (int i = 0; i < num; i++)
			{
				cvSetReal2D(src_mat, i, 0, src_pts[i * 2 + 0]);
				cvSetReal2D(src_mat, i, 1, src_pts[i * 2 + 1]);

				cvSetReal2D(dst_mat, i, 0, dst_pts[i * 2 + 0]);
				cvSetReal2D(dst_mat, i, 1, dst_pts[i * 2 + 1]);

			}
			cvFindHomography(src_mat, dst_mat, H_mat);
			for (int i = 0; i < 3; i++)
			{
				for (int j = 0; j < 3; j++)
					H[i * 3 + j] = cvGetReal2D(H_mat, i, j);
			}
			cvReleaseMat(&src_mat);
			cvReleaseMat(&dst_mat);
			cvReleaseMat(&H_mat);
			return true;

		}

		/* dst' = s*H*src' */
		static bool _find_homography_RANSAC(int n_pts, const T* src_pts, const T* dst_pts, double H[9], double invH[9], bool* mask, double reproj_err_thesh, int ransac_iter, int ransac_inner_iter, double confidence, int levmar_iter, double eps)
		{
			const int min_num = 4; //infact 4
			if (n_pts < min_num)
				return false;
			bool has_result = false;
			int nIter = ransac_iter;
			int nInner_Iter = ransac_inner_iter;
			T sub_src_pts[min_num * 2];
			T sub_dst_pts[min_num * 2];
			bool* tmp_mask = new bool[n_pts];
			double tmp_H[9];
			T tmp_pt[2];
			int count = 0;
			int tmp_count;
			for (int it = 0; it < nIter; it++)
			{

				//printf("%d/%d\n", it, nIter);
				_random_select_subset(min_num, n_pts, src_pts, dst_pts, sub_src_pts, sub_dst_pts);
				memset(tmp_H, 0, sizeof(double)* 9);
				if (!_computeH(tmp_H, min_num, sub_src_pts, sub_dst_pts, levmar_iter, eps))
					continue;

				
				bool reproj_err_flag = false;
				tmp_count = 0;
				for (int in_it = 0; in_it < nInner_Iter; in_it++)
				{
					tmp_count = 0;
					for (int p = 0; p < n_pts; p++)
					{
						if (!Project_with_H(tmp_H, src_pts + p * 2, tmp_pt))
						{
							reproj_err_flag = true;
							break;
						}
						double dis2 = _distance2_two_pts(tmp_pt, dst_pts + p * 2);
						tmp_mask[p] = dis2 <= reproj_err_thesh*reproj_err_thesh;
						tmp_count += tmp_mask[p];
					}
					if (reproj_err_flag)
						break;

					if (tmp_count > min_num)
					{
						T* inlier_src_pts = new T[tmp_count * 2];
						T* inlier_dst_pts = new T[tmp_count * 2];
						int cur_id = 0;
						for (int i = 0; i < n_pts; i++)
						{
							if (tmp_mask[i])
							{
								inlier_src_pts[cur_id * 2 + 0] = src_pts[i * 2 + 0];
								inlier_src_pts[cur_id * 2 + 1] = src_pts[i * 2 + 1];
								inlier_dst_pts[cur_id * 2 + 0] = dst_pts[i * 2 + 0];
								inlier_dst_pts[cur_id * 2 + 1] = dst_pts[i * 2 + 1];
								cur_id++;
							}
						}

						if (!_computeH(tmp_H, tmp_count, inlier_src_pts, inlier_dst_pts, levmar_iter, eps))
						{
							has_result = false;
						}
						delete[]inlier_src_pts;
						delete[]inlier_dst_pts;
					}
				}
				if (reproj_err_flag)
					continue;
				
				//printf("tmp_count/count = %d/%d\n", tmp_count, count);
				if (tmp_count > count)
				{
					count = tmp_count;
					memcpy(mask, tmp_mask, sizeof(bool)*n_pts);
					memcpy(H, tmp_H, sizeof(double)* 9);
					//printf("2.");
					int tmp_iter = _update_ransac_iters(confidence, 1.0 - (double)tmp_count / n_pts, min_num, ransac_iter);
					//printf("3.");
					
					nIter = tmp_iter;
					has_result = true;
				}
				//printf("4.\n");		
			}
			if (has_result)
			{
				T* inlier_src_pts = new T[count * 2];
				T* inlier_dst_pts = new T[count * 2];
				int cur_id = 0;
				for (int i = 0; i < n_pts; i++)
				{
					if (mask[i])
					{
						inlier_src_pts[cur_id * 2 + 0] = src_pts[i * 2 + 0];
						inlier_src_pts[cur_id * 2 + 1] = src_pts[i * 2 + 1];
						inlier_dst_pts[cur_id * 2 + 0] = dst_pts[i * 2 + 0];
						inlier_dst_pts[cur_id * 2 + 1] = dst_pts[i * 2 + 1];
						cur_id++;
					}
				}

				if (!_computeH(H, count, inlier_src_pts, inlier_dst_pts, levmar_iter, eps))
				{
					has_result = false;
				}
				if (!_computeH(invH, count, inlier_dst_pts, inlier_src_pts, levmar_iter, eps))
				{
					has_result = false;
				}
				delete[]inlier_src_pts;
				delete[]inlier_dst_pts;
			}
			delete[]tmp_mask;
			return has_result;
		}
	};
}

#endif