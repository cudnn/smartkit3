#ifndef _QRCODE_TRACKER_H_
#define _QRCODE_TRACKER_H_

#include "general_tracker.h"
#define HOLD_TRACKING_FRAME_NUM 10     //if not detect target frame number large than this threshold,then give up tracking           
class QrCodeTracker : public GeneralTracker
{
public:
	bool				QRCodeDetector(const cv::Mat& frame, std::string& decoded_str, cv::Point2f markers[GLB_NUM_MARKERS]);
	void				InitTracker(const cv::Mat& frame);
	void                init_tracking(const cv::Mat inputIm, cv::Point2f inputMarkers[GLB_NUM_MARKERS]);
	bool                tracking(const cv::Mat inputMat, cv::Point2f outputPoint[GLB_NUM_MARKERS]);
	
public:
	void				estimate_4th_marker(cv::Point2f markers[GLB_NUM_MARKERS]);
	float				evaluate_tracked_markers_with_preknowledge(const cv::Point2f* tracked_markers, const cv::Mat& frame) const;
	float				evaluate_qr_marker(const cv::Mat& band, int i) const;
};

#endif
