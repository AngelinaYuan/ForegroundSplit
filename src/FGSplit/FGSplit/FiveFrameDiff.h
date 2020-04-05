#ifndef _FiveFrameDiff_h
#define _FiveFrameDiff_h

#include <opencv2/opencv.hpp>
#include "FiveFrameDiffMacro.h"

class FiveFrameDiff
{
public:
	FiveFrameDiff() : id(2), capacity(5), neighSum(nullptr) {};

	~FiveFrameDiff() { delete neighSum; }

	cv::Mat Run(cv::Mat img);		// ִ����֡��ַ�������ǰ���ɰ�

private:
	void CalNeighSum(int k);		// ���������;

	cv::Mat frame_vec[5];	// �����֡��������
	int id;							// ��ǰ֡�����������±꣬��ʼΪ2
	int capacity;

	int*** neighSum;				// 8����֮��

	int c_xoff[9] = { -1,  0,  1, -1, 1, -1, 0, 1, 0 };
	int c_yoff[9] = { -1,  0,  1, -1, 1, -1, 0, 1, 0 };
};

#endif _FiveFrameDiff_h