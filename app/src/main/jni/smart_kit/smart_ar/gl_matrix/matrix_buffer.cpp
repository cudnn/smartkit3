#include "matrix_buffer.h"
using namespace std;
using namespace cv;

void MatrixBuffer::push(const cv::Matx44d& mt)
{
	if (m_queue.size() == m_fixed_size)
	{
		const cv::Matx44d& x = m_queue.front();
		m_sum -= x;
		m_queue.pop();
	}
	m_queue.push(mt);
	m_sum += mt;
}

cv::Matx44d MatrixBuffer::get_aver()
{
	double d = std::min((double)m_queue.size(), (double)m_fixed_size) + 1e-6;
	d = 1.0 / d;
	return m_sum * d;
}

void MatrixBuffer::clear()
{
	m_sum = cv::Matx44d::zeros();
	while (!m_queue.empty())
		m_queue.pop();
}
void MatrixBuffer::reset_fixed_size(int len)
{
	clear();
	m_fixed_size = len;
}
