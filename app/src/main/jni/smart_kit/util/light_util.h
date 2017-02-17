/*
 * light_util.h
 *
 *  Created on: 2015/07/28
 *      Author: guoyj7
 *
 * 	Add comments on variable and functions
 *
 *  Revised on: 2016/04/13
 *      Author: fengbin1
 */
#ifndef SMART_KIT_UTIL_LIGHT_UTIL_H_
#define SMART_KIT_UTIL_LIGHT_UTIL_H_

#include <vector>
#include "data_structs.h"

namespace util {

// void getBlock(const int width, const int height, const int DownsampleCodeWidth, 
// 	const int DownsampleCodeHeight, const unsigned char *data, 
// 	std::vector<std::vector<float> > &avgY2, std::vector<std::vector<float> > &block2DYRotation);
/**
 * split input data into different block
 * @param input        input data
 * @param block1D      output data
 * @param BLOCK_SIDE   block size
 * @return
 */
void get_block_1D(const smart::Image* input, float* block1D, const int BLOCK_SIDE);

} //namespace util


#endif //SMART_KIT_UTIL_LIGHT_UTIL_H_
