/*
 * mf_tracking.cpp
 *
 *  Created on: 2015年7月24日
 *      Author: guoyj7
 */

#include "mf_tracking.h"

namespace smart {

	
MFTracking::MFTracking() {
	// TODO Auto-generated constructor stub

}

MFTracking::~MFTracking() {
	// TODO Auto-generated destructor stub
}
int MFTracking::set_parameter(const int type, void* param) {
	if (param == 0) return 0;
	switch (type) {
	case TRACKING_PARAM_MFT: {
		MFTrackingParam *inputParam = (MFTrackingParam*)param;
		optical_flow_pyramid_levels_ = inputParam->optical_flow_pyramid_levels;
		tracking_point_rows_ = inputParam->tracking_point_rows;
		tracking_point_cols_ = inputParam->tracking_point_cols;
		tracking_points_thresh_ = inputParam->matched_points_thresh;
		use_gpu_ = false;
		prev_points_.resize(tracking_point_rows_ * tracking_point_cols_);
		pred_points_.resize(tracking_point_rows_ * tracking_point_cols_);
		status_.resize(tracking_point_rows_ * tracking_point_cols_);

		break;
	}
	case TRACKING_PARAM_MFT_BOUNDING_BOX: {
		MFTrackingBoundingBox *inputParam = (MFTrackingBoundingBox*)param;
		cv::Mat tmp_mat = cv::Mat(inputParam->image_height, inputParam->image_width, CV_8UC1, inputParam->image_data);
		cv::Rect rect = cv::Rect(inputParam->left, inputParam->top, 
			inputParam->right - inputParam->left, inputParam->bottom - inputParam->top);
		start_tracking(tmp_mat, rect);
		break;
	}
	default:
		break;			
	}

	return 0;
}

int MFTracking::obj_track(const void* input, const int width, const int height, void* output) {
	cv::Mat im = cv::Mat(height, width, CV_8UC1, (unsigned char*)input);
	SvRect *boundingbox = (SvRect*)output;
	bool is_tracked = predict(im, *boundingbox);
	return is_tracked == true ? 1 : 0;
}

void MFTracking::start_tracking(const cv::Mat &image, const cv::Rect &boundingbox)
{
	prev_image_ = image.clone();
	prev_boundingbox_ = boundingbox;
}

void MFTracking::generate_tracking_points(std::vector<cv::Point2f> &points, const BoundingBoxType &boundingbox)
{
	assert(points.size() == tracking_point_rows_ * tracking_point_cols_);
	//assert(boundingbox.width > tracking_point_cols_ + 10 && boundingbox.height > tracking_point_rows_ + 10);

	// fill points
	float step_rows = (float)(boundingbox.height - 10) / tracking_point_rows_;
	float step_cols = (float)(boundingbox.width - 10) / tracking_point_cols_;
	int idx = 0;
	for (float r = boundingbox.y, step_r = 0; step_r < tracking_point_rows_; r += step_rows, step_r++)
	{
		for (float c = boundingbox.x, step_c = 0; step_c < tracking_point_cols_; c += step_cols, step_c++)
		{
			points[idx++] = cv::Point2f(c + 5, r + 5);
		}
	}
}

bool MFTracking::predict(const cv::Mat &image, cv::Rect &boundingbox)
{
	this->curr_image_ = image;

	if (prev_image_.size() != curr_image_.size()){
		prev_image_ = curr_image_;
		return false;
	}

	// generate tracking points according to bounding box
	generate_tracking_points(prev_points_, prev_boundingbox_);

	// do optical flow
	std::vector<cv::Point2f> prev_points_backward;
	do_forwardbackward_opticalflow(prev_image_, curr_image_, prev_points_,
		pred_points_, prev_points_backward, status_);
	// filter points according to status
	std::vector<cv::Point2f> filted_prev_points;
	std::vector<cv::Point2f> filted_pred_points;
	std::vector<cv::Point2f> filted_pred_points_backward;
	filter_points_accordingto_status(prev_points_, pred_points_, prev_points_backward,
		filted_prev_points, filted_pred_points, filted_pred_points_backward, status_);
	//printf("filter_points_accordingto_status %lu\n", filted_prev_points.size());
	// points after median filtered is too small
	if (filted_prev_points.size() < 4)
	{
		return false;
	}

	// calculate forward backward errors
	std::vector<float> forward_backward_errors(filted_prev_points.size());
	calculate_forwardbackward_errors(filted_prev_points, filted_pred_points_backward, forward_backward_errors);

	// get median of forward backward errors
	float median_of_forward_backward_errors = get_median_of_forwardbackward_errors(forward_backward_errors);
	// lost !!!
	//printf("get_median_of_forwardbackward_errors %f\n", median_of_forward_backward_errors);
	// 	if (median_of_forward_backward_errors > tracking_lost_threadhold_)
	// 	{
	// 		return false;
	// 	}

	// calculate nccs
	std::vector<float> nccs(filted_prev_points.size());
	calculate_nccs(prev_image_, curr_image_, filted_prev_points, filted_pred_points, nccs);

	// get median of forward backward errors
	float median_of_nccs = get_median_of_nccs(nccs);

	// filter points according to median of fb errors and median of nccs
	std::vector<cv::Point2f> median_filted_prev_points;
	std::vector<cv::Point2f> median_filted_pred_points;
	filter_points_accordingto_medians(filted_prev_points, filted_pred_points, median_filted_prev_points,
		median_filted_pred_points, forward_backward_errors, nccs, median_of_forward_backward_errors, median_of_nccs);
	// points after median filtered is too small
	//printf("filter_points_accordingto_medians %lu\n", median_filted_pred_points.size());
	if (median_filted_pred_points.size() < tracking_points_thresh_)
		//	if (median_filted_pred_points.size() < 4)
	{
		return false;
	}

	// calculate offset and scales
	cv::Point2f offset;
	float scale;
	calculate_offset_and_scales(median_filted_prev_points, median_filted_pred_points, offset, scale);
	// update pred boundingbox
	update_boundingbox(prev_boundingbox_, pred_boundingbox_, offset, scale);

	// update prev image and others
	prev_image_ = curr_image_.clone();
	boundingbox = pred_boundingbox_;
	prev_boundingbox_ = pred_boundingbox_;
	return true;
}

void MFTracking::do_forwardbackward_opticalflow(const cv::Mat &prev_image, const cv::Mat &curr_image,
	const std::vector<cv::Point2f> &prev_points, std::vector<cv::Point2f> &pred_points,
	std::vector<cv::Point2f> &prev_points_backward, std::vector<uchar> &status)
{
	assert(status.size() == pred_points.size());
	std::vector<float> err;
	cv::TermCriteria termcrit(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03);
	cv::Size windows_size(4, 4);
	std::vector<uchar> forward_status(status.size());
	std::vector<uchar> backward_status(status.size());
	std::vector<cv::Point2f> back_prev_points = prev_points;

	// forward
	calcOpticalFlowPyrLK(prev_image, curr_image, prev_points, pred_points, forward_status,
		err, windows_size, optical_flow_pyramid_levels_, termcrit, 0);
	// backward
	calcOpticalFlowPyrLK(curr_image, prev_image, pred_points, back_prev_points, backward_status,
		err, windows_size, optical_flow_pyramid_levels_, termcrit/*, cv::OPTFLOW_USE_INITIAL_FLOW*/);

	prev_points_backward = back_prev_points;
	for (int v = 0; v < status.size(); v++)
	{
		status[v] = forward_status[v] & backward_status[v];
	}
}

void MFTracking::filter_points_accordingto_status(const std::vector<cv::Point2f> &prev_points,
	const std::vector<cv::Point2f> &pred_points, const std::vector<cv::Point2f> &prev_points_backward,
	std::vector<cv::Point2f> &filted_prev_points, std::vector<cv::Point2f> &filted_pred_points,
	std::vector<cv::Point2f> &filted_prev_points_backward, const std::vector<uchar> &status)
{
	filted_prev_points.clear();
	filted_pred_points.clear();
	for (int v = 0; v < status.size(); v++)
	{
		if (status[v] == 1)
		{
			filted_prev_points.push_back(prev_points[v]);
			filted_pred_points.push_back(pred_points[v]);
			filted_prev_points_backward.push_back(prev_points_backward[v]);
		}
	}
}

void MFTracking::calculate_forwardbackward_errors(const std::vector<cv::Point2f> &prev_points,
	const std::vector<cv::Point2f> &pred_points, std::vector<float> &forward_backward_errors)
{
	assert(forward_backward_errors.size() == prev_points.size());
	for (int v = 0; v < prev_points.size(); v++)
	{
		forward_backward_errors[v] = (prev_points[v].x - pred_points[v].x)*
			(prev_points[v].x - pred_points[v].x) +
			(prev_points[v].y - pred_points[v].y) * (prev_points[v].y - pred_points[v].y);
	}
}

float MFTracking::get_median_of_forwardbackward_errors(const std::vector<float> &forward_backward_errors)
{
	assert(forward_backward_errors.size() > 0);
	std::vector<float> tmp = forward_backward_errors;
	std::nth_element(tmp.begin(), tmp.begin() + tmp.size() / 2, tmp.end());
	return tmp[tmp.size() / 2];
}

inline float calculate_cv_tm_ccorr_normed(const cv::Mat &prev, const cv::Mat &pred)
{
	float sum0 = 0;
	float sum1 = 0;
	float sum2 = 0;
	for (int r = 0; r < prev.rows; r++) {
		for (int c = 0; c < prev.cols; c++) {
			sum0 += (float)prev.at<uchar>(r, c) * (float)pred.at<uchar>(r, c);
			sum1 += (float)prev.at<uchar>(r, c) * (float)prev.at<uchar>(r, c);
			sum2 += (float)pred.at<uchar>(r, c) * (float)pred.at<uchar>(r, c);
		}
	}
	return sum0 / (sqrt(sum1 * sum2));
}

void MFTracking::calculate_nccs(const cv::Mat &prev_image, const cv::Mat &curr_image, const std::vector<cv::Point2f> &prev_points,
	const std::vector<cv::Point2f> &pred_points, std::vector<float> &nccs)
{
	assert(pred_points.size() > 0);
	for (int v = 0; v < pred_points.size(); v++)
	{
		cv::Mat prev_subimage;
		cv::getRectSubPix(prev_image, cv::Size(10, 10), prev_points[v], prev_subimage);
		cv::Mat curr_subimage;
		cv::getRectSubPix(curr_image, cv::Size(10, 10), pred_points[v], curr_subimage);
		// 		cv::Mat corr;
		// 		cv::matchTemplate(prev_subimage, curr_subimage, corr, CV_TM_CCORR_NORMED);
		// 		nccs[v] = *((float*)corr.data);
		nccs[v] = calculate_cv_tm_ccorr_normed(prev_subimage, curr_subimage);
	}
}

float MFTracking::get_median_of_nccs(const std::vector<float> &nccs)
{
	assert(nccs.size() > 0);
	std::vector<float> tmp = nccs;
	std::nth_element(tmp.begin(), tmp.begin() + tmp.size() / 2, tmp.end());
	return tmp[tmp.size() / 2];
}

void MFTracking::filter_points_accordingto_medians(const std::vector<cv::Point2f> &prev_points,
	const std::vector<cv::Point2f> &pred_points, std::vector<cv::Point2f> &filted_prev_points,
	std::vector<cv::Point2f> &filted_pred_points, const std::vector<float> &fbs, const std::vector<float> &nccs,
	const float median_fbs, const float median_nccs)
{
	assert(fbs.size() == nccs.size());
	filted_prev_points.clear();
	filted_pred_points.clear();
	for (int v = 0; v < fbs.size(); v++)
	{
		if (fbs[v] <= median_fbs && nccs[v] >= median_nccs)
		{
			filted_prev_points.push_back(prev_points[v]);
			filted_pred_points.push_back(pred_points[v]);
		}
	}
}

static inline float distance(const cv::Point2f &p1, const cv::Point2f &p2)
{
	return sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
}

void MFTracking::calculate_offset_and_scales(const std::vector<cv::Point2f> &prev_points,
	const std::vector<cv::Point2f> &pred_points, cv::Point2f &offset, float &scale)
{
	// get median of offset x
	std::vector<float> offset_x(prev_points.size());
	for (int v = 0; v < prev_points.size(); v++)
	{
		offset_x[v] = pred_points[v].x - prev_points[v].x;
	}

	std::nth_element(offset_x.begin(), offset_x.begin() + offset_x.size() / 2, offset_x.end());

	// get median of offset y
	std::vector<float> offset_y(prev_points.size());
	for (int v = 0; v < prev_points.size(); v++)
	{
		offset_y[v] = pred_points[v].y - prev_points[v].y;
	}

	std::nth_element(offset_y.begin(), offset_y.begin() + offset_y.size() / 2, offset_y.end());
	offset = cv::Point2f(offset_x[offset_x.size() / 2], offset_y[offset_y.size() / 2]);

	// get median of scale
	std::vector<float> candidate_scales;
	for (int v = 0; v < prev_points.size(); v++) {
		for (int u = v + 1; u < prev_points.size(); u++) {
			float dis_prev = distance(prev_points[v], prev_points[u]);
			float dis_pred = distance(pred_points[v], pred_points[u]);
			candidate_scales.push_back(dis_pred / dis_prev);
		}
	}
	std::nth_element(candidate_scales.begin(), candidate_scales.begin() + candidate_scales.size() / 2, candidate_scales.end());
	scale = candidate_scales[candidate_scales.size() / 2];
}

void MFTracking::update_boundingbox(const BoundingBoxType &prev_boundingbox, BoundingBoxType &pred_boundingbox,
	const cv::Point2f &offset, float scale)
{
	float scale_offset_x = 0.5*(scale - 1) * prev_boundingbox.width;
	float scale_offset_y = 0.5*(scale - 1) * prev_boundingbox.height;
	pred_boundingbox.x = prev_boundingbox.x + offset.x - scale_offset_x;
	pred_boundingbox.y = prev_boundingbox.y + offset.y - scale_offset_y;
	pred_boundingbox.width = prev_boundingbox.width * scale;
	pred_boundingbox.height = prev_boundingbox.height * scale;
}

} /* namespace smart */
