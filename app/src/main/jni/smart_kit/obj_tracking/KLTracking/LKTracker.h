#ifndef _LK_TRACKER_H_
#define _LK_TRACKER_H_
#include <opencv2/opencv.hpp>
#include "math_util.h"

class LKTracker
{
public:
	LKTracker();
	~LKTracker();

	void								DetectFeatures(const cv::Mat& frame, int N);
	void								FilterFeatures(const cv::Point2f quadcorners[4], int MaxNumber);
	float								EvaluateFeatures(const cv::Point2f quadcorners[4]) const;

	const std::vector<cv::Point2f>&		GetFeatures() const { return *mPointSet[0]; }
	float								Predict(const cv::Mat& frame, cv::Point2f ptsin[], cv::Point2f ptsout[], int nPts);
	void								PredictPointsWithHomography(cv::Point2f pts0[], cv::Point2f pts1[], int n);
	inline bool							isConfident() const { return mIsConf; }
	inline void							SetConfident() { mIsConf = true; }
	inline cv::Matx33f					GetHomography() const { return Hmat; }

private:
	std::vector<cv::Point2f>*			mPointSet[2];
	cv::Mat								prevFrame;
	cv::TermCriteria					termcrit;
	cv::Matx33f							Hmat;
	bool								mIsConf;

};

#endif
