#ifndef _CDW_2014_Test_h
#define _CDW_2014_Test_h

#include <string>
#include "FrameSet.h"

/*		CDW-2014���ݼ��Ӽ�Ŀ¼��Ϣ����		*/
#define DEFAULT_FOLDER_INPUT	"/input/"			// ��Ƶ֡Ŀ¼
#define DEFAULT_FOLDER_RESULT	"/groundtruth/"		// ����ɰ�Ŀ¼
#define DEFAULT_FILE_MESSAGE	"/temporalROI.txt"	// ������Ϣ����ʼ֡��  ��֡��
#define PREFIX_OF_INPUT_FILE	"in"				// ����ͼƬ����ǰ׺
#define PREFIX_OF_RESULT_FILE	"gt"				// ���ͼƬ����ǰ׺
#define INPUT_IMAGE_TYPE		".jpg"				// ����ͼƬ��ʽ
#define RESULT_IMAGE_TYPE		".png"				// �Ա�ͼƬ��ʽ


class CDW_2014_Test : public FrameSet
{
public:
	CDW_2014_Test(const std::string& path);

	// ͨ�� FrameSet �̳�
	virtual cv::Mat getNextInput(int k = 1) override;

	// ͨ�� FrameSet �̳�
	virtual cv::Mat getNextResult(int k = 1) override;

	void setId(int id);
	void FromStart(int k = 0);		// ��ָ�� input_id �� res_id ָ�� start + k
private:
	int start_id, end_id, input_id, res_id;
};



#endif _CDW_2014_Test_h