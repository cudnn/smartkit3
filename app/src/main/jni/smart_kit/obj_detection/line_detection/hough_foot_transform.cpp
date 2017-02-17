#include "hough_foot_transform.h"
#include <iostream>
#include "georegression.h"

using namespace std;

//const double PI = 3.1415926;
void HoughTransformLineFootOfNorm::transformHough( cv::Mat &drivX, cv::Mat &derivY, cv::Mat &binary )
{
	inputHeight_ = binary.rows;
	inputWidth_ = binary.cols;
	transform_ = cv::Mat::zeros( drivX.size(), CV_16SC1 );

	originX_ = inputWidth_ / 2;
	originY_ = inputHeight_ / 2;

	candidate_.clear();

	// apply the transform to the entire image
	unsigned char *binaryPtr = binary.ptr<unsigned char>(0);
	short int *derivYPtr = derivY.ptr<short int>(0);
	short int *drivXPtr = drivX.ptr<short int>(0);
	for( int y = 0; y < inputHeight_; y++ ) {
		binaryPtr = binary.ptr<unsigned char>(y);
		derivYPtr = derivY.ptr<short int>(y);
		drivXPtr = drivX.ptr<short int>(y);

		for( int x = 0; x < inputWidth_; x++ ) {
			if(binaryPtr[x] != 0 ) {
				parameterize(x,y,drivXPtr[x], derivYPtr[x]);
			}
		}
	}

	//int b = countNonZero(transform);
	//Mat candi = Mat::zeros(drivX.size(), CV_8UC1);
	//unsigned char *ptrC = candi.ptr<unsigned char>(0);
	//for(int i = 0; i < candidate.size(); i++)
	//{
	//	int x = candidate[i].x;
	//	int y = candidate[i].y;
	//	ptrC[y*candi.cols + x] = 255;

	//}
	//int dd = 0;
	//dd++;

}



/**
* Takes the detected point along the line and its gradient and converts it into transform space.
* @param x point in image.
* @param y point in image.
* @param derivX gradient of point.
* @param derivY gradient of point.
*/
inline void HoughTransformLineFootOfNorm::parameterize( int x , int y , float derivX , float derivY )
{
	// put the point in a new coordinate system centered at the image's origin
	// this minimizes error, which is a function of distance from origin
	x -= originX_;
	y -= originY_;

	float v = (x*derivX + y*derivY)/(derivX*derivX + derivY*derivY);
	//float v = (y*derivX + x*derivY)/(derivX*derivX + derivY*derivY);


	// finds the foot a line normal equation and put the point into image coordinates
	int x0 = (int)(v*derivX) + originX_;
	int y0 = (int)(v*derivY) + originY_;


	if(x0 >= 0 && x0 < inputWidth_-1 && y0 >= 0 && y0 < inputHeight_-1){
		//int index = y0*transform.rows+x0;
		// keep track of candidate pixels so that a sparse search can be done
		// to detect lines
		short int *ptr = transform_.ptr<short int>(y0);
		if(++ptr[x0] == 1 )
		{
			candidate_.push_back(cv::Point2i(x0, y0));		
		}
	}
}

//inline bool HoughTransformLineFootOfNorm::isInBounds(int x, int y, int height , int width) {
//	return x >= 0 && x < width && y >= 0 && y < height;
//}

/**
* Searches for local maximas and converts into lines.
*
* @return Found lines in the image.
*/
vector<LineParametric2D_F32> HoughTransformLineFootOfNorm::extractLines()
{
	foundLines_.clear();
	foundIntensity_.clear();

	// stores returned lines
	vector<LineParametric2D_F32> lines;
	examinMaximum( transform_, candidate_, foundLines_ );

	for(int i = 0; i < foundLines_.size(); i++)
	{
		cv::Point2i p = foundLines_[i];

		int x0 = p.x - originX_;
		int y0 = p.y - originY_;


		if( abs(x0) >= minDistanceFromOrigin_ ||
			abs(y0) >= minDistanceFromOrigin_ )
		{
			LineParametric2D_F32 l;
			l.p_ = p;
			l.slope_ = cv::Point2f(-y0, x0);

			float offset = (float)(CV_PI * 0.039);
			float theta;
			if( oritation_ == 1 )//horizon
			    theta =  atanSafe(l.getSlopeY(), l.getSlopeX());
			else
				theta = atanSafe(l.getSlopeX(), l.getSlopeY());
			//cout << "theta "<< theta << endl;
			if( theta > -offset && theta < offset)
			{
			lines.push_back(l);

			short int *ptr = transform_.ptr<short int>(p.y);
			foundIntensity_.push_back(ptr[p.x]);
			}
		}
	}
	return lines;
}

void HoughTransformLineFootOfNorm::examinMaximum( cv::Mat &intensityImage, std::vector<cv::Point2i> &candidate, std::vector<cv::Point2i> &found)
{
	int ignoreBorder = 10;
	int endX = inputWidth_ - ignoreBorder;
	int endY = inputHeight_ - ignoreBorder;

	short int *inten = intensityImage.ptr<short int>(0);
	int step = intensityImage.step / sizeof(short int);
	for(int iter = 0; iter < candidate.size(); iter++)
	{
		cv::Point2i pt = candidate[iter];
		if(pt.x < ignoreBorder || pt.y < ignoreBorder || pt.x >= endX || pt.y >= endY)
			continue;

		//int center = pt.y * intensityImage.cols + pt.x;
// 		inten = intensityImage.ptr<short int>(pt.y);
// 		short int val = inten[pt.x];
		/*short int val = intensityImage.at<short int>(pt);*/
		short int val = inten[pt.y * intensityImage.step/sizeof(short int) + pt.x];

		if (val < thresholdMax_) continue;

		int x0 = MAX(ignoreBorder,pt.x - localMaxRadius_);
		int y0 = MAX(ignoreBorder,pt.y - localMaxRadius_);
		int x1 = MIN(endX, pt.x + localMaxRadius_ + 1);
		int y1 = MIN(endY, pt.y + localMaxRadius_ + 1);

		if( searchMax(val, x0, y0, x1, y1, intensityImage) )
			found.push_back(cv::Point2i(pt.x,pt.y));
	}
}

inline bool HoughTransformLineFootOfNorm::searchMax(float val, int x0, int y0, int x1, int y1, cv::Mat &input) {
	for( int i = y0; i < y1; i++ ) {
		short int *ptr = input.ptr<short int>(i);
		for( int j = x0; j < x1; j++) {
			if (val < ptr[j]) {
				return false;
			}
		}
	}
	return true;
}

