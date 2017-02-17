#include "Ellipse.h"

//Constructors
CEllipse::CEllipse(void)
{
}

CEllipse::CEllipse(float xc, float yc, float A, float B, float rho)
	:
	xc_(xc),
	yc_(yc),
	A_(A),
	B_(B),
	rho_(rho),
	score_(0.f),
	rel_(0.f)
{
}

CEllipse::CEllipse(const CEllipse& other)
	:
	xc_(other.xc_),
	yc_(other.yc_),
	A_(other.A_),
	B_(other.B_),
	rho_(other.rho_),
	score_(other.score_),
	rel_(other.rel_)
{
}

CEllipse::CEllipse(float xc, float yc, float A, float B, float rho, float score, float rel)
	:
	xc_(xc),
	yc_(yc),
	A_(A),
	B_(B),
	rho_(rho),
	score_(score),
	rel_(rel)
{
}

CEllipse::~CEllipse(void)
{
}


//Used for sorting ellipses. 
bool CEllipse::operator<(const CEllipse& other) const
	{
		if(score_ == other.score_)
		{
			if(rel_ == other.rel_)
			{
				float lhs_e = B_ / A_;
				float rhs_e = other.B_ / other.A_;
				if(lhs_e == rhs_e)
				{
					return false;
				}
				return lhs_e > rhs_e;
			}
			return rel_ > other.rel_;
		}
		return score_ > other.score_;
	};


//Approximation of ellipse perimeter.
float CEllipse::GetPerimeter() const
{
	float h = ((A_ - B_)*(A_ - B_)) / ((A_ + B_)*(A_ + B_));
	float p = CV_PI * (A_ + B_) * (1 + (0.25f*h) + (0.015625f*h*h) + (0.00390625f*h*h*h));
	return p;
}

