#ifndef _MEDIAN_FLOW_TRACKER_H__
#define _MEDIAN_FLOW_TRACKER_H__

#include <iostream>
#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/video.hpp>
#include "tld_utils.h"

class MedianFlowTracker
{
public:
	typedef cv::Rect_<float> BoundingBoxType;
	MedianFlowTracker();
	~MedianFlowTracker();
	void start_tracking(const cv::Mat &image, const cv::Rect &boundingbox);
	bool predict(const cv::Mat &image, cv::Rect &boundingbox);
	cv::Rect_<float> get_subpixel_res(){ return pred_boundingbox_; };
	void reset_boundingbox(const BoundingBoxType &reset_boundingbox);
	void setParameter(const float &scale_ratio);
	bool KFPredict(cv::Rect &boundingbox);
	float getErr();
private:
	MedianFlowTracker(const MedianFlowTracker& rhs);
	MedianFlowTracker& operator=(const MedianFlowTracker &rhs);

	bool generate_tracking_points_uniform(std::vector<cv::Point2f> &points, const BoundingBoxType &boundingbox);
	bool generate_tracking_points_GFT(std::vector<cv::Point2f> &points, const BoundingBoxType &boundingbox);
	bool do_forwardbackward_opticalflow(const cv::Mat &prev_image, const cv::Mat &curr_image,
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
	bool track_and_filter_points(const cv::Mat &prev_image, const cv::Mat &curr_image,
		const std::vector<cv::Point2f> &prev_points, std::vector<cv::Point2f> &median_filted_prev_points,
		std::vector<cv::Point2f> &median_filted_pred_points, float &median_of_forward_backward_errors, float &median_of_nccs);
	void kalman_filter_init();
	void copy_GFT_points(const float median_of_forward_backward_errors, const std::vector<cv::Point2f> &prev_points_GFT, const std::vector<float> &forward_backward_errors, const float min_err);		//Copy the GFT points for next tracking frame

private:
	cv::Mat prev_image_;
	cv::Mat curr_image_;
	int tracking_lost_threadhold_;
	int optical_flow_pyramid_levels_;
	int tracking_point_rows_;
	int tracking_point_cols_;
	bool use_gpu_;
	float scale_ratio_;							//Bounds for scaling ratio
	BoundingBoxType prev_boundingbox_;
	BoundingBoxType pred_boundingbox_;
	std::vector<cv::Point2f> prev_points_;
	std::vector<cv::Point2f> pred_points_;
	std::vector<uchar> status_;
	std::vector<cv::Point2f> prev_GFT_points_;
	std::vector<float> forward_backward_errors_GFT_;
	float median_of_forward_backward_errors_uniform;
	float median_of_forward_backward_errors_GFT;
	bool is_GFT;
	bool status_uniform_track;
	bool status_GFT_track;
	//int pred_period_;
	//std::vector<BoundingBoxType> prev_boundingbox_record_;
	//cv::Mat transitionMatrix_;
	//cv::Mat measurementMatrix_;
	//cv::Mat processNoiseCov_;
	//cv::Mat measurementNoiseCov_;
	//int frm_counter_;	
};

#endif