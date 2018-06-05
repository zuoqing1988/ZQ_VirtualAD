#include "GCApplication.h"
//初始化掩码图和各变量
//mask图GrabCut函数中对应第二个参数，标记图片中哪些属于前景，哪些属于背景
//mask图只可存入四种数值，分别为：GC_BGD、GC_FGD、GC_PR_BGD、GC_PR_FGD
//mask初始化为背景，即赋值为GC_BGD
void GCApplication::reset()
{
	if (!mask.empty())
		mask.setTo(Scalar::all(GC_BGD));
	bgdPxls.clear(); fgdPxls.clear();
	prBgdPxls.clear();  prFgdPxls.clear();
	isInitialized = false;
	rectState = NOT_SET;
	lblsState = NOT_SET;
	prLblsState = NOT_SET;
	iterCount = 0;
}
//初始化窗口和图片
//将读取的图片和窗口名存入类中的私有变量image和winName中，有利于存储
//初始化掩码图以及各变量
void GCApplication::setImageAndWinName(const Mat& _image, const string& _winName)
{
	if (_image.empty() || _winName.empty())
		return;
	image = &_image;
	winName = &_winName;
	mask.create(image->size(), CV_8UC1);
	reset();
}
//显示图片
//如果 fgdPxls, bgdPxls, prFgdPxls, prBgdPxls变量非空，则在图片中显示标记的点
//如果 rectState 已经表示被标记，则也在图片中显示标记的矩形
void GCApplication::showImage() const
{
	if (image->empty() || winName->empty())
		return;

	Mat res;
	Mat binMask;
	//如果图像已经被重置，则拷贝整幅图像
	//否则显示已经被处理过的图像
	if (!isInitialized)
		image->copyTo(res);
	else
	{
		getBinMask(mask, binMask);
		image->copyTo(res, binMask);
	}

	vector<Point>::const_iterator it;
	for (it = bgdPxls.begin(); it != bgdPxls.end(); ++it)
		circle(res, *it, radius, BLUE, thickness);
	for (it = fgdPxls.begin(); it != fgdPxls.end(); ++it)
		circle(res, *it, radius, RED, thickness);
	for (it = prBgdPxls.begin(); it != prBgdPxls.end(); ++it)
		circle(res, *it, radius, LIGHTBLUE, thickness);
	for (it = prFgdPxls.begin(); it != prFgdPxls.end(); ++it)
		circle(res, *it, radius, PINK, thickness);

	if (rectState == IN_PROCESS || rectState == SET)
		rectangle(res, Point(rect.x, rect.y), Point(rect.x + rect.width, rect.y + rect.height), GREEN, 2);

	imshow(*winName, res);
}
//通过矩形标记Mask
void GCApplication::setRectInMask()
{
	assert(!mask.empty());
	mask.setTo(GC_BGD);
	rect.x = max(0, rect.x);
	rect.y = max(0, rect.y);
	rect.width = min(rect.width, image->cols - rect.x);
	rect.height = min(rect.height, image->rows - rect.y);
	(mask(rect)).setTo(Scalar(GC_PR_FGD));
}

void GCApplication::setLblsInMask(int flags, Point p, bool isPr)
{
	vector<Point> *bpxls, *fpxls;
	uchar bvalue, fvalue;
	//如果左键按下，则运行以下代码
	if (!isPr)
	{
		bpxls = &bgdPxls;
		fpxls = &fgdPxls;
		bvalue = GC_BGD;
		fvalue = GC_FGD;
	}
	//否则，运行以下代码
	else
	{
		bpxls = &prBgdPxls;
		fpxls = &prFgdPxls;
		bvalue = GC_PR_BGD;
		fvalue = GC_PR_FGD;
	}
	//判断是shift键被按下或者ctrl键被按下，分别执行操作
	if (flags & BGD_KEY)
	{
		bpxls->push_back(p);
		circle(mask, p, radius, bvalue, thickness);
	}
	if (flags & FGD_KEY)
	{
		fpxls->push_back(p);
		circle(mask, p, radius, fvalue, thickness);
	}
}
//鼠标响应
void GCApplication::mouseClick(int event, int x, int y, int flags, void*)
{
	// TODO add bad args check
	switch (event)
	{
	case CV_EVENT_LBUTTONDOWN: // set rect or GC_BGD(GC_FGD) labels
	{
								   bool isb = (flags & BGD_KEY) != 0,
									   isf = (flags & FGD_KEY) != 0;
								   //如果rectState为NOT_SET并且ctrl或者shift没被按下，则运行以下代码，设置矩形框
								   if (rectState == NOT_SET && !isb && !isf)
								   {
									   rectState = IN_PROCESS;
									   rect = Rect(x, y, 1, 1);
								   }
								   //如果rectState为SET，并且ctrl或者shift被按下，则运行以下代码，标记GC_BGD(GC_FGD)
								   if ((isb || isf) && rectState == SET)
									   lblsState = IN_PROCESS;
	}
		break;
	case CV_EVENT_RBUTTONDOWN: // set GC_PR_BGD(GC_PR_FGD) labels
	{
								   //如果rectState为SET，并且ctrl或者shift被按下时，标记GC_PR_BGD(GC_PR_FGD)
								   bool isb = (flags & BGD_KEY) != 0,
									   isf = (flags & FGD_KEY) != 0;
								   if ((isb || isf) && rectState == SET)
									   prLblsState = IN_PROCESS;
	}
		break;
	case CV_EVENT_LBUTTONUP:
		//如果rectState为IN_PROCESS，则确定鼠标走过的整个矩形，并且通过矩形设置Mask
		if (rectState == IN_PROCESS)
		{
			rect = Rect(Point(rect.x, rect.y), Point(x, y));
			rectState = SET;
			setRectInMask();
			assert(bgdPxls.empty() && fgdPxls.empty() && prBgdPxls.empty() && prFgdPxls.empty());
			showImage();
		}
		//如果lblsState为IN_PROCESS，则通过圆圈标记Mask
		if (lblsState == IN_PROCESS)
		{
			setLblsInMask(flags, Point(x, y), false);
			lblsState = SET;
			showImage();
		}
		break;
	case CV_EVENT_RBUTTONUP:
		//如果prLblsState为IN_PROCESS，则通过圆圈标记Mask
		if (prLblsState == IN_PROCESS)
		{
			setLblsInMask(flags, Point(x, y), true);
			prLblsState = SET;
			showImage();
		}
		break;
	case CV_EVENT_MOUSEMOVE:
		//如果rectState为IN_PROCESS，则鼠标移动时生成矩形
		if (rectState == IN_PROCESS)
		{
			rect = Rect(Point(rect.x, rect.y), Point(x, y));
			assert(bgdPxls.empty() && fgdPxls.empty() && prBgdPxls.empty() && prFgdPxls.empty());
			showImage();
		}//如果lblsState为IN_PROCESS，则鼠标移动时用圆圈标记Mask
		else if (lblsState == IN_PROCESS)
		{
			setLblsInMask(flags, Point(x, y), false);
			showImage();
		}//如果prLblsState为IN_PROCESS，则鼠标移动时用圆圈标记Mask
		else if (prLblsState == IN_PROCESS)
		{
			setLblsInMask(flags, Point(x, y), true);
			showImage();
		}
		break;
	}
}
//如果lblsState或者prLblsState被设置为SET，则说明图片已经被鼠标标记处前景和背景，
//而且已经经过矩形处理过一次了，则执行grabCut的GC_INIT_WITH_MASK形式，否则，执行
//GC_INIT_WITH_RECT形式，清除bgdPxls等变量标记，方便下次标记
int GCApplication::nextIter()
{
	if (isInitialized)
		grabCut(*image, mask, rect, bgdModel, fgdModel, 3);
	else
	{
		if (rectState != SET)
			return iterCount;

		if (lblsState == SET || prLblsState == SET)
			grabCut(*image, mask, rect, bgdModel, fgdModel, 3, GC_INIT_WITH_MASK);
		else
			grabCut(*image, mask, rect, bgdModel, fgdModel, 3, GC_INIT_WITH_RECT);

		isInitialized = true;
	}
	iterCount++;

	bgdPxls.clear(); fgdPxls.clear();
	prBgdPxls.clear(); prFgdPxls.clear();

	return iterCount;
}