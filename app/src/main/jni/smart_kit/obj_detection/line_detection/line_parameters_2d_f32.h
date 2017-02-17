#ifndef LineParametric2D_F32_HH
#define LineParametric2D_F32_HH

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#define LineParametric2D_F32 LineParameter
class LineParametric2D_F32
{
public:
	cv::Point2f p_;
	cv::Point2f slope_;
	int intensity_;		// intensity of the line
	float theta_;		// angle of the line
	std::vector<cv::Point2i> terminal_points;

public:
	float getSlopeX();
	float getSlopeY();
	int getX();
	int getY();
};


#endif
