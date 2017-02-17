/*
 * obj_tracking.h
 *
 *  Created on: 2015年7月24日
 *      Author: guoyj7
 */

#ifndef SMART_KIT_OBJ_TRACKING_OBJ_TRACKING_H_
#define SMART_KIT_OBJ_TRACKING_OBJ_TRACKING_H_

#include "log.h"
#include "tracking_param_type.h"

namespace smart {

class ObjTracking {
public:
	ObjTracking();
	virtual ~ObjTracking();

    virtual int set_parameter(const int type, void* param) = 0;
    virtual int obj_track(const void* input, const int width, const int height, void* output) = 0;

};

} /* namespace smart */
#endif /* SMART_KIT_OBJ_TRACKING_OBJ_TRACKING_H_ */
