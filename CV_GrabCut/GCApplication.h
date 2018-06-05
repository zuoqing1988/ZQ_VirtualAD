#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
using namespace std;
using namespace cv;
const Scalar RED = Scalar(0, 0, 255);
const Scalar PINK = Scalar(230, 130, 255);
const Scalar BLUE = Scalar(255, 0, 0);
const Scalar LIGHTBLUE = Scalar(255, 255, 160);
const Scalar GREEN = Scalar(0, 255, 0);

const int BGD_KEY = CV_EVENT_FLAG_CTRLKEY;//当CTRL被按下时，flags返回的值
const int FGD_KEY = CV_EVENT_FLAG_SHIFTKEY;//当SHIFT被按下时，flags返回的值

static void getBinMask(const Mat& comMask, Mat& binMask)
{
	if (comMask.empty() || comMask.type() != CV_8UC1)
		CV_Error(CV_StsBadArg, "comMask is empty or has incorrect type (not CV_8UC1)");
	if (binMask.empty() || binMask.rows != comMask.rows || binMask.cols != comMask.cols)
		binMask.create(comMask.size(), CV_8UC1);
	binMask = comMask & 1;
}
class GCApplication
{
public:
	enum{ NOT_SET = 0, IN_PROCESS = 1, SET = 2 };
	static const int radius = 2;
	static const int thickness = -1;

	void reset();
	void setImageAndWinName(const Mat& _image, const string& _winName);
	void showImage() const;
	void mouseClick(int event, int x, int y, int flags, void* param);
	int nextIter();
	int getIterCount() const { return iterCount; }
private:
	void setRectInMask();
	void setLblsInMask(int flags, Point p, bool isPr);

	const string* winName;
	const Mat* image;
	Mat mask;
	Mat bgdModel, fgdModel;
	//rectState, lblsState, prLblsState三个变量分别表示矩形标记的状态，
	//鼠标左键标记的状态，鼠标右键标记的状态，分别有三个状态：NOT_SET（未处理）
	//IN_PROCESS（处理）、SET（已处理）
	uchar rectState, lblsState, prLblsState;
	bool isInitialized;

	Rect rect;
	//在第一次矩形分割后，第二次标记mask值时，四种值出现的点都分别保存在
	//fgdPxls, bgdPxls, prFgdPxls, prBgdPxls四个变量中
	vector<Point> fgdPxls, bgdPxls, prFgdPxls, prBgdPxls;
	//迭代的次数
	int iterCount;
};