/*
 * ellipse_detection.h
 *
 *  Created on: 2015年7月23日
 *      Author: guoyj7
 */

#ifndef SMART_KIT_OBJ_DETECTION_ELLIPSE_DETECTION_ELLIPSE_DETECTION_H_
#define SMART_KIT_OBJ_DETECTION_ELLIPSE_DETECTION_ELLIPSE_DETECTION_H_

#include "obj_detection.h"

#include "common.h"
#include "Ellipse.h"
#include "EllipseDetector.h"
#include "data_structs.h"

//#include <opencv2/core/core.hpp>
//#include <opencv2/highgui/highgui.hpp>
//#include <opencv2/imgproc/imgproc.hpp>

namespace smart {

class EllipseDetection: public smart::ObjDetection {
private:
	int cam_angle;
	cv::Point2f fInitCentre;
	float fMaxDisFromInitCentre;
	float fInitB;
	float fTolRatioB;
	float mfMinScore;
	float mfMinReliability;

public:
	EllipseDetection();
	virtual ~EllipseDetection();

    virtual int set_parameter(const int type, void* param);
    virtual int obj_detect(const void* input, const int width, const int height, void* output );
    virtual int obj_attractiveness_detect(Image* input_y, Image* input_u, Image* input_v, void* output, float &output_attract);
//    virtual int obj_attractiveness_detect(const void* input_y, const void* input_u, const void* input_v, void* output, float output_attract);
private:
    float detect_food_score(cv::Mat img, CEllipse &ellipse);
};

} /* namespace smart */
#endif /* SMART_KIT_OBJ_DETECTION_ELLIPSE_DETECTION_ELLIPSE_DETECTION_H_ */
