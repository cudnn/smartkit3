/*
 * mf_tracking.h
 *  Median Flow Tracking,sample usage:
	 smart::MFTracking tracker;
	 SvRect tracked_rect;
	 smart::MFTrackingParam tracking_param;
	 smart::MFTrackingBoundingBox bounding_box;
	 bounding_box.image_data = (char*)yuvframe;
	 bounding_box.left = 10;
	 bounding_box.top = 10;
	 bounding_box.bottom = HEIGHT - 1;
	 bounding_box.right = WIDTH - 1;
	 bounding_box.image_height = HEIGHT;
	 bounding_box.image_width = WIDTH;

	 tracking_param.matched_points_thresh = 4;
	 tracking_param.optical_flow_pyramid_levels = 3;
	 tracking_param.tracking_point_cols = 10;
	 tracking_param.tracking_point_rows = 10;

	 tracker.set_parameter(TRACKING_PARAM_MFT, (void*)&tracking_param);
	 tracker.set_parameter(TRACKING_PARAM_MFT_BOUNDING_BOX, (void*)&bounding_box);
	 int ret = tracker.obj_track((void*)yuvframe, WIDTH, HEIGHT, (void*)&tracked_rect);

 *  Created on: 2015年7月24日
 *      Author: guoyj7
 */

#ifndef SMART_KIT_OBJ_TRACKING_MF_TRACKING_MF_TRACKING_H_
#define SMART_KIT_OBJ_TRACKING_MF_TRACKING_MF_TRACKING_H_

#include "obj_tracking.h"

#include <iostream>
#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/video.hpp>

#define SvRect cv::Rect
namespace smart {
	struct MFTrackingBoundingBox{
		char *image_data;
		int image_height, image_width;
		int left, top, bottom, right;
	};
	struct MFTrackingParam {
		int tracking_point_rows;
		int tracking_point_cols;
		int optical_flow_pyramid_levels;
		int matched_points_thresh;
	};

class MFTracking: public smart::ObjTracking {
public:
	MFTracking();
	virtual ~MFTracking();

    virtual int set_parameter(const int type, void* param);
    virtual int obj_track(const void* input, const int width, const int height, void* output);

private:
	typedef cv::Rect_<float> BoundingBoxType;

	void start_tracking(const cv::Mat &image, const cv::Rect &boundingbox);
	bool predict(const cv::Mat &image, cv::Rect &boundingbox);
	MFTracking(const MFTracking& rhs);
	MFTracking& operator=(const MFTracking &rhs);
	void init_parameters();
	void generate_tracking_points(std::vector<cv::Point2f> &points, const BoundingBoxType &boundingbox);
	void do_forwardbackward_opticalflow(const cv::Mat &prev_image, const cv::Mat &curr_image,
		const std::vector<cv::Point2f> &prev_points, std::vector<cv::Point2f> &pred_points,
		std::vector<cv::Point2f> &prev_points_backward, std::vector<uchar> &status);
	void filter_points_accordingto_status(const std::vector<cv::Point2f> &prev_points,
		const std::vector<cv::Point2f> &pred_points, const std::vector<cv::Point2f> &prev_points_backward,
		std::vector<cv::Point2f> &filted_prev_points, std::vector<cv::Point2f> &filted_pred_points,
		std::vector<cv::Point2f> &filted_prev_points_backward, const std::vector<uchar> &status);
	void calculate_forwardbackward_errors(const std::vector<cv::Point2f> &prev_points,
		const std::vector<cv::Point2f> &pred_points, std::vector<float> &forward_backward_errors);
	float get_median_of_forwardbackward_errors(const std::vector<float> &forward_backward_errors);
	void calculate_nccs(const cv::Mat &prev_image, const cv::Mat &curr_imag, const std::vector<cv::Point2f> &prev_points,
		const std::vector<cv::Point2f> &pred_points, std::vector<float> &nccs);
	float get_median_of_nccs(const std::vector<float> &forward_backward_errors);
	void filter_points_accordingto_medians(const std::vector<cv::Point2f> &prev_points,
		const std::vector<cv::Point2f> &pred_points, std::vector<cv::Point2f> &filted_prev_points,
		std::vector<cv::Point2f> &filted_pred_points, const std::vector<float> &fbs, const std::vector<float> &nccs,
		const float median_fbs, const float median_nccs);
	void calculate_offset_and_scales(const std::vector<cv::Point2f> &prev_points,
		const std::vector<cv::Point2f> &pred_points, cv::Point2f &offset, float &scale);
	void update_boundingbox(const BoundingBoxType &prev_boundingbox, BoundingBoxType &pred_boundingbox,
		const cv::Point2f &offset, float scale);
private:
	cv::Mat prev_image_;
	cv::Mat curr_image_;
	int tracking_lost_threadhold_;
	int optical_flow_pyramid_levels_;
	int tracking_point_rows_;
	int tracking_point_cols_;
	bool use_gpu_;
	BoundingBoxType prev_boundingbox_;
	BoundingBoxType pred_boundingbox_;
	std::vector<cv::Point2f> prev_points_;
	std::vector<cv::Point2f> pred_points_;
	std::vector<uchar> status_;
	int tracking_points_thresh_;//number of median flow tracker matched points threshold
};

} /* namespace smart */
#endif /* SMART_KIT_OBJ_TRACKING_MF_TRACKING_MF_TRACKING_H_ */
