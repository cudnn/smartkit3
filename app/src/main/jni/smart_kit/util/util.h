/*
 * util.h
 *
 *  Created on: 2015年7月27日
 *      Author: guoyj7
 *
 * 	Add comments on variable and functions
 *
 *  Revised on: 2016/04/8
 *      Author: fengbin1
 */
#ifndef SMART_KIT_UTIL_UTIL_H_
#define SMART_KIT_UTIL_UTIL_H_

namespace util {

/**
 * calculate feature for light detection
 * @param blockY        (input data) block data
 * @param histYL        (input data) histogram of Y data(256 bin)
 * @param histU         (input data) histogram of U data(256 bin)
 * @param histV         (input data) histogram of V data(256 bin)
 * @param histYS        (input data) histogram of Y data(16 bin)
 * @param featureVector (output data)feature vector
 * @return
 */
void feature_extraction(const float blockY[], const float histYL[], const float histU[], const float histV[], const float histYS[], float featureVector[]);
/**
 * calculate final degree for light detection
 * @param featureVector    (input data) feature vector
 * @param alpha            (input data) alpha mask
 * @param indx             (input data) index mask
 * @param dir              (input data) dir mask
 * @param thld             (input data) threshold mask
 * @param numOfScenes      (input data) number of scene
 * @param vectorLength     (input data) number of weight for light feature
 * @param degree           (output data)final degree score
 * @return
 */
void degree_computation(float featureVector[], const double alpha[], const int indx[], const int dir[], const double thld[], const int numOfScenes, const int vectorLength, float degree[]);

/**
 * calculate average value of data
 * @param hist             histogram of data
 * @param histBinNum       number of histogram bin
 * @return                 average value
 */
float setAvg(const float hist[], int histBinNum);
/**
 * calculate standard deviation value of data
 * @param hist             histogram of data
 * @param histBinNum       number of histogram bin
 * @return                 standard deviation value
 */
float setStd(const float hist[], int histBinNum);
/**
 * calculate the index bin of the max value in histogram
 * @param hist             histogram of data
 * @param histBinNum       number of histogram bin
 * @return                 index bin
 */
int set_peak_from_histo(const float hist[], int histBinNum);
/**
 * calculate the average ratio value of input data
 * @param blockY           (input data) block data
 * @param darkTbl          (output data)binarization mask of block data
 * @param thldY            threshold for binarization
 * @return                 average ratio value
 */
float set_avg_ratio_from_block_y(const float blockY[], int darkTbl[], float thldY);
/**
 * calculate the index bin of the median value in histogram
 * @param hist             histogram of data
 * @param histBinNum       number of histogram bin
 * @return                 index bin
 */
int set_hist_median(const float hist[], int histBinNum);
/**
 * calculate the skewness value of the input data
 * @param hist             histogram of data
 * @param histBinNum       number of histogram bin
 * @return                 skewness value
 */
float set_skewness(const float hist[], int histBinNum);
/**
 * calculate the index bin of the max value in accumulative histogram
 * @param hist             histogram of data
 * @param histBinNum       number of histogram bin
 * @return                 index bin
 */
int set_hist_thld(const float hist[], int histBinNum);
/**
 * calculate the index bin of the max value in histogram
 * @param hist             histogram of data
 * @param histBinNum       number of histogram bin
 * @return                 index bin
 */
int set_hist_thld1(const float hist[], int histBinNum);
/**
 * calculate the front 3 bin accumulative histogram
 * @param hist             histogram of data
 * @param histBinNum       number of histogram bin
 * @return                 accumulative histogram result
 */
float set_hist_low(const float hist[], int histBinNum);
/**
 * calculate the half bin accumulative histogram
 * @param hist             histogram of data
 * @param histBinNum       number of histogram bin
 * @return                 accumulative histogram result
 */
float set_hist_half(const float hist[], int histBinNum);
/**
 * calculate mask feature from top in darkTbl
 * @param blockY          input block data
 * @param darkTbl         input mask data
 * @return                feature value
 */
float set_top_sum_from_block(const float blockY[], int darkTbl[]);
/**
 * calculate mask feature from bottom in darkTbl
 * @param blockY          input block data
 * @param darkTbl         input mask data
 * @return                feature value
 */
float set_down_sum_from_block(const float blockY[], int darkTbl[]);
/**
 * calculate the mean value of ROI in input data
 * @param blockY           (input data) block data
 * @param regionMeanLl     (output data)template matching result
 * @return
 */
void set_region_meanLl_from_block(const float blockY[], float regionMeanLl[]);
/**
 * calculate the mean value of ROI in input data
 * @param blockY           (input data) block data
 * @param regionMeanLl     (output data)template matching result
 * @return
 */
void set_region_meanBl_from_block(const float blockY[], float regionMeanBl[]);
/**
 * size ratio of coordination transformation
 * @param rect             src coordination
 * @param start_index      src array offset index
 * @param current_width    src rect width
 * @param current_height   src rect height
 * @param dest_width       dst rect width
 * @param dest_height      dst rect height
 * @param dest_start_index src array offset index
 * @param out_rect         dst coordination
 * @return
 */
void change_position_from_differnent_size(int* rect, int start_index, int current_width, int current_height, int dest_width, int dest_height, int dest_start_index, int* out_rect);
/**
 * calculate angle between current line and horizon line
 * @param orientation     camera orientation [0, 90, 180, 270]
 * @param orientationRaw  camera orientation 0 - 359
 * @return
 */
int levelAngle(int orientation, int orientationRaw);
} //namespace util


#endif //SMART_KIT_UTIL_UTIL_H_
