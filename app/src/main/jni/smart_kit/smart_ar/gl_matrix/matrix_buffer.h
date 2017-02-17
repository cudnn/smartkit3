#ifndef _BUFFER_H_
#define _BUFFER_H_

#include <opencv2/opencv.hpp>
#include <queue>

class MatrixBuffer
{
public:
	MatrixBuffer() : m_fixed_size(5), m_sum(cv::Matx44d::zeros()) {}
	~MatrixBuffer() {}

	void							push(const cv::Matx44d& mt);
	cv::Matx44d						get_aver();
	void							clear();
	void                            reset_fixed_size(int len);

private:
	std::queue<cv::Matx44d>			m_queue;
	int								m_fixed_size;
	cv::Matx44d						m_sum;
};

#endif
