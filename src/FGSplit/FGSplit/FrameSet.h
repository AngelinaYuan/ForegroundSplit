#ifndef _FrameSet_h
#define _FrameSet_h

#include <opencv2/opencv.hpp>
#include <string>

//===================================
//			֡���м���
//
//	path:			·��
//	getPath():		��ȡ·��
//	getNextFrame(): ��ȡ��һ֡
//	compreResult():	�������Ƚ�
//
//===================================
class FrameSet
{
public:
	FrameSet(const std::string& str) : path(str) {};

	// ��ȡ��һ֡
	virtual cv::Mat getNextInput(int k = 1) = 0;

	virtual cv::Mat getNextResult(int k = 1) = 0;

	std::string getPath() const { return path; }

private:
	std::string path;
};

//===============================================================
//
// ���бȽϣ�����þ�׼��Pr���ٻ���Re��F1ֵ
//
// TP: ����    TN: �渺    FP: ����    FN: �ٸ�
//
// Pr = TP / (TP + FP)	Ԥ��ֵΪ������������ʵֵΪ���ı���
// Re = TP / (TP + FN)	��ʵֵΪ���������б�Ԥ��Ϊ���ı���
// F1 = (2 x Pr x Re) / (Pr + Re)
//
//===============================================================
bool ForegroundCompare(const cv::Mat& src, const cv::Mat& dist, float& Pr, float& Re, float& F1);


#endif _FrameSet_h