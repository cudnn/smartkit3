#ifndef HoughTransformLineFootOfNorm_HH
#define HoughTransformLineFootOfNorm_HH

#include "line_parameters_2d_f32.h"
#include <vector>
#include <opencv2/imgproc/imgproc.hpp>


class HoughTransformLineFootOfNorm
{
private:

	// origin of the transform coordinate system
	int originX_;
	int originY_;

	// contains a set of counts for detected lines in each pixel
	cv::Mat transform_;

	// found lines in transform space
	std::vector<cv::Point2i> foundLines_;

	// list of points in the transform with non-zero values
	std::vector<cv::Point2i> candidate_;

public:
	/**
	* Computes the Hough transform using the image gradient and a binary image which flags pixels as being edges or not.
	*
	* @param derivX Image derivative along x-axis.
	* @param derivY Image derivative along y-axis.
	* @param binary Non-zero pixels are considered to be line pixels.
	*/
	void transformHough( cv::Mat &drivX, cv::Mat &derivY, cv::Mat &binary );

	/**
	* Searches for local maximals and converts into lines.
	*
	* @return Found lines in the image.
	*/
	std::vector<LineParametric2D_F32> extractLines();

	/**
	* Takes the detected point along the line and its gradient and converts it into transform space.
	* @param x point in image.
	* @param y point in image.
	* @param derivX gradient of point.
	* @param derivY gradient of point.
	*/
	inline void parameterize( int x , int y , float derivX , float derivY );

	// 挑出intensity局部最大的xy
	void examinMaximum( cv::Mat &intensityImage, std::vector<cv::Point2i> &candidate, std::vector<cv::Point2i> &foundlines );
	inline bool searchMax( float val, int x0, int y0, int x1, int y1, cv::Mat &input );

	// line intensities for later pruning
	std::vector<int>foundIntensity_;

	int inputHeight_;
	int inputWidth_;
	// lines are ignored if they are less than this distance from the origin
	// because the orientation can't be accurately estimated
	int minDistanceFromOrigin_;
	int thresholdMax_;
	int localMaxRadius_;
	int oritation_;
};

#endif

