#ifndef _GENERAL_TRACKER_H_
#define _GENERAL_TRACKER_H_

#include "LKTracker.h"

#define GLB_NUM_MARKERS 5

class GeneralTracker
{
public:
	GeneralTracker();
	virtual ~GeneralTracker();

	virtual void		InitTracker(const cv::Mat& frame);

	float				evaluate_tracked_markers_with_detected_ones(const cv::Point2f* markers, const cv::Point2f* trackedMarkers) const;
public:
	LKTracker*			mTracker;
	cv::Point2f			mGeneralMarkers[GLB_NUM_MARKERS]; // 3 detected markers, 1 estimated marker, 1 alignment pattern
	int					mFrameNo;
	cv::Mat				mCurFrame;
	bool				mNeedToDetect;
	bool				mBadResult;
};


#endif




