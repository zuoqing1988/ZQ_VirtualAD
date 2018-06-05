#ifndef _ZQ_CONVERT_VIDEO_TO_VIDEO_H_
#define _ZQ_CONVERT_VIDEO_TO_VIDEO_H_
#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <vector>
#include <time.h>

namespace ZQ
{
	class ZQ_ConvertVideoToVideo
	{
	public:
		static bool Convert(const char* in_file, const char* out_file)
		{
			cv::VideoCapture capture(in_file);
			if (!capture.isOpened())
				return false;

			int width = capture.get(CV_CAP_PROP_FRAME_WIDTH);
			int height = capture.get(CV_CAP_PROP_FRAME_HEIGHT);
			double fps = capture.get(CV_CAP_PROP_FPS);
			cv::VideoWriter writer(out_file, CV_FOURCC('D', 'I', 'V', 'X'), fps, cv::Size(width, height));

			cv::Mat frame;
			int count = 0;
			clock_t t1 = clock();
			while (true)
			{
				if (!capture.read(frame))
					break;
				writer.write(frame);
				if (count % 100 == 0)
				{
					clock_t t2 = clock();
					printf("frame [%d], cost: %.1f secs\n", count,0.001*(t2-t1));
				}
				
				count++;
				
			}
			return true;
		}
	};
}

#endif
