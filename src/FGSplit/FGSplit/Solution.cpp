#include "Solution.h"

#include <iostream>
#include "ViBePlus.h"
#include "FiveFrameDiff.h"
#include <queue>
#include <opencv2\imgproc\types_c.h>

using namespace std;
using namespace cv;

void Solution::Run()
{
	Mat frame_in, frame_res;

	ViBePlus vibeplus;
	FiveFrameDiff ffd;

	// ��֡��ַ�ǰ��֡�����
	for (int i = 0; i < 2; ++i)
	{
		frame_in = fs.getNextInput();
		ffd.Run(frame_in);
	}
	// �ڶ�֡ Vibe+��ģ
	vibeplus.Run(frame_in);

	// ���֡����ǰ��֡
	fs.getNextResult(2);

	// �����������֡����֡��ַ���Ҫ��ǰ֪������֡
	queue<Mat> inque, resque;
	for (int i = 0; i < 2; ++i)
	{
		frame_in = fs.getNextInput();
		inque.push(frame_in);
		ffd.Run(frame_in);

		frame_res = fs.getNextResult();
		resque.push(frame_res);
	}

	// ��������ʱ��ͳ�Ʊ���
	double time, start;

	float Pr, Re, F1;
	double Pr_last, Re_last, F1_last;
	Pr_last = Re_last = F1_last = 0;

	cout << "IMAGE ID    IIME(ms)      Pr(%)      Re(%)      F1(%)" << endl;

	while (!inque.empty())
	{
		// vibe�����ffd��������ս������ǰ��������֡����ǰ������֡
		Mat vibe_fg, ffd_fg, fg, input, result;

		start = static_cast<double>(getTickCount());

		// �Ӷ��л�ȡ��ǰ��������֡�����֡
		inque.front().copyTo(input);
		inque.pop();
		if (input.empty())
			continue;
		resque.front().copyTo(result);
		resque.pop();

		// Vibe+�ӵ�ǰ���봦��
		vibe_fg = vibeplus.Run(input);

		// ffd�Ӻ���֡����
		frame_in = fs.getNextInput();
		if (!frame_in.empty())
		{
			ffd_fg = ffd.Run(frame_in);
			inque.push(frame_in);

			frame_res = fs.getNextResult();
			resque.push(frame_res);
		}

		fg = MergeFG(vibe_fg, ffd_fg);

		time = ((double)getTickCount() - start) / getTickFrequency() * 1000;

		std::cout << setw(6) << ++cnt;
		std::cout << setw(14) << time;
		if (!result.empty() && ForegroundCompare(fg, result, Pr, Re, F1))
		{
			std::cout << setw(11) << Pr;
			std::cout << setw(11) << Re;
			std::cout << setw(11) << F1;

			// ����ƽ��ֵ
			Pr_last = (Pr_last * (cnt - 1) + Pr) / cnt;
			Re_last = (Re_last * (cnt - 1) + Re) / cnt;
			F1_last = (F1_last * (cnt - 1) + F1) / cnt;
		}
		//if(Pr < 1)
		//{
		//	imshow("�Ա�", result);
		//	imshow("����ǰ���ɰ�", fg);
		//}
		std::cout << endl;

		if (!ffd_fg.empty() && !vibe_fg.empty() && !fg.empty())
		{
			imshow("����", input);
			if(!result.empty())
				imshow("�Ա�", result);
			//imshow("vibeǰ���ɰ�", vibe_fg);
			//imshow("֡��ǰ���ɰ�", ffd_fg);
			imshow("����ǰ���ɰ�", fg);
			//imshow("�����ɰ�", vibeplus.getupdatemodel());
		}
		cv::waitKey(25);
	}
	cout << "Last Pr: " << Pr_last << endl;
	cout << "Last Re: " << Re_last << endl;
	cout << "Last F1: " << F1_last << endl;
}

cv::Mat Solution::MergeFG(cv::Mat vibe_fg, cv::Mat ffd_fg)
{
	if (vibe_fg.empty())
		return ffd_fg;
	if (ffd_fg.empty())
		return vibe_fg;

	Mat fg;
	vibe_fg.copyTo(fg);

	// �ϲ�����ǰ���ɰ�
	for (int i = 0; i < vibe_fg.rows; ++i)
	{
		for (int j = 0; j < vibe_fg.rows; ++j)
		{
			if (ffd_fg.at<uchar>(i, j) == 255)
				fg.at<uchar>(i, j) = 255;
		}
	}

	Mat imgtmp;
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	fg.copyTo(imgtmp);

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

			// �׶����С��Ĭ��ֵ�Ľ������
			if (area <= FILL_MERGE_FG_AREA)
				drawContours(fg, contours, i, Scalar(255), -1);
		}

		// �޸�������˵��������������Χ��������ǰ���ߵ�����Ĩ����С��ǰ���ߵ�
		if (father < 0)
		{
			if (contourArea(contours[i]) < DEL_MERGE_FG_AREA)
				drawContours(fg, contours, i, Scalar(0), -1);
		}
	}


	return fg;
}


