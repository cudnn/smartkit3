/*
 * obj_detection.h
 *
 *  Created on: 2015年7月23日
 *      Author: guoyj7
 */

#ifndef SMART_KIT_OBJ_DETECTION_OBJ_DETECTION_H_
#define SMART_KIT_OBJ_DETECTION_OBJ_DETECTION_H_

#include "log.h"
#include "detection_param_type.h"

namespace smart {

class ObjDetection {
public:
	ObjDetection();
	virtual ~ObjDetection();

    virtual int set_parameter(const int type, void* param) = 0;
    virtual int obj_detect(const void* input, const int width, const int height, void* output) = 0;

};

} /* namespace smart */
#endif /* SMART_KIT_OBJ_DETECTION_OBJ_DETECTION_H_ */
