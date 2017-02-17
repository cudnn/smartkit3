#pragma once

#include <opencv2\core\core.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <stdio.h>
#include <algorithm>
#include <numeric>
#include <map>
#include <vector>

#include "Ellipse.h"
#include "common.h"
#include <time.h>

#define ELLIPSE_ZERO 0.000001

struct EllipseData
{
	float t1;
	float m1;
	float t2;
	float m2;
	cv::Point2f M1;
	cv::Point2f M2;
	cv::Point2f center;
	bool isValid;
	std::vector<float> slopes1;
	std::vector<float> slopes2;
};

class EllipseDetector
{
public:
	//Constructor and Destructor
	EllipseDetector(void);
	~EllipseDetector(void);

	void DetectAfterPreProcessing( std::vector<CEllipse>& ellipses, cv::Mat1b& E, cv::Mat1f& PHI);

	//Detect the ellipses in the gray image
	void Detect(cv::Mat1b& I, std::vector<CEllipse>& ellipses);

	//Detect only the main ellipse, simple version
	void DetectMainEllipse(const cv::Mat1b &I, CEllipse &ellipse, float thres_area, float thres_score = 0.8);
	
	//Draw the first iTopN ellipses on output
	void DrawDetectedEllipses( cv::Mat3b& output, std::vector<CEllipse>& ellipses, int iTopN=0, int thickness=2 );
	bool GetMainEllipses( int image_w, const std::vector<CEllipse>& ellipses, float *ellipse_para, float thresh);

	//Set the parameters of the detector
	void SetParameters(	cv::Size	szPreProcessingGaussKernelSize,
						double	dPreProcessingGaussSigma,
						double 	fThPosition,
						float	fMaxCenterDistance,
						int		iMinEdgeLength,
						float	fMinOrientedRectSide,
						float	fDistanceToEllipseContour,
						float	fMinScore,
						float	fMinReliability,
						int    iNs,
						cv::Point2f fInitCentre,
						float fMaxDisFromInitCentre,
						float fInitB,
						float fTolRatioB
						);
	
private:

	//keys for hash table
	static const ushort PAIR_12 = 0x00;
	static const ushort PAIR_23 = 0x01;
	static const ushort PAIR_34 = 0x02;
	static const ushort PAIR_14 = 0x03;

	//generate keys from pair and indices
	uint inline GenerateKey(uchar pair, ushort u, ushort v);
	void PrePeocessing(const cv::Mat1b& I, cv::Mat1b& DP, cv::Mat1b& DN );
	void RemoveShortEdges(cv::Mat1b& edges, cv::Mat1b& clean);

	void ClusterEllipses(std::vector<CEllipse>& ellipses);

	int FindMaxK(const std::vector<int>& v) const;
	int FindMaxN(const std::vector<int>& v) const;
	int FindMaxA(const std::vector<int>& v) const;

	int FindMaxK(const int* v) const;
	int FindMaxN(const int* v) const;
	int FindMaxA(const int* v) const;

	float GetMedianSlope( std::vector<cv::Point2f>& med, cv::Point2f& M, std::vector<float>& slopes );
	void GetFastCenter	( std::vector<cv::Point>& e1, std::vector<cv::Point>& e2, EllipseData& data );
	

	void DetectEdges13( cv::Mat1b& DP, VVP& points_1, VVP& points_3 );
	void DetectEdges24( cv::Mat1b& DN, VVP& points_2, VVP& points_4 );

	void FindEllipses( cv::Point2f& center,
					   VP& edge_i,
					   VP& edge_j,
					   VP& edge_k,
					   EllipseData& data_ij,
					   EllipseData& data_ik,
					   std::vector<CEllipse>& ellipses );
	void FindEllipses_simple(cv::Point2f& center,
		VP& edge_i,
		VP& edge_j,
		VP& edge_k,
		EllipseData& data_ij,
		EllipseData& data_ik,
		CEllipse &ellipse, 
		float thres,
		float thres_score);

	cv::Point2f GetCenterCoordinates( EllipseData& data_ij, EllipseData& data_ik );
	cv::Point2f _GetCenterCoordinates( EllipseData& data_ij, EllipseData& data_ik );

	void Triplets124( VVP& pi,
					  VVP& pj,
					  VVP& pk,
					  std::map<uint, EllipseData>& data,
					  std::vector<CEllipse>& ellipses );

	void Triplets231( VVP& pi,
					  VVP& pj,
					  VVP& pk,
					  std::map<uint, EllipseData>& data,
					  std::vector<CEllipse>& ellipses );

	void Triplets342( VVP& pi,
					  VVP& pj,
					  VVP& pk,
					  std::map<uint, EllipseData>& data,
					  std::vector<CEllipse>& ellipses );

	void Triplets413( VVP& pi,
					  VVP& pj,
					  VVP& pk,
					  std::map<uint, EllipseData>& data,
					  std::vector<CEllipse>& ellipses );

	void Triplets124_simple(VVP& pi,
		VVP& pj,
		VVP& pk,
		std::map<uint, EllipseData>& data,
		CEllipse &ellipse, float thres,
		float thres_score = 0.0);

	void Triplets231_simple(VVP& pi,
		VVP& pj,
		VVP& pk,
		std::map<uint, EllipseData>& data,
		CEllipse &ellipse, float thres,
		float thres_score = 0.0);

	void Triplets342_simple(VVP& pi,
		VVP& pj,
		VVP& pk,
		std::map<uint, EllipseData>& data,
		CEllipse &ellipse, float thres,
		float thres_score = 0.0);

	void Triplets413_simple(VVP& pi,
		VVP& pj,
		VVP& pk,
		std::map<uint, EllipseData>& data,
		CEllipse &ellipse, float thres,
		float thres_score = 0.0);

private:
	// Parameters

	// Preprocessing - Gaussian filter. See Sect [] in the paper
	cv::Size	szPreProcessingGaussKernelSize_;	// size of the Gaussian filter in preprocessing step
	double	dPreProcessingGaussSigma_;				// sigma of the Gaussian filter in the preprocessing step

	// Selection strategy - Step 1 - Discard noisy or straight arcs. See Sect [] in the paper
	int		iMinEdgeLength_;					// minimum edge size				
	float	fMinOrientedRectSide_;				// minimum size of the oriented bounding box containing the arc
	float	fMaxRectAxesRatio_;					// maximum aspect ratio of the oriented bounding box containing the arc

	// Selection strategy - Step 2 - Remove according to mutual convexities. See Sect [] in the paper
	float fThPosition_;

	// Selection Strategy - Step 3 - Number of points considered for slope estimation when estimating the center. See Sect [] in the paper
	int iNs_;									// Find at most Ns parallel chords.

	// Selection strategy - Step 3 - Discard pairs of arcs if their estimated center is not close enough. See Sect [] in the paper
	float	fMaxCenterDistance_;				// maximum distance in pixel between 2 center points
	float	fMaxCenterDistance2_;				// _fMaxCenterDistance * _fMaxCenterDistance

	// Validation - Points within a this threshold are considered to lie on the ellipse contour. See Sect [] in the paper
	float	fDistanceToEllipseContour_;			// maximum distance between a point and the contour. See equation [] in the paper

	// Validation - Assign a score. See Sect [] in the paper
	float	fMinScore_;							// minimum score to confirm a detection
	float	fMinReliability_;					// minimum auxiliary score to confirm a detection

	cv::Point2f fInitCentre_ ;
	float fMaxDisFromInitCentre_;

	float fInitB_;     // add via tianxiang 163018
	float fTolRatioB_; // add via tianxiang 163018

	// auxiliary variables
	cv::Size	szImg_;			// input image size

	int ACC_N_SIZE;				// size of accumulator N = B/A
	int ACC_K_SIZE;				// size of accumulator K = tan(rho)
	int ACC_A_SIZE;				// size of accumulator A

	int* accN;					// pointer to accumulator N
	int* accK;					// pointer to accumulator K
	int* accA;					// pointer to accumulator A
};

