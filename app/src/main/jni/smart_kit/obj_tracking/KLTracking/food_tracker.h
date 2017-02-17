#ifndef _FOOD_TRACKER_H_
#define _FOOD_TRACKER_H_

#include "general_tracker.h"
#include <ellipse_detection/EllipseDetector.h>

class FoodTracker : public GeneralTracker
{
public:
	bool				DishDetector(const cv::Mat& frame, cv::Point2f markers[GLB_NUM_MARKERS]);
	void                init_tracking(const cv::Mat inputIm, cv::Point2f inputMarkers[GLB_NUM_MARKERS]);
	bool                tracking(const cv::Mat inputMat, cv::Point2f outputPoint[GLB_NUM_MARKERS]);

private:
	void                InitTracker(const cv::Mat& frame);
	float				evaluate_tracked_markers_with_preknowledge(const cv::Point2f* tracked_markers, const cv::Mat& frame) const;
};

#endif

