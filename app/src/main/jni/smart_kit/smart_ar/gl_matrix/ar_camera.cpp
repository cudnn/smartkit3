/*
 *  ar_camera.cpp
 *
 *  this file is part of smart_ar
 *  class ARCamera
 *
 *  Copyright lenovo
 *
 *  Created on: 2015-11-24
 *  Author(s): MikeyNa
 */
#include "ar_camera.h"
#include "acmath.h"
#include <smart_ar/gl_matrix/gsub_mtx.h>
#include <opencv2/opencv.hpp>

#define _ANDROID_
#define DEBUG

#include "../log.h"

using namespace std;
using namespace cv;

ARCamera::ARCamera() {
	_pCamera = new CvarCamera();
}

ARCamera::~ARCamera() {
	delete _pCamera;
	_pCamera = NULL;
}

/*
 * function : init_param()
 * input    : preview_height--camera preview image height
 *            preview_width--camera preview image width
 * return   : void
 * created  : 2015-11-24
 * Author(s): MikeyNa
 */
void ARCamera::init_param(int preview_height, int preview_width) {
	frame_height_ = preview_height;
	frame_width_ = preview_width;
	calculate_intrinsic_matrix();
}

/*
 * function : calculate_intrinsic_matrix()
 *            calculate camera intrinsic matrix and gl projection matrix
 * return   : void
 * created  : 2015-11-24
 * Author(s): MikeyNa
 */
void ARCamera::calculate_intrinsic_matrix() {
	cv::Mat grayframe, blurframe, blurbwframe, frameout;
	float ratio = (float) frame_height_ / (float) frame_width_;
	if (abs(ratio - 0.75) > abs(ratio - 9.0 / 16.0)) {
		_pCamera->width = 1280;
		_pCamera->height = 720;
		_pCamera->flags = 0;

		_pCamera->avg_error = 0;
		double mat[] = { 1356.59, 0, 636.6, 0, 1360.58, 356.5, 0, 0, 1 };
		memcpy(_pCamera->intrinsic, mat, sizeof(double) * 9);

		double distort[] = { 0, 0, 0, 0 };
		memcpy(_pCamera->distortion, distort, sizeof(double) * 4);
	} else {
		_pCamera->width = 320;
		_pCamera->height = 240;
		_pCamera->flags = 0;

		_pCamera->avg_error = 0;
		double mat[] = { 350, 0, 160, 0, 350, 120, 0, 0, 1 };
		memcpy(_pCamera->intrinsic, mat, sizeof(double) * 9);

		double distort[] = { 0, 0, 0, 0 };
		memcpy(_pCamera->distortion, distort, sizeof(double) * 4);
	}

	//Create OpenGL projection
	param_change_size(_pCamera, frame_width_, frame_height_);
	calculate_gl_projection_matrix(_pCamera, _pCamera->projection);
	acMatrixTransposed(_pCamera->projection);
}

/*
 * function : param_change_size()
 *            Scale the camera matrix by image_height and image width
 * input    : pCam--camera intrinsic parameters
 *            width--camera preview image width
 *            height--camera preview image heiht
 * output   : pCam--camera intrinsic parameters
 * return   : void
 * created  : 2015-11-24
 * Author(s): MikeyNa
 */
void ARCamera::param_change_size(CvarCamera* pCam, int width, int height) {
	//This scaling algorithm refer to ARToolKit arParamChangeSize()

	//Get ratio
	float rt_u = (float) width / pCam->width;
	float rt_v = (float) height / pCam->height;

	//ARToolKit only uses one ratio. But I used two ratio, so that, the data of the matrix
	// need to be scaled separately
	//fx,fy (focal length)
	pCam->intrinsic[0] *= rt_u;
	pCam->intrinsic[1 * 3 + 1] *= rt_v;

	//cx,cy (principal point)
	pCam->intrinsic[2] *= rt_u;
	pCam->intrinsic[1 * 3 + 2] *= rt_v;

	pCam->width = width;
	pCam->height = height;
}

/*
 * function : calculate_gl_projection_matrix
 *            Convert the camera matrix to projection matrix (row-major).
 * input    : pCam--camera intrinsic parameters
 *            glstyle--OpenGL column-majored matrix, default=0
 * output   : projection--Store the OpenGL projection matrix
 * return   : void
 * created  : 2015-11-24
 * Author(s): MikeyNa
 */
void ARCamera::calculate_gl_projection_matrix(CvarCamera* pCam,
		double* projection, int glstyle) {
	//The projection should be 4x4 matrix

	//Set the near plane and far plane, based on ARToolKit
	//No more based on ARToolKit
	float nearplane = 0.1f;
	float farplane = 5000.0f;

//	float nearplane = 0.1f;
//	float farplane = 50.0f;

	//Initialise with 0
	memset(projection, 0, sizeof(double) * 16);

	projection[0] = 2. * pCam->intrinsic[0] / pCam->width;
	projection[1 * 4 + 1] = 2. * pCam->intrinsic[1 * 3 + 1] / pCam->height;
	projection[0 * 4 + 2] = 2. * (pCam->intrinsic[2] / pCam->width) - 1.;
	projection[1 * 4 + 2] = 2. * (pCam->intrinsic[1 * 3 + 2] / pCam->height)
			- 1.;
	projection[2 * 4 + 2] = -(farplane + nearplane) / (farplane - nearplane);
	projection[2 * 4 + 3] = -2. * farplane * nearplane / (farplane - nearplane);
	projection[3 * 4 + 2] = -1;

	if (glstyle)
		acMatrixTransposed(projection);

}

/*
 * function : get_gl_projection_matrix()
 *
 * return   : return gl projection matrix
 * created  : 2015-11-24
 * Author(s): MikeyNa
 */
double *ARCamera::get_gl_projection_matrix()
{
	return _pCamera->projection;
}

/*
 * function : get_distortion()
 *
 * return   : return camera distortion parameters
 * created  : 2015-11-24
 * Author(s): MikeyNa
 */
double *ARCamera::get_distortion()
{
	return _pCamera->distortion;
}

/*
 * function : get_intrinsic()
 *
 * return   : return camera intrinsic parameters
 * created  : 2015-11-24
 * Author(s): MikeyNa
 */
double *ARCamera::get_intrinsic()
{
	return _pCamera->intrinsic;
}
