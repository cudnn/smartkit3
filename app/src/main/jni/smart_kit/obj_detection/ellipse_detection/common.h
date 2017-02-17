#pragma once
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

typedef std::vector<cv::Point>	VP;
typedef std::vector<VP>	VVP;
typedef unsigned int uint;

#define _INFINITY 1024


int inline sgn(float val) {
    return (0.f < val) - (val < 0.f);
};


bool inline isInf(float x)
{
	union
	{
		float f;
		int	  i;
	} u;

	u.f = x;
	u.i &= 0x7fffffff;
	return !(u.i ^ 0x7f800000);
};


float inline Slope(float x1, float y1, float x2, float y2)
{
	//reference slope
		float den = float(x2 - x1);
		float num = float(y2 - y1);
		if(den != 0)
		{
			return (num / den);
		}
		else
		{
			return ((num > 0) ? float(_INFINITY) : float(-_INFINITY));
		}
};

void Canny3( cv::InputArray image, cv::OutputArray _edges,
			 cv::OutputArray _sobel_x, cv::OutputArray _sobel_y,
             int apertureSize, bool L2gradient );


float inline ed2(const cv::Point& A, const cv::Point& B)
{
	return float(((B.x - A.x)*(B.x - A.x) + (B.y - A.y)*(B.y - A.y)));
}

float inline ed2f(const cv::Point2f& A, const cv::Point2f& B)
{
	return (B.x - A.x)*(B.x - A.x) + (B.y - A.y)*(B.y - A.y);
}


void Labeling( cv::Mat1b& image, std::vector<std::vector<cv::Point> >& segments, int iMinLength );
void LabelingRect( cv::Mat1b& image, VVP& segments, int iMinLength, std::vector<cv::Rect>& bboxes );
void Thinning( cv::Mat1b& imgMask, uchar byF=255, uchar byB=0 );

bool SortBottomLeft2TopRight(const cv::Point& lhs, const cv::Point& rhs);
bool SortTopLeft2BottomRight(const cv::Point& lhs, const cv::Point& rhs);

bool SortBottomLeft2TopRight2f(const cv::Point2f& lhs, const cv::Point2f& rhs);


struct Ellipse
{
	float xc_;
	float yc_;
	float a_;
	float b_;
	float rad_;
	float score_;

	Ellipse() : xc_(0.f), yc_(0.f), a_(0.f), b_(0.f), rad_(0.f), score_(0.f) {};
	Ellipse(float xc, float yc, float a, float b, float rad, float score = 0.f) : xc_(xc), yc_(yc), a_(a), b_(b), rad_(rad), score_(score) {};
	Ellipse(const Ellipse& other) : xc_(other.xc_), yc_(other.yc_), a_(other.a_), b_(other.b_), rad_(other.rad_), score_(other.score_) {};

	void Draw( cv::Mat& img, const cv::Scalar& color, const int thickness )
	{
		cv::ellipse( img, cv::Point(cvRound(xc_),cvRound(yc_)), cv::Size(cvRound(a_),cvRound(b_)), rad_ * 180.0 / CV_PI, 0.0, 360.0, color, thickness );
	};

	void Draw( cv::Mat3b& img, const int thickness )
	{
		cv::Scalar color(0, cvFloor(255.f * score_), 0);
		cv::ellipse( img, cv::Point(cvRound(xc_),cvRound(yc_)), cv::Size(cvRound(a_),cvRound(b_)), rad_ * 180.0 / CV_PI, 0.0, 360.0, color, thickness);
	};

	bool operator<(const Ellipse& other) const
	{
		if( score_ == other.score_ )
		{
			float lhs_e = b_ / a_;
			float rhs_e = other.b_ / other.a_;
			if(lhs_e == rhs_e)
			{
				return false;
			}
			return lhs_e > rhs_e;
		}
		return score_ > other.score_;
	};
};




