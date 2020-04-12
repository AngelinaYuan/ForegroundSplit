#ifndef FIVEFRAMEDIFF_H
#define FIVEFRAMEDIFF_H

#include <opencv2/opencv.hpp>
#include "FiveFrameDiffMacro.h"

class FiveFrameDiff
{
public:
    FiveFrameDiff() : id(2), capacity(5) {} //, neighSum(nullptr) {};

    ~FiveFrameDiff() {} //{ delete neighSum; }

    cv::Mat Run(cv::Mat img);		// ִ����֡��ַ�������ǰ���ɰ�

private:
    //void CalNeighSum(int k);		// ���������;

    cv::Mat FirstProcess(cv::Mat img);  // Ԥ������ֵ�˲���ͼ��ҶȻ�

    cv::Mat FinalProcess(cv::Mat img);  // ���ڴ����ߵ�ȥ�����׶����

    cv::Mat frame_vec[5];           // �����֡��������
    int id;							// ��ǰ֡�����������±꣬��ʼΪ2
    int capacity;

    //int*** neighSum;				// 8����֮��

    int c_xoff[9] = { -1,  0,  1, -1, 1, -1, 0, 1, 0 };
    int c_yoff[9] = { -1,  0,  1, -1, 1, -1, 0, 1, 0 };
};

#endif // FIVEFRAMEDIFF_H
