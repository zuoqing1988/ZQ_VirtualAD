#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <time.h>

#define ZQ_LINK_OPENCV_VERSION_2413
#include "ZQ_Link_OpenCV_Lib.h"


using namespace std;
using namespace cv;

double _get_dis_avg_L2(const Mat& frame, const Mat& last_frame);

double _get_dis_hist(const Mat& frame, const Mat& last_frame);

int main(int argc, const char **argv)
{
	const char* videofile = "C:\\Users\\ZQ\\Desktop\\人民检察官HDTV28.mp4";
	//const char* videofile = "C:\\Users\\ZQ\\Desktop\\我的新野蛮女友.HD1280超清韩语中字.mp4";
	//const char* videofile = "data1.mp4";
	const char* out_fold = "scene_cut";

	VideoCapture capture(videofile);
	
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

	int* id_flag = new int[totalFrameNumber];
	memset(id_flag, 0, sizeof(int)*totalFrameNumber);
	int cur_id = 1;
	

	//设置开始帧()
	long frameToStart = 0;// frame_pos 0-based
	long cur_frame = frameToStart;
	int skip = 5;
	capture.set(CV_CAP_PROP_POS_FRAMES, frameToStart);
	cout << "从第" << frameToStart << "帧开始读" << endl;

	double thresh_dis_avg_L2 = 10;
	double thresh_dis_hist = 0.7;

	char outfile[200];
	// 获取图像帧率
	double rate = capture.get(CV_CAP_PROP_FPS);
	bool stop(false);
	Mat frame, last_frame;

	int frame_id = 0;
	// 每一帧之间的延迟
	int delay = 5; //1000 / rate;
	clock_t t1 = clock();
	// 遍历每一帧
	while (!stop)
	{
		// 尝试读取下一帧
		if (!capture.read(frame))
			break;

		/*if (1)
		{
			imshow("Extracted Frame", frame);
			if (waitKey(delay) == 27)
				stop = true;
		}*/
		
		bool scene_id_changed = false;
		cvtColor(frame, frame, CV_BGR2GRAY);
		if (frame_id > 0)
		{
			double dis_avg_L2 = _get_dis_avg_L2(frame, last_frame);
			double dis_hist = _get_dis_hist(frame, last_frame);
			if (dis_avg_L2 > thresh_dis_avg_L2 && dis_hist < thresh_dis_hist)
			{
				cur_id++;
				scene_id_changed = true;
				
			}
			id_flag[frame_id] = cur_id;
		}
		else
		{
			id_flag[frame_id] = 1;
		}
		//sprintf(outfile, "%s\\%d_%d.jpg", out_fold, cur_id, frame_id);
		//imwrite(outfile, frame);
		clock_t t2 = clock();
		if (scene_id_changed)
			printf("fr=[%6d],scene=[%5d], time=%5.f secs\n", frame_id, cur_id, 0.001*(t2 - t1));
		frame.copyTo(last_frame);
		frame_id++;
	}
	clock_t t3 = clock();
	
	printf("time=%5.f secs\n", 0.001*(t3 - t1));
	return 0;
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
	int histSize[] = {256};
	float h_ranges[] = { 0, 256 };
	const float* ranges[] = { h_ranges};

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