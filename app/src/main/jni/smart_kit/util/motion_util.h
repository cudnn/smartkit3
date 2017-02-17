/*
 * motion_util.h
 *
 *  Created on: 2015/07/28
 *      Author: guoyj7
 *
 * 	Add comments on variable and functions
 *
 *  Revised on: 2016/04/13
 *      Author: fengbin1
 */
#ifndef SMART_KIT_UTIL_MOTION_UTIL_H_
#define SMART_KIT_UTIL_MOTION_UTIL_H_

#include <vector>
#include <algorithm>

namespace util {

/**
 * difference between input data
 * @param data   input data for composition process
 * @return       difference result
 */
inline float IVCLib_accDiff(float* data) {
	return (abs(data[3] - data[0]) + abs(data[4] - data[1]) + abs(data[5] - data[2]));
}

} //namespace util


#endif //SMART_KIT_UTIL_MOTION_UTIL_H_
