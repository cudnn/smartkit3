/*
 *  ar_camera.h
 *
 *  this file is part of smart_ar
 *  class ARCamera
 *
 *  Copyright lenovo
 *
 *  Created on: 2015-11-24
 *  Author(s): MikeyNa
 */
#ifndef _GL_CAMERA_H_
#define _GL_CAMERA_H_


#define GLB_NUM_MARKERS 5

/**
 * \brief The data structure, based on OpenCV intrinsic camera parameter
 */
struct CvarCamera {
	int width;
	int height;
	int flags;
	double avg_error; /**< Average reprojection error */

	double intrinsic[9]; /**< Camera matrix */
	double distortion[4]; /**< Distortion coefficients */
	double projection[16]; /**< OpenGL projection */
};

class ARCamera {
public:
	ARCamera();
	virtual ~ARCamera();
	/**init parameters,calculate intrinsic matrix and gl projection matrix**/
	void    init_param(int preview_height, int preview_width);
	/**get gl projection matrix**/
	double *get_gl_projection_matrix();
	/**get distortion coefficients**/
	double *get_distortion();
	/**get camera intrinsic parameters**/
	double *get_intrinsic();
private:
	void calculate_intrinsic_matrix();
	void param_change_size(CvarCamera* pCam, int width, int height);
	void calculate_gl_projection_matrix(CvarCamera* pCam, double* projection,
			int glstyle = 0);
private:
	CvarCamera* _pCamera;
	int frame_height_;
	int frame_width_;
};
#endif

