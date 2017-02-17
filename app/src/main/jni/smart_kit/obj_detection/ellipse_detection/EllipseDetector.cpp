#include "EllipseDetector.h"

EllipseDetector::EllipseDetector(void)
{
	// Default Parameters Settings
	szPreProcessingGaussKernelSize_	= cv::Size(5,5);
	dPreProcessingGaussSigma_		= 1.0;
	fThPosition_					= 1.0f;
	fMaxCenterDistance_				= 100.0f * 0.05f;
	fMaxCenterDistance2_			= fMaxCenterDistance_ * fMaxCenterDistance_;
	iMinEdgeLength_					= 16;
	fMinOrientedRectSide_			= 3.0f;
	fDistanceToEllipseContour_		= 0.1f;
	fMinScore_						= 0.4f;
	fMinReliability_				= 0.4f;
	iNs_							= 16;
	fInitCentre_                    = cv::Point2f(0.0,0.0);
	fMaxDisFromInitCentre_          = FLT_MAX;
	fInitB_                         = 0.0f;
	fTolRatioB_                     = 0.0f;

	 srand ( unsigned(time(NULL)) );	
}


EllipseDetector::~EllipseDetector(void)
{	
}

void EllipseDetector::SetParameters( cv::Size szPreProcessingGaussKernelSize,
									 double	dPreProcessingGaussSigma,
									 double fThPosition,
									 float	fMaxCenterDistance,
									 int	iMinEdgeLength,
									 float	fMinOrientedRectSide,
									 float	fDistanceToEllipseContour,
									 float	fMinScore,
									 float	fMinReliability,
									 int    iNs,
									 cv::Point2f fInitCentre,
									 float fMaxDisFromInitCentre,
									 float fInitB,
									 float fTolRatioB
)
{
	szPreProcessingGaussKernelSize_		= szPreProcessingGaussKernelSize;
	dPreProcessingGaussSigma_			= dPreProcessingGaussSigma;
	fThPosition_						= fThPosition;	
	fMaxCenterDistance_					= fMaxCenterDistance;
	iMinEdgeLength_						= iMinEdgeLength;
	fMinOrientedRectSide_				= fMinOrientedRectSide;
	fDistanceToEllipseContour_			= fDistanceToEllipseContour;
	fMinScore_							= fMinScore;
	fMinReliability_					= fMinReliability;
	iNs_								= iNs;
	
	fInitCentre_                        = fInitCentre ;      		// add via huna 20160309
	fMaxDisFromInitCentre_      		= fMaxDisFromInitCentre;	// add via huna 20160309
	fMaxCenterDistance2_				= fMaxCenterDistance_ * fMaxCenterDistance_;
	fInitB_     						= fInitB;
	fTolRatioB_ 						= fTolRatioB;
}

uint inline EllipseDetector::GenerateKey(uchar pair, ushort u, ushort v)
{
	return (pair << 30) + (u << 15) + v;
};



int EllipseDetector::FindMaxK( const int* v ) const
{
	int max_val = 0;
	int max_idx = 0;
	for( unsigned i=0; i < ACC_K_SIZE; ++i )
	{
		( v[i] > max_val ) ? max_val = v[i], max_idx = i : NULL;
	}
	
	return max_idx + 90;
};

int EllipseDetector::FindMaxN(const int* v) const
{
	int max_val = 0;
	int max_idx = 0;
	for(unsigned i=0; i<ACC_N_SIZE; ++i)
	{
		(v[i] > max_val) ? max_val = v[i], max_idx = i : NULL;		
	}
	
	return max_idx;
};

int EllipseDetector::FindMaxA(const int* v) const
{
	int max_val = 0;
	int max_idx = 0;
	for(unsigned i=0; i<ACC_A_SIZE; ++i)
	{
		(v[i] > max_val) ? max_val = v[i], max_idx = i : NULL;		
	}
	
	return max_idx;
};


float EllipseDetector::GetMedianSlope( std::vector<cv::Point2f>& med, cv::Point2f& M, std::vector<float>& slopes)
{
	//med	: vector of points
	//M		: centroid of the points in med

	unsigned iNofPoints = med.size();
	//CV_Assert(iNofPoints >= 2);

	unsigned halfSize = iNofPoints >> 1;
	unsigned quarterSize = halfSize >> 1;

	std::vector<float> xx, yy;
	slopes.reserve(halfSize);
	xx.reserve(iNofPoints);
	yy.reserve(iNofPoints);

	for(unsigned i=0; i<halfSize; ++i)
	{
		cv::Point2f& p1 = med[i];
		cv::Point2f& p2 = med[halfSize + i];

		xx.push_back(p1.x);
		xx.push_back(p2.x);
		yy.push_back(p1.y);
		yy.push_back(p2.y);

		float den = (p2.x - p1.x);
		float num = (p2.y - p1.y);

		if(den == 0) den = 0.00001f;
				
		slopes.push_back(num / den);
	}

	nth_element(slopes.begin(), slopes.begin() + quarterSize, slopes.end());
	nth_element(xx.begin(), xx.begin() + halfSize, xx.end());
	nth_element(yy.begin(), yy.begin() + halfSize, yy.end());
	M.x = xx[halfSize];
	M.y = yy[halfSize];

	return slopes[quarterSize];
};




void EllipseDetector::GetFastCenter	(	std::vector<cv::Point>& e1,
												std::vector<cv::Point>& e2,
												EllipseData& data
											)
{
	data.isValid = true;

	uint size_1 = e1.size();
	uint size_2 = e2.size();

	uint hsize_1 = size_1 >> 1;
	uint hsize_2 = size_2 >> 1;

	cv::Point& med1 = e1[hsize_1];
	cv::Point& med2 = e2[hsize_2];

	cv::Point2f M12,M34;
	float q2, q4;

	{
		//first to second

		//reference slope
		
		float dx_ref =  e1[0].x - med2.x;
		float dy_ref =  e1[0].y - med2.y;
		if(dy_ref == 0) dy_ref = 0.00001f;

		float m_ref = dy_ref / dx_ref;
		data.m1 = m_ref;

		//find points with same slope as reference
		std::vector<cv::Point2f> med;
		med.reserve(hsize_2);

		uint minPoints = (iNs_ < hsize_2) ? iNs_ : hsize_2;

		std::vector<uint> indexes(minPoints);
		if(iNs_ < hsize_2)
		{
			unsigned iSzBin = hsize_2 / unsigned(iNs_);
			unsigned iIdx = hsize_2 + (iSzBin / 2);

			for(int i=0; i<iNs_; ++i)
			{
				indexes[i] =  iIdx;
				iIdx += iSzBin;
			}
		}
		else
		{
			// iota(indexes.begin(), indexes.end(), hsize_2);
			int tmp = hsize_2;
			for (int i = 0; i < indexes.size(); ++i)
			{
				indexes[i] = tmp++;
			}
		}

		for(uint ii=0; ii<minPoints; ++ii)
		{
			uint i = indexes[ii];

			float x1 = float(e2[i].x);
			float y1 = float(e2[i].y);

			uint begin = 0;
			uint end   = size_1-1;

			float xb = float(e1[begin].x);
			float yb = float(e1[begin].y);
			float res_begin = ((xb - x1) * dy_ref) - ((yb - y1) * dx_ref);
			int sign_begin = sgn(res_begin);
			if(sign_begin == 0)
			{
				//found
				med.push_back(cv::Point2f((xb+x1)* 0.5f, (yb+y1)* 0.5f));
				continue;
			}

			float xe = float(e1[end].x);
			float ye = float(e1[end].y);
			float res_end = ((xe - x1) * dy_ref) - ((ye - y1) * dx_ref);
			int sign_end = sgn(res_end);
			if(sign_end == 0)
			{
				//found
				med.push_back(cv::Point2f((xe+x1)* 0.5f, (ye+y1)* 0.5f));
				continue;
			}

			if((sign_begin + sign_end) != 0)
			{
				continue;
			}

			uint j = (begin + end) >> 1;

			while(end - begin > 2)
			{
				float x2 = float(e1[j].x);
				float y2 = float(e1[j].y);
				float res = ((x2 - x1) * dy_ref) - ((y2 - y1) * dx_ref);
				int sign_res = sgn(res);

				if(sign_res == 0)
				{
					//found
					med.push_back(cv::Point2f((x2+x1)* 0.5f, (y2+y1)* 0.5f));
					break;
				}

				if(sign_res + sign_begin == 0)
				{
					sign_end = sign_res;
					end = j;
				}
				else
				{
					sign_begin = sign_res;
					begin = j;
				}
				j = (begin + end) >> 1;
			}

			med.push_back(cv::Point2f((e1[j].x+x1)* 0.5f, (e1[j].y+y1)* 0.5f));
		}

		if(med.size() < 2)
		{
			data.isValid = false;
			return;
		}

		q2 = GetMedianSlope(med, M12, data.slopes1);
	}

	{
		//second to first

		//reference slope
		float dx_ref =  med1.x - e2[0].x;
		float dy_ref =  med1.y - e2[0].y;		
		if(dy_ref == 0) dy_ref = 0.00001f;

		float m_ref = dy_ref / dx_ref;
		data.m2 = m_ref;

		//find points with same slope as reference
		std::vector<cv::Point2f> med;
		med.reserve(hsize_1);

		uint minPoints = (iNs_ < hsize_1) ? iNs_ : hsize_1;
		
		std::vector<uint> indexes(minPoints);
		if(iNs_ < hsize_1)
		{
			unsigned iSzBin = hsize_1 / unsigned(iNs_);
			unsigned iIdx = hsize_1 + (iSzBin / 2);

			for(int i=0; i<iNs_; ++i)
			{
				indexes[i] =  iIdx;
				iIdx += iSzBin;
			}
		}
		else
		{
			// iota(indexes.begin(), indexes.end(), hsize_1);
			int tmp = hsize_1;
			for (int i = 0; i < indexes.size(); ++i)
			{
				indexes[i] = tmp++;
			}
		}

		for(uint ii=0; ii<minPoints; ++ii)
		{
			uint i = indexes[ii];

			float x1 = float(e1[i].x);
			float y1 = float(e1[i].y);

			uint begin = 0;
			uint end   = size_2-1;

			float xb = float(e2[begin].x);
			float yb = float(e2[begin].y);
			float res_begin = ((xb - x1) * dy_ref) - ((yb - y1) * dx_ref);
			int sign_begin = sgn(res_begin);
			if(sign_begin == 0)
			{
				//found
				med.push_back(cv::Point2f((xb+x1)* 0.5f, (yb+y1)* 0.5f));
				continue;
			}

			float xe = float(e2[end].x);
			float ye = float(e2[end].y);
			float res_end = ((xe - x1) * dy_ref) - ((ye - y1) * dx_ref);
			int sign_end = sgn(res_end);
			if(sign_end == 0)
			{
				//found
				med.push_back(cv::Point2f((xe+x1)* 0.5f, (ye+y1)* 0.5f));
				continue;
			}

			if((sign_begin + sign_end) != 0)
			{
				continue;
			}

			uint j = (begin + end) >> 1;

			while(end - begin > 2)
			{
				float x2 = float(e2[j].x);
				float y2 = float(e2[j].y);
				float res = ((x2 - x1) * dy_ref) - ((y2 - y1) * dx_ref);
				int sign_res = sgn(res);

				if(sign_res == 0)
				{
					//found
					med.push_back(cv::Point2f((x2+x1)* 0.5f, (y2+y1)* 0.5f));
					break;
				}

				if(sign_res + sign_begin == 0)
				{
					sign_end = sign_res;
					end = j;
				}
				else
				{
					sign_begin = sign_res;
					begin = j;
				}
				j = (begin + end) >> 1;
			}
			med.push_back(cv::Point2f((e2[j].x+x1)* 0.5f, (e2[j].y+y1)* 0.5f));
		}

		if(med.size() < 2)
		{
			data.isValid = false;
			return;
		}
		q4 = GetMedianSlope(med, M34, data.slopes2);
	}

	if(q2 == q4)
	{
		data.isValid = false;
		return;
	}

	float invDen = 1 / (q2 - q4);
	data.center.x = (M34.y - q4*M34.x - M12.y + q2*M12.x) * invDen;
	data.center.y = (q2*M34.y - q4*M12.y + q2*q4*(M12.x - M34.x)) * invDen;	
	data.t1 = q2;
	data.t2 = q4;
	data.M1 = M12;
	data.M2 = M34;
};




void EllipseDetector::DetectEdges13	(	cv::Mat1b& DP,
												VVP& points_1,
												VVP& points_3
											)
{
	//vector of connected edge points
	VVP contours;

	//labeling 8-connected edge points, discarding edge too small
	Labeling(DP, contours, iMinEdgeLength_);

	int iContoursSize = contours.size();
		
	//for each edge
	cv::RotatedRect oriented;
	int i, k, o_min, iEdgeSegmentSize, iCountTop, xx, iCountBottom;
	for(i=0; i<iContoursSize; ++i)
	{		
		VP& edgeSegment = contours[i];

		//Selection strategy - Step 1 - See Sect [] of the paper
		//constraint on axes aspect ratio
		oriented = minAreaRect(edgeSegment);
		o_min = std::min(oriented.size.width, oriented.size.height);
		if(o_min < fMinOrientedRectSide_)
		{
			continue;
		}

		//order edge points of the same arc
		sort(edgeSegment.begin(), edgeSegment.end(), SortTopLeft2BottomRight);
		iEdgeSegmentSize = edgeSegment.size();
		
		// get extrema of the arc
		cv::Point& left  = edgeSegment[0];
		cv::Point& right = edgeSegment[iEdgeSegmentSize-1];

		//find convexity - See Sect [] of the paper
		iCountTop = 0;
		xx = left.x;
		for(k = 1; k < iEdgeSegmentSize; ++k)
		{
			if(edgeSegment[k].x == xx) continue;

			iCountTop += (edgeSegment[k].y - left.y);
			xx = edgeSegment[k].x;
		}

		iCountBottom = ((right.x - left.x) * (right.y - left.y)) - iEdgeSegmentSize - iCountTop;

		if(iCountBottom > iCountTop)
		{	//1
			points_1.push_back(edgeSegment);
		}
		else if (iCountBottom < iCountTop)
		{	//3
			points_3.push_back(edgeSegment);
		}
	}
};


void EllipseDetector::DetectEdges24	(	cv::Mat1b& DN,
												VVP& points_2,
												VVP& points_4
											)
{
	//vector of connected edge points
	VVP contours;

	///labeling 8-connected edge points, discarding edge too small
	Labeling(DN, contours, iMinEdgeLength_);

	int iContoursSize = contours.size();

	//for each edge
	cv::RotatedRect oriented;
	int i, k, o_min, iEdgeSegmentSize, iCountTop, xx, iCountBottom;
	for(i=0; i<iContoursSize; ++i)
	{
		VP& edgeSegment = contours[i];
		
		//Selection strategy - Step 1 - See Sect [] of the paper
		//constraint on axes aspect ratio
		oriented = minAreaRect(edgeSegment);
		o_min = std::min(oriented.size.width, oriented.size.height);
		if(o_min < fMinOrientedRectSide_)
		{
			continue;
		}

		//order edge points of the same arc
		sort(edgeSegment.begin(), edgeSegment.end(), SortBottomLeft2TopRight);
		iEdgeSegmentSize = edgeSegment.size();

		// get extrema of the arc
		cv::Point& left  = edgeSegment[0];
		cv::Point& right = edgeSegment[iEdgeSegmentSize-1];

		//find convexity - See Sect [] of the paper
		iCountBottom = 0;
		xx = left.x;
		for(k = 1; k < iEdgeSegmentSize; ++k)
		{
			if(edgeSegment[k].x == xx) continue;

			iCountBottom += (left.y - edgeSegment[k].y);
			xx = edgeSegment[k].x;
		}

		iCountTop = (abs(right.x - left.x) * abs(right.y - left.y)) - iEdgeSegmentSize - iCountBottom;

		if(iCountBottom > iCountTop)
		{
			//2
			points_2.push_back(edgeSegment);
		}
		else if (iCountBottom < iCountTop)
		{
			//4
			points_4.push_back(edgeSegment);
		}
	}
};

//most important function for detecting ellipses. See Sect[] of the paper
void EllipseDetector::FindEllipses	(	cv::Point2f& center,
											VP& edge_i,
											VP& edge_j,
											VP& edge_k,
											EllipseData& data_ij,
											EllipseData& data_ik,
											std::vector<CEllipse>& ellipses
										)
{

	//FIND ELLIPSE PARAMETERS

	// 0-initialize accumulators
	memset(accN, 0, sizeof(int)*ACC_N_SIZE);
	memset(accK, 0, sizeof(int)*ACC_K_SIZE);
	memset(accA, 0, sizeof(int)*ACC_A_SIZE);		

	// get size of the 4 vectors of slopes (2 pairs of arcs)
	int sz_ij1 = int(data_ij.slopes1.size());
	int sz_ij2 = int(data_ij.slopes2.size());
	int sz_ik1 = int(data_ik.slopes1.size());
	int sz_ik2 = int(data_ik.slopes2.size());

	 //get the size of the 3 arcs
	ushort sz_ei = edge_i.size();
	ushort sz_ej = edge_j.size();
	ushort sz_ek = edge_k.size();

	// center of the estimated ellipse
	float a0 = center.x;
	float b0 = center.y;


	// estimation of remaining parameters
	// uses 4 combinations of parameters. See Tab [] and Sect [] in the paper.
	{
		float q1 = data_ij.m1;
		float q3 = data_ik.m1;
		float q5 = data_ik.m2;
		
		for(int ij1=0; ij1<sz_ij1; ++ij1)
		{
			float q2 = data_ij.slopes1[ij1];

			float q1xq2 = q1*q2;

			for(int ik1=0; ik1<sz_ik1; ++ik1)
			{
				float q4 = data_ik.slopes1[ik1];

				float q3xq4 = q3*q4;

				float b = (q3xq4 + 1)*(q1+q2) - (q1xq2 + 1)*(q3+q4);
				float a = (q1xq2 - q3xq4);
				float Kp = (-b + sqrt(b*b + 4*a*a)) / (2*a);
				float zplus = ((q1-Kp)*(q2-Kp)) / ((1+q1*Kp)*(1+q2*Kp));

				if(zplus >= 0.0f)
				{
					continue;
				}

				float Np = sqrt(-zplus);
				float rho = atan(Kp);
				int Kdeg;
				if(Np > 1.f)
				{
					Np = 1.f / Np;
					Kdeg = cvRound((rho * 180 / CV_PI) + 180) % 180; // [0,180)
					rho += CV_PI * 0.5f;
				}
				else
				{
					Kdeg = cvRound((rho * 180 / CV_PI) + 90) % 180; // [0,180)
				}

				int iNp = cvRound(Np*100); // [0, 100]

				if	(	0 <= iNp	&& iNp  < ACC_N_SIZE &&
						0 <= Kdeg	&& Kdeg < ACC_K_SIZE
					)
				{
					++ accN[iNp];
					++ accK[Kdeg];
				}
			}

			
			for(int ik2=0; ik2<sz_ik2; ++ik2)
			{
				float q4 = data_ik.slopes2[ik2];

				float q5xq4 = q5*q4;

				float b = (q5xq4 + 1)*(q1+q2) - (q1xq2 + 1)*(q5+q4);
				float a = (q1xq2 - q5xq4);
				float Kp = (-b + sqrt(b*b + 4*a*a)) / (2*a);
				float zplus = ((q1-Kp)*(q2-Kp)) / ((1+q1*Kp)*(1+q2*Kp));

				if(zplus >= 0.0f)
				{
					continue;
				}

				float Np = sqrt(-zplus);
				float rho = atan(Kp);
				int Kdeg;
				if(Np > 1.f)
				{
					Np = 1.f / Np;
					Kdeg = cvRound((rho * 180 / CV_PI) + 180) % 180; // [0,180)
					rho += CV_PI * 0.5f;
				}
				else
				{
					Kdeg = cvRound((rho * 180 / CV_PI) + 90) % 180; // [0,180)
				}

				int iNp = cvRound(Np*100); // [0, 100]

				if	(	0 <= iNp	&& iNp  < ACC_N_SIZE &&
						0 <= Kdeg	&& Kdeg < ACC_K_SIZE
					)
				{
					++ accN[iNp];
					++ accK[Kdeg];
				}
			}
			
		}
	}

	
	{
		float q1 = data_ij.m2;
		float q3 = data_ik.m2;
		float q5 = data_ik.m1;

		for(int ij2=0; ij2<sz_ij2; ++ij2)
		{
			float q2 = data_ij.slopes2[ij2];

			float q1xq2 = q1*q2;

			for(int ik2=0; ik2<sz_ik2; ++ik2)
			{
				float q4 = data_ik.slopes2[ik2];

				float q3xq4 = q3*q4;

				float b = (q3xq4 + 1)*(q1+q2) - (q1xq2 + 1)*(q3+q4);
				float a = (q1xq2 - q3xq4);
				float Kp = (-b + sqrt(b*b + 4*a*a)) / (2*a);
				float zplus = ((q1-Kp)*(q2-Kp)) / ((1+q1*Kp)*(1+q2*Kp));

				if(zplus >= 0.0f)
				{
					continue;
				}

				float Np = sqrt(-zplus);
				float rho = atan(Kp);
				int Kdeg;
				if(Np > 1.f)
				{
					Np = 1.f / Np;
					Kdeg = cvRound((rho * 180 / CV_PI) + 180) % 180; // [0,180)
					rho += CV_PI * 0.5f;
				}
				else
				{
					Kdeg = cvRound((rho * 180 / CV_PI) + 90) % 180; // [0,180)
				}

				int iNp = cvRound(Np*100); // [0, 100]

				if	(	0 <= iNp	&& iNp  < ACC_N_SIZE &&
						0 <= Kdeg	&& Kdeg < ACC_K_SIZE
					)
				{
					++ accN[iNp];
					++ accK[Kdeg];
				}
			}

			
			for(int ik1=0; ik1<sz_ik1; ++ik1)
			{
				float q4 = data_ik.slopes1[ik1];

				float q5xq4 = q5*q4;

				float b = (q5xq4 + 1)*(q1+q2) - (q1xq2 + 1)*(q5+q4);
				float a = (q1xq2 - q5xq4);
				float Kp = (-b + sqrt(b*b + 4*a*a)) / (2*a);
				float zplus = ((q1-Kp)*(q2-Kp)) / ((1+q1*Kp)*(1+q2*Kp));

				if(zplus >= 0.0f)
				{
					continue;
				}

				float Np = sqrt(-zplus);
				float rho = atan(Kp);
				int Kdeg;
				if(Np > 1.f)
				{
					Np = 1.f / Np;
					Kdeg = cvRound((rho * 180 / CV_PI) + 180) % 180; // [0,180)
					rho += CV_PI * 0.5f;
				}
				else
				{
					Kdeg = cvRound((rho * 180 / CV_PI) + 90) % 180; // [0,180)
				}

				int iNp = cvRound(Np*100); // [0, 100]

				if	(	0 <= iNp	&& iNp  < ACC_N_SIZE &&
						0 <= Kdeg	&& Kdeg < ACC_K_SIZE
					)
				{
					++ accN[iNp];
					++ accK[Kdeg];
				}
			}
			
		}
	}

	//find peak in N and K accumulator
	int iN	= FindMaxN(accN);
	int iK	= FindMaxK(accK);

	//recover real values
	float fK = float(iK);
	float Np = float(iN) * 0.01f;
	float rho = fK * CV_PI / 180.f;	//deg 2 rad
	float Kp = tan(rho);

	// estimate A. See Sect [] in the paper

	for(ushort l=0; l<sz_ei; ++l)
	{
		cv::Point& pp = edge_i[l];
		float sk = 1.f / sqrt(Kp*Kp + 1.f);
		float x0 = ((pp.x - a0) * sk) + (((pp.y - b0)*Kp) * sk);
		float y0 = -(((pp.x - a0) * Kp) * sk) + ((pp.y - b0) * sk);
		float Ax = sqrt((x0*x0*Np*Np + y0*y0) / ((Np*Np)*(1.f + Kp*Kp)));
		int A  = cvRound(abs(Ax / cos(rho)));
		if( (abs(A*Np) > fInitB_*(1 + fTolRatioB_) || abs(A*Np) < fInitB_*(1 - fTolRatioB_)) && fInitB_ >ELLIPSE_ZERO )
			continue;
		if((0 <= A) && (A < ACC_A_SIZE))
		{
			++ accA[A];
		}
	}

	for(ushort l=0; l<sz_ej; ++l)
	{
		cv::Point& pp = edge_j[l];
		float sk = 1.f / sqrt(Kp*Kp + 1.f);
		float x0 = ((pp.x - a0) * sk) + (((pp.y - b0)*Kp) * sk);
		float y0 = -(((pp.x - a0) * Kp) * sk) + ((pp.y - b0) * sk);
		float Ax = sqrt((x0*x0*Np*Np + y0*y0) / ((Np*Np)*(1.f + Kp*Kp)));
		int A  = cvRound(abs(Ax / cos(rho)));
		if( (abs(A*Np) > fInitB_*(1 + fTolRatioB_) || abs(A*Np) < fInitB_*(1 - fTolRatioB_)) && fInitB_ >ELLIPSE_ZERO )
			continue;
		if((0 <= A) && (A < ACC_A_SIZE))
		{
			++ accA[A];
		}
	}

	for(ushort l=0; l<sz_ek; ++l)
	{
		cv::Point& pp = edge_k[l];
		float sk = 1.f / sqrt(Kp*Kp + 1.f);
		float x0 = ((pp.x - a0) * sk) + (((pp.y - b0)*Kp) * sk);
		float y0 = -(((pp.x - a0) * Kp) * sk) + ((pp.y - b0) * sk);
		float Ax = sqrt((x0*x0*Np*Np + y0*y0) / ((Np*Np)*(1.f + Kp*Kp)));
		int A  = cvRound(abs(Ax / cos(rho)));
		if( (abs(A*Np) > fInitB_*(1 + fTolRatioB_) || abs(A*Np) < fInitB_*(1 - fTolRatioB_)) && fInitB_ >ELLIPSE_ZERO )
			continue;
		if((0 <= A) && (A < ACC_A_SIZE))
		{
			++ accA[A];
		}
	}

	// find peak in A accumulator
	int A = FindMaxA(accA);
	float fA = float(A);

	// find B value
	float fB = abs(fA * Np);

	// got all ellipse parameters!
	CEllipse ell(a0, b0, fA, fB, fmod(rho, float(CV_PI)));

	// get the score. See Sect [] in the paper

	//find the number of edge pixel lying on the ellipse
	float _cos = cos(-ell.rho_);
	float _sin = sin(-ell.rho_);

	float invA2 = 1.f / (ell.A_ * ell.A_);
	float invB2 = 1.f / (ell.B_ * ell.B_);

	float invNofPoints = 1.f / float(sz_ei + sz_ej + sz_ek);
	int counter_on_perimeter = 0;

	for(ushort l=0; l<sz_ei; ++l)
	{
		float tx = float(edge_i[l].x) - ell.xc_;
		float ty = float(edge_i[l].y) - ell.yc_;
		float rx = (tx*_cos - ty*_sin);
		float ry = (tx*_sin + ty*_cos);

		float h = (rx*rx)*invA2 + (ry*ry)*invB2;
		if(abs(h - 1.f) < fDistanceToEllipseContour_)
		{
			++counter_on_perimeter;
		}
	}

	for(ushort l=0; l<sz_ej; ++l)
	{
		float tx = float(edge_j[l].x) - ell.xc_;
		float ty = float(edge_j[l].y) - ell.yc_;
		float rx = (tx*_cos - ty*_sin);
		float ry = (tx*_sin + ty*_cos);

		float h = (rx*rx)*invA2 + (ry*ry)*invB2;
		if(abs(h - 1.f) < fDistanceToEllipseContour_)
		{
			++counter_on_perimeter;
		}
	}

	for(ushort l=0; l<sz_ek; ++l)
	{
		float tx = float(edge_k[l].x) - ell.xc_;
		float ty = float(edge_k[l].y) - ell.yc_;
		float rx = (tx*_cos - ty*_sin);
		float ry = (tx*_sin + ty*_cos);

		float h = (rx*rx)*invA2 + (ry*ry)*invB2;
		if(abs(h - 1.f) < fDistanceToEllipseContour_)
		{
			++counter_on_perimeter;
		}
	}

	//no points found on the ellipse
	if(counter_on_perimeter <= 0) return;


	//compute score
	ell.score_ = float(counter_on_perimeter) * invNofPoints;
	//no enough points found on the ellipse
	if(ell.score_ < fMinScore_) return;

	//compute reliability	
	float di,dj,dk;
	{
		cv::Point2f p1(float(edge_i[0].x), float(edge_i[0].y));
		cv::Point2f p2(float(edge_i[sz_ei-1].x), float(edge_i[sz_ei-1].y));
		p1.x -= ell.xc_;
		p1.y -= ell.yc_;
		p2.x -= ell.xc_;
		p2.y -= ell.yc_;
		cv::Point2f r1((p1.x*_cos - p1.y*_sin),(p1.x*_sin + p1.y*_cos));
		cv::Point2f r2((p2.x*_cos - p2.y*_sin),(p2.x*_sin + p2.y*_cos));
		di = abs(r2.x - r1.x) + abs(r2.y - r1.y);
	}
	{
		cv::Point2f p1(float(edge_j[0].x), float(edge_j[0].y));
		cv::Point2f p2(float(edge_j[sz_ej-1].x), float(edge_j[sz_ej-1].y));
		p1.x -= ell.xc_;
		p1.y -= ell.yc_;
		p2.x -= ell.xc_;
		p2.y -= ell.yc_;
		cv::Point2f r1((p1.x*_cos - p1.y*_sin),(p1.x*_sin + p1.y*_cos));
		cv::Point2f r2((p2.x*_cos - p2.y*_sin),(p2.x*_sin + p2.y*_cos));
		dj = abs(r2.x - r1.x) + abs(r2.y - r1.y);
	}
	{
		cv::Point2f p1(float(edge_k[0].x), float(edge_k[0].y));
		cv::Point2f p2(float(edge_k[sz_ek-1].x), float(edge_k[sz_ek-1].y));
		p1.x -= ell.xc_;
		p1.y -= ell.yc_;
		p2.x -= ell.xc_;
		p2.y -= ell.yc_;
		cv::Point2f r1((p1.x*_cos - p1.y*_sin),(p1.x*_sin + p1.y*_cos));
		cv::Point2f r2((p2.x*_cos - p2.y*_sin),(p2.x*_sin + p2.y*_cos));
		dk = abs(r2.x - r1.x) + abs(r2.y - r1.y);
	}

	ell.rel_ = std::min(1.f, ((di + dj + dk) / (3*(ell.A_ + ell.B_))));
	
	//not enough reliable
	if(ell.rel_ < fMinReliability_) return;

	//the tentative detection has been confirmed. Save it!
	ellipses.push_back(ell);
};

//most important function for detecting ellipses. See Sect[] of the paper
void EllipseDetector::FindEllipses_simple(cv::Point2f& center,
	VP& edge_i,
	VP& edge_j,
	VP& edge_k,
	EllipseData& data_ij,
	EllipseData& data_ik,
	CEllipse &ellipse, float thres,
	float thres_score
	)
{

	//FIND ELLIPSE PARAMETERS

	// 0-initialize accumulators
	memset(accN, 0, sizeof(int)*ACC_N_SIZE);
	memset(accK, 0, sizeof(int)*ACC_K_SIZE);
	memset(accA, 0, sizeof(int)*ACC_A_SIZE);

	// get size of the 4 vectors of slopes (2 pairs of arcs)
	int sz_ij1 = int(data_ij.slopes1.size());
	int sz_ij2 = int(data_ij.slopes2.size());
	int sz_ik1 = int(data_ik.slopes1.size());
	int sz_ik2 = int(data_ik.slopes2.size());

	//get the size of the 3 arcs
	ushort sz_ei = edge_i.size();
	ushort sz_ej = edge_j.size();
	ushort sz_ek = edge_k.size();

	// center of the estimated ellipse
	float a0 = center.x;
	float b0 = center.y;


	// estimation of remaining parameters
	// uses 4 combinations of parameters. See Tab [] and Sect [] in the paper.
	{
		float q1 = data_ij.m1;
		float q3 = data_ik.m1;
		float q5 = data_ik.m2;

		for (int ij1 = 0; ij1 < sz_ij1; ++ij1)
		{
			float q2 = data_ij.slopes1[ij1];

			float q1xq2 = q1*q2;

			for (int ik1 = 0; ik1 < sz_ik1; ++ik1)
			{
				float q4 = data_ik.slopes1[ik1];

				float q3xq4 = q3*q4;

				float b = (q3xq4 + 1)*(q1 + q2) - (q1xq2 + 1)*(q3 + q4);
				float a = (q1xq2 - q3xq4);
				float Kp = (-b + sqrt(b*b + 4 * a*a)) / (2 * a);
				float zplus = ((q1 - Kp)*(q2 - Kp)) / ((1 + q1*Kp)*(1 + q2*Kp));

				if (zplus >= 0.0f)
				{
					continue;
				}

				float Np = sqrt(-zplus);
				float rho = atan(Kp);
				int Kdeg;
				if (Np > 1.f)
				{
					Np = 1.f / Np;
					Kdeg = cvRound((rho * 180 / CV_PI) + 180) % 180; // [0,180)
					rho += CV_PI * 0.5f;
				}
				else
				{
					Kdeg = cvRound((rho * 180 / CV_PI) + 90) % 180; // [0,180)
				}

				int iNp = cvRound(Np * 100); // [0, 100]

				if (0 <= iNp	&& iNp < ACC_N_SIZE &&
					0 <= Kdeg	&& Kdeg < ACC_K_SIZE
					)
				{
					++accN[iNp];
					++accK[Kdeg];
				}
			}


			for (int ik2 = 0; ik2 < sz_ik2; ++ik2)
			{
				float q4 = data_ik.slopes2[ik2];

				float q5xq4 = q5*q4;

				float b = (q5xq4 + 1)*(q1 + q2) - (q1xq2 + 1)*(q5 + q4);
				float a = (q1xq2 - q5xq4);
				float Kp = (-b + sqrt(b*b + 4 * a*a)) / (2 * a);
				float zplus = ((q1 - Kp)*(q2 - Kp)) / ((1 + q1*Kp)*(1 + q2*Kp));

				if (zplus >= 0.0f)
				{
					continue;
				}

				float Np = sqrt(-zplus);
				float rho = atan(Kp);
				int Kdeg;
				if (Np > 1.f)
				{
					Np = 1.f / Np;
					Kdeg = cvRound((rho * 180 / CV_PI) + 180) % 180; // [0,180)
					rho += CV_PI * 0.5f;
				}
				else
				{
					Kdeg = cvRound((rho * 180 / CV_PI) + 90) % 180; // [0,180)
				}

				int iNp = cvRound(Np * 100); // [0, 100]

				if (0 <= iNp	&& iNp < ACC_N_SIZE &&
					0 <= Kdeg	&& Kdeg < ACC_K_SIZE
					)
				{
					++accN[iNp];
					++accK[Kdeg];
				}
			}

		}
	}


	{
		float q1 = data_ij.m2;
		float q3 = data_ik.m2;
		float q5 = data_ik.m1;

		for (int ij2 = 0; ij2 < sz_ij2; ++ij2)
		{
			float q2 = data_ij.slopes2[ij2];

			float q1xq2 = q1*q2;

			for (int ik2 = 0; ik2 < sz_ik2; ++ik2)
			{
				float q4 = data_ik.slopes2[ik2];

				float q3xq4 = q3*q4;

				float b = (q3xq4 + 1)*(q1 + q2) - (q1xq2 + 1)*(q3 + q4);
				float a = (q1xq2 - q3xq4);
				float Kp = (-b + sqrt(b*b + 4 * a*a)) / (2 * a);
				float zplus = ((q1 - Kp)*(q2 - Kp)) / ((1 + q1*Kp)*(1 + q2*Kp));

				if (zplus >= 0.0f)
				{
					continue;
				}

				float Np = sqrt(-zplus);
				float rho = atan(Kp);
				int Kdeg;
				if (Np > 1.f)
				{
					Np = 1.f / Np;
					Kdeg = cvRound((rho * 180 / CV_PI) + 180) % 180; // [0,180)
					rho += CV_PI * 0.5f;
				}
				else
				{
					Kdeg = cvRound((rho * 180 / CV_PI) + 90) % 180; // [0,180)
				}

				int iNp = cvRound(Np * 100); // [0, 100]

				if (0 <= iNp	&& iNp < ACC_N_SIZE &&
					0 <= Kdeg	&& Kdeg < ACC_K_SIZE
					)
				{
					++accN[iNp];
					++accK[Kdeg];
				}
			}


			for (int ik1 = 0; ik1 < sz_ik1; ++ik1)
			{
				float q4 = data_ik.slopes1[ik1];

				float q5xq4 = q5*q4;

				float b = (q5xq4 + 1)*(q1 + q2) - (q1xq2 + 1)*(q5 + q4);
				float a = (q1xq2 - q5xq4);
				float Kp = (-b + sqrt(b*b + 4 * a*a)) / (2 * a);
				float zplus = ((q1 - Kp)*(q2 - Kp)) / ((1 + q1*Kp)*(1 + q2*Kp));

				if (zplus >= 0.0f)
				{
					continue;
				}

				float Np = sqrt(-zplus);
				float rho = atan(Kp);
				int Kdeg;
				if (Np > 1.f)
				{
					Np = 1.f / Np;
					Kdeg = cvRound((rho * 180 / CV_PI) + 180) % 180; // [0,180)
					rho += CV_PI * 0.5f;
				}
				else
				{
					Kdeg = cvRound((rho * 180 / CV_PI) + 90) % 180; // [0,180)
				}

				int iNp = cvRound(Np * 100); // [0, 100]

				if (0 <= iNp	&& iNp < ACC_N_SIZE &&
					0 <= Kdeg	&& Kdeg < ACC_K_SIZE
					)
				{
					++accN[iNp];
					++accK[Kdeg];
				}
			}

		}
	}

	//find peak in N and K accumulator
	int iN = FindMaxN(accN);
	int iK = FindMaxK(accK);

	//recover real values
	float fK = float(iK);
	float Np = float(iN) * 0.01f;
	float rho = fK * CV_PI / 180.f;	//deg 2 rad
	float Kp = tan(rho);

	// estimate A. See Sect [] in the paper

	for (ushort l = 0; l < sz_ei; ++l)
	{
		cv::Point& pp = edge_i[l];
		float sk = 1.f / sqrt(Kp*Kp + 1.f);
		float x0 = ((pp.x - a0) * sk) + (((pp.y - b0)*Kp) * sk);
		float y0 = -(((pp.x - a0) * Kp) * sk) + ((pp.y - b0) * sk);
		float Ax = sqrt((x0*x0*Np*Np + y0*y0) / ((Np*Np)*(1.f + Kp*Kp)));
		int A = cvRound(abs(Ax / cos(rho)));
		if ((0 <= A) && (A < ACC_A_SIZE))
		{
			++accA[A];
		}
	}

	for (ushort l = 0; l < sz_ej; ++l)
	{
		cv::Point& pp = edge_j[l];
		float sk = 1.f / sqrt(Kp*Kp + 1.f);
		float x0 = ((pp.x - a0) * sk) + (((pp.y - b0)*Kp) * sk);
		float y0 = -(((pp.x - a0) * Kp) * sk) + ((pp.y - b0) * sk);
		float Ax = sqrt((x0*x0*Np*Np + y0*y0) / ((Np*Np)*(1.f + Kp*Kp)));
		int A = cvRound(abs(Ax / cos(rho)));
		if ((0 <= A) && (A < ACC_A_SIZE))
		{
			++accA[A];
		}
	}

	for (ushort l = 0; l < sz_ek; ++l)
	{
		cv::Point& pp = edge_k[l];
		float sk = 1.f / sqrt(Kp*Kp + 1.f);
		float x0 = ((pp.x - a0) * sk) + (((pp.y - b0)*Kp) * sk);
		float y0 = -(((pp.x - a0) * Kp) * sk) + ((pp.y - b0) * sk);
		float Ax = sqrt((x0*x0*Np*Np + y0*y0) / ((Np*Np)*(1.f + Kp*Kp)));
		int A = cvRound(abs(Ax / cos(rho)));
		if ((0 <= A) && (A < ACC_A_SIZE))
		{
			++accA[A];
		}
	}

	// find peak in A accumulator
	int A = FindMaxA(accA);
	float fA = float(A);

	// find B value
	float fB = abs(fA * Np);

	// got all ellipse parameters!
	CEllipse ell(a0, b0, fA, fB, fmod(rho, float(CV_PI)));

	// get the score. See Sect [] in the paper

	//find the number of edge pixel lying on the ellipse
	float _cos = cos(-ell.rho_);
	float _sin = sin(-ell.rho_);

	float invA2 = 1.f / (ell.A_ * ell.A_);
	float invB2 = 1.f / (ell.B_ * ell.B_);

	float invNofPoints = 1.f / float(sz_ei + sz_ej + sz_ek);
	int counter_on_perimeter = 0;

	for (ushort l = 0; l < sz_ei; ++l)
	{
		float tx = float(edge_i[l].x) - ell.xc_;
		float ty = float(edge_i[l].y) - ell.yc_;
		float rx = (tx*_cos - ty*_sin);
		float ry = (tx*_sin + ty*_cos);

		float h = (rx*rx)*invA2 + (ry*ry)*invB2;
		if (abs(h - 1.f) < fDistanceToEllipseContour_)
		{
			++counter_on_perimeter;
		}
	}

	for (ushort l = 0; l < sz_ej; ++l)
	{
		float tx = float(edge_j[l].x) - ell.xc_;
		float ty = float(edge_j[l].y) - ell.yc_;
		float rx = (tx*_cos - ty*_sin);
		float ry = (tx*_sin + ty*_cos);

		float h = (rx*rx)*invA2 + (ry*ry)*invB2;
		if (abs(h - 1.f) < fDistanceToEllipseContour_)
		{
			++counter_on_perimeter;
		}
	}

	for (ushort l = 0; l < sz_ek; ++l)
	{
		float tx = float(edge_k[l].x) - ell.xc_;
		float ty = float(edge_k[l].y) - ell.yc_;
		float rx = (tx*_cos - ty*_sin);
		float ry = (tx*_sin + ty*_cos);

		float h = (rx*rx)*invA2 + (ry*ry)*invB2;
		if (abs(h - 1.f) < fDistanceToEllipseContour_)
		{
			++counter_on_perimeter;
		}
	}

	//no points found on the ellipse
	if (counter_on_perimeter <= 0) return;


	//compute score
	ell.score_ = float(counter_on_perimeter) * invNofPoints;
	//no enough points found on the ellipse
	if (ell.score_ < fMinScore_) return;

	//compute reliability	
	float di, dj, dk;
	{
		cv::Point2f p1(float(edge_i[0].x), float(edge_i[0].y));
		cv::Point2f p2(float(edge_i[sz_ei - 1].x), float(edge_i[sz_ei - 1].y));
		p1.x -= ell.xc_;
		p1.y -= ell.yc_;
		p2.x -= ell.xc_;
		p2.y -= ell.yc_;
		cv::Point2f r1((p1.x*_cos - p1.y*_sin), (p1.x*_sin + p1.y*_cos));
		cv::Point2f r2((p2.x*_cos - p2.y*_sin), (p2.x*_sin + p2.y*_cos));
		di = abs(r2.x - r1.x) + abs(r2.y - r1.y);
	}
	{
		cv::Point2f p1(float(edge_j[0].x), float(edge_j[0].y));
		cv::Point2f p2(float(edge_j[sz_ej - 1].x), float(edge_j[sz_ej - 1].y));
		p1.x -= ell.xc_;
		p1.y -= ell.yc_;
		p2.x -= ell.xc_;
		p2.y -= ell.yc_;
		cv::Point2f r1((p1.x*_cos - p1.y*_sin), (p1.x*_sin + p1.y*_cos));
		cv::Point2f r2((p2.x*_cos - p2.y*_sin), (p2.x*_sin + p2.y*_cos));
		dj = abs(r2.x - r1.x) + abs(r2.y - r1.y);
	}
	{
		cv::Point2f p1(float(edge_k[0].x), float(edge_k[0].y));
		cv::Point2f p2(float(edge_k[sz_ek - 1].x), float(edge_k[sz_ek - 1].y));
		p1.x -= ell.xc_;
		p1.y -= ell.yc_;
		p2.x -= ell.xc_;
		p2.y -= ell.yc_;
		cv::Point2f r1((p1.x*_cos - p1.y*_sin), (p1.x*_sin + p1.y*_cos));
		cv::Point2f r2((p2.x*_cos - p2.y*_sin), (p2.x*_sin + p2.y*_cos));
		dk = abs(r2.x - r1.x) + abs(r2.y - r1.y);
	}

	ell.rel_ = std::min(1.f, ((di + dj + dk) / (3 * (ell.A_ + ell.B_))));

	//not enough reliable
	if (ell.rel_ < fMinReliability_) return;

	//the tentative detection has been confirmed. Save it!
	if (ell.score_ > thres_score && ell.score_ > ellipse.score_ && ell.A_*ell.B_ > thres){
		ellipse = ell;
	}
};

// Get the coordinates of the center, given the intersection of the estimated lines. See Sect [] in the paper.
cv::Point2f EllipseDetector::GetCenterCoordinates	(	EllipseData& data_ij,
														EllipseData& data_ik
													)
{
	float xx[7];
	float yy[7];

	xx[0] = data_ij.center.x;
	xx[1] = data_ik.center.x;
	yy[0] = data_ij.center.y;
	yy[1] = data_ik.center.y;
	
	{
		//1-1
		float q2 = data_ij.t1;
		float q4 = data_ik.t1;
		cv::Point2f& M12 = data_ij.M1;
		cv::Point2f& M34 = data_ik.M1;

		float invDen = 1 / (q2 - q4);
		xx[2] = (M34.y - q4*M34.x - M12.y + q2*M12.x) * invDen;
		yy[2] = (q2*M34.y - q4*M12.y + q2*q4*(M12.x - M34.x)) * invDen;
	}

	{
		//1-2
		float q2 = data_ij.t1;
		float q4 = data_ik.t2;
		cv::Point2f& M12 = data_ij.M1;
		cv::Point2f& M34 = data_ik.M2;

		float invDen = 1 / (q2 - q4);
		xx[3] = (M34.y - q4*M34.x - M12.y + q2*M12.x) * invDen;
		yy[3] = (q2*M34.y - q4*M12.y + q2*q4*(M12.x - M34.x)) * invDen;
	}

	{
		//2-2
		float q2 = data_ij.t2;
		float q4 = data_ik.t2;
		cv::Point2f& M12 = data_ij.M2;
		cv::Point2f& M34 = data_ik.M2;

		float invDen = 1 / (q2 - q4);
		xx[4] = (M34.y - q4*M34.x - M12.y + q2*M12.x) * invDen;
		yy[4] = (q2*M34.y - q4*M12.y + q2*q4*(M12.x - M34.x)) * invDen;
	}

	{
		//2-1
		float q2 = data_ij.t2;
		float q4 = data_ik.t1;
		cv::Point2f& M12 = data_ij.M2;
		cv::Point2f& M34 = data_ik.M1;

		float invDen = 1 / (q2 - q4);
		xx[5] = (M34.y - q4*M34.x - M12.y + q2*M12.x) * invDen;
		yy[5] = (q2*M34.y - q4*M12.y + q2*q4*(M12.x - M34.x)) * invDen;
	}

	xx[6] = (xx[0] + xx[1]) * 0.5f;
	yy[6] = (yy[0] + yy[1]) * 0.5f;


	//MEDIAN
	std::nth_element(xx, xx+3,xx+7);
	std::nth_element(yy, yy+3,yy+7);
	float xc = xx[3];
	float yc = yy[3];

	return cv::Point2f(xc,yc);
};


//verify triplets of arcs with convexity: i=1, j=2, k=4
void EllipseDetector::Triplets124	(	VVP& pi,
											VVP& pj,
											VVP& pk,
											std::map<uint, EllipseData>& data,
											std::vector<CEllipse>& ellipses
										)
{
	// get arcs length
	ushort sz_i = ushort(pi.size());
	ushort sz_j = ushort(pj.size());
	ushort sz_k = ushort(pk.size());

	//for each edge i
	for(ushort i=0; i<sz_i; ++i)
	{
		VP& edge_i = pi[i];
		ushort sz_ei = ushort(edge_i.size());

		cv::Point& pif = edge_i[0];
		cv::Point& pil = edge_i[sz_ei - 1];

		//1,2 -> reverse 1, swap
		VP rev_i(edge_i.size());
		reverse_copy(edge_i.begin(), edge_i.end(), rev_i.begin());

		//for each edge j
		for(ushort j=0; j<sz_j; ++j)
		{
			VP& edge_j = pj[j];
			ushort sz_ej = ushort(edge_j.size());

			cv::Point& pjf = edge_j[0];
			cv::Point& pjl = edge_j[sz_ej - 1];

			//CONSTRAINTS on position
			if(pjl.x > pif.x) //is right				
			{				
				//discard
				continue;
			}
			
			uint key_ij = GenerateKey(PAIR_12, i, j);

			//for each edge k
			for(ushort k=0; k<sz_k; ++k)
			{
				VP& edge_k = pk[k];
				ushort sz_ek = ushort(edge_k.size());

				cv::Point& pkf = edge_k[0];
				cv::Point& pkl = edge_k[sz_ek - 1];
								
				//CONSTRAINTS on position
				if	(	(pkl.y < pil.y) ||		//is above i
						(pkl.y < pjl.y)	||		//is above j
						(pkl.x < pjf.x)			//is left j
					)
				{					
					//discard
					continue;
				}

				uint key_ik = GenerateKey(PAIR_14, i, k);

				//find centers
			
				EllipseData data_ij, data_ik;							

				//if the data for the pair i-j have not been computed yet
				if(data.count(key_ij) == 0)
				{					
					//1,2 -> reverse 1, swap

					//compute data!
					GetFastCenter(edge_j, rev_i, data_ij);
					//insert computed data in the hash table
					data.insert(std::pair<uint,EllipseData>(key_ij,data_ij));
				}
				else
				{
					//otherwise, just lookup the data in the hash table
					data_ij = data.at(key_ij);
				}

				//if the data for the pair i-k have not been computed yet
				if(data.count(key_ik) == 0)
				{
					//1,4 -> ok

					//compute data!
					GetFastCenter(edge_i, edge_k, data_ik);	
					//insert computed data in the hash table
					data.insert(std::pair<uint,EllipseData>(key_ik,data_ik));
				}
				else
				{
					//otherwise, just lookup the data in the hash table
					data_ik = data.at(key_ik);
				}

				//INVALID CENTERS
				if(!data_ij.isValid || !data_ik.isValid)
				{
					continue;
				}

				//Selection strategy - Step 3. See Sect [] in the paper
				//the computer centers are not close enough
				if(ed2(data_ij.center, data_ik.center) > fMaxCenterDistance2_)
				{
					//discard
					continue;
				}

				// if all constraints of the selection strategy have been satisfied, 
				// we can start estimating the ellipse parameters

				//FIND ELLIPSE PARAMETERS
				
				// get the coordinates of the center (xc, yc)
				cv::Point2f center = GetCenterCoordinates(data_ij, data_ik);
				//find remaining parameters (A,B,rho)
				FindEllipses(center, edge_i, edge_j, edge_k, data_ij, data_ik, ellipses);
			}				
		}
	}
};



void EllipseDetector::Triplets231	(	VVP& pi,
											VVP& pj,
											VVP& pk,
											std::map<uint, EllipseData>& data,
											std::vector<CEllipse>& ellipses
										)
{
	ushort sz_i = ushort(pi.size());
	ushort sz_j = ushort(pj.size());
	ushort sz_k = ushort(pk.size());

	//for each edge i
	for(ushort i=0; i<sz_i; ++i)
	{
		VP& edge_i = pi[i];
		ushort sz_ei = ushort(edge_i.size());

		cv::Point& pif = edge_i[0];
		cv::Point& pil = edge_i[sz_ei - 1];

		VP rev_i(edge_i.size());
		std::reverse_copy(edge_i.begin(), edge_i.end(), rev_i.begin());

		//for each edge j
		for(ushort j=0; j<sz_j; ++j)
		{
			VP& edge_j = pj[j];
			ushort sz_ej = ushort(edge_j.size());

			cv::Point& pjf = edge_j[0];
			cv::Point& pjl = edge_j[sz_ej - 1];			

			//CONSTRAINTS on position
			if(pjf.y < pif.y) //is above
			{
				//discard
				continue;
			}

			VP rev_j(edge_j.size());
			std::reverse_copy(edge_j.begin(), edge_j.end(), rev_j.begin());

			uint key_ij = GenerateKey(PAIR_23, i, j);

			//for each edge k
			for(ushort k=0; k<sz_k; ++k)
			{
				VP& edge_k = pk[k];
				ushort sz_ek = ushort(edge_k.size());

				cv::Point& pkf = edge_k[0];
				cv::Point& pkl = edge_k[sz_ek - 1];

				//CONSTRAINTS on position
				if	(	(pkf.x < pil.x) ||		//is left i
						(pkl.x < pjf.x)	||		//is left j
						(pkf.y > pjl.y)			//is below j
					)
				{
					//discard
					continue;
				}

				uint key_ik = GenerateKey(PAIR_12, k, i);
				
				//find centers

				EllipseData data_ij, data_ik;

				if(data.count(key_ij) == 0)
				{
					//2,3 -> reverse 2,3
					
					GetFastCenter(rev_i, rev_j, data_ij);
					data.insert(std::pair<uint,EllipseData>(key_ij,data_ij));
				}
				else
				{
					data_ij = data.at(key_ij);
				}

				if(data.count(key_ik) == 0)
				{
					//2,1 -> reverse 1
					VP rev_k(edge_k.size());
					reverse_copy(edge_k.begin(), edge_k.end(), rev_k.begin());

					GetFastCenter(edge_i, rev_k, data_ik);
					data.insert(std::pair<uint,EllipseData>(key_ik,data_ik));
				}
				else
				{
					data_ik = data.at(key_ik);
				}

				//INVALID CENTERS
				if(!data_ij.isValid || !data_ik.isValid)
				{
					continue;
				}

				//CONSTRAINT ON CENTERS
				if(ed2(data_ij.center, data_ik.center) > fMaxCenterDistance2_)
				{
					//discard
					continue;
				}

				//FIND ELLIPSE PARAMETERS
				cv::Point2f center = GetCenterCoordinates(data_ij, data_ik);

				FindEllipses(center, edge_i, edge_j, edge_k, data_ij, data_ik, ellipses);

			}
		}
	}
};


void EllipseDetector::Triplets342	(	VVP& pi,
											VVP& pj,
											VVP& pk,
											std::map<uint, EllipseData>& data,
											std::vector<CEllipse>& ellipses
										)
{
	ushort sz_i = ushort(pi.size());
	ushort sz_j = ushort(pj.size());
	ushort sz_k = ushort(pk.size());
	
	//for each edge i
	for(ushort i=0; i<sz_i; ++i)
	{
		VP& edge_i = pi[i];
		ushort sz_ei = ushort(edge_i.size());

		cv::Point& pif = edge_i[0];
		cv::Point& pil = edge_i[sz_ei - 1];

		VP rev_i(edge_i.size());
		reverse_copy(edge_i.begin(), edge_i.end(), rev_i.begin());
		
		//for each edge j
		for(ushort j=0; j<sz_j; ++j)
		{
			VP& edge_j = pj[j];
			ushort sz_ej = ushort(edge_j.size());

			cv::Point& pjf = edge_j[0];
			cv::Point& pjl = edge_j[sz_ej - 1];


			//CONSTRAINTS on position
			if	(pjf.x < pil.x) 		//is left
			{
				//discard
				continue;
			}
			VP rev_j(edge_j.size());
			std::reverse_copy(edge_j.begin(), edge_j.end(), rev_j.begin());

			uint key_ij = GenerateKey(PAIR_34, i, j);

			//for each edge k
			for(ushort k=0; k<sz_k; ++k)
			{
				VP& edge_k = pk[k];
				ushort sz_ek = ushort(edge_k.size());

				cv::Point& pkf = edge_k[0];
				cv::Point& pkl = edge_k[sz_ek - 1];

				//CONSTRAINTS on position
				if	(	(pkf.y > pif.y) ||		//is below i
						(pkf.y > pjf.y)	||		//is below j
						(pkf.x > pjl.x)			//is right j
					)
				{
					//discard
					continue;
				}				

				uint key_ik = GenerateKey(PAIR_23, k, i);

				//find centers

				EllipseData data_ij, data_ik;

				if(data.count(key_ij) == 0)
				{
					//3,4 -> reverse 4
					

					GetFastCenter(edge_i, rev_j, data_ij);

					data.insert(std::pair<uint,EllipseData>(key_ij,data_ij));
				}
				else
				{
					data_ij = data.at(key_ij);
				}

				if(data.count(key_ik) == 0)
				{
					//3,2 -> reverse 3,2
					
					VP rev_k(edge_k.size());
					reverse_copy(edge_k.begin(), edge_k.end(), rev_k.begin());

					GetFastCenter(rev_i, rev_k, data_ik);

					data.insert(std::pair<uint,EllipseData>(key_ik,data_ik));
				}
				else
				{
					data_ik = data.at(key_ik);
				}

				
				//INVALID CENTERS
				if(!data_ij.isValid || !data_ik.isValid)
				{
					continue;
				}

				//CONSTRAINT ON CENTERS
				if(ed2(data_ij.center, data_ik.center) > fMaxCenterDistance2_)
				{
					//discard
					continue;
				}

				//FIND ELLIPSE PARAMETERS
				cv::Point2f center = GetCenterCoordinates(data_ij, data_ik);				
				FindEllipses(center, edge_i, edge_j, edge_k, data_ij, data_ik, ellipses);
			}			
		}
				
	}
};


void EllipseDetector::Triplets413	(	VVP& pi,
											VVP& pj,
											VVP& pk,
											std::map<uint, EllipseData>& data,
											std::vector<CEllipse>& ellipses
										)
{
	ushort sz_i = ushort(pi.size());
	ushort sz_j = ushort(pj.size());
	ushort sz_k = ushort(pk.size());

	//for each edge i
	for(ushort i=0; i<sz_i; ++i)
	{
		VP& edge_i = pi[i];
		ushort sz_ei = ushort(edge_i.size());

		cv::Point& pif = edge_i[0];
		cv::Point& pil = edge_i[sz_ei - 1];

		VP rev_i(edge_i.size());
		reverse_copy(edge_i.begin(), edge_i.end(), rev_i.begin());

		//for each edge j
		for(ushort j=0; j<sz_j; ++j)
		{
			VP& edge_j = pj[j];
			ushort sz_ej = ushort(edge_j.size());

			cv::Point& pjf = edge_j[0];
			cv::Point& pjl = edge_j[sz_ej - 1];


			//CONSTRAINTS on position
			if(pjl.y > pil.y)  		//is below
			{
				//discard
				continue;
			}

			uint key_ij = GenerateKey(PAIR_14, j, i);

			//for each edge k
			for(ushort k=0; k<sz_k; ++k)
			{
				VP& edge_k = pk[k];
				ushort sz_ek = ushort(edge_k.size());

				cv::Point& pkf = edge_k[0];
				cv::Point& pkl = edge_k[sz_ek - 1];


				//CONSTRAINTS on position
				if	(	(pkl.x > pif.x) ||		//is right i
						(pkl.x > pjl.x)	||		//is right j
						(pkl.y < pjf.y)		//is above j
					)
				{
					//discard
					continue;
				}

				uint key_ik = GenerateKey(PAIR_34, k, i);

				//find centers

				EllipseData data_ij, data_ik;

				if(data.count(key_ij) == 0)
				{
					//4,1 -> OK
					GetFastCenter(edge_i, edge_j, data_ij);
					data.insert(std::pair<uint,EllipseData>(key_ij,data_ij));
				}
				else
				{
					data_ij = data.at(key_ij);
				}

				if(data.count(key_ik) == 0)
				{
					//4,3 -> reverse 4
					GetFastCenter(rev_i, edge_k, data_ik);
					data.insert(std::pair<uint,EllipseData>(key_ik,data_ik));
				}
				else
				{
					data_ik = data.at(key_ik);
				}

				//INVALID CENTERS
				if(!data_ij.isValid || !data_ik.isValid)
				{					
					continue;
				}

				//CONSTRAIN ON CENTERS
				if(ed2(data_ij.center, data_ik.center) > fMaxCenterDistance2_)
				{
					//discard
					continue;
				}

				//FIND ELLIPSE PARAMETERS
				cv::Point2f center = GetCenterCoordinates(data_ij, data_ik);

				FindEllipses(center, edge_i, edge_j, edge_k, data_ij, data_ik, ellipses);

			}
		}
	}
};

//verify triplets of arcs with convexity: i=1, j=2, k=4
void EllipseDetector::Triplets124_simple(VVP& pi,
	VVP& pj,
	VVP& pk,
	std::map<uint, EllipseData>& data,
	CEllipse &ellipse, float thres,
	float thres_score
	)
{
	// get arcs length
	ushort sz_i = ushort(pi.size());
	ushort sz_j = ushort(pj.size());
	ushort sz_k = ushort(pk.size());

	//for each edge i
	for (ushort i = 0; i < sz_i; ++i)
	{
		VP& edge_i = pi[i];
		ushort sz_ei = ushort(edge_i.size());

		cv::Point& pif = edge_i[0];
		cv::Point& pil = edge_i[sz_ei - 1];

		//1,2 -> reverse 1, swap
		VP rev_i(edge_i.size());
		reverse_copy(edge_i.begin(), edge_i.end(), rev_i.begin());

		//for each edge j
		for (ushort j = 0; j<sz_j; ++j)
		{
			VP& edge_j = pj[j];
			ushort sz_ej = ushort(edge_j.size());

			cv::Point& pjf = edge_j[0];
			cv::Point& pjl = edge_j[sz_ej - 1];

			//CONSTRAINTS on position
			if (pjl.x > pif.x) //is right				
			{
				//discard
				continue;
			}

			uint key_ij = GenerateKey(PAIR_12, i, j);

			//for each edge k
			for (ushort k = 0; k < sz_k; ++k)
			{
				VP& edge_k = pk[k];
				ushort sz_ek = ushort(edge_k.size());

				cv::Point& pkf = edge_k[0];
				cv::Point& pkl = edge_k[sz_ek - 1];

				//CONSTRAINTS on position
				if ((pkl.y < pil.y) ||		//is above i
					(pkl.y < pjl.y) ||		//is above j
					(pkl.x < pjf.x)			//is left j
					)
				{
					//discard
					continue;
				}

				uint key_ik = GenerateKey(PAIR_14, i, k);

				//find centers

				EllipseData data_ij, data_ik;

				//if the data for the pair i-j have not been computed yet
				if (data.count(key_ij) == 0)
				{
					//1,2 -> reverse 1, swap

					//compute data!
					GetFastCenter(edge_j, rev_i, data_ij);
					//insert computed data in the hash table
					data.insert(std::pair<uint, EllipseData>(key_ij, data_ij));
				}
				else
				{
					//otherwise, just lookup the data in the hash table
					data_ij = data.at(key_ij);
				}

				//if the data for the pair i-k have not been computed yet
				if (data.count(key_ik) == 0)
				{
					//1,4 -> ok

					//compute data!
					GetFastCenter(edge_i, edge_k, data_ik);
					//insert computed data in the hash table
					data.insert(std::pair<uint, EllipseData>(key_ik, data_ik));
				}
				else
				{
					//otherwise, just lookup the data in the hash table
					data_ik = data.at(key_ik);
				}

				//INVALID CENTERS
				if (!data_ij.isValid || !data_ik.isValid)
				{
					continue;
				}

				//Selection strategy - Step 3. See Sect [] in the paper
				//the computer centers are not close enough
				if (ed2(data_ij.center, data_ik.center) > fMaxCenterDistance2_)
				{
					//discard
					continue;
				}

				// if all constraints of the selection strategy have been satisfied, 
				// we can start estimating the ellipse parameters

				//FIND ELLIPSE PARAMETERS

				// get the coordinates of the center (xc, yc)
				cv::Point2f center = GetCenterCoordinates(data_ij, data_ik);
				//find remaining parameters (A,B,rho)

				if (ed2(center, fInitCentre_) > fMaxDisFromInitCentre_)
				{
				//	discard
				//	SMART_DEBUG("fInitCentre_ inside discard");
					continue;
				}

				FindEllipses_simple(center, edge_i, edge_j, edge_k, data_ij, data_ik, ellipse, thres, thres_score);
			}
		}
	}
};



void EllipseDetector::Triplets231_simple(VVP& pi,
	VVP& pj,
	VVP& pk,
	std::map<uint, EllipseData>& data,
	CEllipse &ellipse, float thres,
	float thres_score
	)
{
	ushort sz_i = ushort(pi.size());
	ushort sz_j = ushort(pj.size());
	ushort sz_k = ushort(pk.size());

	//for each edge i
	for (ushort i = 0; i < sz_i; ++i)
	{
		VP& edge_i = pi[i];
		ushort sz_ei = ushort(edge_i.size());

		cv::Point& pif = edge_i[0];
		cv::Point& pil = edge_i[sz_ei - 1];

		VP rev_i(edge_i.size());
		std::reverse_copy(edge_i.begin(), edge_i.end(), rev_i.begin());

		//for each edge j
		for (ushort j = 0; j < sz_j; ++j)
		{
			VP& edge_j = pj[j];
			ushort sz_ej = ushort(edge_j.size());

			cv::Point& pjf = edge_j[0];
			cv::Point& pjl = edge_j[sz_ej - 1];

			//CONSTRAINTS on position
			if (pjf.y < pif.y) //is above
			{
				//discard
				continue;
			}

			VP rev_j(edge_j.size());
			std::reverse_copy(edge_j.begin(), edge_j.end(), rev_j.begin());

			uint key_ij = GenerateKey(PAIR_23, i, j);

			//for each edge k
			for (ushort k = 0; k < sz_k; ++k)
			{
				VP& edge_k = pk[k];
				ushort sz_ek = ushort(edge_k.size());

				cv::Point& pkf = edge_k[0];
				cv::Point& pkl = edge_k[sz_ek - 1];

				//CONSTRAINTS on position
				if ((pkf.x < pil.x) ||		//is left i
					(pkl.x < pjf.x) ||		//is left j
					(pkf.y > pjl.y)			//is below j
					)
				{
					//discard
					continue;
				}

				uint key_ik = GenerateKey(PAIR_12, k, i);

				//find centers

				EllipseData data_ij, data_ik;

				if (data.count(key_ij) == 0)
				{
					//2,3 -> reverse 2,3

					GetFastCenter(rev_i, rev_j, data_ij);
					data.insert(std::pair<uint, EllipseData>(key_ij, data_ij));
				}
				else
				{
					data_ij = data.at(key_ij);
				}

				if (data.count(key_ik) == 0)
				{
					//2,1 -> reverse 1
					VP rev_k(edge_k.size());
					reverse_copy(edge_k.begin(), edge_k.end(), rev_k.begin());

					GetFastCenter(edge_i, rev_k, data_ik);
					data.insert(std::pair<uint, EllipseData>(key_ik, data_ik));
				}
				else
				{
					data_ik = data.at(key_ik);
				}

				//INVALID CENTERS
				if (!data_ij.isValid || !data_ik.isValid)
				{
					continue;
				}

				//CONSTRAINT ON CENTERS
				if (ed2(data_ij.center, data_ik.center) > fMaxCenterDistance2_)
				{
					//discard
					continue;
				}

				//FIND ELLIPSE PARAMETERS
				cv::Point2f center = GetCenterCoordinates(data_ij, data_ik);

				if (ed2(center, fInitCentre_) > fMaxDisFromInitCentre_)
				{
				//	discard
				//	SMART_DEBUG("fInitCentre_ inside discard");
					continue;
				}


				FindEllipses_simple(center, edge_i, edge_j, edge_k, data_ij, data_ik, ellipse, thres, thres_score);

			}
		}
	}
};


void EllipseDetector::Triplets342_simple(VVP& pi,
	VVP& pj,
	VVP& pk,
	std::map<uint, EllipseData>& data,
	CEllipse &ellipse, float thres,
	float thres_score
	)
{
	ushort sz_i = ushort(pi.size());
	ushort sz_j = ushort(pj.size());
	ushort sz_k = ushort(pk.size());

	//for each edge i
	for (ushort i = 0; i < sz_i; ++i)
	{
		VP& edge_i = pi[i];
		ushort sz_ei = ushort(edge_i.size());

		cv::Point& pif = edge_i[0];
		cv::Point& pil = edge_i[sz_ei - 1];

		VP rev_i(edge_i.size());
		reverse_copy(edge_i.begin(), edge_i.end(), rev_i.begin());

		//for each edge j
		for (ushort j = 0; j < sz_j; ++j)
		{
			VP& edge_j = pj[j];
			ushort sz_ej = ushort(edge_j.size());

			cv::Point& pjf = edge_j[0];
			cv::Point& pjl = edge_j[sz_ej - 1];


			//CONSTRAINTS on position
			if (pjf.x < pil.x) 		//is left
			{
				//discard
				continue;
			}
			VP rev_j(edge_j.size());
			std::reverse_copy(edge_j.begin(), edge_j.end(), rev_j.begin());

			uint key_ij = GenerateKey(PAIR_34, i, j);

			//for each edge k
			for (ushort k = 0; k<sz_k; ++k)
			{
				VP& edge_k = pk[k];
				ushort sz_ek = ushort(edge_k.size());

				cv::Point& pkf = edge_k[0];
				cv::Point& pkl = edge_k[sz_ek - 1];

				//CONSTRAINTS on position
				if ((pkf.y > pif.y) ||		//is below i
					(pkf.y > pjf.y) ||		//is below j
					(pkf.x > pjl.x)			//is right j
					)
				{
					//discard
					continue;
				}

				uint key_ik = GenerateKey(PAIR_23, k, i);

				//find centers

				EllipseData data_ij, data_ik;

				if (data.count(key_ij) == 0)
				{
					//3,4 -> reverse 4


					GetFastCenter(edge_i, rev_j, data_ij);

					data.insert(std::pair<uint, EllipseData>(key_ij, data_ij));
				}
				else
				{
					data_ij = data.at(key_ij);
				}

				if (data.count(key_ik) == 0)
				{
					//3,2 -> reverse 3,2

					VP rev_k(edge_k.size());
					reverse_copy(edge_k.begin(), edge_k.end(), rev_k.begin());

					GetFastCenter(rev_i, rev_k, data_ik);

					data.insert(std::pair<uint, EllipseData>(key_ik, data_ik));
				}
				else
				{
					data_ik = data.at(key_ik);
				}


				//INVALID CENTERS
				if (!data_ij.isValid || !data_ik.isValid)
				{
					continue;
				}

				//CONSTRAINT ON CENTERS
				if (ed2(data_ij.center, data_ik.center) > fMaxCenterDistance2_)
				{
					//discard
					continue;
				}

				//FIND ELLIPSE PARAMETERS
				cv::Point2f center = GetCenterCoordinates(data_ij, data_ik);

				if (ed2(center, fInitCentre_) > fMaxDisFromInitCentre_)
				{
				//	discard
				//	SMART_DEBUG("fInitCentre_ inside discard");
					continue;
				}

				FindEllipses_simple(center, edge_i, edge_j, edge_k, data_ij, data_ik, ellipse, thres, thres_score);
			}
		}

	}
};


void EllipseDetector::Triplets413_simple(VVP& pi,
	VVP& pj,
	VVP& pk,
	std::map<uint, EllipseData>& data,
	CEllipse &ellipse, float thres,
	float thres_score
	)
{
	ushort sz_i = ushort(pi.size());
	ushort sz_j = ushort(pj.size());
	ushort sz_k = ushort(pk.size());

	//for each edge i
	for (ushort i = 0; i < sz_i; ++i)
	{
		VP& edge_i = pi[i];
		ushort sz_ei = ushort(edge_i.size());

		cv::Point& pif = edge_i[0];
		cv::Point& pil = edge_i[sz_ei - 1];

		VP rev_i(edge_i.size());
		reverse_copy(edge_i.begin(), edge_i.end(), rev_i.begin());

		//for each edge j
		for (ushort j = 0; j<sz_j; ++j)
		{
			VP& edge_j = pj[j];
			ushort sz_ej = ushort(edge_j.size());

			cv::Point& pjf = edge_j[0];
			cv::Point& pjl = edge_j[sz_ej - 1];


			//CONSTRAINTS on position
			if (pjl.y > pil.y)  		//is below
			{
				//discard
				continue;
			}

			uint key_ij = GenerateKey(PAIR_14, j, i);

			//for each edge k
			for (ushort k = 0; k<sz_k; ++k)
			{
				VP& edge_k = pk[k];
				ushort sz_ek = ushort(edge_k.size());

				cv::Point& pkf = edge_k[0];
				cv::Point& pkl = edge_k[sz_ek - 1];


				//CONSTRAINTS on position
				if ((pkl.x > pif.x) ||		//is right i
					(pkl.x > pjl.x) ||		//is right j
					(pkl.y < pjf.y)		//is above j
					)
				{
					//discard
					continue;
				}

				uint key_ik = GenerateKey(PAIR_34, k, i);

				//find centers

				EllipseData data_ij, data_ik;

				if (data.count(key_ij) == 0)
				{
					//4,1 -> OK
					GetFastCenter(edge_i, edge_j, data_ij);
					data.insert(std::pair<uint, EllipseData>(key_ij, data_ij));
				}
				else
				{
					data_ij = data.at(key_ij);
				}

				if (data.count(key_ik) == 0)
				{
					//4,3 -> reverse 4
					GetFastCenter(rev_i, edge_k, data_ik);
					data.insert(std::pair<uint, EllipseData>(key_ik, data_ik));
				}
				else
				{
					data_ik = data.at(key_ik);
				}

				//INVALID CENTERS
				if (!data_ij.isValid || !data_ik.isValid)
				{
					continue;
				}

				//CONSTRAIN ON CENTERS
				if (ed2(data_ij.center, data_ik.center) > fMaxCenterDistance2_)
				{
					//discard
					continue;
				}

				//FIND ELLIPSE PARAMETERS
				cv::Point2f center = GetCenterCoordinates(data_ij, data_ik);

				if (ed2(center, fInitCentre_) > fMaxDisFromInitCentre_)
				{
				//	discard
				//	SMART_DEBUG("fInitCentre_ inside discard");
					continue;
				}

				FindEllipses_simple(center, edge_i, edge_j, edge_k, data_ij, data_ik, ellipse, thres, thres_score);

			}
		}
	}
};

void EllipseDetector::RemoveShortEdges( cv::Mat1b& edges, cv::Mat1b& clean )
{
	VVP contours;

	//labeling and constraints on length
	Labeling(edges, contours, iMinEdgeLength_);

	int iContoursSize = contours.size();
	for(int i=0; i<iContoursSize; ++i)
	{
		VP& edge = contours[i];
		unsigned szEdge = edge.size();

		//constraint on axes aspect ratio
		cv::RotatedRect oriented = minAreaRect(edge);
		if(	oriented.size.width < fMinOrientedRectSide_ ||
			oriented.size.height < fMinOrientedRectSide_ ||
			oriented.size.width > oriented.size.height * fMaxRectAxesRatio_ ||
			oriented.size.height > oriented.size.width * fMaxRectAxesRatio_ )
		{
			continue;
		}

		for(unsigned j=0; j<szEdge; ++j)
		{
			clean(edge[j]) = (uchar)255;
		}
	}
}



void EllipseDetector::PrePeocessing(const cv::Mat1b& I,
									 cv::Mat1b& DP,
									 cv::Mat1b& DN )
{
	// 1. smooth image
	//cv::GaussianBlur(I, I, szPreProcessingGaussKernelSize_, dPreProcessingGaussSigma_ );
	cv::blur( I, I, cv::Size( 3, 3 ) );

	// temp variables
	cv::Mat1b E;				//	edge mask
	cv::Mat1s DX, DY;			//	sobel derivatives

	// detect edges
	//Canny2(I, E, DX, DY, 0.5, 0.55, 3, false);
	
	// 2. detect edges, use automatic thresholds. Slower but more general
	Canny3( I, E, DX, DY, 3, false );

	// for each edge points, compute the edge direction
	for( int i = 0; i < szImg_.height; ++i )
	{		
		short* dx = DX.ptr<short>(i);
		short* dy = DY.ptr<short>(i);
		uchar* e = E.ptr<uchar>(i);
		uchar* dp = DP.ptr<uchar>(i);
		uchar* dn = DN.ptr<uchar>(i);

		for( int j = 0; j < szImg_.width; ++j )
		{
			if (e[j]>0 && dx[j]!=0 && dy[j]!=0)
				//if (!((e[j] <= 0) || (dx[j] == 0) || (dy[j] == 0)))
			{
				// angle of the tangent
				float phi = -( float( dx[ j ] ) / float( dy[ j ] ) );
				
				// along positive or negative diagonal
				if ( phi > 0 )	
					dp[ j ] = (uchar)255;
				//else if	( phi < 0 )
				else
					dn[ j ] = (uchar)255;
			}
		}
	}
};


void EllipseDetector::DetectAfterPreProcessing(std::vector<CEllipse>& ellipses, cv::Mat1b& E, cv::Mat1f& PHI)
{
//	cv::Mat1f PHI = cv::Mat1f();
	// set the image size
	szImg_ = E.size();

	// initialize temporary data structures
	cv::Mat1b DP	= cv::Mat1b::zeros(szImg_);		// arcs along positive diagonal
	cv::Mat1b DN	= cv::Mat1b::zeros(szImg_);		// arcs along negative diagonal
	
	for(int i=0; i<szImg_.height; ++i)
	{
		float* _phi		= PHI.ptr<float>(i);
		uchar* _e		= E.ptr<uchar>(i);
		uchar* _dp		= DP.ptr<uchar>(i);
		uchar* _dn		= DN.ptr<uchar>(i);

		for(int j=0; j<szImg_.width; ++j)
		{
			if((_e[j] > 0) && (_phi[j] != 0))
			{
				// angle
				float phi = std::atan(-1.f / _phi[j]);

				// along positive or negative diagonal
				if		(phi > 0)	_dp[j] = (uchar)255;
				else if	(phi < 0)	_dn[j] = (uchar)255;
			}
		}
	}	

	//initialize accumulator dimensions
	ACC_N_SIZE = 101;
	ACC_K_SIZE = 180;
	ACC_A_SIZE = std::max(szImg_.height, szImg_.width);

	//allocate accumulators
	accN = new int[ACC_N_SIZE];
	accK = new int[ACC_K_SIZE];
	accA = new int[ACC_A_SIZE];

	//other temporary 
	VVP points_1, points_2, points_3, points_4;		//vector of points, one for each convexity class
	std::map<uint, EllipseData> centers;		//hash map for reusing already computed EllipseData

	//detect edges and find convexities
	DetectEdges13(DP, points_1, points_3);
	DetectEdges24(DN, points_2, points_4);
		
	//find triplets
	Triplets124(points_1, points_2, points_4, centers, ellipses);
	Triplets231(points_2, points_3, points_1, centers, ellipses);
	Triplets342(points_3, points_4, points_2, centers, ellipses);
	Triplets413(points_4, points_1, points_3, centers, ellipses);

	//sort detected ellipses with respect to score
	sort(ellipses.begin(), ellipses.end());

	//free accumulator memory
	delete [] accN;
	delete [] accK;
	delete [] accA;

	//cluster detections
	ClusterEllipses(ellipses);
};

void EllipseDetector::DetectMainEllipse(const cv::Mat1b& I, CEllipse &ellipse, float thresh_area, float thres_score)
{
	// set the image size
	szImg_ = I.size();

	// initialize temporary data structures
	cv::Mat1b DP = cv::Mat1b::zeros(szImg_);		// arcs along positive diagonal
	cv::Mat1b DN = cv::Mat1b::zeros(szImg_);		// arcs along negative diagonal

	// initialize accumulator dimensions
	ACC_N_SIZE = 101;
	ACC_K_SIZE = 180;
	ACC_A_SIZE = std::max(szImg_.height, szImg_.width);

	// allocate accumulators
	accN = new int[ACC_N_SIZE];
	accK = new int[ACC_K_SIZE];
	accA = new int[ACC_A_SIZE];

	// other temporary 
	VVP points_1, points_2, points_3, points_4;		// vector of points, one for each convexity class
	std::map<uint, EllipseData> centers;	// hash map for reusing already computed EllipseData

	// Preprocessing
	// From input image I, find edge point with coarse convexity along positive (DP) or negative (DN) diagonal
	PrePeocessing(I, DP, DN);

	// detect edges and find convexities
	DetectEdges13(DP, points_1, points_3);
	DetectEdges24(DN, points_2, points_4);

	// find triplets
	Triplets124_simple(points_1, points_2, points_4, centers, ellipse, thresh_area, thres_score);
	if (ellipse.score_ > 0)		goto label_finished;
	Triplets231_simple(points_2, points_3, points_1, centers, ellipse, thresh_area, thres_score);
	if (ellipse.score_ > 0)		goto label_finished;
	Triplets342_simple(points_3, points_4, points_2, centers, ellipse, thresh_area, thres_score);
	if (ellipse.score_ > 0)		goto label_finished;
	Triplets413_simple(points_4, points_1, points_3, centers, ellipse, thresh_area, thres_score);

label_finished:
	//free accumulator memory
	delete[] accN;
	delete[] accK;
	delete[] accA;
};

void EllipseDetector::Detect( cv::Mat1b& I, std::vector<CEllipse>& ellipses )
{
	// set the image size
	szImg_ = I.size();

	// initialize temporary data structures
	cv::Mat1b DP = cv::Mat1b::zeros( szImg_ );		// arcs along positive diagonal
	cv::Mat1b DN = cv::Mat1b::zeros( szImg_ );		// arcs along negative diagonal

	// initialize accumulator dimensions
	ACC_N_SIZE = 101;
	ACC_K_SIZE = 180;
	ACC_A_SIZE = std::max( szImg_.height, szImg_.width );

	// allocate accumulators
	accN = new int[ ACC_N_SIZE ];
	accK = new int[ ACC_K_SIZE ];
	accA = new int[ ACC_A_SIZE ];
	
	// other temporary 
	VVP points_1, points_2, points_3, points_4;		// vector of points, one for each convexity class
	std::map<uint, EllipseData> centers;	// hash map for reusing already computed EllipseData

	// Preprocessing
	// From input image I, find edge point with coarse convexity along positive (DP) or negative (DN) diagonal
	PrePeocessing( I, DP, DN );

	// detect edges and find convexities
	DetectEdges13(DP, points_1, points_3);
	DetectEdges24(DN, points_2, points_4);
			
	// find triplets
	Triplets124(points_1, points_2, points_4, centers, ellipses);
	Triplets231(points_2, points_3, points_1, centers, ellipses);
	Triplets342(points_3, points_4, points_2, centers, ellipses);
	Triplets413(points_4, points_1, points_3, centers, ellipses);

	// sort detected ellipses with respect to score
	sort(ellipses.begin(), ellipses.end());

	//free accumulator memory
	delete [] accN;
	delete [] accK;
	delete [] accA;

	//cluster detections
	ClusterEllipses( ellipses );
};

// Ellipse clustering procedure. See Sect [] in the paper.
void EllipseDetector::ClusterEllipses( std::vector<CEllipse>& ellipses )
{	
	float _fMaxCenterDistance	= 16.f;
	float _fMaxEccDistance		= 0.2f;
	float _fMaxADistance		= 0.1f;
	float _fMaxAngleDistance	= 0.1f;
	float _fThresholdCircle		= 0.8f;

	int iNumOfEllipses = int( ellipses.size() );
	if( iNumOfEllipses == 0 ) return;

	//the first ellipse is assigned to a cluster
	std::vector<CEllipse> clusters;
	clusters.push_back(ellipses[0]);

	bool bFoundCluster = false;

	for(int i=1; i<iNumOfEllipses; ++i)
	{
		CEllipse& e1 = ellipses[i];

		int sz_clusters = clusters.size();

		float Decc1 = e1.B_ / e1.A_;

		bool bFoundCluster = false;
		for(int j=0; j<sz_clusters; ++j)
		{
			CEllipse& e2 = clusters[j];

			//centers
			float Dx = abs(e1.xc_ - e2.xc_);
			float Dy = abs(e1.yc_ - e2.yc_);

			if(Dx > _fMaxCenterDistance || Dy > _fMaxCenterDistance)
			{
				//not same cluster
				continue;
			}

			//axes
			float Da = 1.f - (std::min(e1.A_, e2.A_) / std::max(e1.A_, e2.A_));
			if(Da > _fMaxADistance)
			{
				//not same cluster
				continue;
			}

			//eccentricity
			float Decc2 = e2.B_ / e2.A_;

			float De = 1.f - (std::min(Decc1,Decc2) / std::max(Decc1,Decc2));
			if(De > _fMaxEccDistance) 
			{
				//not same cluster
				continue;
			}			

			//angle
			float Drho = fmod(abs(e1.rho_ - e2.rho_), float(CV_PI)) / float(CV_PI);
			if(Drho > _fMaxAngleDistance)
			{
				if(Decc1 < _fThresholdCircle && Decc2 < _fThresholdCircle)
				{
					//not same cluster
					continue;
				}
			}
						

			//same cluster as e2
			bFoundCluster = true;
			//discard, no need to create a new cluster
			break;
		}

		if(!bFoundCluster)
		{
			//create a new cluster			
			clusters.push_back(e1);
		}
	}

	clusters.swap(ellipses);
};

void EllipseDetector::DrawDetectedEllipses( cv::Mat3b& output, std::vector<CEllipse>& ellipses, int iTopN, int thickness )
{	
	int sz_ell = int(ellipses.size());
	int n = (iTopN==0) ? sz_ell : std::min(iTopN, sz_ell);

	float score_max = 0;
	int id = -1;
	for(int i=0; i < n; ++i)
	{
		CEllipse& e = ellipses[sz_ell - i - 1];
		//std::cout<<e.score_<< std::endl;
		if (i==0)
		{
			score_max = e.score_;
			id = i;
		}
		else
		{
			if (score_max < e.score_)
			{
				score_max = e.score_;
				id = i;
			}
		}
	}


	if (id > -1)
	{
		CEllipse& e = ellipses[sz_ell - id - 1];
		int g = cvRound( e.score_ * 255.f);		
		cv::Scalar color(0, g, 0);
		ellipse(output, cv::Point(e.xc_, e.yc_), cv::Size(e.A_, e.B_), e.rho_*180.f/CV_PI, 0.0, 360.0, color, thickness);	
	}

}

bool EllipseDetector::GetMainEllipses(int image_w, const std::vector<CEllipse>& ellipses, float *ellipse_para, float thresh)
{	
	size_t sz_ell = ellipses.size();

	float score_max = 0;
	int id = -1;
	float stub = thresh * image_w / 2.0;
	for (size_t i = 0; i<sz_ell; ++i){
		if (ellipses[i].A_ > stub){
			if (score_max < ellipses[i].score_){
				score_max = ellipses[i].score_;
				id = i;
			}
		}
	}

	if (id > -1){
		ellipse_para[0] = ellipses[id].xc_;
		ellipse_para[1] = ellipses[id].yc_;
		ellipse_para[2] = ellipses[id].A_;
		ellipse_para[3] = ellipses[id].B_;
		ellipse_para[4] = ellipses[id].rho_;

		return true;
	}

	return false;
}
