#ifndef _Solution_h
#define _Solution_h

#include "FrameSet.h"

#define FILL_MERGE_FG_AREA		20		// �ϲ�������ǰ���ն���С
#define	DEL_MERGE_FG_AREA		10		// �ϲ���Ĩ����ǰ���ߵ��С

class Solution
{

public:
	Solution(FrameSet& p) : fs(p), cnt(0) {};

	void Run();

private:
	cv::Mat MergeFG(cv::Mat vibe_fg, cv::Mat ffd_fg);		// �ϲ������㷨���

	FrameSet& fs;
	int cnt;
};


#endif _Solution_h
