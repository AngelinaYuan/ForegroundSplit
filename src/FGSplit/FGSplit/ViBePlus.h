#ifndef _ViBePlus_h
#define _ViBePlus_h

#include <opencv2/opencv.hpp>
#include "ViBePlusMacro.h"

class ViBePlus
{
public:
	ViBePlus(int num_samples = DEFAULT_NUM_SAMPLES,
		int min_match = DEFAULT_MIN_MATCH,
		int radius = DEFAULT_RADIUS,
		int rand_sam = DEFAULT_RAND_SAM);

	~ViBePlus(void);

	cv::Mat Run(cv::Mat img);	// ִ��Vibe+�㷨������ɰ�

	cv::Mat getSegModel();		// ���طָ��ɰ� 
	cv::Mat getUpdateModel();	// ���ظ����ɰ�

private:
	void FrameCapture(cv::Mat img);	// ��RGB�ͻҶȸ�ʽ�洢ͼ��

	void InitMemory();	// �����ڴ�

	void InitModel();	// ��ʼ������ģ��	

	void ExtractBG();		// ��ȡ�ָ��ɰ�

	void CalUpdateModel();	// ��������ɰ�

	void UpdateModel();		// ���±���ģ��s

	void UpdatePixSampleAveAndSumSqr(int i, int j);		// �����������ֵ������

	int num_samples;	// ������
	int min_match;		// #min����
	int radius;			// �뾶
	int rand_sam;		// ��������

	cv::Mat frame_rgb;		// ��ǰ֡ͼ�� RGB�汾
	cv::Mat frame_gray;		// �Ҷ�ͼ��汾
	int Channels;		// ͨ������RGBΪ3���Ҷ�Ϊ1

	unsigned char**** samples_rgb = nullptr;	// RGB��������
	unsigned char*** samples_gray = nullptr;	// �Ҷ���������

	double** samples_ave;		// ��������ֵ
	double** samples_sumSqr;	// ����������

	int** samples_fgCnt;		// ��������ǰ������

	bool** samples_bgInner;		// �����Ƿ�Ϊ������Ե
	int** samples_innerState;	// ����������״̬

	int** samples_bLinkLevel;	// ������˸�ȼ�

	int** samples_maxInnerGrad;	// ���������ݶ����ֵ

	cv::Mat segModel;		// �ָ��ɰ�
	cv::Mat updateModel;	// �����ɰ�

	int c_xoff[9] = { -1,  0,  1, -1, 1, -1, 0, 1, 0 };		// x���ھӵ�
	int c_yoff[9] = { -1,  0,  1, -1, 1, -1, 0, 1, 0 };		// y���ھӵ�
};



#endif _ViBePlus_h
