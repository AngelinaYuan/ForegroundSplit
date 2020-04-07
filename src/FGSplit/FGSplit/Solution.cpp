#include "Solution.h"

#include <iostream>
#include "ViBePlus.h"
#include "FiveFrameDiff.h"
#include <queue>
#include <opencv2/imgproc/types_c.h>
#include <iomanip>
#include <fstream>

using namespace std;
using namespace cv;

void CalNewAve(double& ave, double val, int n)
{
    ave = (ave * (n - 1) + val) / n;
}

void Solution::Run()
{
    ofstream msgfile, resfile;
    if (msg_save)
        msgfile = ofstream(file_name_msg, ios::out);

    Mat frame_in, frame_res;

    ViBePlus vibeplus;
    FiveFrameDiff ffd;

    // ��֡��ַ�ǰ��֡�����
    for (int i = 0; i < 2; ++i)
    {
        frame_in = fs.getNextInput();
        if (frame_in.empty())
            return;
        ffd.Run(frame_in);
    }
    // �ڶ�֡ Vibe+��ģ
    vibeplus.Run(frame_in);

    // ���֡����ǰ��֡
    fs.getNextResult(2);
    start_id += 2;

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
    double start, time_vibe, time_ffd, time_merge;
    double time_ave_vibe, time_ave_ffd, time_ave_merge;
    time_ave_vibe = time_ave_ffd = time_ave_merge = 0;

    // ViBe+
    double Pr_vibe, Re_vibe, F1_vibe;
    double Pr_ave_vibe, Re_ave_vibe, F1_ave_vibe;
    Pr_ave_vibe = Re_ave_vibe = F1_ave_vibe = 0;

    // ffd
    double Pr_ffd, Re_ffd, F1_ffd;
    double Pr_ave_ffd, Re_ave_ffd, F1_ave_ffd;
    Pr_ave_ffd = Re_ave_ffd = F1_ave_ffd = 0;

    // merge
    double Pr_merge, Re_merge, F1_merge;
    double Pr_ave_merge, Re_ave_merge, F1_ave_merge;
    Pr_ave_merge = Re_ave_merge = F1_ave_merge = 0;

    int part_cnt = part;
    while (!inque.empty())
    {
        Pr_vibe = Re_vibe = F1_vibe = 0;
        Pr_ffd = Re_ffd = F1_ffd = 0;
        Pr_merge = Re_merge = F1_merge = 0;

        ++cnt;
        // vibe+�����ffd��������ս������ǰ��������֡����ǰ������֡
        Mat vibe_fg, ffd_fg, fg, input, result;

        //======  ViBe+ START  =======//
        start = static_cast<double>(getTickCount());

        // �Ӷ��л�ȡ��ǰ��������֡�����֡
        inque.front().copyTo(input);
        inque.pop();
        if (input.empty())
            break;
        resque.front().copyTo(result);
        resque.pop();

        // Vibe+�ӵ�ǰ���봦��
        vibe_fg = vibeplus.Run(input);

        time_vibe = ((double)getTickCount() - start) / getTickFrequency() * 1000;
        //======  ViBe+ MESSAGE  =======//
        // ����ƽ��ֵ
        time_ave_vibe = (time_ave_vibe * (cnt - 1) + time_vibe) / cnt;
        if (!result.empty() && ForegroundCompare(vibe_fg, result, Pr_vibe, Re_vibe, F1_vibe))
        {
            // ����ƽ��ֵ
            CalNewAve(Pr_ave_vibe, Pr_vibe, cnt);
            CalNewAve(Re_ave_vibe, Re_vibe, cnt);
            CalNewAve(F1_ave_vibe, F1_vibe, cnt);

        }
        //======  Vibe+ END =======//

        //======  ffd START  =======//
        start = static_cast<double>(getTickCount());

        // ffd�Ӻ���֡����
        frame_in = fs.getNextInput();
        if (!frame_in.empty())
        {
            ffd_fg = ffd.Run(frame_in);
            inque.push(frame_in);

            frame_res = fs.getNextResult();
            resque.push(frame_res);
        }

        time_ffd = ((double)getTickCount() - start) / getTickFrequency() * 1000;
        //======  ffd MESSAGE  =======//
        // ����ƽ��ֵ
        time_ave_ffd = (time_ave_ffd * (cnt - 1) + time_vibe) / cnt;
        if (!result.empty() && ForegroundCompare(ffd_fg, result, Pr_ffd, Re_ffd, F1_ffd))
        {
            // ����ƽ��ֵ
            CalNewAve(Pr_ave_ffd, Pr_ffd, cnt);
            CalNewAve(Re_ave_ffd, Re_ffd, cnt);
            CalNewAve(F1_ave_ffd, F1_ffd, cnt);
        }
        //======  Vibe+ END =======//

        //======  merge START  =======//
        start = static_cast<double>(getTickCount());

        fg = MergeFG(vibe_fg, ffd_fg);

        time_merge = ((double)getTickCount() - start) / getTickFrequency() * 1000;
        time_merge += time_vibe + time_ffd;
        //======  merge MESSAGE  =======//
        // ����ƽ��ֵ
        time_ave_merge = (time_ave_merge * (cnt - 1) + time_merge) / cnt;
        if (!result.empty() && ForegroundCompare(fg, result, Pr_merge, Re_merge, F1_merge))
        {
            // ����ƽ��ֵ
            CalNewAve(Pr_ave_merge, Pr_merge, cnt);
            CalNewAve(Re_ave_merge, Re_merge, cnt);
            CalNewAve(F1_ave_merge, F1_merge, cnt);
        }
        //======  merge END =======//

        if (showed_input && !input.empty())
            imshow("����", input);

        if (showed_res_fg && !result.empty())
            imshow("�Ա�", result);

        if (showed_viBe_fg && !vibe_fg.empty())
            imshow("vibeǰ���ɰ�", vibe_fg);

        if (showed_vibe_up)
            imshow("�����ɰ�", vibeplus.getUpdateModel());

        if (showed_ffd_fg && !ffd_fg.empty())
            imshow("֡��ǰ���ɰ�", ffd_fg);

        if (showed_output && !fg.empty())
            imshow("����ǰ���ɰ�", fg);

        cv::waitKey(25);

        if (--part_cnt > 0)
            continue;
        part_cnt = part;

        stringstream ss;
        ss << "======================================================\n";
        // PRINT IMAGE NUMBER
        ss << "NO: ";
        ss << setw(4) << (cnt + start_id) << "  TIME[ms]      Pr[%]      Re[%]      F1[%]\n";

        ss << "ViBe+   ";
        ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << time_vibe << " ";
        ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << Pr_vibe << " ";
        ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << Re_vibe << " ";
        ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << F1_vibe << " ";
        ss << "\n";

        ss << "FFD     ";
        ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << time_ffd << " ";
        ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << Pr_ffd << " ";
        ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << Re_ffd << " ";
        ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << F1_ffd << " ";
        ss << "\n";

        ss << "Merge   ";
        ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << time_merge << " ";
        ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << Pr_merge << " ";
        ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << Re_merge << " ";
        ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << F1_merge << " ";
        ss << "\n";

        if (msg_prt)
            cout << ss.str();

        if (msg_save)
            msgfile << ss.str();
    }
    msgfile.close();

    stringstream ss;

    ss << "======================================================\n";
    ss << "AVERAGE " << "  TIME[ms]      Pr[%]      Re[%]      F1[%]\n";

    ss << "ViBe+   ";
    ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << time_ave_vibe << " ";
    ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << Pr_ave_vibe << " ";
    ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << Re_ave_vibe << " ";
    ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << F1_ave_vibe << " ";
    ss << "\n";

    ss << "FFD     ";
    ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << time_ave_ffd << " ";
    ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << Pr_ave_ffd << " ";
    ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << Re_ave_ffd << " ";
    ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << F1_ave_ffd << " ";
    ss << "\n";

    ss << "Merge   ";
    ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << time_ave_merge << " ";
    ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << Pr_ave_merge << " ";
    ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << Re_ave_merge << " ";
    ss << setw(10) << setiosflags(ios::fixed) << setprecision(2) << F1_ave_merge << " ";
    ss << "\n";

    cout << ss.str();
    if (res_save)
    {
        if (res_save)
            resfile = ofstream(file_name_res, ios::out);
        resfile << ss.str();
        resfile.close();
    }
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

void Solution::setShowed_vibe_up(bool value)
{
    showed_vibe_up = value;
}

void Solution::setFile_name_res(const std::string& value)
{
    file_name_res = value;
}

void Solution::setFile_name_msg(const std::string& value)
{
    file_name_msg = value;
}

void Solution::setRes_save(bool value)
{
    res_save = value;
}

void Solution::setMsg_save(bool value)
{
    msg_save = value;
}

void Solution::setMsg_prt(bool value)
{
    msg_prt = value;
}

void Solution::setPart(int value)
{
    part = value;
    if (part <= 0)
        part = 1;
}

void Solution::setShowed_res_fg(bool value)
{
    showed_res_fg = value;
}

void Solution::setShowed_output(bool value)
{
    showed_output = value;
}

void Solution::setShowed_input(bool value)
{
    showed_input = value;
}

void Solution::setShowed_ffd_fg(bool value)
{
    showed_ffd_fg = value;
}

void Solution::setShowed_viBe_fg(bool value)
{
    showed_viBe_fg = value;
}
