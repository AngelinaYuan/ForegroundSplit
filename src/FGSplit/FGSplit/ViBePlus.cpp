#include "ViBePlus.h"
#include <iostream>
#include <opencv2\imgproc\types_c.h>

using namespace std;
using namespace cv;

// ʹ��Ĭ�ϲ���
ViBePlus::ViBePlus(int num_samples, int min_match, int radius, int rand_sam)
{
	this->num_samples = num_samples;
	this->min_match = min_match;
	this->radius = radius;
	this->rand_sam = rand_sam;
}

// �ͷ��ڴ�
ViBePlus::~ViBePlus(void)
{
	delete samples_rgb;
	delete samples_gray;
	delete samples_ave;
	delete samples_sumSqr;
	delete samples_fgCnt;
	delete samples_bgInner;
	delete samples_innerState;
	delete samples_bLinkLevel;
	delete samples_maxInnerGrad;
}

// ������Ƶ֡��ִ��Vibe+�㷨���ǰ���ɰ棬�����±���ģ��
Mat ViBePlus::Run(Mat img)
{
	if (img.empty())
	{
		cout << "ERROR Run: The image is empty." << endl;
		return Mat();
	}
	// �ָ�ʽ�洢ͼ��
	FrameCapture(img);
	// �������ⶼΪ�գ���ʾδ��ʼ�������г�ʼ��
	if (!samples_rgb && !samples_gray)
	{
		// �����ڴ�
		InitMemory();
		// ��ʼ��ģ��
		InitModel();
		return Mat();
	}
	// ��ȡ�ָ��ɰ�
	ExtractBG();

	// �ɷָ��ɰ��������ɰ�
	CalUpdateModel();

	// ���ݸ����ɰ���±���ģ��
	UpdateModel();

	// ���طָ��ɰ�
	return segModel;
}

cv::Mat ViBePlus::getSegModel()
{
	return segModel;
}

cv::Mat ViBePlus::getUpdateModel()
{
	return updateModel;
}

// ��RGB�ͻҶȸ�ʽ�洢ͼ��
void ViBePlus::FrameCapture(cv::Mat img)
{
	img.copyTo(frame_rgb);
	if (img.channels() == 3)
	{
		cvtColor(frame_rgb, frame_gray, CV_BGR2GRAY);
		Channels = 3;
	}
	else
	{
		img.copyTo(frame_gray);
		Channels = 1;
	}
}

// �����ڴ�
void ViBePlus::InitMemory()
{
	segModel = Mat::zeros(frame_gray.size(), CV_8UC1);
	updateModel = Mat::zeros(frame_gray.size(), CV_8UC1);

	// Ϊ����������ڴ�
	samples_rgb = new unsigned char*** [frame_gray.rows];	// RGB������
	samples_gray = new unsigned char** [frame_gray.rows];	// �Ҷ�������

	// Ϊ��������ֵ����������ڴ�
	samples_ave = new double* [frame_gray.rows];
	samples_sumSqr = new double* [frame_gray.rows];

	// Ϊ����ģ�������Ϣ�����ڴ�
	samples_fgCnt = new int* [frame_gray.rows];
	samples_bgInner = new bool* [frame_gray.rows];
	samples_innerState = new int* [frame_gray.rows];
	samples_bLinkLevel = new int* [frame_gray.rows];
	samples_maxInnerGrad = new int* [frame_gray.rows];

	for (int i = 0; i < frame_gray.rows; ++i)
	{
		samples_rgb[i] = new unsigned char** [frame_gray.cols];
		samples_gray[i] = new unsigned char* [frame_gray.cols];
		samples_ave[i] = new double[frame_gray.cols];
		samples_sumSqr[i] = new double[frame_gray.cols];
		samples_fgCnt[i] = new int[frame_gray.cols];
		samples_bgInner[i] = new bool[frame_gray.cols];
		samples_innerState[i] = new int[frame_gray.cols];
		samples_bLinkLevel[i] = new int[frame_gray.cols];
		samples_maxInnerGrad[i] = new int[frame_gray.cols];
		for (int j = 0; j < frame_gray.cols; ++j)
		{
			samples_rgb[i][j] = new unsigned char* [num_samples];
			samples_gray[i][j] = new unsigned char[num_samples];
			for (int k = 0; k < num_samples; ++k)
			{
				samples_rgb[i][j][k] = new unsigned char[3];
				for (int m = 0; m < 3; ++m)
					samples_rgb[i][j][k][m] = 0;
				samples_gray[i][j][k] = 0;
			}
			samples_ave[i][j] = 0;
			samples_sumSqr[i][j] = 0;
			samples_fgCnt[i][j] = 0;
			samples_bgInner[i][j] = false;
			samples_innerState[i][j] = 0;
			samples_bLinkLevel[i][j] = 0;
			samples_maxInnerGrad[i][j] = 0;
		}
	}
}

// ��ʼ������ģ��
void ViBePlus::InitModel()
{
	RNG rng;	// RNG��OpenCV�е�C++�汾�����
	int row, col;
	// Ϊÿ�����ص㽨��������
	for (int i = 0; i < frame_gray.rows; ++i)
	{
		for (int j = 0; j < frame_gray.cols; ++j)
		{
			// ÿ�����ص���num_samples��������
			for (int k = 0; k < num_samples; ++k)
			{
				// 8���������ѡ��
				int random;
				random = rng.uniform(0, 9);		// ���ȷֲ�
				row = i + c_xoff[random];
				random = rng.uniform(0, 9);
				col = j + c_yoff[random];

				// ��ֹ���ص�Խ��
				if (row < 0) row = 0;
				if (row >= frame_gray.rows) row = frame_gray.rows - 1;
				if (col < 0) col = 0;
				if (col >= frame_gray.cols) col = frame_gray.cols - 1;

				// ���浽�Ҷȡ�RGB������
				samples_gray[i][j][k] = frame_gray.at<uchar>(row, col);
				for (int m = 0; m < 3; ++m)
					samples_rgb[i][j][k][m] = frame_rgb.at<Vec3b>(row, col)[m];

				// �ۼӵ�������ֵ
				samples_ave[i][j] += samples_gray[i][j][k];
			}
			// �����ֵ
			samples_ave[i][j] /= num_samples;

			// ���㷽��
			for (int k = 0; k < num_samples; ++k)
				samples_sumSqr[i][j] += pow(samples_gray[i][j][k] - samples_ave[i][j], 2);
			samples_sumSqr[i][j] /= num_samples;
		}
	}
}

// ��ȡ�ָ��ɰ�
void ViBePlus::ExtractBG()
{
	RNG rng;
	for (int i = 0; i < frame_gray.rows; ++i)
	{
		for (int j = 0; j < frame_gray.cols; ++j)
		{
			//====  ��������Ӧ��ֵ  ====//
			// ���������Ӧ��ֵ
			double adaThreshold = DEFAULT_THRESHOLD_MIN;
			// ���ݷ�������׼��
			double sigma = sqrt(samples_sumSqr[i][j]);
			// �õ����������Ӧ��ֵ
			adaThreshold = sigma * AMP_MULTIFACTOR;

			// ��������Ӧ��ֵ��Χ
			if (adaThreshold < DEFAULT_THRESHOLD_MIN)
				adaThreshold = DEFAULT_THRESHOLD_MIN;
			if (adaThreshold > DEFAULT_THRESHOLD_MAX)
				adaThreshold = DEFAULT_THRESHOLD_MAX;

			//====  ������ɫ�����ƥ�����  ====//
			// ��ǰ֡��(i, j)���RGBͨ��ֵ
			int B = frame_rgb.at<Vec3b>(i, j)[0];
			int G = frame_rgb.at<Vec3b>(i, j)[1];
			int R = frame_rgb.at<Vec3b>(i, j)[2];

			int k = 0, matches = 0;
			for (; matches < min_match && k < num_samples; ++k)
			{
				// ��ǰ֡��(i, j)��ĵ�k���������RGBͨ��ֵ
				int B_sam = samples_rgb[i][j][k][0];
				int G_sam = samples_rgb[i][j][k][1];
				int R_sam = samples_rgb[i][j][k][2];

				// ��ɫ����
				//double colorDist, RGB_Norm2, RGBSam_Norm2, RGB_Vev, p2;
				double RGB_Norm2 = pow(B, 2) + pow(G, 2) + pow(R, 2);
				double RGBSam_Norm2 = pow(B_sam, 2) + pow(G_sam, 2) + pow(R_sam, 2);
				double RGB_Vec = pow(R_sam * R + G_sam * G + B_sam * B, 2);
				double p2 = RGB_Vec / RGBSam_Norm2;
				double colordist = (RGB_Norm2 > p2) ? sqrt(RGB_Norm2 - p2) : 0;

				// ����ǰֵ������ֵ֮��С������Ӧ��ֵ������ɫ����С����ֵ������ƥ������
				int dist = abs(samples_gray[i][j][k] - frame_gray.at<uchar>(i, j));
				if (dist < adaThreshold && colordist < DEFAULT_COLOR_THRESHOLD)
					++matches;
			}
			// ƥ���������#minָ��Ϊ�����㣬����Ϊǰ����
			if (matches >= min_match)
			{
				// ��ǰ��Ϊ�����㣬����ǰ��ͳ����
				samples_fgCnt[i][j] = 0;

				// �ڷָ��ɰ��б��Ϊ������
				segModel.at<uchar>(i, j) = 0;
			}
			else
			{
				// ��ǰ��Ϊǰ���㣬ǰ��ͳ������һ
				samples_fgCnt[i][j]++;

				// �ڷָ��ɰ��б��Ϊǰ����
				segModel.at<uchar>(i, j) = 255;

				// ����ǰ��������������ֵʱ���õ�Ӧ�ñ���Ϊ�Ǳ�����
				if (samples_fgCnt[i][j] > DEFAULT_FG_COUNT)
				{
					// ���ѡ��õ��������һ��λ�ñ��浱ǰ��
					int random = rng.uniform(0, num_samples);
					samples_gray[i][j][random] = frame_gray.at<uchar>(i, j);
					for (int m = 0; m < 3; ++m)
						samples_rgb[i][j][random][m] = frame_rgb.at<Vec3b>(i, j)[m];
				}
			}
		}
	}
}

// �ɷָ��ɰ��������ɰ�
void ViBePlus::CalUpdateModel()
{
	Mat img, imgtmp;
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;

	segModel.copyTo(updateModel);

	segModel.copyTo(img);
	updateModel.copyTo(imgtmp);

	// ��ȡ����
	findContours(imgtmp, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE);

	// ��������
	for (int i = 0; i < contours.size(); ++i)
	{
		// һ��������
		int father = hierarchy[i][3];
		// ����������
		int grandpa = -1;
		if (father >= 0)
			grandpa = hierarchy[father][3];


		// ��һ����������û�ж�����������˵����Ϊǰ���׶�
		if (father >= 0 && grandpa < 0)
		{
			// ���������С
			double area = contourArea(contours[i]);

			// �Ը����ɰ��п׶����С��Ĭ��ֵ�Ľ������
			if (area <= FILL_UP_AREA_SIZE)
				drawContours(updateModel, contours, i, Scalar(255), -1);

			// �Էָ��ɰ��п׶����С��Ĭ��ֵ�Ľ������
			if (area <= FILL_SEG_AREA_SIZE)
				drawContours(segModel, contours, i, Scalar(255), -1);
		}

		// �޸�������˵��������������Χ��������ǰ���ߵ�����Ĩ����С��ǰ���ߵ�
		if (father < 0)
		{
			if (contourArea(contours[i]) < DEL_SEG_AREA_SIZE)
				drawContours(segModel, contours, i, Scalar(0), -1);
		}
	}

	// ����ӵ������8������ڲ����ص�
	for (int i = 1; i < frame_gray.rows - 1; ++i)
	{
		for (int j = 1; j < frame_gray.cols - 1; ++j)
		{
			// ������״̬λ
			int state = 0;
			int maxGrad = 0;

			// ���ұ����ڱ�Ե
			if (img.at<uchar>(i, j) == 0)
			{
				// ����8����Χ
				int i_min = i - 1;
				int i_max = i + 1;
				int j_min = j - 1;
				int j_max = j + 1;

				// ����8����״̬������Ҷ�����ݶ�
				for (int x = i_min; x < i_max; ++x)
				{
					for (int y = j_max; y < j_max; ++y)
					{
						// �ų���ǰ��
						if (x == i && y == j)
							continue;

						// ������Ƿ�Ϊǰ����
						int bitstate = (img.at<uchar>(x, y) == 255) ? 1 : 0;
						state = (state << 1) + bitstate;

						// ��������ݶ�
						maxGrad = max(abs(frame_gray.at<uchar>(i, j) - frame_gray.at<uchar>(x, y)), maxGrad);
					}
				}
				// ��ǰ״̬λ�޶���8bit��
				state = state & 255;

				// ״̬λ����0����˵��8��������ǰ���㣬����ǰ��Ϊ�����ڱ�Ե
				samples_bgInner[i][j] = (state > 0);
			}
			else
			{
				samples_bgInner[i][j] = false;
				samples_innerState[i][j] = 0;
			}

			// ������˸�ȼ�
			if (samples_bgInner[i][j])
			{
				// ��ǰ��8����״̬����һ֡��ͬ��˵����ǰ�㲻��˸����˸�ȼ�����
				if (state == samples_innerState[i][j])
					samples_bLinkLevel[i][j] = max(samples_bLinkLevel[i][j] - MINUS_BLINK_LEVEL, 0);
				else
				{
					// ��ǰ����˸����˸�ȼ�����
					samples_bLinkLevel[i][j] += ADD_BLINK_LEVEL;
					samples_bLinkLevel[i][j] = min(samples_bLinkLevel[i][j], MAX_BLINK_LEVEL);
				}
			}
			else
			{
				// �Ǳ����ڱ�Ե��˸�ȼ�����
				samples_bLinkLevel[i][j] = max(samples_bLinkLevel[i][j] - MINUS_BLINK_LEVEL, 0);
			}

			// ����״̬λ������ݶ�
			samples_bLinkLevel[i][j] = state;
			samples_maxInnerGrad[i][j] = maxGrad;

			// ��˸�ȼ�������ֵʱ���Ӹ����ɰ����Ƴ�
			if (samples_bLinkLevel[i][j] > TC_BLINK_LEVEL)
				updateModel.at<uchar>(i, j) = 255;
		}
	}
}

// ���ݸ����ɰ���±���ģ��
void ViBePlus::UpdateModel()
{
	RNG rng;
	for (int i = 0; i < frame_gray.rows; ++i)
	{
		for (int j = 0; j < frame_gray.cols; ++j)
		{
			// ֻ�Ա�������и��£�����ǰ����
			if (updateModel.at<uchar>(i, j) > 0)
				continue;

			// ���¸õ������⣬����Ϊ 1/rand_sam
			int random = rng.uniform(0, rand_sam);
			if (random == 0)
			{
				// ���ѡȡ�������е�������
				random = rng.uniform(0, num_samples);

				// ����������
				uchar newVal = frame_gray.at<uchar>(i, j);
				samples_gray[i][j][random] = newVal;
				for (int m = 0; m < 3; ++m)
					samples_rgb[i][j][random][m] = frame_rgb.at<Vec3b>(i, j)[m];

				// �����������ֵ������
				UpdatePixSampleAveAndSumSqr(i, j);
			}

			// ͬʱ�����ھӵ�������⣬����Ϊ 1/rand_sam
			random = rng.uniform(0, rand_sam);
			if (random == 0)
			{
				// ���ݵ�ǰ������ݶ�
				if (samples_maxInnerGrad[i][j] > MAX_INNER_GRAD)
					continue;

				// ���ѡȡ�ھӵ�
				random = rng.uniform(0, 9);
				int row = i + c_xoff[random];
				random = rng.uniform(0, 9);
				int col = j + c_yoff[random];

				// ��ֹ���ص�Խ��
				if (row < 0) row = 0;
				if (row >= frame_gray.rows) row = frame_gray.rows - 1;
				if (col < 0) col = 0;
				if (col >= frame_gray.cols) col = frame_gray.cols - 1;

				// ���ѡȡ�������е�������
				random = rng.uniform(0, num_samples);

				// ����������
				uchar newVal = frame_gray.at<uchar>(i, j);
				samples_gray[row][col][random] = newVal;
				for (int m = 0; m < 3; ++m)
					samples_rgb[row][col][random][m] = frame_rgb.at<Vec3b>(i, j)[m];

				// �����������ֵ������
				UpdatePixSampleAveAndSumSqr(row, col);
			}
		}
	}
}

// �����������ֵ������
void ViBePlus::UpdatePixSampleAveAndSumSqr(int i, int j)
{
	double ave = 0, sumSqr = 0;

	// �����ֵ
	for (int m = 0; m < num_samples; ++m)
		ave += samples_gray[i][j][m];
	ave /= num_samples;

	// д���ֵ
	samples_ave[i][j] = ave;

	// ���㷽��
	for (int m = 0; m < num_samples; ++m)
		sumSqr += pow(samples_gray[i][j][m] - ave, 2);
	sumSqr /= num_samples;

	// д�뷽��
	samples_sumSqr[i][j] = sumSqr;
}
