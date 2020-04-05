#ifndef _VideoTest_h
#define _VideoTest_h

#include "FrameSet.h"
#include <string>

class VideoTest : public FrameSet
{
public:
	VideoTest(const std::string& path);

	// ͨ�� FrameSet �̳�
	virtual cv::Mat getNextInput(int k = 1) override;

	// ͨ�� FrameSet �̳�
	virtual cv::Mat getNextResult(int k = 1) override;

private:
	cv::VideoCapture capture;
};


#endif _VideoTest_h
