#ifndef _ZQ_VIRTUAL_AD_KEY_IO_CV2_H_
#define _ZQ_VIRTUAL_AD_KEY_IO_CV2_H_
#pragma once

#include <vector>
#include <string.h>
#include <opencv2/opencv.hpp>
#include <opencv2/features2d/features2d.hpp>
#include "ZQ_DoubleImage.h"

namespace ZQ
{
	class ZQ_VirtualAdKeyIO_CV2
	{
	public:

		static void Filter_repeat_key(const std::vector<cv::KeyPoint>& input_key, std::vector<cv::KeyPoint>& output_key, int width, int height)
		{
			int input_size = input_key.size();
			output_key.reserve(input_size);
			output_key.clear();
			bool* flag = new bool[width*height];
			memset(flag, 0, sizeof(bool)*width*height);
			for (int i = 0; i < input_size; i++)
			{
				float x = input_key[i].pt.x;
				float y = input_key[i].pt.y;
				int ix = x;
				int iy = y;
				bool already_used = false;
				for (int hh = iy - 1; hh <= iy + 1; hh++)
				{
					for (int ww = ix - 1; ww <= ix + 1; ww++)
					{
						if (hh >= 0 && hh < height && ww >= 0 && ww < width)
						{
							if (flag[hh*width + ww])
							{
								already_used = true;
								break;
							}
						}
					}
				}
				if (!already_used)
				{
					output_key.push_back(input_key[i]);
					for (int hh = iy - 1; hh <= iy + 1; hh++)
					{
						for (int ww = ix - 1; ww <= ix + 1; ww++)
						{
							if (hh >= 0 && hh < height && ww >= 0 && ww < width)
							{
								flag[hh*width + ww] = true;
							}
						}
					}
				}
			}

			delete[]flag;
		}

		static void Filter_repeat_key(const std::vector<cv::KeyPoint>& input_key, std::vector<cv::KeyPoint>& output_key, const ZQ_DImage<bool>& foremask)
		{
			int input_size = input_key.size();
			output_key.reserve(input_size);
			output_key.clear();
			int width = foremask.width();
			int height = foremask.height();
			if (width <= 0 || height <= 0 || foremask.nchannels() <= 0)
				return;

			ZQ_DImage<bool> flag(foremask);
			bool*& flag_data = flag.data();
			
			for (int i = 0; i < input_size; i++)
			{
				float x = input_key[i].pt.x;
				float y = input_key[i].pt.y;
				int ix = x;
				int iy = y;
				bool already_used = false;
				for (int hh = iy - 1; hh <= iy + 1; hh++)
				{
					for (int ww = ix - 1; ww <= ix + 1; ww++)
					{
						if (hh >= 0 && hh < height && ww >= 0 && ww < width)
						{
							if (flag_data[hh*width + ww])
							{
								already_used = true;
								break;
							}
						}
					}
				}
				if (!already_used)
				{
					output_key.push_back(input_key[i]);
					for (int hh = iy - 1; hh <= iy + 1; hh++)
					{
						for (int ww = ix - 1; ww <= ix + 1; ww++)
						{
							if (hh >= 0 && hh < height && ww >= 0 && ww < width)
							{
								flag_data[hh*width + ww] = true;
							}
						}
					}
				}
			}
		}

		/**************** for key file (sift or surf)*****************/
	public:
		static bool SaveSiftSurfKey(const char* file, const std::vector<cv::KeyPoint>& keypoints, const cv::Mat& description)
		{
			if (file == 0)
				return false;
			int len = strlen(file);
			int i;
			for (i = len - 1; i >= 0; i--)
			{
				if (file[i] == '.')
					break;
			}
			if (i < 0)
				return false;
			if (_strcmpi(file + i, ".key") == 0)
			{
				return SaveSiftSurfKey_ascii(file, keypoints, description);
			}
			else if (_strcmpi(file + i, ".keyb") == 0)
			{
				return SaveSiftSurfKey_binary(file, keypoints, description);
			}
			return false;
		}

		static bool SaveSiftSurfKey_ascii(const char* file, const std::vector<cv::KeyPoint>& keypoints, const cv::Mat& description)
		{
			if (keypoints.size() != description.rows)
				return false;
			FILE* out = fopen(file, "w");
			if (out == 0)
				return false;
			int num = keypoints.size();
			int dim = description.cols;
			fprintf(out, "%d %d\n", num, dim);
			for (int p = 0; p < num; p++)
			{
				double tmp_val1 = 0, tmp_val2 = 0;
				fprintf(out, "%.3f %.3f %.3f %.3f\n", keypoints[p].pt.y, keypoints[p].pt.x, tmp_val1, tmp_val2);
				//const int* data = (const int*)(description.data + p * dim * 4);
				const float* data = description.ptr<float>(p);
				float len2 = 0;
				for (int d = 0; d < dim; d++)
				{
					len2 += data[d] * data[d];
				}
				if (len2 != 0)
				{
					float len = sqrt(len2);
					for (int d = 0; d < dim; d++)
					{
						fprintf(out, "%.3f ", data[d] / len);
					}
				}
				else
				{
					for (int d = 0; d < dim; d++)
					{
						fprintf(out, "%.0f ", data[d]);
					}
				}
				fprintf(out, "\n");
			}
			fclose(out);
			return true;
		}

		static bool SaveSiftSurfKey_binary(const char* file, const std::vector<cv::KeyPoint>& keypoints, const cv::Mat& description)
		{
			if (keypoints.size() != description.rows)
				return false;
			FILE* out = fopen(file, "wb");
			if (out == 0)
				return false;
			int num = keypoints.size();
			int dim = description.cols;
			fwrite(&num, sizeof(int), 1, out);
			fwrite(&dim, sizeof(int), 1, out);
			float* tmp_coords = new float[num * 2];
			float* tmp_vals = new float[num*dim];
			unsigned short* final_vals = new unsigned short[num*dim];
			for (int p = 0; p < num; p++)
			{
				tmp_coords[p * 2 + 0] = keypoints[p].pt.x;
				tmp_coords[p * 2 + 1] = keypoints[p].pt.y;
			}
			for (int p = 0; p < num; p++)
			{
				//const int* data = (const int*)(description.data + p * dim * 4);
				const float* data = description.ptr<float>(p);
				for (int d = 0; d < dim; d++)
				{
					tmp_vals[p*dim + d] = data[d];
				}
			}
			for (int p = 0; p < num; p++)
			{
				float len2 = 0;
				for (int d = 0; d < dim; d++)
				{
					len2 += tmp_vals[p*dim + d] * tmp_vals[p*dim + d];
				}
				if (len2 != 0)
				{
					float len = sqrt(len2);
					for (int d = 0; d < dim; d++)
					{
						tmp_vals[p*dim + d] /= len;
					}
				}
			}
			for (int p = 0; p < num*dim; p++)
			{
				final_vals[p] = __min(65535, __max(0, (tmp_vals[p] * 65535 + 0.5)));
			}
			fwrite(tmp_coords, sizeof(float), num * 2, out);
			fwrite(final_vals, sizeof(unsigned short), num * dim, out);
			fclose(out);

			delete[]tmp_coords;
			delete[]tmp_vals;
			delete[]final_vals;
			return true;
		}

	public:
		template<class T>
		static bool ConvertSiftSurfKey(const std::vector<cv::KeyPoint>& keypoints, const cv::Mat& description, int& num, int& dim, T*& coords, T*& vals)
		{
			if (keypoints.size() != description.rows)
				return false;

			num = keypoints.size();
			dim = description.cols;

			if (coords != 0)
				delete[]coords;
			if (vals != 0)
				delete[]vals;
			coords = new T[num * 2];
			vals = new T[num*dim];

			for (int p = 0; p < num; p++)
			{
				coords[p * 2 + 0] = keypoints[p].pt.x;
				coords[p * 2 + 1] = keypoints[p].pt.y;
			}
			
			for (int p = 0; p < num; p++)
			{
				//const int* data = (const int*)(description.data + p * dim * 4);
				const float* data = description.ptr<float>(p);
				for (int d = 0; d < dim; d++)
				{
					vals[p*dim + d] = data[d];
				}
			}
			for (int p = 0; p < num; p++)
			{
				double len2 = 0;
				for (int d = 0; d < dim; d++)
				{
					len2 += vals[p*dim + d] * vals[p*dim + d];
				}
				if (len2 != 0)
				{
					double len = sqrt(len2);
					for (int d = 0; d < dim; d++)
					{
						vals[p*dim + d] /= len;
					}
				}
			}
			return true;
		}
	};
}

#endif