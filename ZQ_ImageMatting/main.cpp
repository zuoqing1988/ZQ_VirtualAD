#define _USE_UMFPACK 1
#include "ZQ_GraphCut.h"
#include "ZQ_Kmeans.h"
#include "ZQ_ImageIO.h"
#include "ZQ_BilateralFilter.h"
#include "ZQ_StructureFromTexture.h"
#include "ZQ_LazySnapping.h"
#include "ZQ_ClosedFormImageMatting.h"
#include <vector>
#include <iostream>
#include <cmath>

#define ZQ_LINK_OPENCV_VERSION_2413
#include "ZQ_Link_OpenCV_Lib.h"

using namespace std;
using namespace ZQ;


typedef double BaseType;
const int MAX_CLUSTER_NUM = 24;

// global
vector<CvPoint> forePts;
vector<CvPoint> backPts;
vector<CvPoint> add_forePts;
vector<CvPoint> add_backPts;
int currentMode = 0;// indicate foreground or background, foreground as default
double color_draw[2][3] =
{
	{ 0, 0, 255 },
	{ 255, 0, 0 }
};

IplImage* image = NULL;
char* winName = "SampleLazySnapping";
IplImage* imageDraw = NULL;
ZQ_DImage<BaseType> ori_image, scaled_image, im2;
ZQ_LazySnappingOptions ls_opt;
ZQ_StructureFromTextureOptions opt;
ZQ_DImage<bool> mask;
int standard_width = 480;
int standard_height = 270;
bool has_scaled = false;
ZQ_LazySnapping<BaseType, MAX_CLUSTER_NUM>* lazySnap = 0;

void on_mouse(int event, int x, int y, int flags, void*);
void drawMask(IplImage* imageDraw, const ZQ_DImage<bool>& mask);
void drawMask(IplImage* imageDraw, const bool* mask);
void drawSelectPoints(IplImage* imageDraw, const vector<CvPoint>& forePts, const vector<CvPoint>& backPts);

bool closedFormMatting(const ZQ_DImage<BaseType>& im, const ZQ_DImage<bool>& rough_mask, ZQ_DImage<BaseType>& alpha, ZQ_DImage<BaseType>& fore, ZQ_DImage<BaseType>& back, int pfilter_size = 10);

int main_lazy(int argc, const char** argv);
int main_closedform(int argc, const char** argv);
int main_lazy_to_trimap(int argc, const char** argv);

int main(int argc, const char** argv)
{
	if (argc < 2)
	{
		printf(".exe lazy [opt]\n or \n");
		printf(".exe closedform [opt] or\n");
		printf(".exe lazy_to_trimap [opt]\n");
		return -1;
	}
	if (_strcmpi(argv[1], "lazy") == 0)
	{
		return main_lazy(argc - 1, argv + 1);
	}
	else if (_strcmpi(argv[1], "closedform") == 0)
	{
		return main_closedform(argc - 1, argv + 1);
	}
	else if (_strcmpi(argv[1], "lazy_to_trimap") == 0)
	{
		return main_lazy_to_trimap(argc - 1, argv + 1);
	}
	else
	{
		printf("unknown arg: %s\n", argv[1]);
		return -1;
	}
	return -1;
}

int main_lazy_to_trimap(int argc, const char** argv)
{
	if (argc != 5)
	{
		printf(".exe image_file lazy_alpha_file pfilter_size trimap_file\n");
		return -1;
	}
	const char* image_file = argv[1];
	const char* lazy_alpha_file = argv[2];
	int pfilter_size = atoi(argv[3]);
	const char* trimap_file = argv[4];

	ZQ_DImage<BaseType> image;
	if (!ZQ_ImageIO::loadImage(image, image_file, 1))
	{
		printf("failed to load %s\n", image_file);
		return -1;
	}

	ZQ_DImage<bool> lazy_alpha;
	if (!ZQ_ImageIO::loadImage(lazy_alpha, lazy_alpha_file, 0))
	{
		printf("failed to load %s\n", lazy_alpha_file);
		return -1;
	}

	int width = image.width();
	int height = image.height();
	int nChannels = image.nchannels();

	if (!lazy_alpha.matchDimension(width, height, 1))
	{
		printf("dimension does not match!\n");
		return -1;
	}

	ZQ_DImage<bool> tmp1_mask(width, height), tmp2_mask(width, height);
	bool*& tmp1_mask_data = tmp1_mask.data();
	bool*& tmp2_mask_data = tmp2_mask.data();
	bool*& lazy_alpha_data = lazy_alpha.data();
	BaseType*& image_data = image.data();

	bool filter2D[9] = { 1, 1, 1, 1, 1, 1, 1, 1, 1 };
	tmp1_mask = lazy_alpha;
	
	for (int i = 0; i < pfilter_size; i++)
	{
		ZQ_BinaryImageProcessing::Erode(tmp1_mask_data, tmp2_mask_data, width, height, filter2D, 1, 1);
		tmp1_mask = tmp2_mask;
	}

	for (int i = 0; i < width*height; i++)
	{
		if (tmp1_mask_data[i])
		{
			for (int c = 0; c < nChannels; c++)
				image_data[i*nChannels + c] = 1;
		}
	}

	tmp1_mask = lazy_alpha;
	for (int i = 0; i < pfilter_size; i++)
	{
		ZQ_BinaryImageProcessing::Dilate(tmp1_mask_data, tmp2_mask_data, width, height, filter2D, 1, 1);
		tmp1_mask = tmp2_mask;
	}

	for (int i = 0; i < width*height; i++)
	{
		if (!tmp1_mask_data[i])
		{
			for (int c = 0; c < nChannels; c++)
				image_data[i*nChannels + c] = 0;
		}
	}

	if (!ZQ_ImageIO::saveImage(image, trimap_file))
	{
		printf("failed to save %s\n", trimap_file);
		return -1;
	}

	return 0;
}

int main_closedform(int argc, const char** argv)
{
	if (argc < 6)
	{
		printf(".exe closedform imagefile trimapfile alphafile forefile backfile [opt]\n");
		return -1;
	}
	
	const char* imagefile = argv[1];
	const char* trimapfile = argv[2];
	const char* alphafile = argv[3];
	const char* forefile = argv[4];
	const char* backfile = argv[5];
	int max_level = 1;
	if (argc >= 7)
		max_level = atoi(argv[6]);
	max_level = __max(max_level, 1);
	ZQ_DImage<BaseType> im1, im2;
	if (!ZQ_ImageIO::loadImage(im1, imagefile, 1))
	{
		printf("failed to load %s\n", imagefile);
		return -1;
	}

	if (!ZQ_ImageIO::loadImage(im2, trimapfile, 1))
	{
		printf("failed to load %s\n", trimapfile);
		return -1;
	}

	int width = im1.width();
	int height = im1.height();
	int nChannels = im1.nchannels();
	if (!im2.matchDimension(im1))
	{
		printf("dimension does not match\n");
		return -1;
	}


	BaseType*& im1_data = im1.data();
	BaseType*& im2_data = im2.data();

	ZQ_DImage<bool> consts_map(width, height);
	bool*& consts_map_data = consts_map.data();

	for (int i = 0; i < width*height; i++)
	{
		BaseType sum_diff = 0;
		for (int c = 0; c < nChannels; c++)
			sum_diff += fabs(im1_data[i*nChannels + c] - im2_data[i*nChannels + c]);

		consts_map_data[i] = sum_diff > 0.001;
	}

	ZQ_ImageIO::loadImage(im2, trimapfile, 0);
	for (int i = 0; i < width*height; i++)
		im2.data()[i] *= consts_map_data[i];

	ZQ_DImage<BaseType> alpha(width, height);
	float epsilon = 1e-7;
	int win_size = 1;
	float consts_thresh = 0.02;
	//if (!ZQ_ClosedFormImageMatting::Coarse2FineSolveAlpha(im1, consts_map, im2, alpha, 3, consts_thresh, epsilon, win_size, true))
	if (!ZQ_ClosedFormImageMatting::SolveAlpha(im1, consts_map, im2, alpha, epsilon, win_size, true))
	{
		printf("failed to Solve Alpha!\n");
		return -1;
	}

	alpha.imclamp(0, 1);
	if (!ZQ_ImageIO::saveImage(alpha, alphafile))
	{
		printf("failed to save %s\n", alphafile);
		return -1;
	}

	ZQ_DImage<BaseType> fore, back;
	int max_iter = 100;
	if (!ZQ_ClosedFormImageMatting::SolveForeBack_ori_paper(im1, alpha, fore, back, max_iter, true))
	{
		printf("failed to Solve Foreground and Background!\n");
		return -1;
	}
	ZQ_DImage<BaseType> alpha2, alpha3;
	alpha2.assemble(alpha, alpha);
	alpha3.assemble(alpha, alpha2);
	fore.Multiplywith(alpha3);
	ZQ_DImage<BaseType> alpha3_1(alpha3);
	alpha3_1.Multiplywith(-1);
	alpha3_1.Addwith(1);
	back.Multiplywith(alpha3_1);
	if (!ZQ_ImageIO::saveImage(fore, forefile))
	{
		printf("failed to save %s\n", forefile);
		return -1;
	}
	if(!ZQ_ImageIO::saveImage(back, backfile))
	{
		printf("failed to save %s\n", backfile);
		return -1;
	}
	return 0;
}

int main_lazy(int argc, const char** argv)
{
	if (argc < 4)
	{
		printf(".exe infile alphafile forefile [opt]\n");
		return -1;
	}

	const char* infile = argv[1];
	const char* alphafile = argv[2];
	const char* forefile = argv[3];
	
	if (argc > 4)
	{
		if (!ls_opt.HandleArgs(argc - 4, argv + 4))
			return -1;
	}


	if (!ZQ_ImageIO::loadImage(ori_image, infile, 1))
	{
		printf("failed to load %s\n", infile);
		return -1;
	}
	opt.fsize_for_filter = 1;
	ls_opt.dilate_erode_size = 2;

	int ori_width = ori_image.width();
	int ori_height = ori_image.height();
	if (ori_width > standard_width || ori_height > standard_height)
	{
		double scale_x = (double)standard_width / ori_width;
		double scale_y = (double)standard_height / ori_height;
		double scale = __min(scale_x, scale_y);
		int dst_width = ori_width*scale + 0.5;
		int dst_height = ori_height*scale + 0.5;
		ori_image.imresize(scaled_image, dst_width, dst_height);
		im2 = scaled_image;
		if (!ZQ_StructureFromTexture::StructureFromTexture(scaled_image, im2, opt))
		{
			printf("failed to run StructureFromTexture\n");
			return -1;
		}
		has_scaled = true;
	}
	else
	{

		if (!ZQ_StructureFromTexture::StructureFromTexture(ori_image, im2, opt))
		{
			printf("failed to run StructureFromTexture\n");
			return -1;
		}
	}


	int width = im2.width();
	int height = im2.height();
	int nChannels = im2.nchannels();
	mask.allocate(width, height, 1);

	lazySnap = new ZQ_LazySnapping<BaseType, MAX_CLUSTER_NUM>(width, height);
	lazySnap->SetImage(im2, ls_opt.lambda_for_E2, ls_opt.color_scale_for_E2);
	
	image = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
	BaseType*& im2_data = im2.data();
	for (int h = 0; h < height; h++)
	{
		for (int w = 0; w < width; w++)
		{
			int offset = h*width + w;
			if (nChannels == 1)
				cvSet2D(image, h, w, cvScalar(im2_data[offset] * 255, im2_data[offset] * 255, im2_data[offset] * 255));
			else
				cvSet2D(image, h, w, cvScalar(im2_data[offset * 3 + 0] * 255, im2_data[offset * 3 + 1] * 255, im2_data[offset * 3 + 2] * 255));
		}
	}
	cvNamedWindow(winName, 1);
	cvSetMouseCallback(winName, on_mouse, 0);
	imageDraw = cvCloneImage(image);
	cvShowImage(winName, image);

	while (true)
	{
		int c = cvWaitKey(0);
		c = (char)c;
		if (c == 27)
		{
			break;
		}
		else if (c == 'r')
		{
			imageDraw = cvCloneImage(image);
			forePts.clear();
			backPts.clear();
			add_forePts.clear();
			add_backPts.clear();
			currentMode = 0;
			cvShowImage(winName, image);
		}
		else if (c == 'b')
		{
			currentMode = 1;
		}
		else if (c == 'f')
		{
			currentMode = 0;
		}
		cvNamedWindow(winName, 1);
	}
	cvReleaseImage(&image);
	cvReleaseImage(&imageDraw);
	if (has_scaled)
	{
		mask.imresize(ori_width, ori_height);
	}

	ZQ_ImageIO::saveImage(mask, alphafile);
	ZQ_DImage<BaseType> fore(ori_image),back(ori_image),alpha;
	for (int i = 0; i < ori_height*ori_width; i++)
	{
		if (!mask.data()[i])
		{
			for (int c = 0; c < nChannels; c++)
				fore.data()[i*nChannels + c] = 0;
		}
		else
		{
			for (int c = 0; c < nChannels; c++)
				back.data()[i*nChannels + c] = 0;
		}
	}
	ZQ_ImageIO::saveImage(fore, forefile);

	return 0;
}

void on_mouse(int event, int x, int y, int flags, void*)
{
	if (event == CV_EVENT_LBUTTONUP)
	{

		int add_fore_num = add_forePts.size();
		int add_back_num = add_backPts.size();
		if (add_fore_num == 0 && add_back_num == 0)
		{
			return;
		}
		cvReleaseImage(&imageDraw);
		imageDraw = cvCloneImage(image);
		int* add_fore_pts = NULL;
		int* add_back_pts = NULL;
		if (add_fore_num > 0)
		{
			add_fore_pts = new int[add_fore_num * 2];

			for (int i = 0; i < add_fore_num; i++)
			{
				add_fore_pts[i * 2 + 0] = add_forePts[i].x;
				add_fore_pts[i * 2 + 1] = add_forePts[i].y;
			}
			if (!lazySnap->EditSnappingAddForeground(add_fore_num, add_fore_pts))
			{
				delete[]add_fore_pts;
				add_fore_pts = NULL;
			}
			else
			{
				forePts.insert(forePts.end(), add_forePts.begin(), add_forePts.end());
				add_forePts.clear();
			}
		}

		if (add_back_num > 0)
		{
			add_back_pts = new int[add_back_num * 2];

			for (int i = 0; i < add_back_num; i++)
			{
				add_back_pts[i * 2 + 0] = add_backPts[i].x;
				add_back_pts[i * 2 + 1] = add_backPts[i].y;

			}

			if (!lazySnap->EditSnappingAddBackground(add_back_num, add_back_pts))
			{
				delete[]add_back_pts;
				add_back_pts = NULL;
			}
			else
			{
				backPts.insert(backPts.end(), add_backPts.begin(), add_backPts.end());
				add_backPts.clear();
			}
			
			if (!ZQ_LazySnapping<BaseType, MAX_CLUSTER_NUM>::FilterMask(lazySnap->GetForegroundMaskPtr(), mask.data(), mask.width(), mask.height(), ls_opt.area_thresh, ls_opt.dilate_erode_size))
			{
				memcpy(mask.data(), lazySnap->GetForegroundMaskPtr(), sizeof(bool)*mask.width()*mask.height());
			}
			drawMask(imageDraw, mask);
			if (add_fore_pts)
			{
				delete[]add_fore_pts;
			}
			add_fore_pts = 0;
			if (add_back_pts)
			{
				delete[]add_back_pts;
			}
			add_back_pts = 0;
		}
		drawSelectPoints(imageDraw, forePts, backPts);
		cvShowImage(winName, imageDraw);
	}
	else if (event == CV_EVENT_LBUTTONDOWN)
	{
	}
	else if (event == CV_EVENT_MOUSEMOVE && (flags & CV_EVENT_FLAG_LBUTTON))
	{
		CvPoint pt = cvPoint(x, y);
		if (currentMode == 0)
		{
			add_forePts.push_back(cvPoint(x, y));
		}
		else
		{
			add_backPts.push_back(cvPoint(x, y));
		}
		CvScalar color = cvScalar(color_draw[currentMode][0], color_draw[currentMode][1], color_draw[currentMode][2]);
		cvCircle(imageDraw, pt, 2, color);
		cvShowImage(winName, imageDraw);
	}
}

void drawMask(IplImage* imageDraw, const ZQ_DImage<bool>& mask)
{
	drawMask(imageDraw, mask.data());
}

void drawMask(IplImage* imageDraw, const bool* mask)
{
	int width = imageDraw->width;
	int height = imageDraw->height;
	CvScalar border_color = cvScalar(0, 255, 0);
	for (int h = 0; h < height; h++)
	{
		for (int w = 0; w < width; w++)
		{
			int offset = h*width + w;
			if (!mask[offset] && ((w > 0 && mask[offset - 1]) || (w < width - 1 && mask[offset + 1]) || (h > 0 && mask[offset - width]) || (h < height - 1 && mask[offset + width])))
			{
				cvSet2D(imageDraw, h, w, border_color);
			}

		}
	}
}

void drawSelectPoints(IplImage* imageDraw, const vector<CvPoint>& forePts, const vector<CvPoint>& backPts)
{
	for (int i = 0; i < forePts.size(); i++)
	{
		CvScalar color = cvScalar(color_draw[0][0], color_draw[0][1], color_draw[0][2]);
		cvCircle(imageDraw, forePts[i], 2, color);
	}
	for (int i = 0; i < backPts.size(); i++)
	{
		CvScalar color = cvScalar(color_draw[1][0], color_draw[1][1], color_draw[1][2]);
		cvCircle(imageDraw, backPts[i], 2, color);
	}
}

bool closedFormMatting(const ZQ_DImage<BaseType>& im, const ZQ_DImage<bool>& rough_mask, ZQ_DImage<BaseType>& alpha, ZQ_DImage<BaseType>& fore, ZQ_DImage<BaseType>& back, int pfilter_size/* = 10*/)
{
	int width = im.width();
	int height = im.height();
	int nChannels = im.nchannels();

	if (!rough_mask.matchDimension(width, height, 1))
		return false;

	bool pfilter2D[9] = { 1, 1, 1, 1, 1, 1, 1, 1, 1 };
	
	const bool*& rough_mask_data = rough_mask.data();
	ZQ_DImage<bool> consts_map(width, height);
	bool*& consts_map_data = consts_map.data();
	ZQ_DImage<bool> tmp1_mask(width, height), tmp2_mask(width,height);
	bool*& tmp1_mask_data = tmp1_mask.data();
	bool*& tmp2_mask_data = tmp2_mask.data();

	if (pfilter_size <= 0)
		consts_map = rough_mask;
	else
	{
		tmp1_mask = rough_mask;
		for (int i = 0; i < pfilter_size; i++)
		{
			ZQ_BinaryImageProcessing::Erode(tmp1_mask_data, tmp2_mask_data, width, height, pfilter2D, 1, 1);
			tmp1_mask = tmp2_mask;
		}
		consts_map = tmp2_mask;
	}
	
	if (pfilter_size <= 0)
		tmp2_mask = rough_mask;
	else
	{
		tmp1_mask = rough_mask;
		for (int i = 0; i < pfilter_size; i++)
		{
			ZQ_BinaryImageProcessing::Dilate(tmp1_mask_data, tmp2_mask_data, width, height, pfilter2D, 1, 1);
			tmp1_mask = tmp2_mask;
		}
	}
	
	ZQ_DImage<BaseType> consts_vals(width, height);
	BaseType*& consts_vals_data = consts_vals.data();
	for (int i = 0; i < width*height; i++)
	{
		if (consts_map_data[i])
			consts_vals_data[i] = 1.0;
		else if (!tmp2_mask_data[i])
			consts_vals_data[i] = 0.0;
		else
			consts_vals_data[i] = 0.5;
		consts_map_data[i] |= !tmp2_mask_data[i];
	}
		
	float epsilon = 1e-7;

	if (!ZQ_ClosedFormImageMatting::SolveAlpha(im, consts_map, consts_vals, alpha, epsilon))
	{
		printf("failed to Solve Alpha!\n");
		return false;
	}

	alpha.imclamp(0, 1);
	
	int max_iter = 200;
	if (!ZQ_ClosedFormImageMatting::SolveForeBack_ori_paper(im, alpha, fore, back, max_iter, true))
	{
		printf("failed to Solve Foreground and Background!\n");
		return false;
	}
	ZQ_DImage<BaseType> alpha2, alpha3;
	alpha2.assemble(alpha, alpha);
	alpha3.assemble(alpha, alpha2);
	fore.Multiplywith(alpha3);
	ZQ_DImage<BaseType> alpha3_1(alpha3);
	alpha3_1.Multiplywith(-1);
	alpha3_1.Addwith(1);
	back.Multiplywith(alpha3_1);
	
	return true;
}