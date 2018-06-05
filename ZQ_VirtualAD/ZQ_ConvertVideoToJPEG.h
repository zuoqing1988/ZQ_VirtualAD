#ifndef _ZQ_CONVERT_VIDEO_TO_JPEG_H_
#define _ZQ_CONVERT_VIDEO_TO_JPEG_H_
#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <vector>
#include <time.h>
#include <windows.h>

namespace ZQ
{
	class ZQ_ConvertVideoToJPEG
	{
	public:
		static bool Convert(const char* video_file, const char* out_fold, int quality = 95)
		{
			cv::VideoCapture capture(video_file);
			if (!capture.isOpened())
				return false;

			int total_frame = capture.get(CV_CAP_PROP_FRAME_COUNT);
			printf("total frames:%d\n", total_frame);
			cv::Mat frame;
			int id = 0;
			std::vector<int> para;
			para.push_back(CV_IMWRITE_JPEG_QUALITY);
			para.push_back(quality);
			char filename[200];
			clock_t t1 = clock();
			while (true)
			{
				if (!capture.read(frame))
					break;
				sprintf(filename, "%s\\%d.jpg", out_fold, id);
				if (!cv::imwrite(filename, frame, para))
				{
					printf("failed to save image %s\n", filename);
					return false;
				}
				id++;
				if (id % 100 == 0)
				{
					clock_t t2 = clock();
					printf("%d frames exported, total = %d, cost: %.1f\n", id, total_frame, 0.001*(t2 - t1));
				}
			}
			
			return true;
		}

		static bool ConvertMultiThread(const char* video_file, const char* out_fold, int ncores, int quality = 95)
		{
			if (ncores <= 1)
				return Convert(video_file, out_fold, quality);
			
			cv::VideoCapture capture(video_file);
			if (!capture.isOpened())
				return false;

			int total_frame = capture.get(CV_CAP_PROP_FRAME_COUNT);

			CRITICAL_SECTION cs;
			InitializeCriticalSection(&cs);
			int cur_frame = 0;
			_convert_arg* args = new _convert_arg[ncores];
			for (int i = 0; i < ncores; i++)
			{
				args[i].cs_ptr = &cs;
				args[i].capture_ptr = &capture;
				args[i].total_frame = total_frame;
				args[i].cur_frame = &cur_frame;
				args[i].done_flag = false;
				args[i].out_fold = out_fold;
				args[i].quailty = quality;
			}

			for (int i = 1; i < ncores;i++)
				_beginthread(_convert_one_thread, 0, &args[i]);
			_convert_one_thread(&args[0]);

			bool all_done_flag = false;
			while (!all_done_flag)
			{
				all_done_flag = true;
				for (int i = 0; i < ncores; i++)
				{
					if (!args[i].done_flag)
					{
						all_done_flag = false;
						break;
					}
				}
			}

			return true;
		}

	private:
		struct _convert_arg
		{
			LPCRITICAL_SECTION cs_ptr;
			cv::VideoCapture* capture_ptr;
			int total_frame;
			int* cur_frame;
			bool done_flag;
			const char* out_fold;
			int quailty;
		};

		static void _convert_one_thread(void* in_para)
		{
			clock_t t1 = clock();
			cv::Mat frame;
			char filename[200];
			_convert_arg* arg = (_convert_arg*)in_para;
			std::vector<int> para;
			para.push_back(CV_IMWRITE_JPEG_QUALITY);
			para.push_back(arg->quailty);
			while (true)
			{
				EnterCriticalSection(arg->cs_ptr);
				int id = *(arg->cur_frame);
				int total_frame = arg->total_frame;
				if (id >= arg->total_frame)
				{
					arg->done_flag = true;
					LeaveCriticalSection(arg->cs_ptr);
					break;
				}

				(arg->capture_ptr)->read(frame);
				*(arg->cur_frame) = id + 1;
				LeaveCriticalSection(arg->cs_ptr);

				sprintf(filename, "%s\\%d.jpg", arg->out_fold, id);
				if (!cv::imwrite(filename, frame, para))
				{
					printf("failed to save image %s\n", filename);
				}
				
				printf("save image %s succ\n", filename);

				if (id % 100 == 0)
				{
					clock_t t2 = clock();
					printf("%d frames exported, total = %d, cost: %.1f\n", id, total_frame, 0.001*(t2 - t1));
				}
			}
		}
	};
}

#endif
