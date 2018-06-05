#ifndef _ZQ_EXTRACT_SIFT_SURF_H_
#define _ZQ_EXTRACT_SIFT_SURF_H_
#pragma once

#include "ZQ_ExtractSiftSurfOptions.h"
#include "ZQ_VirtualAdKeyIO.h"
#include "ZQ_VirtualAdKeyIO_CV2.h"
#include "ZQ_ImageIO.h"
#include "ZQ_DoubleImage.h"
#include <opencv2/nonfree/nonfree.hpp>
#include <time.h>
#include <stdio.h>
#include <process.h>

namespace ZQ
{
	class ZQ_ExtractSiftSurf
	{
	public:
		template<class T>
		static bool ExtractSiftSurfFromImage(const cv::Mat& image, int& num, int& dim, T*& coords, T*& vals, const ZQ_ExtractSiftSurfOptions& opt)
		{
			if (opt.methodType == ZQ_ExtractSiftSurfOptions::METHOD_SIFT)
			{
				cv::SiftFeatureDetector feature(0, 3, opt.sift_contrast_thresh, 10, 1.6);

				std::vector<cv::KeyPoint> keypoints;
				std::vector<cv::KeyPoint> out_keypoints;

				clock_t t1 = clock();
				feature.detect(image, keypoints);
				ZQ_VirtualAdKeyIO_CV2::Filter_repeat_key(keypoints, out_keypoints, image.rows, image.cols);
				clock_t t2 = clock();

				cv::SiftDescriptorExtractor descript;

				cv::Mat description;
				descript.compute(image, out_keypoints, description);

				if (!ZQ_VirtualAdKeyIO_CV2::ConvertSiftSurfKey(out_keypoints, description, num, dim, coords, vals))
				{
					return false;
				}
				clock_t t3 = clock();
				float detect_time = 0.001*(t2 - t1);
				float extract_time = 0.001*(t3 - t2);
				float total_time = detect_time + extract_time;
				printf("%d pts found in %.3f secs (detect: %.3f, extract: %.3f)\n", num, total_time, detect_time, extract_time);
			}
			else if (opt.methodType == ZQ_ExtractSiftSurfOptions::METHOD_SURF)
			{
				cv::SurfFeatureDetector feature;
				feature.hessianThreshold = opt.surf_hessian_thresh;

				std::vector<cv::KeyPoint> keypoints;
				std::vector<cv::KeyPoint> out_keypoints;

				clock_t t1 = clock();
				feature.detect(image, keypoints);
				ZQ_VirtualAdKeyIO_CV2::Filter_repeat_key(keypoints, out_keypoints, image.rows, image.cols);
				clock_t t2 = clock();
				cv::SurfDescriptorExtractor descript;

				cv::Mat description;
				descript.compute(image, out_keypoints, description);

				if (!ZQ_VirtualAdKeyIO_CV2::ConvertSiftSurfKey(out_keypoints, description, num, dim, coords, vals))
				{
					return false;
				}
				clock_t t3 = clock();
				float detect_time = 0.001*(t2 - t1);
				float extract_time = 0.001*(t3 - t2);
				float total_time = detect_time + extract_time;
				if (opt.display_running_info)
					printf("%d pts found in %.3f secs (detect: %.3f, extract: %.3f)\n", num, total_time, detect_time, extract_time);
			}
			else
			{
				if (opt.display_running_info)
					printf("unknown method!\n");
				return false;
			}
			return true;
		}

		static bool ExtractSiftSurfFromSequenceOneThread(const ZQ_ExtractSiftSurfOptions& opt)
		{
			clock_t t1 = clock();
			char imagefile[200];
			char keyfile[200];
			char foremaskfile[200];
			for (int i = opt.base_id; i < opt.base_id + opt.frame_num; i++)
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
						printf("warning: failed to load %s\n", foremaskfile);
						continue;
					}
				}

				cv::Mat image = cv::imread(imagefile, 1);
				if (image.empty())
				{
					printf("warning: failed to load %s\n", imagefile);
					continue;
				}

				int num = 0, dim = 0;
				float* coords = 0;
				float* vals = 0;
				if (!ExtractSiftSurfFromImage(image, num, dim, coords, vals, opt))
				{
					printf("warning: failed to extract keys in %s\n", imagefile);
					continue;
				}
				
				if (!ZQ_VirtualAdKeyIO::SaveSiftSurfKey(keyfile, num, dim, coords, vals))
				{
					printf("warning: failed to save keyfile %s\n", keyfile);
					if (coords) delete[]coords;
					if (vals) delete[]vals;
					continue;
				}
				if (coords) delete[]coords;
				if (vals) delete[]vals;
			}
			clock_t t2 = clock();
			if (opt.display_running_info)
			{
				printf("cost: %.3f secs\n", 0.001*(t2 - t1));
			}
			return true;
		}

		static bool ExtractSiftSurfFromSequence(const ZQ_ExtractSiftSurfOptions& opt)
		{	
			if (opt.ncores <= 1)
			{
				return ExtractSiftSurfFromSequenceOneThread(opt);
			}
			else
			{
				clock_t t1 = clock();
				ZQ_ExtractSiftSurfOptions* opts = new ZQ_ExtractSiftSurfOptions[opt.ncores];
				int each_num = ceil((float)opt.frame_num / opt.ncores);
				for (int i = 0; i < opt.ncores; i++)
				{
					opts[i] = opt;
					opts[i].base_id = opt.base_id + each_num*i;
					opts[i].frame_num = __min(each_num, opt.base_id + opt.frame_num - opts[i].base_id);
					opts[i].done_flag = false;
				}

				for (int i = 0; i < opt.ncores - 1; i++)
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
				clock_t t2 = clock();
				if (opt.display_running_info)
					printf("total cost: %.3f secs\n", 0.001*(t2 - t1));
			}
		
			return true;
		}

	private:
		static void _handle_sequence_thread(void* arg)
		{
			ZQ_ExtractSiftSurfOptions* opt = (ZQ_ExtractSiftSurfOptions*)arg;
			ExtractSiftSurfFromSequenceOneThread(*opt);
			opt->done_flag = true;
		}
	};
}

#endif