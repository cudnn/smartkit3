/*
 *  base_ar.cpp
 *
 *  this file is part of smart_ar
 *  class BaseAR
 *
 *  Copyright lenovo
 *
 *  Created on: 2015-11-24
 *  Author(s): MikeyNa
 */
#define _ANDROID_
#define DEBUG
#include "base_ar.h"
#include "../log.h"
#include "acmath.h"
#include <smart_ar/gl_matrix/gsub_mtx.h>

BaseAR::BaseAR(ARCamera *ar_camera)
{
	_buffer_enable = false;
	_ar_camera = ar_camera;
	_mvp_matrix = new float[16];
	_extrinsic_matrix = new float[16];
}

BaseAR::~BaseAR()
{
	delete _mvp_matrix;
	_mvp_matrix = NULL;
	delete _extrinsic_matrix;
	_extrinsic_matrix = NULL;
}
/*
 * function : enable_matrix_buffer()
 * input    : val--whether enable matrix buffer
 * created  : 2015-11-24
 * Author(s): MikeyNa
 */
void BaseAR::enable_matrix_buffer(bool val)
{
	_buffer_enable = val;
}

/*
 * function : set_buffer_size()
 *            set matrix buffer length, default = 5
 * input    : buffer_len--buffer length
 * created  : 2015-11-24
 * Author(s): MikeyNa
 */
void BaseAR::set_buffer_size(int buffer_len)
{
	_extrinsic_buffer.reset_fixed_size(buffer_len);
}

/*
 * function : calculate_mvp_matrix()
 *            calculate model view projection matrix
 * input    : size--scale of the model
 *            t_x,t_y,t_z--transition of the model in x,y,z axis
 * created  : 2015-11-24
 * Author(s): MikeyNa
 */
void BaseAR::calculate_mvp_matrix(float size, float t_x, float t_y, float t_z)
{
	float *external_matrxi = get_extrinsic_matrix();
	double *gl_project_matrix = (double *)_ar_camera->get_gl_projection_matrix();
	double *inner_matrix = (double *)_ar_camera->get_intrinsic();

	SMART_DEBUG(
			"zxing qrcode matrix external_matrix: %.2f, %.2f,  %.2f, %.2f,  %.2f, %.2f,  %.2f, %.2f,%.2f, %.2f,  %.2f, %.2f,  %.2f, %.2f,  %.2f, %.2f",
			external_matrxi[0], external_matrxi[1], external_matrxi[2],
			external_matrxi[3], external_matrxi[4], external_matrxi[5],
			external_matrxi[6], external_matrxi[7], external_matrxi[8],
			external_matrxi[9], external_matrxi[10], external_matrxi[11],
			external_matrxi[12], external_matrxi[13], external_matrxi[14],
			external_matrxi[15]);
	SMART_DEBUG(
			"zxing qrcode matrix inner_matrix: %.2f, %.2f,  %.2f, %.2f,  %.2f, %.2f,  %.2f, %.2f,%.2f, %.2f,  %.2f, %.2f,  %.2f, %.2f,  %.2f, %.2f",
			inner_matrix[0], inner_matrix[1], inner_matrix[2], inner_matrix[3],
			inner_matrix[4], inner_matrix[5], inner_matrix[6], inner_matrix[7],
			inner_matrix[8]);
	SMART_DEBUG(
			"zxing qrcode matrix  glproje_matrix: %.2f, %.2f,  %.2f, %.2f,  %.2f, %.2f,  %.2f, %.2f,%.2f, %.2f,  %.2f, %.2f,  %.2f, %.2f,  %.2f, %.2f",
			gl_project_matrix[0], gl_project_matrix[1], gl_project_matrix[2],
			gl_project_matrix[3], gl_project_matrix[4], gl_project_matrix[5],
			gl_project_matrix[6], gl_project_matrix[7], gl_project_matrix[8],
			gl_project_matrix[9], gl_project_matrix[10], gl_project_matrix[11],
			gl_project_matrix[12], gl_project_matrix[13], gl_project_matrix[14],
			gl_project_matrix[15]);
//   float projectionMatrix[16];
	float posT[16];
	for (int i = 0; i < 16; i++) {
		posT[i] = (float) external_matrxi[i];
	}

	float project_matrix[16];
	for (int i = 0; i < 16; i++) {
		project_matrix[i] = (float) gl_project_matrix[i];
	}
	float viewProjection2[16];
	mtxLoadMatrixf(viewProjection2, project_matrix);
	mtxMultMatrixf(viewProjection2, posT);

	float modelViewProjection[16];
	mtxLoadMatrixf(modelViewProjection, viewProjection2);
	//mtxRotatef(modelViewProjection, -90.0f, 1.0f, 0.0f, 0.0f);
	mtxTranslatef(modelViewProjection, t_x, t_y, t_z); // Rotate about z axis.
	mtxScalef(modelViewProjection, size, size, size);

	//Project Matrix Output
	mtxLoadMatrixf(_mvp_matrix, modelViewProjection);
}

/*
 * function : calculate_extrinsic_matrix()
 * input    : is_good_result--is a flag that identify whether the image points  is available or not
 *            points--point of the object in image coordinate
 *            ratio--Ratio of width:height, 1 = square
 * created  : 2015-11-24
 * Author(s): MikeyNa
 */
void BaseAR::calculate_extrinsic_matrix(bool is_good_result, CvPoint2D32f* points, float ratio)
{
	_is_result_good = is_good_result;
	cv::Matx<double, 4, 2> ImgPoints;
		//modified on 2016-04-12 match world coordination with the order of object coordination 保持世界坐标和检测到的目标的坐标顺序相一致
		cv::Matx<double, 4, 3> ObjPoints(-0.5, -0.5, 0.0, -0.5, 0.5, 0.0, 0.5, 0.5,
						0.0, 0.5, -0.5, 0.0);
//		cv::Matx<double, 4, 3> ObjPoints(-0.5, -0.5, 0.0, 0.5, -0.5, 0.0, 0.5, 0.5,
//				0.0, -0.5, 0.5, 0.0);
		ObjPoints *= ratio;

		for (int i = 0; i < 4; i++) {
			ImgPoints(i, 0) = points[i].x;
			ImgPoints(i, 1) = points[i].y;
		}
		cv::Mat camera = cv::Mat(3, 3, CV_64F, _ar_camera->get_intrinsic());
		cv::Mat dist = cv::Mat(1, 4, CV_64F, _ar_camera->get_distortion());
		cv::Mat rotate, rotate3, translate;
		solvePnP(ObjPoints, ImgPoints, camera, dist, rotate, translate);
		Rodrigues(rotate, rotate3);
		double extrisic_tmp[16] = {0};
		to_gl_matrix(extrisic_tmp, rotate3, translate);
		for(int i = 0; i < 16; i++)
			_extrinsic_matrix[i] = extrisic_tmp[i];
//
//		SMART_DEBUG(
//				"zxing qrcode matrix _extrinsic_matrixx: %.2lf, %.2lf,  %.2lf, %.2lf,  %.2lf, %.2lf,  %.2lf, %.2lf,%.2lf, %.2lf,  %.2lf, %.2lf,  %.2lf, %.2lf,  %.2lf, %.2lf",
//				_extrinsic_matrix[0], _extrinsic_matrix[1], _extrinsic_matrix[2],
//				_extrinsic_matrix[3], _extrinsic_matrix[4], _extrinsic_matrix[5],
//				_extrinsic_matrix[6], _extrinsic_matrix[7], _extrinsic_matrix[8],
//				_extrinsic_matrix[9], _extrinsic_matrix[10], _extrinsic_matrix[11],
//				_extrinsic_matrix[12], _extrinsic_matrix[13], _extrinsic_matrix[14],
//				_extrinsic_matrix[15]);
		if(_buffer_enable)
		{
			if(_is_result_good)
			{
				cv::Matx44d ext_mat;
		        ext_mat<< _extrinsic_matrix[0], _extrinsic_matrix[1], _extrinsic_matrix[2],
									_extrinsic_matrix[3], _extrinsic_matrix[4], _extrinsic_matrix[5],
									_extrinsic_matrix[6], _extrinsic_matrix[7], _extrinsic_matrix[8],
									_extrinsic_matrix[9], _extrinsic_matrix[10], _extrinsic_matrix[11],
									_extrinsic_matrix[12], _extrinsic_matrix[13], _extrinsic_matrix[14],
									_extrinsic_matrix[15];
		        _extrinsic_buffer.push(ext_mat);
			}
			else
			{
				_extrinsic_buffer.clear();
			}
		}

}

/*
 * function : get_mvp_matrix()
 *            get model view projection matrix
 * created  : 2015-11-24
 * Author(s): MikeyNa
 */
float* BaseAR::get_mvp_matrix()
{
	SMART_DEBUG(
			"zxing qrcode matrix _mvpxx: %.2f, %.2f,  %.2f, %.2f,  %.2f, %.2f,  %.2f, %.2f,%.2f, %.2f,  %.2f, %.2f,  %.2f, %.2f,  %.2f, %.2f",
			_mvp_matrix[0], _mvp_matrix[1], _mvp_matrix[2],
			_mvp_matrix[3], _mvp_matrix[4], _mvp_matrix[5],
			_mvp_matrix[6], _mvp_matrix[7], _mvp_matrix[8],
			_mvp_matrix[9], _mvp_matrix[10], _mvp_matrix[11],
			_mvp_matrix[12], _mvp_matrix[13], _mvp_matrix[14],
			_mvp_matrix[15]);
	if(!_is_result_good)
		_mvp_matrix[0] = 0;
	return _mvp_matrix;
}

/*
 * function : get_extrinsic_matrix()
 * created  : 2015-11-24
 * Author(s): MikeyNa
 */
float* BaseAR::get_extrinsic_matrix()
{
	if(_buffer_enable)
	{
		cv::Matx44d ext_matrix = _extrinsic_buffer.get_aver();;

		_extrinsic_matrix[0] = 	ext_matrix(0,0);
		_extrinsic_matrix[1] = 	ext_matrix(0,1);
		_extrinsic_matrix[2] = 	ext_matrix(0,2);
		_extrinsic_matrix[3] = 	ext_matrix(0,3);
		_extrinsic_matrix[4] = 	ext_matrix(1,0);
		_extrinsic_matrix[5] = 	ext_matrix(1,1);
		_extrinsic_matrix[6] = 	ext_matrix(1,2);
		_extrinsic_matrix[7] = 	ext_matrix(1,3);
		_extrinsic_matrix[8] = 	ext_matrix(2,0);
		_extrinsic_matrix[9] = 	ext_matrix(2,1);
		_extrinsic_matrix[10] = 	ext_matrix(2,2);
		_extrinsic_matrix[11] = 	ext_matrix(2,3);
		_extrinsic_matrix[12] = 	ext_matrix(3,0);
		_extrinsic_matrix[13] = 	ext_matrix(3,1);
		_extrinsic_matrix[14] = 	ext_matrix(3,2);
		_extrinsic_matrix[15] = 	ext_matrix(3,3);

		SMART_DEBUG(
				"zxing qrcode _extrinsic_matrix mmpxx: %.2f, %.2f,  %.2f, %.2f,  %.2f, %.2f,  %.2f, %.2f,%.2f, %.2f,  %.2f, %.2f,  %.2f, %.2f,  %.2f, %.2f",
				_extrinsic_matrix[0], _extrinsic_matrix[1], _extrinsic_matrix[2],
				_extrinsic_matrix[3], _extrinsic_matrix[4], _extrinsic_matrix[5],
				_extrinsic_matrix[6], _extrinsic_matrix[7], _extrinsic_matrix[8],
				_extrinsic_matrix[9], _extrinsic_matrix[10], _extrinsic_matrix[11],
				_extrinsic_matrix[12], _extrinsic_matrix[13], _extrinsic_matrix[14],
				_extrinsic_matrix[15]);
	}

	return _extrinsic_matrix;
}
/*
 * function : to_gl_matrix()
 *            convert to opengl matrix
 * input    : rotate3--rotation value
 *            translate--translation value
 * output   : modelview length 16
 * created  : 2015-11-24
 * Author(s): MikeyNa
 */
void BaseAR::to_gl_matrix(double* modelview, const cv::Mat &rotate3,
		const cv::Mat &translate) {
	memset(modelview, 0, 16 * sizeof(double));
	for (int j = 0; j < 3; j++) {
		for (int i = 0; i < 3; i++) {
			modelview[i * 4 + j] = rotate3.ptr<double>(j)[i];
		}
	}
	double qua[4];
	acMatrixToQuaternion(modelview, qua);
	qua[1] = -qua[1];
	qua[2] = -qua[2];
	acQuaternionToMatrix(qua, modelview);
	modelview[12] = translate.ptr<double>(0)[0];
	modelview[13] = translate.ptr<double>(0)[1];
	modelview[14] = -translate.ptr<double>(0)[2];
	modelview[15] = 1;
}

