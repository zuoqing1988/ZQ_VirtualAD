#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/nonfree/nonfree.hpp>
#include <vector>
#include <time.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <process.h>
#include "ZQ_ExtractSiftSurfOptions.h"
#include "ZQ_DoubleImage.h"
#include "ZQ_ImageIO.h"

#define ZQ_LINK_OPENCV_VERSION_2413
#include "ZQ_Link_OpenCV_Lib.h"

using namespace cv;
using namespace std;
using namespace ZQ;

void Filter_repeat_key(const vector<KeyPoint>& input_key, vector<KeyPoint>& output_key, int width, int height);
void Filter_repeat_key(const vector<KeyPoint>& input_key, vector<KeyPoint>& output_key, const ZQ_DImage<bool>& foremask);
bool SaveKey(const char* file, const vector<KeyPoint>& keypoints, const Mat& description);
bool SaveKey_ascii(const char* file, const vector<KeyPoint>& keypoints, const Mat& description);
bool SaveKey_binary(const char* file, const vector<KeyPoint>& keypoints, const Mat& description);
void handle_sequence(const ZQ_ExtractSiftSurfOptions& opt);
void _handle_sequence_thread(void* arg);
int main_sequence(int argc, const char** argv);
int main_single(int argc, const char** argv);

int main(int argc, const char** argv)
{
	if (argc < 2)
	{
		printf(".exe single imagefile keyfile\n");
		printf(".exe sequence [opts]\n");
		return -1;
	}

	if (_strcmpi(argv[1], "single") == 0)
	{
		return main_single(argc - 1, argv + 1);
	}
	else if (_strcmpi(argv[1], "sequence") == 0)
	{
		return main_sequence(argc - 1, argv + 1);
	}
	else
	{
		printf(".exe single imagefile keyfile\n");
		printf(".exe sequence [opts]\n");
		return -1;
	}
}

int main_sequence(int argc, const char** argv)
{
	ZQ_ExtractSiftSurfOptions opt;
	if (!opt.HandleArgs(argc - 1, argv + 1))
	{
		return -1;
	}
	clock_t t1 = clock();
	if (opt.ncores <= 1)
	{
		handle_sequence(opt);
		
	}
	else
	{
		ZQ_ExtractSiftSurfOptions* opts = new ZQ_ExtractSiftSurfOptions[opt.ncores];
		int each_num = ceil((float)opt.frame_num/opt.ncores);
		for (int i = 0; i < opt.ncores; i++)
		{
			opts[i] = opt;
			opts[i].base_id = opt.base_id + each_num*i;
			opts[i].frame_num = __min(each_num, opt.base_id + opt.frame_num - opts[i].base_id);
			opts[i].done_flag = false;
		}
		
		for (int i = 0; i < opt.ncores - 1;i++)
			_beginthread(_handle_sequence_thread, 0, opts + i);
		_handle_sequence_thread(opts + opt.ncores - 1);
		bool all_done = false;
		while (!all_done)
		{
			all_done = true;
			for (int i = 0; i < opt.ncores; i++)
			{
				if (!opts[i].done_flag)
				{
					all_done = false;
				}
			}
		}
	}
	clock_t t2 = clock();
	printf("total cost: %.3f secs\n", 0.001*(t2 - t1));
	return 0;
}

int main_single(int argc, const char** argv)
{
	if (argc < 3)
		return -1;

	clock_t t1 = clock();
	const char* imagefile = argv[1];
	const char* keyfile = argv[2];
	Mat image = imread(imagefile, 1);

	if (image.empty())
	{
		printf("failed to load %s \n", imagefile);
		return -1;
	}


	ZQ_ExtractSiftSurfOptions opt;
	if (!opt.HandleArgs(argc - 3, argv + 3))
		return -1;

	if (opt.methodType == ZQ_ExtractSiftSurfOptions::METHOD_SIFT)
	{
		SiftFeatureDetector feature(0, 3, opt.sift_contrast_thresh, 10, 1.6);

		vector<KeyPoint> keypoints;
		vector<KeyPoint> out_keypoints;

		clock_t t2 = clock();
		feature.detect(image, keypoints);
		Filter_repeat_key(keypoints, out_keypoints, image.rows, image.cols);
		clock_t t3 = clock();

		SiftDescriptorExtractor descript;

		Mat description;
		descript.compute(image, out_keypoints, description);

		clock_t t4 = clock();
		if (!SaveKey(keyfile, out_keypoints, description))
		{
			printf("failed to save sift keys to file %s\n", keyfile);
			return -1;
		}
		clock_t t5 = clock();
		float load_save_time = 0.001*(t2 - t1) + 0.001*(t5 - t4);
		float detect_time = 0.001*(t3 - t2);
		float extract_time = 0.001*(t4 - t3);
		printf("%d key points found!\n", out_keypoints.size());
		printf("cost %.3f secs(load&save: %.3f, detect: %.3f, extract: %.3f)\n", 0.001*(t5 - t1), load_save_time, detect_time, extract_time);
	}
	else if (opt.methodType == ZQ_ExtractSiftSurfOptions::METHOD_SURF)
	{
		SurfFeatureDetector feature;
		feature.hessianThreshold = opt.surf_hessian_thresh;

		vector<KeyPoint> keypoints;
		vector<KeyPoint> out_keypoints;

		clock_t t2 = clock();
		feature.detect(image, keypoints);
		Filter_repeat_key(keypoints, out_keypoints, image.rows, image.cols);
		clock_t t3 = clock();
		SurfDescriptorExtractor descript;

		Mat description;
		descript.compute(image, out_keypoints, description);

		clock_t t4 = clock();
		if (!SaveKey(keyfile, out_keypoints, description))
		{
			printf("failed to save sift keys to file %s\n", keyfile);
			return -1;
		}
		clock_t t5 = clock();
		float load_save_time = 0.001*(t2 - t1) + 0.001*(t5 - t4);
		float detect_time = 0.001*(t3 - t2);
		float extract_time = 0.001*(t4 - t3);
		printf("%d key points found!\n", out_keypoints.size());
		printf("cost %.3f secs(load&save: %.3f, detect: %.3f, extract: %.3f)\n", 0.001*(t5 - t1), load_save_time, detect_time, extract_time);
	}
	else
	{
		printf("unknown method!\n");
		return -1;
	}
	return 0;
}

void _handle_sequence_thread(void* arg)
{
	ZQ_ExtractSiftSurfOptions* opt = (ZQ_ExtractSiftSurfOptions*)arg;
	handle_sequence(*opt);
	opt->done_flag = true;
}


void handle_sequence(const ZQ_ExtractSiftSurfOptions& opt)
{
	char imagefile[200];
	char keyfile[200];
	char foremaskfile[200];
	for (int i = opt.base_id; i < opt.base_id+opt.frame_num; i++)
	{
		clock_t t1 = clock();
		sprintf(imagefile, "%s\\%d.%s", opt.image_fold, i, opt.image_suffix);
		sprintf(keyfile, "%s\\%d.%s", opt.key_fold, i, opt.key_suffix);

		ZQ_DImage<bool> foremask;
		if (opt.has_foremask)
		{
			sprintf(foremaskfile, "%s\\%d.%s", opt.foremask_fold, i, opt.foremask_suffix);
			if (!ZQ_ImageIO::loadImage(foremask, foremaskfile, 0))
			{
				printf("failed to load %s\n", foremaskfile);
				continue;
			}
		}

		Mat image = imread(imagefile, 1);
		if (image.empty())
		{
			printf("failed to load %s\n", imagefile);
			continue;
		}

		if (opt.methodType == ZQ_ExtractSiftSurfOptions::METHOD_SIFT)
		{
			SiftFeatureDetector feature(0, 3, opt.sift_contrast_thresh, 10, 1.6);

			vector<KeyPoint> keypoints;
			vector<KeyPoint> out_keypoints;
			feature.detect(image, keypoints);
			if (opt.has_foremask)
				Filter_repeat_key(keypoints, out_keypoints, foremask);
			else
				Filter_repeat_key(keypoints, out_keypoints, image.rows, image.cols);
			
			SiftDescriptorExtractor descript;

			Mat description;
			descript.compute(image, out_keypoints, description);

			if (!SaveKey(keyfile, out_keypoints, description))
			{
				printf("failed to save sift keys to file %s\n", keyfile);
				continue;
			}
			clock_t t2 = clock();
			printf("%5d key points found in %4d.%s, cost %.3f secs!\n", out_keypoints.size(),i,opt.image_suffix, 0.001*(t2 - t1));
		}
		else if (opt.methodType == ZQ_ExtractSiftSurfOptions::METHOD_SURF)
		{
			SurfFeatureDetector feature;
			feature.hessianThreshold = opt.surf_hessian_thresh;

			vector<KeyPoint> keypoints;
			vector<KeyPoint> out_keypoints;
			feature.detect(image, keypoints);
			if (opt.has_foremask)
				Filter_repeat_key(keypoints, out_keypoints, foremask);
			else
				Filter_repeat_key(keypoints, out_keypoints, image.rows, image.cols);
			
			SurfDescriptorExtractor descript;
			Mat description;
			descript.compute(image, out_keypoints, description);

			if (!SaveKey(keyfile, out_keypoints, description))
			{
				printf("failed to save surf keys to file %s\n", keyfile);
				continue;
			}
			clock_t t2 = clock();
			printf("%5d key points found in %4d.%s, cost %.3f secs!\n", out_keypoints.size(), i, opt.image_suffix, 0.001*(t2 - t1));
		}
	}
}

bool SaveKey(const char* file, const vector<KeyPoint>& keypoints, const Mat& description)
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
		return SaveKey_ascii(file, keypoints, description);
	}
	else if (_strcmpi(file + i, ".keyb") == 0)
	{
		return SaveKey_binary(file, keypoints, description);
	}
	return false;
}

bool SaveKey_ascii(const char* file, const vector<KeyPoint>& keypoints, const Mat& description)
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
		const int* data = (const int*)(description.data + p * dim*4);
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

bool SaveKey_binary(const char* file, const vector<KeyPoint>& keypoints, const Mat& description)
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
		const int* data = (const int*)(description.data + p * dim*4);
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

void Filter_repeat_key(const vector<KeyPoint>& input_key, vector<KeyPoint>& output_key, int width, int height)
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

void Filter_repeat_key(const vector<KeyPoint>& input_key, vector<KeyPoint>& output_key, const ZQ_DImage<bool>& foremask)
{
	int input_size = input_key.size();
	output_key.reserve(input_size);
	output_key.clear();
	int width = foremask.width();
	int height = foremask.height();
	if (width <= 0 || height <= 0 || foremask.nchannels() <= 0)
		return;

	bool* flag = new bool[width*height];
	memcpy(flag, foremask.data(), sizeof(bool)*width*height);
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