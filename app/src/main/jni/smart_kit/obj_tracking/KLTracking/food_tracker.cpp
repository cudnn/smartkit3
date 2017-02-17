#include "food_tracker.h"

#define _ANDROID_
#define DEBUG
#include "log.h"


using namespace std;
using namespace cv;
void FoodTracker::InitTracker(const cv::Mat& frame)
{
	GeneralTracker::InitTracker(frame);
	int MaxNumFeatures = 1500;
	mTracker->FilterFeatures(mGeneralMarkers, MaxNumFeatures);
}


void  FoodTracker::init_tracking(const cv::Mat inputMat, cv::Point2f inputMarkers[GLB_NUM_MARKERS])
{
	cv::Mat grayframe;
	if (inputMat.channels() == 3)
		cvtColor(inputMat, grayframe, COLOR_BGR2GRAY);
	else
		grayframe = inputMat;
//	std::copy(inputMarkers, inputMarkers + GLB_NUM_MARKERS, mGeneralMarkers);
//	InitTracker(grayframe);
//	mNeedToDetect = false;
	if(mTracker != NULL) delete mTracker;
	mTracker = NULL;
}

bool  FoodTracker::tracking(const cv::Mat inputMat, cv::Point2f outputPoint[GLB_NUM_MARKERS])
{
	mCurFrame = inputMat;
	cv::Mat grayframe, blurframe, blurbwframe;
	bool detected;
	string decoded_str;
	int N = 100;
	Point2f detected_markers[GLB_NUM_MARKERS], tracked_markers[GLB_NUM_MARKERS];

	float max_hw = 960.0;
	float marker_prior_score_tol0 = 2500.0, marker_prior_score_tol1 = 9999.9;

//	mCurFrame = curframe;

	if (mCurFrame.empty())
		return false;

//	if (max(mCurFrame.rows, mCurFrame.cols) > max_hw)
//	{
//		float ratio = max_hw / max(mCurFrame.rows, mCurFrame.cols);
//		cv::resize(mCurFrame, mCurFrame, cv::Size(), ratio, ratio);
//	}

	if(mCurFrame.channels() == 3 || mCurFrame.channels() == 4)
	cvtColor(mCurFrame, grayframe, COLOR_BGR2GRAY);
	else
		grayframe = mCurFrame;
	cv::GaussianBlur(grayframe, blurframe, cv::Size(3, 3), 1.0);
	cv::threshold(blurframe, blurbwframe, 0, 255, THRESH_OTSU | THRESH_BINARY);

	mBadResult = false;
	float marker_prior_score = 0.0;

	cout << "frame: " << mFrameNo << ": ";
	if (mTracker == NULL)
	{
		detected = DishDetector(grayframe, detected_markers);//ellipseDet(iplrgb, qrPos);
		if (detected)
		{
			std::copy(detected_markers, detected_markers + GLB_NUM_MARKERS, mGeneralMarkers);
			InitTracker(grayframe);
//			generate_info_image(decoded_str);
//			cout << "Tracker 1st Initialized!" << endl;
			SMART_DEBUG("FoodART: Tracker 1st Initialized!");
			SMART_DEBUG("FoodART: Detected init %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f ",
					mGeneralMarkers[0].x, mGeneralMarkers[0].y,
					mGeneralMarkers[1].x, mGeneralMarkers[1].y,
					mGeneralMarkers[2].x, mGeneralMarkers[2].y,
					mGeneralMarkers[3].x, mGeneralMarkers[3].y,
					mGeneralMarkers[4].x, mGeneralMarkers[4].y
					);
			mNeedToDetect = false;
		}
		else{
//			cout << "Tracker Not Initialized Yet!" << endl;
			SMART_DEBUG("FoodART: Tracker Not Initialized Yet!");
			mBadResult = true;
		}
	}
	else
	{
		float err = mTracker->Predict(grayframe, mGeneralMarkers, tracked_markers, GLB_NUM_MARKERS);

		if (mFrameNo % N == 0)
			mNeedToDetect = true;

		// 1. Pre-check tracked_markers:
		marker_prior_score = evaluate_tracked_markers_with_preknowledge(tracked_markers, blurframe);
		mNeedToDetect = (marker_prior_score > marker_prior_score_tol0) ? true : mNeedToDetect;

		// 2. Second-check
		if (!mNeedToDetect)
		{
//			cout << "Tracker Good! - Use tracked markers!" << endl;
			SMART_DEBUG("FoodART: Tracker Good! - Use tracked markers!");
			std::copy(tracked_markers, tracked_markers + GLB_NUM_MARKERS, mGeneralMarkers);
		}
		else
		{
			detected = DishDetector(grayframe, detected_markers);//ellipseDet(iplrgb, qrPos);
			if (detected)
			{
				SMART_DEBUG("FoodART: Detected mNeedToDetect %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f ",
						detected_markers[0].x, detected_markers[0].y,
						detected_markers[1].x, detected_markers[1].y,
						detected_markers[2].x, detected_markers[2].y,
						detected_markers[3].x, detected_markers[3].y,
						detected_markers[4].x, detected_markers[4].y
						);
				float maxdist2 = evaluate_tracked_markers_with_detected_ones(detected_markers, tracked_markers);
				if (maxdist2 > 500.0)
				{
					std::copy(detected_markers, detected_markers + GLB_NUM_MARKERS, mGeneralMarkers);
					InitTracker(grayframe);
//					cout << "Checked: Tracker Re-initialized!" << endl;
					SMART_DEBUG("FoodART: Checked: Tracker Re-initialized!");
				}
				else
				{
					std::copy(tracked_markers, tracked_markers + GLB_NUM_MARKERS, mGeneralMarkers);
//					cout << "Checked: Tracker Good!" << endl;
					SMART_DEBUG("FoodART: Checked: Tracker Good!");
				}

				mNeedToDetect = false;
			}
			else
			{
				if (marker_prior_score > marker_prior_score_tol1 || mTracker->EvaluateFeatures(tracked_markers) < 0.85)
				{
					mBadResult = true;
					SMART_DEBUG("FoodART: Not Detected! - Dont Use tracked markers!"
							"%.2f %.2f", marker_prior_score, marker_prior_score_tol1);
				}

//				cout << "Not Detected! - Use tracked markers!" << endl;
				SMART_DEBUG("FoodART: Not Detected! - Use tracked markers!");
				std::copy(tracked_markers, tracked_markers + GLB_NUM_MARKERS, mGeneralMarkers);
			}
		}
	}

	double ratio = 0.2;
	Point2f rst[4];

	Point2f v03 = mGeneralMarkers[3] - mGeneralMarkers[0];
	Point2f v12 = mGeneralMarkers[2] - mGeneralMarkers[1];
	for (int i = 0; i < 4; i++)
	{
		rst[i] = mGeneralMarkers[i];
	}

	mGeneralMarkers[3] = ratio*v03 + mGeneralMarkers[3];
	mGeneralMarkers[0] = -ratio*v03 + mGeneralMarkers[0];
	mGeneralMarkers[2] = -ratio*v12 + mGeneralMarkers[2];
	mGeneralMarkers[1] = +ratio*v12 + mGeneralMarkers[1];

//	if (mTracker)
//	{
//		char ss[300];
//		if (mBadResult)
//		{
//			sprintf(ss, "BAD: WILL HIDE: ERROR: %3.2f", marker_prior_score);
//			putText(frameout, ss, cv::Point2f(20, 30), FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0, 0, 255), 1, CV_AA);
//		}
//		else
//		{
//			DrawFeaturePoints(frameout);
//			DrawMarkers(frameout);
//			//DrawInfoImage(frameout);
//			sprintf(ss, "MAY BE GOOD: ERROR: %3.2f", marker_prior_score);
//			putText(frameout, ss, cv::Point2f(20, 30), FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0, 255, 0), 1, CV_AA);
//		}
//	}

//	mARCamera->ComputeExtrinsicMatrix(mGeneralMarkers);

	mFrameNo++;
	for (int i = 0; i < GLB_NUM_MARKERS; i++)
	{
		outputPoint[i] = mGeneralMarkers[i];
	}

	for (int i = 0; i < 4; i++)
	{
		mGeneralMarkers[i] = rst[i];
	}

	return !mBadResult;

}



bool FoodTracker::DishDetector(const cv::Mat& frame, cv::Point2f markers[GLB_NUM_MARKERS])
{
	EllipseDetector yaed;
		cv::Size sz = frame.size();
		Size	szPreProcessingGaussKernelSize = Size(5, 5);
		double	dPreProcessingGaussSigma = 1.0;
		float	fThPosition = 1.0f;
		float	fCenterDistance = 0.05f;
		float	fMaxCenterDistance = sqrt(float(sz.width * sz.width + sz.height * sz.height)) * fCenterDistance;
		int		iMinEdgeLength = 16;
		float	fMinOrientedRectSide = 3.0f;
		float	fDistanceToEllipseContour = 0.1f;
		float	fMinScore = 0.4f;
		float	fMinReliability = 0.4f;
		int 	iNs = 16;
		yaed.SetParameters
		(
			szPreProcessingGaussKernelSize,
			dPreProcessingGaussSigma,
			fThPosition,
			fMaxCenterDistance,
			iMinEdgeLength,
			fMinOrientedRectSide,
			fDistanceToEllipseContour,
			fMinScore,
			fMinReliability,
			iNs,
			cv::Point2f(0.0f,0.0f),
			FLT_MAX,
			0.0f,
			0.0f
		);
		float thres_area = 0.03125 * frame.total();
		CEllipse ells;
		ells.score_ = -1;
		yaed.DetectMainEllipse(frame, ells, thres_area);

		SMART_DEBUG("FoodART: Detected centerxy %.2f %.2f score:%.2f",
						ells.xc_, ells.yc_, ells.score_);
		SMART_DEBUG("FoodART: Detected centerxy %d ",
				isnan(ells.score_));
		SMART_DEBUG("FoodART: Detected centerxy %d ",
				isnan(ells.score_));
		if (isnan(ells.score_) !=0 || ells.score_ < 0.0001)
		{
			SMART_DEBUG("FoodART: Detected centerxy return false %d ",
					isnan(ells.score_));
			return false;
		}
//		if (ells.score_ <= 0)
//			return false;

		typedef Matx<float, 2, 4> Matx24f;
		float longAxis = ells.A_, shortAxis = ells.B_, tx = ells.xc_, ty = ells.yc_;
		Matx34f X(-longAxis, 0, longAxis, 0,
			0, -shortAxis, 0, shortAxis, 1, 1, 1, 1);
		float theta = ells.rho_ >= CV_PI*0.5 ? ells.rho_ - CV_PI : ells.rho_;
		float cos_theta = cos(theta), sin_theta = sin(theta);
		Matx23f RT(cos_theta, -sin_theta, tx, sin_theta, cos_theta, ty);
		Matx24f Y = RT*X;
		cv::Point2f axisMarkers[5];
		for (int i = 0; i < 4; i++)
			axisMarkers[i] = cv::Point2f(Y(0, i), Y(1, i));
		axisMarkers[4] = cv::Point2f(ells.xc_, ells.yc_);

		float ratio = 1.0;//shortAxis / (longAxis+1e-6);
		ratio = sqrt(ratio);
		markers[0] = axisMarkers[3] + axisMarkers[0] - axisMarkers[4] + (1.0 - ratio)*(axisMarkers[0] - axisMarkers[4]);
		markers[1] = axisMarkers[0] + axisMarkers[1] - axisMarkers[4] - (1.0 - ratio)*(axisMarkers[0] - axisMarkers[4]);
		markers[2] = axisMarkers[1] + axisMarkers[2] - axisMarkers[4] - (1.0 - ratio)*(axisMarkers[2] - axisMarkers[4]);
		markers[3] = axisMarkers[2] + axisMarkers[3] - axisMarkers[4] + (1.0 - ratio)*(axisMarkers[2] - axisMarkers[4]);
		markers[4] = axisMarkers[4];

		SMART_DEBUG("FoodART: Detected DishDetector%.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f ",
				markers[0].x, markers[0].y,
				markers[1].x, markers[1].y,
				markers[2].x, markers[2].y,
				markers[3].x, markers[3].y,
				markers[4].x, markers[4].y
				);
		SMART_DEBUG("FoodART: Detected centerxy %.2f %.2f score:%.2f",
				ells.xc_, ells.yc_, ells.score_);
		return true;
}

float FoodTracker::evaluate_tracked_markers_with_preknowledge(const cv::Point2f* tracked_markers, const cv::Mat& grayframe) const
{
	Mat bwframe;
	cv::threshold(grayframe, bwframe, 0, 255, CV_THRESH_OTSU | CV_THRESH_BINARY);

	float error0 = 0.0, error1 = 0.0, error2 = 0.0;
	// 1. tracker_markers convex:
	int n = GLB_NUM_MARKERS - 1;
	int zs = 0;
	for (int i = 0; i < n; i++)
	{
		cv::Point2f v0 = tracked_markers[i%n];
		cv::Point2f v1 = tracked_markers[(i + 1) % n];
		cv::Point2f v2 = tracked_markers[(i + 2) % n];
		cv::Vec2f v01 = v1 - v0;
		cv::Vec2f v12 = v2 - v1;
		float z = v01(0)*v12(1) - v12(0)*v01(1);
		int zstmp = isgn(z, 1e-7);

		if (zstmp == 0)
		{
			//error0 = 10000.0;
			break;
		}

		if (i == 0)
		{
			zs = zstmp;
			continue;
		}

		error0 += (zs == zstmp) ? 0.0 : 10000.0;
		i = (zs == zstmp) ? i : n;
	}

	for (int i = 0; i < 4; i++)
	{
		cv::Point2f v0 = tracked_markers[i%n];
		cv::Point2f v1 = tracked_markers[(i + 1) % n];
		cv::Point2f v2 = tracked_markers[(i + 2) % n];
		cv::Vec2f v01 = v1 - v0;
		cv::Vec2f v12 = v2 - v1;
		error1 += abs(v01.dot(v12))/(norm(v01)+1e-6)/(norm(v12)+1e-6);
	}

	return error0 + 10000.0*error1 + error2;
}
