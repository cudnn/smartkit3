#ifndef SMART_KIT_UTIL_FACE_UTIL_H_
#define SMART_KIT_UTIL_FACE_UTIL_H_
/*
 * face_util.h
 *
 *  Created on: 2015/07/28
 *      Author: guoyj7
 *
 * 	Add comments on variable and functions
 *
 *  Revised on: 2016/04/13
 *      Author: fengbin1
 */
#include <vector>
#include <opencv2/core/core.hpp>

namespace util {

/**
 * find max value in the ROI
 * @param yuv420sp   input data
 * @param ystart     top of ROI
 * @param xstart     left of ROI
 * @param yend       bottom of ROI
 * @param xend       right of ROI
 * @param height     height of input data
 * @param width      width of input data
 * @return           max value
 */
unsigned char imgMaxVal(unsigned char *yuv420sp, int ystart, 
			int xstart, int yend, int xend, int height, int width);

/**
 * calculate mean value in the ROI
 * @param yuv420sp   input data
 * @param xstart     left of ROI
 * @param ystart     top of ROI
 * @param xend       right of ROI
 * @param yend       bottom of ROI
 * @param height     height of input data
 * @param width      width of input data
 * @return           mean value
 */
int imgMean(unsigned char *yuv420sp, int xstart, int ystart, 
				   int xend, int yend, int height, int width);


/**
 * check the face results of input data
 * @param mat_face   input data
 * @return           true: portrait|false: no-portrait
 */
bool check_face_YUV(const cv::Mat &mat_face);
/**
 * get the color histogram of input data
 * @param img   input data
 * @return      color histogram
 */
std::vector<float> get_color_histogram_of_image_YUV(const cv::Mat &img);
/**
 * normalize the input data
 * @param img   input data
 * @return
 */
void normalized(std::vector<float> &a);

} //namespace util


#endif //SMART_KIT_UTIL_FACE_UTIL_H_
