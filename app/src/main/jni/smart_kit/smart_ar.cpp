/*
 * smart_ar.cpp
 *
 *  Created on: 2015-07-27
 *      Author: guoyj7
 *
 * 	Add comments on variable
 *
 *  Revised on: 2016/04/8
 *      Author: fengbin1
 */

#include "smart_ar.h"
#include "smart_type.h"
#include <opencv2/core/core.hpp>
#include <smart_ar/gl_matrix/base_ar.h>
#include <smart_ar/gl_matrix/ar_camera.h>

namespace smart {

ARCamera * _ar_camera;
BaseAR   * _food_ar;
BaseAR   * _qrcode_ar;

SmartAR::SmartAR() {
	// TODO Auto-generated constructor stub
	_ar_camera = new ARCamera();
	_food_ar = new BaseAR(_ar_camera);
	_qrcode_ar = new BaseAR(_ar_camera);
	_food_ar->enable_matrix_buffer(true);
	extern_manager = SmartExternManager::get_instance();
	_preview_height = 0;
	_preview_width = 0;
	_scene_flag = -1;
}

SmartAR::~SmartAR() {
	// TODO Auto-generated destructor stub
	delete _ar_camera;
	delete _food_ar;
	delete _qrcode_ar;
}
/**
 * init camera parameters for AR module
 * @return
 */
void SmartAR::init_camera_param() {
	_ar_camera->init_param(extern_manager->preview_height, extern_manager->preview_width);
	_preview_height = extern_manager->preview_height;
	_preview_width = extern_manager->preview_width;
}
/**
 * calculate model view projection matrix
 * @param scene_flag        specific scene
 * @param is_good_result    do AR process flag
 * @param points_data       ROI
 * @param point_lenth       number of ROI points
 * @param scale
 * @param t_x
 * @param t_y
 * @param t_z
 * @return
 */
void SmartAR::calculate_mvp_matrix(int scene_flag, bool is_good_result, float* points_data, int point_lenth,
		float scale, float t_x, float t_y, float t_z) {
	CvPoint2D32f points[4];
	point_lenth = point_lenth;
	float point_temp[8];

	//coordination transformation for AR renderer
	point_temp[0] = points_data[0];
	point_temp[1] = _preview_height - 1 - points_data[1];
	point_temp[2] = points_data[2];
	point_temp[3] = _preview_height - 1 - points_data[3];
	point_temp[4] = points_data[4];
	point_temp[5] = _preview_height - 1 - points_data[5];
	point_temp[6] = points_data[6];
	point_temp[7] = _preview_height - 1 - points_data[7];

	for (int i = 0; i < point_lenth; i++) {
		points[i].x = point_temp[i * 2];
		points[i].y = point_temp[i * 2 + 1];
	}
//		points[0].x = 84.04;
//		points[0].y = 315.78;
//		points[1].x = 94.12;
//		points[1].y = 125.54;
//		points[2].x = 285.76;
//		points[2].y = 116.09;
//		points[3].x = 303.79;
//		points[3].y = 296.03;
	_scene_flag = scene_flag;
	switch(_scene_flag) {
	case FLAG_DETECTION_QRCODE:
		_qrcode_ar->calculate_extrinsic_matrix(is_good_result, points, scale);
		_qrcode_ar->calculate_mvp_matrix(scale, t_x, t_y, t_z);
		break;
	case FLAG_DETECTION_FOOD:
		_food_ar->calculate_extrinsic_matrix(is_good_result, points, scale);
		_food_ar->calculate_mvp_matrix(scale, t_x, t_y, t_z);
		break;
	default:
		break;
	}
}
/**
 * get model view projection matrix
 * @return   projection matrix array
 */
float* SmartAR::get_mvp_matrix() {
	switch(_scene_flag) {
	case FLAG_DETECTION_QRCODE:
		_qrcode_ar->get_mvp_matrix();
		break;
	case FLAG_DETECTION_FOOD:
		_food_ar->get_mvp_matrix();
		break;
	default:
		break;
	}
}
/**
 * get extrinsic matrix
 * @return   matrix array
 */
float* SmartAR::get_extrinsic_matrix() {
	switch(_scene_flag) {
	case FLAG_DETECTION_QRCODE:
		_qrcode_ar->get_extrinsic_matrix();
		break;
	case FLAG_DETECTION_FOOD:
		_food_ar->get_extrinsic_matrix();
		break;
	default:
		break;
	}
}
/**
 * get projection matrix
 * @return   projection matrix array
 */
float* SmartAR::get_gl_projection_matrix() {
	double *temp_data = _ar_camera->get_gl_projection_matrix();
	for(int i = 0; i < 16; i++)
		_gl_projection_matrix[i] = (float)temp_data[i];
	return _gl_projection_matrix;

}


} /* namespace smart */
