#ifndef LINEDETECT_HHH_H
#define LINEDETECT_HHH_H

#include "hough_foot_transform.h"
#include <list> 
#include <opencv2/imgproc/imgproc.hpp>

class DetectLineHoughFoot
{
private:
	// transform algorithm
	HoughTransformLineFootOfNorm alg_;
	cv::Mat derivX_;
	cv::Mat derivY_;
	cv::Mat binary_;
	int thresholdEdge_;
	int maxLines_;

public:
	//参数设置
	void ConfigHoughFoot( int localMaxRadius, int minCounts, int minDistanceFromOrigin,
						  float thresholdEdge, int maxLines);
	
	void detect( cv::Mat &input, std::vector<LineParametric2D_F32>  &ret, int oritation );
	
	//求xy方向的梯度并二值化
	void gradientAndBinary( cv::Mat &im, int thresholdEdge, cv::Mat &derivX, cv::Mat &derivY, cv::Mat &binary);

	//筛选直线
	std::vector<LineParametric2D_F32> pruneLines( cv::Mat &input, std::vector<LineParametric2D_F32> &ret);
	std::vector<LineParametric2D_F32>  pruneSimilar( std::vector<LineParametric2D_F32> &ret, float toleranceAngle, float toleranceDist, int imgWidth, int imgHeight);
	std::vector<LineParametric2D_F32> pruneNBest( std::vector<LineParametric2D_F32> &ret, int N );
	
	//find the point in which the line intersects the image border and create a line segment at this piont
	void borderIntersectPoint( LineParametric2D_F32 l, int width, int height, std::vector<cv::Point2i> &inside );

	cv::Point2i computePoint( LineParametric2D_F32 l, double t );
	void checkAddInside( int width, int height, cv::Point2i a, std::vector<cv::Point2i> &inside );

	int inputHeight_;
	int inputWidth_;
};

#endif

