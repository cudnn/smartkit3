/*
 * line_detection.cpp
 *
 *  Created on: 2015年7月23日
 *      Author: guoyj7
 */

#include "line_detection.h"
#include "opencv2/imgproc/imgproc.hpp"

namespace smart {

LineDetection::LineDetection() {
	// TODO Auto-generated constructor stub
	line_detector_ = new DetectLineHoughFoot();

}

LineDetection::~LineDetection() {
	// TODO Auto-generated destructor stub
	if (line_detector_ != NULL)	{
		delete line_detector_;
	}
}
int LineDetection::set_parameter(const int type, void* param) {	
	switch (type) {
	case DETECTION_PARAM_HOUGHFOOT: {
		smart::HoughFootLineParam *input_param = (smart::HoughFootLineParam *)param;
		param_.max_line_num = input_param->max_line_num;
		param_.max_radius = input_param->max_radius;
		param_.min_density = input_param->min_density;
		param_.min_distance = input_param->min_distance;
		param_.binary_threshold = input_param->binary_threshold;
		param_.orientation = input_param->orientation;
		line_detector_->ConfigHoughFoot(param_.max_radius, param_.min_density, param_.min_distance,
			param_.binary_threshold, param_.max_line_num);
		break;
	}
	case DETECTION_PARAM_HOUGHFOOT_ORIENTATION: {
		int orientaion = *(int*)param;
		param_.orientation = orientaion;
		break;
	}
		default:
			break;
	}
	return 0;
}
int LineDetection::obj_detect(const void* input, const int width, const int height, void* output) {
	std::vector<LineParametric2D_F32> *detected_lines = (std::vector<LineParametric2D_F32>*)output;
	cv::Mat input_im = cv::Mat(cv::Size(width, height), CV_8UC1, (unsigned char*)input);
	line_detector_->detect(input_im, *detected_lines, param_.orientation);
	return 0;
}
} /* namespace smart */
