/*
 * ellipse_detection.cpp
 *
 *  Created on: 2015年7月23日
 *      Author: guoyj7
 *
 *
 *  simple code:
 *
    int ori = 270;
	mElipseDetector.set_parameter(DETECTION_PARAM_ELLIPSE_DETECT_ANGLE, &ori);  // param set

	int arrElParams[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	std::vector<int>   elParams;
	elParams.assign(arrElParams, arrElParams + 10);
	int res = mElipseDetector.obj_detect((void *)boofCVData, resW, resH, (void *)&elParams);  // detecting

 */
#define _ANDROID_
#define DEBUG
#include "ellipse_detection.h"
#include "log.h"

namespace smart {

EllipseDetection::EllipseDetection() {
	// TODO Auto-generated constructor stub
	fInitCentre = cv::Point2f(0.0f,0.0f);
	fMaxDisFromInitCentre = FLT_MAX;
	fInitB = 0.0f;
	fTolRatioB = 0.0f;
	mfMinScore = 0.4f;
	mfMinReliability = 0.4f;
}

EllipseDetection::~EllipseDetection() {
	// TODO Auto-generated destructor stub
}

int EllipseDetection::set_parameter(const int type, void* param) {

	int* tmp_ori;
	float* tmp_xy;
	float* tmp_fMaxDisFromInitCentre;
	float* tmp_fInitB;
	float* tmp_fTolRatioB;
	float* tmp_mfMinScore;
	float* tmp_mfMinReliability;

	switch(type){
	case DETECTION_PARAM_ELLIPSE_DETECT_ANGLE :
		tmp_ori = (int* )param;
		cam_angle = *tmp_ori;
		break;
	case DETECTION_PARAM_ELLIPSE_DETECT_INITCENTRE :
		//tmp_xy = (float* )param;
		//fInitCentre = cv::Point2f(tmp_xy[0],tmp_xy[1]);
		fInitCentre = *((cv::Point2f *)param);
		break;
	case DETECTION_PARAM_ELLIPSE_DETECT_MAXDISFROMINITCENTRE:
		tmp_fMaxDisFromInitCentre = (float* )param;
		fMaxDisFromInitCentre = *tmp_fMaxDisFromInitCentre;
		break;
	case DETECTION_PARAM_ELLIPSE_DETECT_INITB:
		tmp_fInitB = (float* )param;
		fInitB = *tmp_fInitB;
		break;
	case DETECTION_PARAM_ELLIPSE_DETECT_TOLRATIOB:
		tmp_fTolRatioB = (float* )param;
		fTolRatioB = *tmp_fTolRatioB;
		break;
	case DETECTION_PARAM_ELLIPSE_DETECT_MINSCORE:
		tmp_mfMinScore = (float* )param;
		mfMinScore = *tmp_mfMinScore;
		break;
	case DETECTION_PARAM_ELLIPSE_DETECT_MINRELIABILITY:
		tmp_mfMinReliability = (float* )param;
		mfMinReliability = *tmp_mfMinReliability;
		break;
	default:
		break;
	}


	return 0;
}
/*
* ellipse_detection.cpp
* output : res_1: -1 : no ellipse
		   res_2:  1 : ellipse was detected ,where
					[0] = null
					[1]:center x of ellipse
					[2]:center y of ellipse
					[3]:long axis of ellipse
					[4]:short axis of ellipse
					[5]:angle of ellipse
					[6]:roi_lefttop.x
					[7]:roi_lefttop.y
					[8]:roi_rightbottom.x
					[9]:roi_rightbottom.y
* Created on: 2015年7月29日
*     Author: bobxu
*/
int EllipseDetection::obj_attractiveness_detect(Image* input_y, Image* input_u, Image* input_v, void* output, float &output_attract) {

	Image* yuv420sp_y = input_y;
	Image* yuv420sp_u = input_u;
	Image* yuv420sp_v = input_v;

	int* params = (int*) output;
	int  width = yuv420sp_y->width;
	int  height = yuv420sp_y->height;

	//------food attractiveness data struct--------------
	cv::Mat srcYUV = cv::Mat(202, 360, CV_8UC3);
	cv::Mat dstImg = cv::Mat(202, 360, CV_8UC3);
	cv::Mat Y_s2 = cv::Mat(yuv420sp_y->height, yuv420sp_y->width, CV_8UC1, yuv420sp_y->data);
	cv::Mat Y_s2Cb = cv::Mat(yuv420sp_u->height, yuv420sp_u->width, CV_8UC1, yuv420sp_u->data);
	cv::Mat Y_s2Cr = cv::Mat(yuv420sp_v->height, yuv420sp_v->width, CV_8UC1, yuv420sp_v->data);
//	IVCSCENE_DEBUG("test_Mat_1-1 Line= %d \n",__LINE__);
	std::vector<cv::Mat> mv;
	mv.push_back(Y_s2);
	mv.push_back(Y_s2Cr);
	mv.push_back(Y_s2Cb);
	cv::merge(mv, srcYUV);
//	IVCSCENE_DEBUG("test_Mat_1-2 Line= %d \n",__LINE__);
	cv::cvtColor(srcYUV, dstImg, CV_YCrCb2RGB);
//	IVCSCENE_DEBUG("test_Mat_1-3 Line= %d \n",__LINE__);


	//std::vector<int> params = *tmp_params;
	cv::Mat1b gray = cv::Mat(height, width, CV_8UC1, yuv420sp_y->data);
	cv::Size sz = gray.size();

	//imshow("Y_s2", gray);

	// Parameters Settings
	cv::Size	szPreProcessingGaussKernelSize = cv::Size(5, 5);
	double		dPreProcessingGaussSigma = 1.0;
	float		fThPosition = 1.0f;
	float		fCenterDistance = 0.05f;
	float		fMaxCenterDistance = sqrt(float(sz.width * sz.width + sz.height * sz.height)) * fCenterDistance;
	int			iMinEdgeLength = 16;
	float		fMinOrientedRectSide = 3.0f;
	float		fDistanceToEllipseContour = 0.1f;
	float		fMinScore = mfMinScore;
	float		fMinReliability = mfMinReliability;
	int 		iNs = 16;
//	SMART_DEBUG("fooddetect fMinScore = %f , fMinReliability=%f , fMaxDisFromInitCentre=%f , fInitB=%f , fTolRatioB=%f",
//			fMinScore  , fMinReliability, fMaxDisFromInitCentre , fInitB, fTolRatioB);
//	SMART_DEBUG("fooddetect centre_x2 = %f , centre_y2 = %f",fInitCentre.x, fInitCentre.y);

	EllipseDetector yaed;
	yaed.SetParameters(szPreProcessingGaussKernelSize,
		dPreProcessingGaussSigma,
		fThPosition,
		fMaxCenterDistance,
		iMinEdgeLength,
		fMinOrientedRectSide,
		fDistanceToEllipseContour,
		fMinScore,
		fMinReliability,
		iNs,
		fInitCentre,
		fMaxDisFromInitCentre,
		fInitB,
		fTolRatioB
		);

	float ratio = 0;
	if (cam_angle == 270 || cam_angle == 90)
		ratio = 0.0625;
	else
		ratio = 0.03125;

	// area of the ellipse should be at least this large
	float thres_area = ratio * width * height;

	// detect ellipse here
	CEllipse ellsYaed;
	ellsYaed.score_ = -1;
	yaed.DetectMainEllipse(gray, ellsYaed, thres_area);

	// yaed.GetMainEllipses_thresh(gray.rows * gray.cols, ellsYaed, ellipse_para, ratio);  // version 3
	if (ellsYaed.score_ > 0){

		output_attract = detect_food_score(dstImg, ellsYaed);
		int left   = (ellsYaed.xc_ - ellsYaed.B_);
		int top    = (ellsYaed.yc_ - ellsYaed.B_);
		int right  = (ellsYaed.xc_ + ellsYaed.B_);
		int bottom = (ellsYaed.yc_ + ellsYaed.B_);

		if (left < 0)
			left = 0;
		if (top < 0)
			top = 0;
		if (right > width)
			right = width;
		if (bottom > height)
			bottom = height;

		params[1] = ellsYaed.xc_;
		params[2] = ellsYaed.yc_;
		params[3] = ellsYaed.A_;  // major semi-axis length
		params[4] = ellsYaed.B_;  // minor semi-axis length
		params[5] = ellsYaed.rho_ * 180.0f / CV_PI;
		params[6] = left;
		params[7] = top;
		params[8] = right;
		params[9] = bottom;

		return 1;
	}

	return -1;
}

int EllipseDetection::obj_detect(const void* input, const int width, const int height, void* output ) {

	unsigned char* yuv420sp = (unsigned char* )input;
	int* params = (int*) output;
	//std::vector<int> params = *tmp_params;
	cv::Mat1b gray = cv::Mat(height, width, CV_8UC1, yuv420sp);
	cv::Size sz = gray.size();

	//imshow("Y_s2", gray);

	// Parameters Settings
	cv::Size	szPreProcessingGaussKernelSize = cv::Size(5, 5);
	double		dPreProcessingGaussSigma = 1.0;
	float		fThPosition = 1.0f;
	float		fCenterDistance = 0.05f;
	float		fMaxCenterDistance = sqrt(float(sz.width * sz.width + sz.height * sz.height)) * fCenterDistance;
	int			iMinEdgeLength = 16;
	float		fMinOrientedRectSide = 3.0f;
	float		fDistanceToEllipseContour = 0.1f;
	float		fMinScore = mfMinScore ; //0.4f;
	float		fMinReliability =  mfMinReliability ; //0.4f;
	int 		iNs = 16;


	EllipseDetector yaed;
	yaed.SetParameters(szPreProcessingGaussKernelSize,
		dPreProcessingGaussSigma,
		fThPosition,
		fMaxCenterDistance,
		iMinEdgeLength,
		fMinOrientedRectSide,
		fDistanceToEllipseContour,
		fMinScore,
		fMinReliability,
		iNs,
		fInitCentre,
		fMaxDisFromInitCentre,
		fInitB,
		fTolRatioB
		);

	float ratio = 0;
	if (cam_angle == 270 || cam_angle == 90)
		ratio = 0.0625;
	else
		ratio = 0.03125;

	// area of the ellipse should be at least this large
	float thres_area = ratio * width * height;

	// detect ellipse here
	CEllipse ellsYaed;
	ellsYaed.score_ = -1;
	yaed.DetectMainEllipse(gray, ellsYaed, thres_area);

	// yaed.GetMainEllipses_thresh(gray.rows * gray.cols, ellsYaed, ellipse_para, ratio);  // version 3
	if (ellsYaed.score_ > 0){


		int left   = (ellsYaed.xc_ - ellsYaed.B_);
		int top    = (ellsYaed.yc_ - ellsYaed.B_);
		int right  = (ellsYaed.xc_ + ellsYaed.B_);
		int bottom = (ellsYaed.yc_ + ellsYaed.B_);

		if (left < 0)
			left = 0;
		if (top < 0)
			top = 0;
		if (right > width)
			right = width;
		if (bottom > height)
			bottom = height;

		params[1] = ellsYaed.xc_;
		params[2] = ellsYaed.yc_;
		params[3] = ellsYaed.A_;  // major semi-axis length
		params[4] = ellsYaed.B_;  // minor semi-axis length
		params[5] = ellsYaed.rho_ * 180.0f / CV_PI;
		params[6] = left;
		params[7] = top;
		params[8] = right;
		params[9] = bottom;

		return 1;
	}

	return -1;
}

float EllipseDetection::detect_food_score(cv::Mat img, CEllipse &ellipse){
	float score = 0;

	img.convertTo(img, CV_32FC3, 1 / 255.0);

	cv::Mat mask = cv::Mat::zeros(cv::Size(img.cols, img.rows), CV_8UC3);

	cv::ellipse(mask,
		cv::Point(ellipse.xc_, ellipse.yc_), cv::Size(ellipse.A_, ellipse.B_),
		ellipse.rho_*180.f / CV_PI,
		0.0, 360.0, cv::Scalar(255, 255, 255), -1);

	int ind = 0;
	float sumR = 0.0;
	float sumG = 0.0;
	float sumB = 0.0;

	double sumGray = 0.0;
	double colorEn = 0.0;
	for (int i = 0; i < img.rows; i++)
		for (int j = 0; j < img.cols; j++)
		{
		cv::Vec3b color = mask.at<cv::Vec3b>(i, j);
		if (color[0] == 255 && color[1] == 255 && color[2] == 255)
		{
			cv::Vec3f color2 = img.at<cv::Vec3f>(i, j);
			sumB += color2[0];
			sumG += color2[1];
			sumR += color2[2];

			double gray = 0.299*color2[2] + 0.587*color2[1] + 0.114*color2[0];
			sumGray += gray;

			ind++;
		}
		}
//	IVCSCENE_DEBUG("test_Foods_1-1 Line= %d, sumGray = %f, sumB = %f, sumG = %f, sumR = %f\n",__LINE__,sumGray,sumB,sumG,sumG);

	if (sumGray < 0.000001)
	{
		colorEn = 1.0;
	}
	else
	{
		for (int i = 0; i < img.rows; i++)
			for (int j = 0; j < img.cols; j++)
			{
			cv::Vec3b color = mask.at<cv::Vec3b>(i, j);
			if (color[0] == 255 && color[1] == 255 && color[2] == 255)
			{
				cv::Vec3f color2 = img.at<cv::Vec3f>(i, j);
				double gray = 0.299*color2[2] + 0.587*color2[1] + 0.114*color2[0];
				if (gray < 0.000001)
				{
					colorEn += 0.0;
				}
				else
				{
					double p = (double)gray / (double)sumGray;

//						colorEn += -p*Log2l(p);
					colorEn += -p*(log(p)/log(2.0));
				}

			}
			}
	}

//		colorEn = (double)colorEn / Log2l((double)(img.rows*img.cols));
	colorEn = (double)colorEn / (log((double)(img.rows*img.cols))/log(2.0));

	score = (1.0 - colorEn) * 100;

	float tempB = sumB / ind;
	float tempG = sumG / ind;
	float tempR = sumR / ind;

	score += (0.8*tempR + 0.15*tempG + 0.05*tempB) * 100;
//	IVCSCENE_DEBUG("test_Foods_1-1 Line= %d Score = %f\n",__LINE__,score);
	return score;
}

} /* namespace smart */
