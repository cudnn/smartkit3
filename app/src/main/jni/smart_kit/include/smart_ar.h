/*
/*
 *  smart_ar.h
 *
 *  this file is part of smart_ar
 *  class SmartAR
 *
 *  Copyright lenovo
 *
 *  Created on: 2015-11-24
 *  Author(s): MikeyNa
 *
 * 	Add comments on variable
 *
 *  Revised on: 2016/04/8
 *      Author: fengbin1
 */
#ifndef SMART_KIT_SMART_AR_H_
#define SMART_KIT_SMART_AR_H_

#include "smart_extern_manager.h"

namespace smart {
class SmartAR {
public:
	SmartAR();
	virtual ~SmartAR();
    /**
     * init camera parameters for AR module
     * @return
     */
	void init_camera_param();
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
	void calculate_mvp_matrix(int scene_flag, bool is_good_result, float* points_data, int point_lenth,
			float scale=2, float t_x=0, float t_y=0, float t_z=1);
    /**
     * get model view projection matrix
     * @return   projection matrix array
     */
	float *get_mvp_matrix();
	/**
	 * get extrinsic matrix
	 * @return   matrix array
	 */
	float *get_extrinsic_matrix();
	/**
	 * get projection matrix
	 * @return   projection matrix array
	 */
	float *get_gl_projection_matrix();

private:
	int _scene_flag;
	int _preview_height;
	int _preview_width;
	float _extrinsic_matrix[16];
	float _gl_projection_matrix[16];
	SmartExternManager* extern_manager;             //sensor data manager
};

} /* namespace smart */
#endif /* SMART_AR_H_ */


