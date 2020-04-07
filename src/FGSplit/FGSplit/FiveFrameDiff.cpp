#include "FiveFrameDiff.h"
#include <opencv2/imgproc/types_c.h>

using namespace std;
using namespace cv;

cv::Mat FiveFrameDiff::Run(cv::Mat img)
{
    if (img.empty())
    {
        cout << "ERROR FiveFrameDiff: The image is empty." << endl;
        return Mat();
    }

    Mat frame;

    // ͼ��ҶȻ�����ֵ�˲�
    Mat imgtmp;
    if (img.channels() == 3)
    {
        cvtColor(img, imgtmp, CV_BGR2GRAY);
        medianBlur(imgtmp, frame, MEDIAN_FILTER_SIZE);
    }
    else
        medianBlur(img, frame, MEDIAN_FILTER_SIZE);

    // δ����֡ʱ�����в��
    if (capacity != 0)
    {
        // ���浽��������
        frame.copyTo(frame_vec[5 - capacity]);
        // �����֡8�����
        CalNeighSum(5 - capacity);
        // �պ�����֡�ɿ�ʼ
        if (--capacity == 0)
            goto startpos;
        return Mat();
    }
    else
    {
        // �滻������һ֡
        int oldest = (id + 5 - 2) % 5;
        frame.copyTo(frame_vec[oldest]);
        // �����֡8�����
        CalNeighSum(oldest);
        id = (id + 1) % 5;
    }
startpos:
    // �����±�
    int index[5];
    for (int i = 0; i < 5; ++i)
        index[i] = (id + 5 - 2 + i) % 5;

    Mat fgModel = Mat::zeros(frame_vec[0].size(), CV_8UC1);

    // ������� diffs[4][rows][cols]
    vector<vector<vector<bool>>> diffs(4, vector<vector<bool>>(frame.rows, vector<bool>(frame.cols, false)));
    for (int i = 1; i < frame.rows - 1; ++i)
    {
        for (int j = 1; j < frame.cols - 1; ++j)
        {
            for (int k = 0; k < 4; ++k)
            {
                // С����ֵT2��Ϊ������
                if (neighSum[index[k + 1]][i][j] <= FRAME_DIFF_THRESHOLD_2)
                    continue;
                uchar res = abs(frame_vec[index[k]].at<uchar>(i, j) - frame_vec[index[k + 1]].at<uchar>(i, j));
                // ������ֵT1Ϊǰ����
                if (res > FRAME_DIFF_THRESHOLD_1)
                    diffs[k][i][j] = true;
            }

            // Diff1 = (d12 || d23) && (d23 || d34)
            bool Diff1 = (diffs[0][i][j] || diffs[1][i][j]) && (diffs[1][i][j] || diffs[2][i][j]);
            // Diff2 = (d23 || d34) && (d34 || d45)
            bool Diff2 = (diffs[1][i][j] || diffs[2][i][j]) && (diffs[2][i][j] || diffs[3][i][j]);

            // Dfg = Diff1 & Diff2
            if (Diff1 && Diff2)
                fgModel.at<uchar>(i, j) = 255;
        }
    }

    return fgModel;
}

// �����֡8�����
void FiveFrameDiff::CalNeighSum(int k)
{
    // ��ʼ���������ڴ�
    if (neighSum == nullptr)
    {
        if (frame_vec[k].empty())
        {
            cout << "ERROR FiveFrameDiff: Can not get image size." << endl;
            return;
        }
        neighSum = new int** [5];
        for (int i = 0; i < 5; ++i)
        {
            neighSum[i] = new int* [frame_vec[k].rows];
            for (int j = 0; j < frame_vec[k].rows; ++j)
            {
                neighSum[i][j] = new int[frame_vec[k].cols];
                for (int m = 0; m < frame_vec[k].cols; ++m)
                    neighSum[i][j][m] = 0;
            }
        }
    }

    // �������������е�k֡�������
    for (int x = 1; x < frame_vec[k].rows - 1; ++x)
    {
        for (int y = 1; y < frame_vec[k].cols - 1; ++y)
        {
            neighSum[k][x][y] = 0;
            for (int m = 0; m < 8; ++m)
            {
                int row = x + c_xoff[m];
                int col = y + c_yoff[m];
                neighSum[k][x][y] += frame_vec[k].at<uchar>(row, col);

            }
        }
    }
}
