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

const int BGD_KEY = CV_EVENT_FLAG_CTRLKEY;//��CTRL������ʱ��flags���ص�ֵ
const int FGD_KEY = CV_EVENT_FLAG_SHIFTKEY;//��SHIFT������ʱ��flags���ص�ֵ

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
	//rectState, lblsState, prLblsState���������ֱ��ʾ���α�ǵ�״̬��
	//��������ǵ�״̬������Ҽ���ǵ�״̬���ֱ�������״̬��NOT_SET��δ������
	//IN_PROCESS����������SET���Ѵ�����
	uchar rectState, lblsState, prLblsState;
	bool isInitialized;

	Rect rect;
	//�ڵ�һ�ξ��ηָ�󣬵ڶ��α��maskֵʱ������ֵ���ֵĵ㶼�ֱ𱣴���
	//fgdPxls, bgdPxls, prFgdPxls, prBgdPxls�ĸ�������
	vector<Point> fgdPxls, bgdPxls, prFgdPxls, prBgdPxls;
	//�����Ĵ���
	int iterCount;
};