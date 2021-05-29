#include <iostream>
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

using namespace std;
using namespace cv;

IplImage* DrawHistogram(CvHistogram *hist, float scalesX = 1, float scaleY = 1);
void CreateHistogram(IplImage *img, const string &name);

int main()
{
	 string skyFile1 = "sky1.jpg";

	IplImage* sky1 = cvLoadImage(skyFile1.c_str());

	CreateHistogram(sky1, skyFile1);
}


void CreateHistogram(IplImage *img, const string &name)
{
	// [ Loading & splitting the image ]
	int numBins = 256;			// 256 bins
	float range[] = { 0, 255 }; // range 0~255

	float *ranges[] = { range };

	// 히스토그램 그릴 1채널 이미지
	CvHistogram *hist = cvCreateHist(1, &numBins, CV_HIST_ARRAY, ranges, 1);	// 256 bin으로 자동으로 범위 나눔 
	cvClearHist(hist);

	// 이미지 채널 분할
	IplImage *imgRed = cvCreateImage(cvGetSize(img), IPL_DEPTH_8U, 1);	// 메모리 할당
	IplImage *imgGreen = cvCreateImage(cvGetSize(img), IPL_DEPTH_8U, 1);
	IplImage *imgBlue = cvCreateImage(cvGetSize(img), IPL_DEPTH_8U, 1);

	// 채널분할, BGR 순서 중요 - 이미지가 로드될 때 메모리에 어떻게 저장될 지 결정
	cvSplit(img, imgBlue, imgGreen, imgRed, NULL);

	// [ Rendering histograms ]
	cvCalcHist(&imgRed, hist, 0, NULL);
	// 히스토그램 정규화
	cvNormalizeHist(hist, 20*255);
	IplImage *imgHistRed = DrawHistogram(hist);
	cvClearHist(hist);

	cvCalcHist(&imgGreen, hist, 0, NULL);
	IplImage *imgHistGreen = DrawHistogram(hist);
	cvClearHist(hist);

	cvCalcHist(&imgBlue, hist, 0, NULL);
	IplImage *imgHistBlue = DrawHistogram(hist);
	cvClearHist(hist);

	// 히스토그램을 색 입힐 3채널 이미지
	IplImage *imgHistCR = cvCreateImage(cvSize(256, 64), IPL_DEPTH_8U, 3);
	IplImage *imgHistCG = cvCreateImage(cvSize(256, 64), IPL_DEPTH_8U, 3);
	IplImage *imgHistCB = cvCreateImage(cvSize(256, 64), IPL_DEPTH_8U, 3);

	cvSet(imgHistCR, cvScalarAll(0), NULL);
	cvSet(imgHistCG, cvScalarAll(0), NULL);
	cvSet(imgHistCB, cvScalarAll(0), NULL);
	cvMerge(0, 0, imgHistRed, 0, imgHistCR);
	cvMerge(0, imgHistGreen, 0, 0, imgHistCG);
	cvMerge(imgHistBlue, 0, 0, 0, imgHistCB);

	string red = "Red_" + name;
	string green = "Green_" + name;
	string blue = "Blue_" + name;

	cvNamedWindow(red.c_str());
	cvNamedWindow(green.c_str());
	cvNamedWindow(blue.c_str());

	cvShowImage(red.c_str(), imgHistCR);
	cvShowImage(green.c_str(), imgHistCG);
	cvShowImage(blue.c_str(), imgHistCB);

	cvWaitKey(0);	// windows don't close automatically
	cvDestroyAllWindows();
	cvReleaseImage(&img);
}

// default histogram size : 256x64
// Scale 사용으로 사이즈 조정 가능
IplImage* DrawHistogram(CvHistogram *hist, float scaleX, float scaleY)
{
	// max값으로 다른 값들 scale - image의 세로 크기에 맞춤
	float histMax = 0;
	cvGetMinMaxHistValue(hist, 0, &histMax, 0, 0);

	// 히스토그램을 그릴 1채널 이미지, 이미지를 사이즈에 맞게 조절 
	IplImage *imgHist = cvCreateImage(cvSize(256 * scaleX, 64 * scaleY), IPL_DEPTH_8U, 1);
	cvZero(imgHist);

	// 모든 bin을 이용해서 이미지 렌더링
	for (int i = 0; i < 255; i++)
	{
		float histValue = cvGetReal1D(hist->bins, i);
		float nextValue = cvGetReal1D(hist->bins, i + 1);

		CvPoint pt1 = cvPoint(i*scaleX, 64*scaleY);
		CvPoint pt2 = cvPoint(i*scaleX+scaleX, 64*scaleY);
		CvPoint pt3 = cvPoint(i*scaleX+scaleX, (64 - nextValue*64/histMax)*scaleY);
		CvPoint pt4 = cvPoint(i*scaleX, (64 - nextValue * 64 / histMax)*scaleY);

		int numPts = 5;

		CvPoint pts[] = { pt1, pt2, pt3, pt4, pt1 };

		cvFillConvexPoly(imgHist, pts, numPts, cvScalar(255));
	}

	return imgHist;
}