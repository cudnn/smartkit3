#include "hough_foot_line_detection.h"

#include <iostream>
#include <list>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "georegression.h"


using namespace std;
using namespace cv;

//const float PI = 3.1415926;

void edgeExtert( cv::Mat &im, int thresholdEdge, cv::Mat &derivX, cv::Mat &derivY, cv::Mat &binary )
{
	// binary = 0;
	cv::Sobel( im, derivX, derivX.type(), 1, 0, 3, 1.0, 0, 4 );		// b
	cv::Sobel( im, derivY, derivY.type(), 0, 1, 3, 1.0, 0, 4 );		//a

	Mat edge = abs( derivX ) + abs( derivY );
	cv::threshold( edge, binary, thresholdEdge, 255, THRESH_BINARY );
	binary.convertTo( binary, CV_8UC1, 1.0, 0.0 );
}
void DetectLineHoughFoot::detect( cv::Mat &input, std::vector<LineParametric2D_F32>  &ret, int oritation )
{
	alg_.oritation_ = oritation;
	inputHeight_ = input.rows;
	inputWidth_ = input.cols;
	Size inputSize = input.size();

	// cv::TickMeter tm;
	// tm.start();

	derivX_ = cv::Mat::zeros(inputSize, CV_16SC1);
	derivY_ = cv::Mat::zeros(inputSize, CV_16SC1);
	binary_ = cv::Mat::zeros(inputSize, CV_8UC1);

	//edgeExtert(input,thresholdEdge, derivX, derivY, binary);//slower
	gradientAndBinary(input, thresholdEdge_, derivX_, derivY_, binary_);//faster
	alg_.transformHough(derivX_, derivY_, binary_);

	vector <LineParametric2D_F32> lines = alg_.extractLines();

	//删除相近的线，并留下maxlinenum根
	//vector<LineParametric2D_F32> 
	ret.clear();
	ret = pruneLines(input, lines);
}

// set the parameters
void DetectLineHoughFoot::ConfigHoughFoot( int localMaxRadius, 
										   int minCounts,
										   int minDistanceFromOrigin,
										   float thresholdEdge, 
										   int maxLines )
{
	this->thresholdEdge_ = thresholdEdge;
	this->maxLines_ = maxLines;
	alg_.minDistanceFromOrigin_ = minDistanceFromOrigin;
	alg_.thresholdMax_ = minCounts;
	alg_.localMaxRadius_ = localMaxRadius;
}


void DetectLineHoughFoot::gradientAndBinary( cv::Mat &im, int thresholdEdge, cv::Mat &derivX, cv::Mat &derivY, cv::Mat &binary )
{
	unsigned char *imPtr = im.ptr<unsigned char>(0);
	unsigned char *binaryPtr = binary.ptr<unsigned char>(0);
	short int *derivYPtr = derivY.ptr<short int>(0);
	short int *derivXPtr = derivX.ptr<short int>(0);

	int xomit = 2;
	int yomit = 2;
	for (int i = yomit; i < inputHeight_ - yomit; i++)
	{
		imPtr = im.ptr<unsigned char>(i);
		binaryPtr = binary.ptr<unsigned char>(i);
		derivYPtr = derivY.ptr<short int>(i);
		derivXPtr = derivX.ptr<short int>(i);
		for (int j = xomit; j < inputWidth_ - xomit; j++)
		{
			derivXPtr[j] = -1 * (imPtr[j - inputWidth_ + 1] - imPtr[j - inputWidth_ - 1])
				- 2 * (imPtr[j + 1] - imPtr[j - 1])
				- (imPtr[j + inputWidth_ + 1] - imPtr[j + inputWidth_ - 1]);
			derivYPtr[j] = (imPtr[j - inputWidth_ - 1] - imPtr[j + inputWidth_ - 1])
				+ 2 * (imPtr[j - inputWidth_] - imPtr[j + inputWidth_])
				+ (imPtr[j - inputWidth_ + 1] - imPtr[j + inputWidth_ + 1]);
			if (abs(derivXPtr[j]) + abs(derivYPtr[j]) > thresholdEdge)
			{
				binaryPtr[j] = 255;
			}
		}
	}
}


/**
* Find the point in which the line intersects the image border and create a line segment at those points
*/
void DetectLineHoughFoot::borderIntersectPoint( LineParametric2D_F32 l, int width, int height, vector<Point2i> &inside )
{
	double t0 = (0 - l.p_.x) / l.getSlopeX();
	double t1 = (0 - l.p_.y) / l.getSlopeY();
	double t2 = (width - l.p_.x) / l.getSlopeX();
	double t3 = (height - l.p_.y) / l.getSlopeY();

	Point2i a = computePoint(l, t0);
	Point2i b = computePoint(l, t1);
	Point2i c = computePoint(l, t2);
	Point2i d = computePoint(l, t3);

	//vector<Point2i> inside;
	inside.clear();
	checkAddInside(width, height, a, inside);
	checkAddInside(width, height, b, inside);
	checkAddInside(width, height, c, inside);
	checkAddInside(width, height, d, inside);

	if (inside.size() != 2) {
		inside.clear();
	}
	//	return inside;
}

cv::Point2i DetectLineHoughFoot::computePoint( LineParametric2D_F32 l, double t ) 
{
	return cv::Point2i( (float)( t * l.slope_.x + l.p_.x ), ( float )( t * l.slope_.y + l.p_.y ) );
}

void DetectLineHoughFoot::checkAddInside( int width, int height, cv::Point2i a, std::vector<cv::Point2i> &inside ) 
{
	int foo = 0;
	if ( a.x >= -foo && a.x <= width + foo && a.y >= -foo && a.y <= height + foo )
		inside.push_back( a );
}

std::vector<LineParametric2D_F32> DetectLineHoughFoot::pruneLines( cv::Mat &input, std::vector<LineParametric2D_F32> &ret )
{
	std::vector<LineParametric2D_F32> dsimilar = pruneSimilar( ret, (float)( CV_PI * 0.04 ), 10, input.cols, input.rows );
	std::vector<LineParametric2D_F32> bestline = pruneNBest( dsimilar, maxLines_ );
	return bestline;

}
bool comp(const LineParametric2D_F32 &lhs, const LineParametric2D_F32 &rhs)
{
	return lhs.intensity_ > rhs.intensity_;
}
vector<LineParametric2D_F32> DetectLineHoughFoot::pruneSimilar(vector<LineParametric2D_F32> &lines, float toleranceAngle, float toleranceDist, int imgWidth, int imgHeight)
{
	list<LineParametric2D_F32>linelist;

	vector<int> intensity = alg_.foundIntensity_;
	for (int i = 0; i < lines.size(); i++)
	{
		lines[i].intensity_ = intensity[i];
		lines[i].theta_ = atanSafe(lines[i].getSlopeY(), lines[i].getSlopeX());
		borderIntersectPoint(lines[i], binary_.cols, binary_.rows, lines[i].terminal_points);
		linelist.push_back(lines[i]);
	}
	linelist.sort(comp);//sortByIntensity

	vector<LineParametric2D_F32> ret;
	list<LineParametric2D_F32>::iterator iter = linelist.begin();
	for (int i = 0; i < linelist.size(); i++)
	{
		ret.push_back(*(iter++));
	}

	for (int i = 0; i < ret.size(); i++)
	{
		LineParametric2D_F32 a = ret[i];
		if (a.intensity_ == 0 || a.terminal_points.size() != 2) continue;

		for (int j = i + 1; j < ret.size(); j++)
		{
			LineParametric2D_F32 b = ret[j];
			if (b.intensity_ == 0 || b.terminal_points.size() != 2) continue;

			//see if tyey are nearly parallel
			if (distHalf(a.theta_, b.theta_) > toleranceAngle)
			{
				int b = 0;
				continue;
			}
			Point2i *p = intersection(a, b);
			// see if it is nearly parallel and intersects inside the image
			if (p != NULL && p->x >= 0 && p->y >= 0 && p->x < imgWidth && p->y < imgHeight) {
				ret[j].intensity_ = 0;
			}
			else {
				// now just see if they are very close
				float distA = distance(a, b.terminal_points[0]);
				float distB = distance(a, b.terminal_points[1]);

				if (distA <= toleranceDist || distB < toleranceDist) {
					ret[j].intensity_ = 0;
				}
			}
		}
	}

	return ret;
}

vector<LineParametric2D_F32> DetectLineHoughFoot::pruneNBest(vector<LineParametric2D_F32> &ret, int N)
{
	vector<LineParametric2D_F32> bestline;

	int index = 0;
	for (int i = 0; i < ret.size(); i++)
	{
		if (ret[i].intensity_ > 0 && ret[i].terminal_points.size() == 2 && index < N)
		{
			bestline.push_back(ret[i]);
			index++;
		}
	}
	return bestline;
}
