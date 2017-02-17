#include "LKTracker.h"

using namespace std;
using namespace cv;

LKTracker::LKTracker() : termcrit(TermCriteria(cv::TermCriteria::COUNT | cv::TermCriteria::EPS, 20, 0.03))
{
	mPointSet[0] = new std::vector<cv::Point2f>(0);
	mPointSet[1] = new std::vector<cv::Point2f>(0);
}


LKTracker::~LKTracker()
{
	delete mPointSet[0];
	delete mPointSet[1];

	mPointSet[0] = NULL;
	mPointSet[1] = NULL;
}

void LKTracker::DetectFeatures(const cv::Mat& frame, int N)
{
	cv::Size subPixWinSize(10, 10);
	goodFeaturesToTrack(frame, *mPointSet[0], N, 0.01, 10, cv::Mat(), 3, 0, 0.004);
	cornerSubPix(frame, *mPointSet[0], subPixWinSize, cv::Size(-1, -1), termcrit);
	prevFrame = frame.clone();
	mIsConf = true;
}

void LKTracker::FilterFeatures(const cv::Point2f quadcorners[4], int MaxNumber)
{
	vector<Point2f> OrigPoints = *mPointSet[0];
	random_shuffle(OrigPoints.begin(), OrigPoints.end());
	mPointSet[0]->clear();
	mPointSet[0]->reserve(OrigPoints.size());

	cv::Vec2f vx[4] = { quadcorners[1] - quadcorners[0], quadcorners[2] - quadcorners[1], 
		quadcorners[3] - quadcorners[2], quadcorners[0] - quadcorners[3] };

	bool flag;
	for (int i = 0; i < OrigPoints.size() && mPointSet[0]->size() < MaxNumber; i++)
	{
		flag = true;
		for (int j = 0; j < 4; j++)
		{
			Vec2f vxc = OrigPoints[i] - quadcorners[j];
			float m = vxc(0)*vx[j](1) - vxc(1)*vx[j](0);
			if (m > 1e-6)
			{
				flag = false;
				break;
			}
		}

		if (flag)
			mPointSet[0]->push_back(OrigPoints[i]);
	}
	*mPointSet[1] = *mPointSet[0];
	//cout << "num fpts: " << mPointSet[0]->size() << endl;
	mIsConf = true;
}

float LKTracker::Predict(const cv::Mat& gray, cv::Point2f ptsin[], cv::Point2f ptsout[], int nPts)
{
	cv::Size winSize(31, 31);
	vector<uchar> status;
	vector<float> err;
	float trackingError = 0.0;
	Mat Htemp;

	if (mPointSet[0]->size())
	{
		Mat diff = gray - prevFrame;

		calcOpticalFlowPyrLK(prevFrame, gray, *mPointSet[0], *mPointSet[1], status, err, winSize, 3, termcrit, 0, 0.001);
	}
	int k = 0;
	for (int i=0; i < mPointSet[0]->size(); i++)
	{
		if (!status[i])
			continue;

		(*mPointSet[0])[k] = (*mPointSet[0])[i];
		(*mPointSet[1])[k] = (*mPointSet[1])[i];
		k++;
	}

	if (k < 10)
		trackingError += 20;
	if (k < (float)mPointSet[0]->size()*0.7)
		trackingError += 10;

	mPointSet[0]->resize(k);
	mPointSet[1]->resize(k);
	
	float errtemp = 0.0;
	for (int i = 0; i < err.size(); i++)
		errtemp += status[i] ? err[i] : 0.0;
	errtemp /= (k+1e-6);
	trackingError += errtemp;

	if (mPointSet[0]->size() >= 4)
		Htemp = findHomography(*mPointSet[0], *mPointSet[1], CV_LMEDS);

	if (Htemp.empty())
		Hmat = Matx33f::zeros();
	else
		Hmat = Htemp;

	swap(mPointSet[0], mPointSet[1]);
	gray.copyTo(prevFrame);
	
	mIsConf = (trackingError < 40.0) && mIsConf;

	PredictPointsWithHomography(ptsin, ptsout, nPts);

	return trackingError;
}

void LKTracker::PredictPointsWithHomography(cv::Point2f pts0[], cv::Point2f pts1[], int n)
{
	if (abs(Hmat(2, 2)) < 1e-6)
		return;

	cv::Matx31f v(0,0,1), nv(0,0,1);
	for (int i = 0; i < n; i++)
	{
		v(0) = pts0[i].x;
		v(1) = pts0[i].y;
		nv = Hmat*v;
		pts1[i].x = nv(0) / nv(2);
		pts1[i].y = nv(1) / nv(2);
	}
}

float LKTracker::EvaluateFeatures(const cv::Point2f quadcorners[4]) const
{
	cv::Vec2f vx[4] = { quadcorners[1] - quadcorners[0], quadcorners[2] - quadcorners[1],
		quadcorners[3] - quadcorners[2], quadcorners[0] - quadcorners[3] };

	bool flag;
	int insideCount = 0;
	for (int i = 0; i < mPointSet[0]->size(); i++)
	{
		flag = true;
		for (int j = 0; j < 4; j++)
		{
			Vec2f vxc = (*mPointSet[0])[i] - quadcorners[j];
			float m = vxc(0)*vx[j](1) - vxc(1)*vx[j](0);
			if (m > 1e-6)
			{
				flag = false;
				break;
			}
		}

		if (flag)
			insideCount++;
	}
	
	return insideCount / (float)mPointSet[0]->size();
}
