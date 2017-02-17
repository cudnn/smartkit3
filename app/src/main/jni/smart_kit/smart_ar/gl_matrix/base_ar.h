/*
 *  base_ar.h
 *
 *  this file is part of smart_ar
 *  class BaseAR
 *
 *  Copyright lenovo
 *
 *  Created on: 2015-11-24
 *  Author(s): MikeyNa
 */
#ifndef _BASE_AR_H_
#define _BASE_AR_H_

#include "ar_camera.h"
#include "matrix_buffer.h"

class BaseAR{
public:
	BaseAR(ARCamera *gl_matrix);
	~BaseAR();
	/**enable matrix buffer or not,default matrix buffer is disabled **/
	void enable_matrix_buffer(bool val);
	/**set matrix buffer size**/
	void set_buffer_size(int buffer_len);
	/**calculate extrinsic matrix**/
	void calculate_extrinsic_matrix(bool is_good_result, CvPoint2D32f* points, float ratio = 1);
	/**calculate model view projection matrix**/
	void calculate_mvp_matrix(float size=2, float t_x=0, float t_y=0, float t_z=1);
	/**get model view projection matrix**/
	float *get_mvp_matrix();
	/**get camera extrinsic matrix**/
	float *get_extrinsic_matrix();
private:
	/**
	 * \brief Convert to OpenGL matrix
	 * \param modelview	[Out] 16 size
	 */
	void to_gl_matrix(double* modelview, const cv::Mat &rotate3,
			const cv::Mat &translate);
private:
	ARCamera *_ar_camera;
	bool _buffer_enable;
	bool _is_result_good;
	float *_mvp_matrix;
	float *_extrinsic_matrix;
	MatrixBuffer _extrinsic_buffer;




};
#endif

