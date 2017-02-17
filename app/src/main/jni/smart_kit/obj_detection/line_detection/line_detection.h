/*
 *      line detection. Smaple usage:
 		smart::LineDetection lindetector;
		smart::HoughFootLineParam param;
		std::vector<LineParameter> detected_lines;
		param.binary_threshold = 60;
		param.max_line_num = 1;
		param.max_radius = 5;
		param.min_density = 30;
		param.orientation = smart::IMAGE_HORIZONTAL;
		param.min_distance = 0;

		lindetector.set_parameter(DETECTION_PARAM_HOUGHFOOT, (void*)&param);
		int a = smart::IMAGE_HORIZONTAL;
		lindetector.set_parameter(DETECTION_PARAM_HOUGHFOOT_ORIENTATION, (void*)(&a));
		lindetector.obj_detect(yuvframe, WIDTH, HEIGHT, (void*)(&detected_lines));
 *  Created on: 2015年7月23日
 *      Author: guoyj7
 */

#ifndef SMART_KIT_OBJ_DETECTION_LINE_DETECTION_LINE_DETECTION_H_
#define SMART_KIT_OBJ_DETECTION_LINE_DETECTION_LINE_DETECTION_H_

#include "obj_detection.h"
#include "hough_foot_line_detection.h"
namespace smart {
	struct HoughFootLineParam
	{
		float binary_threshold;
		int max_radius;
		int min_density;
		int min_distance;
		int max_line_num;
		int orientation;
	};

	enum ScreenOrientation
	{
		IMAGE_VERTICAL = 0,
		IMAGE_HORIZONTAL = 1,
	};

class LineDetection: public smart::ObjDetection {
public:
	LineDetection();
	virtual ~LineDetection();

    virtual int set_parameter(const int type, void* param);
    virtual int obj_detect(const void* input, const int width, const int height, void* output);
private:
	smart::HoughFootLineParam param_;
	DetectLineHoughFoot *line_detector_;
	
};

} /* namespace smart */
#endif /* SMART_KIT_OBJ_DETECTION_LINE_DETECTION_LINE_DETECTION_H_ */
