
#include "qrcode_tracker.h"
#include <code_detection/zxing_code_detection.h>

using namespace std;
using namespace cv;

bool QrCodeTracker::QRCodeDetector(const cv::Mat& frame, std::string& decoded_str, cv::Point2f markers[5])
{
	ZxingCodeDetection qrcodeDetector;
    bool reFlag = false;
	int codePosition[41] = {0};
	unsigned char * y_data_ptr = (unsigned char*)(&frame.data[0]);
	qrcodeDetector.detect(y_data_ptr, frame.rows, frame.cols, 0, 0, frame.cols-1, frame.rows-1, true);
	qrcodeDetector.getCodePositon(codePosition, 41);
	if(codePosition[0] > 0)
	{
		for(int i = 0; i < 4; i++)
		{
			markers[i].x = codePosition[i*2+1];
			markers[i].y = codePosition[i*2+2];
		}
	}
	return reFlag;
}


void QrCodeTracker::estimate_4th_marker(cv::Point2f markers[GLB_NUM_MARKERS])
{
	cv::Point2f y = markers[0] + markers[2] - markers[1];
	cv::Point2f n = markers[3] - markers[1];
	std::swap(n.x, n.y);
	n.x = -n.x;
	Vec2f tmp(n);
	tmp /= norm(tmp);
	n.x = tmp[0];
	n.y = tmp[1];
	cv::Point2f z = markers[3];
	float lambda = 50;
	cv::Matx22f nnt(n.x*n.x, n.x*n.y, n.x*n.y, n.y*n.y);
	cv::Matx21f rhs(z.x, z.y);
	rhs = lambda*nnt*rhs + cv::Matx21f(y.x, y.y);
	cv::Matx22f A = lambda*nnt;
	A += cv::Matx22f::eye();
	cv::Matx21f x;
	cv::solve(A, rhs, x);
	markers[4].x = x(0, 0);
	markers[4].y = x(1, 0);
	swap(markers[3], markers[4]);
}

float QrCodeTracker::evaluate_qr_marker(const cv::Mat& band, int k) const
{
	int state_count[5] = { 0 }; // b, w, b, w, b ~= 1,1,3,1,1
	float lowthres = 64.0, highthres = 192.0;
	for (int i = 0; i < band.rows; i++)
	{
		int j = k;
		while (j > 0 && band.at<unsigned char>(i, j--) < lowthres)
			state_count[2]++;
		while (j > 0 && band.at<unsigned char>(i, j--) > highthres)
			state_count[1]++;
		while (j > 0 && band.at<unsigned char>(i, j--) < lowthres)
			state_count[0]++;

		j = k + 1;
		while (j < band.cols && band.at<unsigned char>(i, j++) < lowthres)
			state_count[2]++;
		while (j < band.cols && band.at<unsigned char>(i, j++) > highthres)
			state_count[3]++;
		while (j < band.cols && band.at<unsigned char>(i, j++) < lowthres)
			state_count[4]++;
	}

	float error = 0.0;
	for (int i = 0; i < 5; i++)
	{
		float x = state_count[i] / (state_count[2] + 1e-6) - 1.0 / 3.0;
		error += (i == 2) ? 0.0 : x*x;
	}

	return min(1e4, error*1e5);
}

void QrCodeTracker::InitTracker(const cv::Mat& frame)
{
	GeneralTracker::InitTracker(frame);
	int MaxNumFeatures = 1500;
	mTracker->FilterFeatures(mGeneralMarkers, MaxNumFeatures);
}

float QrCodeTracker::evaluate_tracked_markers_with_preknowledge(const cv::Point2f* tracked_markers, const cv::Mat& grayframe) const
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
			error0 = 10000.0;
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

	// 2. 0,1,2,5-black:
	int ncount = 0;
	for (int i = 0; i < GLB_NUM_MARKERS; i++)
	{
		if (i == GLB_NUM_MARKERS - 2)
			continue;

		float g = 0.0;
		bool counted = tracked_markers[i].inside(cv::Rect_<int>(0, 0, bwframe.cols - 1, bwframe.rows - 1));
		if (counted)
		{
			g = grayframe.at<unsigned char>(tracked_markers[i]) / 255.0;
			ncount++;
		}
		error1 += (i != GLB_NUM_MARKERS - 1) ? 1000.0*g*g : 0.0;
		error1 += (i == GLB_NUM_MARKERS - 1) ? 250.0*g*g : 0.0;
	}
	error1 /= (ncount + 1e-6);

	// 3. Check Whether QR Marker:
	int bandwidth = 3, minindex, maxindex;
	ncount = 0;
	Mat band;
	for (int k = 0; k < GLB_NUM_MARKERS - 2; k++)
	{
		if (!tracked_markers[k].inside(cv::Rect_<int>(0, 0, bwframe.cols - 1, bwframe.rows - 1)))
			continue;

		// horizontal: 
		int i = tracked_markers[k].y;
		int j = tracked_markers[k].x;

		minindex = max(0, i - (bandwidth >> 1));
		maxindex = min(bwframe.rows - 1, i + (bandwidth >> 1));
		band = bwframe.rowRange(minindex, maxindex + 1);
		float errortmp = evaluate_qr_marker(band, j);

		minindex = max(0, j - (bandwidth >> 1));
		maxindex = min(bwframe.cols - 1, j + (bandwidth >> 1));
		band = bwframe.colRange(minindex, maxindex + 1);
		errortmp = min(errortmp, evaluate_qr_marker(band.t(), i));

		error2 += errortmp;
		ncount++;
	}
	error2 /= (ncount + 1e-6);

	return error0 + error1 + error2;
}
void  QrCodeTracker::init_tracking(const cv::Mat inputMat, cv::Point2f inputMarkers[GLB_NUM_MARKERS])
{
	cv::Mat grayframe;
	mCurFrame = inputMat;
	estimate_4th_marker(inputMarkers);
	std::copy(inputMarkers, inputMarkers + GLB_NUM_MARKERS, mGeneralMarkers);
	if(mCurFrame.channels() == 3)
		cvtColor(mCurFrame, grayframe, COLOR_BGR2GRAY);
	else
		grayframe = mCurFrame;
	InitTracker(grayframe);
}

bool  QrCodeTracker::tracking(const cv::Mat inputMat, cv::Point2f outputPoint[GLB_NUM_MARKERS])
{
	mCurFrame = inputMat;
	cv::Mat grayframe, blurframe, blurbwframe, frameout;

	bool detected;
	string decoded_str;
	int N = 50;
	Point2f detected_markers[GLB_NUM_MARKERS], tracked_markers[GLB_NUM_MARKERS];

	float max_hw = 960.0;
	float marker_prior_score_tol0 = 15000.0, marker_prior_score_tol1 = 15000.0;
//	float marker_prior_score_tol0 = 2500.0, marker_prior_score_tol1 = 10000.0;

	if (mCurFrame.empty())
		return false;

//	if (max(mCurFrame.rows, mCurFrame.cols) > max_hw)
//	{
//		float ratio = max_hw / max(mCurFrame.rows, mCurFrame.cols);
//		cv::resize(mCurFrame, mCurFrame, cv::Size(), ratio, ratio);
//	}

	if (mCurFrame.channels() == 3)
	{
		cvtColor(mCurFrame, grayframe, COLOR_BGR2GRAY);
	}
	else
	{
		grayframe = mCurFrame;
	}
	cv::GaussianBlur(grayframe, blurframe, cv::Size(3, 3), 1.0);
	cv::threshold(blurframe, blurbwframe, 0, 255, THRESH_OTSU | THRESH_BINARY);
	//frameout = mCurFrame.clone();
	frameout = mCurFrame;

	mBadResult = false;
	float marker_prior_score = 0.0;

//	cout << "frame: " << mFrameNo << ": ";
	{
		float err = mTracker->Predict(grayframe, mGeneralMarkers, tracked_markers, GLB_NUM_MARKERS);

		if (mFrameNo % N == 0)
			mNeedToDetect = true;

		// 1. Pre-check tracked_markers:
		marker_prior_score = evaluate_tracked_markers_with_preknowledge(tracked_markers, blurframe);
		mNeedToDetect = (marker_prior_score > marker_prior_score_tol0) ? true : mNeedToDetect;

		//mNeedToDetect = false;
		// 2. Second-check
		if (!mNeedToDetect)
		{
//			cout << "Tracker Good! - Use tracked markers!" << endl;
			std::copy(tracked_markers, tracked_markers + GLB_NUM_MARKERS, mGeneralMarkers);
		}
		else
		{
			detected = QRCodeDetector(grayframe, decoded_str, detected_markers);
			if (detected)
			{
				estimate_4th_marker(detected_markers);
				float maxdist2 = evaluate_tracked_markers_with_detected_ones(detected_markers, tracked_markers);
				if (maxdist2 > 5.0)
				{
					std::copy(detected_markers, detected_markers + GLB_NUM_MARKERS, mGeneralMarkers);
					InitTracker(grayframe);
//					cout << "Checked: Tracker Re-initialized!" << endl;
				}
				else
				{
					std::copy(tracked_markers, tracked_markers + GLB_NUM_MARKERS, mGeneralMarkers);
//					cout << "Checked: Tracker Good!" << endl;
				}
				mNeedToDetect = false;
			}
			else
			{
				if (marker_prior_score > marker_prior_score_tol1 || mTracker->EvaluateFeatures(tracked_markers) < 0.85)
				{
					mBadResult = true;
				}

//				cout << "Not Detected! - Use tracked markers!" << endl;
				std::copy(tracked_markers, tracked_markers + GLB_NUM_MARKERS, mGeneralMarkers);

			}
		}
	}

	//if (mTracker)
//	{
//		char ss[300];
//		if (mBadResult)
//		{
//			sprintf(ss, "BAD: WILL HIDE: ERROR: %3.2f", marker_prior_score);
//			putText(frameout, ss, cv::Point2f(20, 30), FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0, 0, 255), 1);
//		}
//		else
//		{
//			DrawFeaturePoints(frameout);
//			DrawMarkers(frameout);
//		//	DrawInfoImage(frameout);
//			sprintf(ss, "MAY BE GOOD: ERROR: %3.2f", marker_prior_score);
//			putText(frameout, ss, cv::Point2f(20, 30), FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0, 255, 0), 1);
//		}
//	}

	mFrameNo++;
	//outputPoint.clear();
	//outputPoint.resize(GLB_NUM_MARKERS);
	for (int i = 0; i < GLB_NUM_MARKERS; i++)
	{
		outputPoint[i] =mGeneralMarkers[i];
	}
	return !mBadResult;
}

