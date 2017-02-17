/*
 * util.h
 *
 *  Created on: 2015年7月27日
 *      Author: guoyj7
 *
 * 	Add comments on variable and functions
 *
 *  Revised on: 2016/04/8
 *      Author: fengbin1
 */
#define _ANDROID_
#define DEBUG

#include "util.h"
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>

#include "log.h"

#ifndef MIN
#  define MIN(a,b)  ((a) > (b) ? (b) : (a))
#endif

#ifndef MAX
#  define MAX(a,b)  ((a) < (b) ? (b) : (a))
#endif

#define BLOCK_X_NUM 7
#define BLOCK_Y_NUM 7
#define HIST_LONG_BINNUM 256
#define HIST_SHORT_BINNUM 16
#define FEATURE_NUM 24

namespace util {

/**
 * calculate feature for light detection
 * @param blockY        (input data) block data
 * @param histYL        (input data) histogram of Y data(256 bin)
 * @param histU         (input data) histogram of U data(256 bin)
 * @param histV         (input data) histogram of V data(256 bin)
 * @param histYS        (input data) histogram of Y data(16 bin)
 * @param featureVector (output data)feature vector
 * @return
 */
void feature_extraction(const float blockY[], const float histYL[], const float histU[], const float histV[], const float histYS[], float featureVector[]) {
	int *darkTbl = (int *)malloc(BLOCK_Y_NUM * BLOCK_X_NUM * sizeof(int));
	float regionMeanLl[5] = {0, 0, 0, 0, 0};
	float regionMeanBl[6] = {0, 0, 0, 0, 0, 0};

	float avgY = setAvg(histYL, HIST_LONG_BINNUM);
	float stdY = setStd(histYL, HIST_LONG_BINNUM) / 0.5;
	float stdU = setStd(histU, HIST_LONG_BINNUM) / 0.5;
	float stdV = setStd(histV, HIST_LONG_BINNUM) / 0.5;
	float histPeakPos = (float) set_peak_from_histo(histYS, HIST_SHORT_BINNUM) / HIST_SHORT_BINNUM;
	float histMedian = (float) set_hist_median(histYS, HIST_SHORT_BINNUM) / HIST_SHORT_BINNUM;
	float underExpoRatio = set_hist_low(histYS, HIST_SHORT_BINNUM);
	float leftHistRatio = set_hist_half(histYS, HIST_SHORT_BINNUM);
	float biThld = (float) set_hist_thld(histYS,HIST_SHORT_BINNUM) / HIST_SHORT_BINNUM;
	float LDRatio = (float) set_avg_ratio_from_block_y(blockY, darkTbl, avgY) / 20;
	float darkTopRatio = set_top_sum_from_block(blockY, darkTbl);
	float darkBottomRatio = set_down_sum_from_block(blockY, darkTbl);
	set_region_meanLl_from_block(blockY, regionMeanLl);
	set_region_meanBl_from_block(blockY, regionMeanBl);
	float skewY = set_skewness(histYL, HIST_LONG_BINNUM);

	featureVector[0] = avgY;
	featureVector[1] = stdY;
	featureVector[2] = LDRatio;
	featureVector[3] = darkTopRatio;
	featureVector[4] = darkBottomRatio;
	featureVector[5] = histPeakPos;
	featureVector[6] = histMedian;
	featureVector[7] = underExpoRatio;
	featureVector[8] = leftHistRatio;
	featureVector[9] = biThld; // 10

	// featureVector[10] = stdU; // 11
	// featureVector[11] = stdV; // 12
	/** 20150827 ** this solves the bug that u and v are upside-down */
	featureVector[10] = stdV; // 11
	featureVector[11] = stdU; // 12

	featureVector[12] = regionMeanLl[0]; // 13
	featureVector[13] = regionMeanLl[1]; // 14
	featureVector[14] = regionMeanLl[2]; // 15
	featureVector[15] = regionMeanLl[3]; // 16
	featureVector[16] = regionMeanLl[4]; // 17
	featureVector[17] = regionMeanBl[0]; // 13
	featureVector[18] = regionMeanBl[1]; // 14
	featureVector[19] = regionMeanBl[2]; // 15
	featureVector[20] = regionMeanBl[3]; // 16
	featureVector[21] = regionMeanBl[4]; // 17
	featureVector[22] = regionMeanBl[5]; // 18
	featureVector[23] = skewY; // ll - 18, bl - 19

	free(darkTbl);
//	free(regionMeanLl);
//	free(regionMeanBl);
}
/**
 * calculate final degree for light detection
 * @param featureVector    (input data) feature vector
 * @param alpha            (input data) alpha mask
 * @param indx             (input data) index mask
 * @param dir              (input data) dir mask
 * @param thld             (input data) threshold mask
 * @param numOfScenes      (input data) number of scene
 * @param vectorLength     (input data) number of weight for light feature
 * @param degree           (output data)final degree score
 * @return
 */
void degree_computation(float featureVector[], const double alpha[], const int indx[], const int dir[], const  double thld[], const int numOfScenes, const int vectorLength, float degree[]) {
	float sumAlpha;
	float ret;
	for (int k = 0; k < numOfScenes; k++){
		sumAlpha = 0;
		degree[k] = 0;
		for (int i = 0; i < vectorLength; i++) {
			if (dir[k * vectorLength + i] == 1) {
			    ret = featureVector[indx[k * vectorLength + i]] >= thld[k * vectorLength + i] ? 1 : -1;
			} else {
			    ret = featureVector[indx[k * vectorLength + i]] < thld[k * vectorLength + i] ? 1 : -1;
			}
			sumAlpha += alpha[k * vectorLength + i];
			degree[k] += alpha[k * vectorLength + i] * ret;
		}
		degree[k] = (degree[k] / sumAlpha + 1) / 2;
	}
}
/**
 * calculate average value of data
 * @param hist             histogram of data
 * @param histBinNum       number of histogram bin
 * @return                 average value
 */
float setAvg(const float hist[], int histBinNum) {
	float avg = 0;
	for (int i = 0; i < histBinNum; i++){
		avg += i * hist[i];
	}
	avg /= 255;
	return avg;
}
/**
 * calculate standard deviation value of data
 * @param hist             histogram of data
 * @param histBinNum       number of histogram bin
 * @return                 standard deviation value
 */
float setStd(const float hist[], int histBinNum) {
	float avg = setAvg(hist, histBinNum);
	float var = 0;
	for (int i = 0; i < histBinNum; i++){
		var += ((float) i / 255 - avg) * ((float) i / 255 - avg) * hist[i];
	}
	float std = sqrt(var);
	return std;
}
/**
 * calculate the index bin of the max value in histogram
 * @param hist             histogram of data
 * @param histBinNum       number of histogram bin
 * @return                 index bin
 */
int set_peak_from_histo(const float hist[], int histBinNum) {
	float maxV = -1;
	int bin = 0;
	for (int i = 0; i < histBinNum; i++){
		if (hist[i] > maxV) {
			maxV = hist[i];
			bin = i + 1;
		}
	}
	return bin;
}
/**
 * calculate the average ratio value of input data
 * @param blockY           (input data) block data
 * @param darkTbl          (output data)binarization mask of block data
 * @param thldY            threshold for binarization
 * @return                 average ratio value
 */
float set_avg_ratio_from_block_y(const float blockY[], int darkTbl[], float thldY) {
	float denominator = 0, numerator = 0;
	int dNum = 0, nNum = 0;
	float avgRatio;
	for (int j = 0; j < BLOCK_Y_NUM; j++){
		for (int i = 0; i < BLOCK_X_NUM; i++) {
			if (blockY[j * BLOCK_X_NUM + i] >= thldY){
				numerator += blockY[j * BLOCK_X_NUM + i];
				darkTbl[j * BLOCK_X_NUM + i] = 0;
				nNum ++;
			} else {
				denominator += blockY[j * BLOCK_X_NUM + i];
				darkTbl[j * BLOCK_X_NUM + i] = 1;
				dNum ++;
			}
		}
	}
//	denominator += 0.0001;
	if (nNum != 0 && dNum != 0 && denominator != 0) {
		avgRatio = (numerator / nNum) / (denominator / dNum);
	} else {
		avgRatio = 0;
	}

	return avgRatio;
}
/**
 * calculate the index bin of the median value in histogram
 * @param hist             histogram of data
 * @param histBinNum       number of histogram bin
 * @return                 index bin
 */
int set_hist_median(const float hist[], int histBinNum) {
	float ratio = 0, ratioNext = 0;
	int bin = 0;
	for (bin = 0; bin < histBinNum - 1; bin++){
		ratio += hist[bin];
		ratioNext = ratio + hist[bin + 1];
		if (ratio <= 0.5 && ratioNext > 0.5) {
			break;
		}
	}
	bin += 1;
	if ((ratio - 0.5) / (ratioNext - 0.5) <= 1) {
		return bin;
	} else {
		return (bin + 1);
	}
}
/**
 * calculate the skewness value of the input data
 * @param hist             histogram of data
 * @param histBinNum       number of histogram bin
 * @return                 skewness value
 */
float set_skewness(const float hist[], int histBinNum) {
	float ratio;
	float numerator = 0, denominator, avg = setAvg(hist, histBinNum);
	for (int i = 0; i < histBinNum; i++){
		numerator += ((float) i / 255 - avg) * ((float) i / 255 - avg) * ((float) i / 255 - avg) * hist[i];
	}
	denominator = pow(setStd(hist, histBinNum), 3);

	ratio = numerator / denominator;
	ratio = 1 / (1 + exp(-1 * ratio));
		
	return ratio;
}
/**
 * calculate the index bin of the max value in accumulative histogram
 * @param hist             histogram of data
 * @param histBinNum       number of histogram bin
 * @return                 index bin
 */
int set_hist_thld(const float hist[], int histBinNum) {
	float mLeft, mRight, mTotal = 1 / (float) histBinNum;
	float vLeft, vRight, vTotal;
	float cov, maxV = -1;
	int ret = 0;
	for (int bin = 1; bin <= histBinNum - 1; bin++){
		mRight = 0;
		mLeft = 0;
		for (int i = 0; i < bin; i++) {
			mLeft += hist[i];
		}
		mRight = 1 - mLeft;
		mLeft /= (float) bin; 
		mRight /= (float) (histBinNum - bin);
			
		vLeft = 0; vRight = 0; vTotal = 0;
		for (int i = 0; i < bin; i++) {
			vLeft += (hist[i] - mLeft) * (hist[i] - mLeft);
			vTotal += (hist[i] - mTotal) * (hist[i] - mTotal);
		}
		for (int i = bin; i < histBinNum; i++) {
			vRight += (hist[i] - mRight) * (hist[i] - mRight);
			vTotal += (hist[i] - mTotal) * (hist[i] - mTotal);			
		}
		cov = vTotal - vRight - vLeft;
		if (cov > maxV) {
			maxV = cov;
			ret = bin;
		}
	}
	return ret;
}
/**
 * calculate the index bin of the max value in histogram
 * @param hist             histogram of data
 * @param histBinNum       number of histogram bin
 * @return                 index bin
 */
int set_hist_thld1(const float hist[], int histBinNum) {
	float wLeft = 0, wRight = 0, mLeft = 0, mRight = 0, mTotal = 0;
	float cov, maxV = -1;
	int ret = 0;
	for (int bin = 1; bin <= histBinNum; bin++){
		for (int i = 0; i < bin; i++) {
			wLeft += hist[i];
			mLeft += ((float) i / 255) * hist[i];
		}
		for (int i = bin; i < histBinNum; i++) {
			wRight += hist[i];
			mRight += ((float) i / 255) * hist[i];
		}
		mTotal = mLeft + mRight;
		mLeft /= wLeft;
		mRight /= wRight;

		cov = wLeft * (mLeft - mTotal)* (mLeft - mTotal) + wRight * (mRight - mTotal)* (mRight - mTotal);
		if (cov > maxV) {
			maxV = cov;
			ret = bin;
		}
	}
	return ret;
}
/**
 * calculate the front 3 bin accumulative histogram
 * @param hist             histogram of data
 * @param histBinNum       number of histogram bin
 * @return                 accumulative histogram result
 */
float set_hist_low(const float hist[], int histBinNum) {
	return (hist[0] + hist[1] + hist[2]);
}
/**
 * calculate the half bin accumulative histogram
 * @param hist             histogram of data
 * @param histBinNum       number of histogram bin
 * @return                 accumulative histogram result
 */
float set_hist_half(const float hist[], int histBinNum) {
	float ret = 0;
	for (int i = 0; i < histBinNum/2; i++) {
		ret += hist[i];
	}
	return ret;
}
/**
 * calculate mask feature from top in darkTbl
 * @param blockY          input block data
 * @param darkTbl         input mask data
 * @return                feature value
 */
float set_top_sum_from_block(const float blockY[], int darkTbl[]) {
	float cnt = 0;
	for (int i = 0; i < BLOCK_X_NUM; i++){
		for (int j = 0; j < BLOCK_Y_NUM; j++) {
			if (darkTbl[j * BLOCK_X_NUM + i] == 1) {
				cnt = cnt + 1;
			} else {
				break;
			}
		}
	}
	cnt /= BLOCK_Y_NUM * BLOCK_X_NUM;
	return cnt;
}
/**
 * calculate mask feature from bottom in darkTbl
 * @param blockY          input block data
 * @param darkTbl         input mask data
 * @return                feature value
 */
float set_down_sum_from_block(const float blockY[], int darkTbl[]) {
	float cnt = 0;
	for (int i = 0; i < BLOCK_X_NUM; i++){
		for (int j = BLOCK_Y_NUM - 1; j >= 0; j--) {
			if (darkTbl[j * BLOCK_X_NUM + i] == 1) {
				cnt = cnt + 1;
			} else {
				break;
			}
		}
	}
	cnt /= BLOCK_Y_NUM * BLOCK_X_NUM;
	return cnt;
}
/**
 * calculate the mean value of ROI in input data
 * @param blockY           (input data) block data
 * @param regionMeanLl     (output data)template matching result
 * @return
 */
void set_region_meanLl_from_block(const float blockY[], float regionMeanLl[]) {
	float mask0[BLOCK_Y_NUM * BLOCK_X_NUM] = {
		1, 1, 1, 1, 1, 1, 1, 
		1, 0, 0, 0, 0, 0, 1, 
		0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0
	};
	float mask1[BLOCK_Y_NUM * BLOCK_X_NUM] = {
		0, 0, 0, 0, 0, 0, 0,
		0, 1, 1, 1, 1, 1, 0,
		1, 0, 0, 0, 0, 0, 1,
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		1, 0, 0, 0, 0, 0, 1,
		1, 1, 1, 1, 1, 1, 1
	};
	float mask2[BLOCK_Y_NUM * BLOCK_X_NUM] = {
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		0, 1, 1, 1, 1, 1, 0,
		0, 0, 0, 0, 0, 0, 0
	};
	float mask3[BLOCK_Y_NUM * BLOCK_X_NUM] = {
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		0, 1, 1, 1, 1, 1, 0,
		1, 1, 0, 0, 0, 1, 1,
		1, 1, 0, 0, 0, 1, 1,
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0
	};
	float mask4[BLOCK_Y_NUM * BLOCK_X_NUM] = {
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 1, 1, 1, 0, 0,
		0, 0, 1, 1, 1, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0
	};

	regionMeanLl[0] = 0; regionMeanLl[1] = 0; regionMeanLl[2] = 0; regionMeanLl[3] = 0; regionMeanLl[4] = 0;
	for(int j = 0; j < BLOCK_Y_NUM; j++){
		for (int i = 0; i < BLOCK_X_NUM; i++) {
			regionMeanLl[0] += blockY[j * BLOCK_X_NUM + i] * mask0[j * BLOCK_X_NUM + i] / 9;
			regionMeanLl[1] += blockY[j * BLOCK_X_NUM + i] * mask1[j * BLOCK_X_NUM + i] / 16;
			regionMeanLl[2] += blockY[j * BLOCK_X_NUM + i] * mask2[j * BLOCK_X_NUM + i] / 5;
			regionMeanLl[3] += blockY[j * BLOCK_X_NUM + i] * mask3[j * BLOCK_X_NUM + i] / 13;
			regionMeanLl[4] += blockY[j * BLOCK_X_NUM + i] * mask4[j * BLOCK_X_NUM + i] / 6;
		}
	}
}
/**
 * calculate the mean value of ROI in input data
 * @param blockY           (input data) block data
 * @param regionMeanLl     (output data)template matching result
 * @return
 */
void set_region_meanBl_from_block(const float blockY[], float regionMeanBl[]) {
	float mask0[BLOCK_Y_NUM * BLOCK_X_NUM] = {
		1, 1, 1, 1, 1, 1, 1,
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0
	};
	float mask1[BLOCK_Y_NUM * BLOCK_X_NUM] = {
		0, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 1, 1, 1, 1,
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0
	};
	float mask2[BLOCK_Y_NUM * BLOCK_X_NUM] = {
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 1, 1, 1, 1,
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0
	};
	float mask3[BLOCK_Y_NUM * BLOCK_X_NUM] = {
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 1, 1, 1, 1,
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0
	};
	float mask4[BLOCK_Y_NUM * BLOCK_X_NUM] = {
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		1, 1, 0, 0, 0, 1, 1,
		1, 1, 0, 0, 0, 1, 1,
		1, 1, 0, 0, 0, 1, 1
	};
	float mask5[BLOCK_Y_NUM * BLOCK_X_NUM] = {
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 1, 1, 1, 0, 0,
		0, 0, 1, 1, 1, 0, 0,
		0, 0, 1, 1, 1, 0, 0
	};

	regionMeanBl[0] = 0; regionMeanBl[1] = 0; regionMeanBl[2] = 0; regionMeanBl[3] = 0; regionMeanBl[4] = 0; regionMeanBl[5] = 0;
	for(int j = 0; j < BLOCK_Y_NUM; j++){
		for (int i = 0; i < BLOCK_X_NUM; i++) {
			regionMeanBl[0] += blockY[j * BLOCK_X_NUM + i] * mask0[j * BLOCK_X_NUM + i] / 7;
			regionMeanBl[1] += blockY[j * BLOCK_X_NUM + i] * mask1[j * BLOCK_X_NUM + i] / 7;
			regionMeanBl[2] += blockY[j * BLOCK_X_NUM + i] * mask2[j * BLOCK_X_NUM + i] / 7;
			regionMeanBl[3] += blockY[j * BLOCK_X_NUM + i] * mask3[j * BLOCK_X_NUM + i] / 7;
			regionMeanBl[4] += blockY[j * BLOCK_X_NUM + i] * mask4[j * BLOCK_X_NUM + i] / 12;
			regionMeanBl[5] += blockY[j * BLOCK_X_NUM + i] * mask5[j * BLOCK_X_NUM + i] / 9;
		}
	}
}
/**
 * size ratio of coordination transformation
 * @param rect             src coordination
 * @param start_index      src array offset index
 * @param current_width    src rect width
 * @param current_height   src rect height
 * @param dest_width       dst rect width
 * @param dest_height      dst rect height
 * @param dest_start_index src array offset index
 * @param out_rect         dst coordination
 * @return
 */
void change_position_from_differnent_size(int* rect, int start_index, int current_width, int current_height, int dest_width, int dest_height, int dest_start_index, int* dest_rect) {
	float scale_width = ((float)dest_width / current_width);
	float scale_height = ((float)dest_height / current_height);

	dest_rect[dest_start_index + 0] = (int) (scale_width * rect[start_index + 0]);
	dest_rect[dest_start_index + 1] = (int) (scale_height * rect[start_index + 1]);
	dest_rect[dest_start_index + 2] = (int) (scale_width * rect[start_index + 2]);
	dest_rect[dest_start_index + 3] = (int) (scale_height * rect[start_index + 3]);
}
/**
 * calculate angle between current line and horizon line
 * @param orientation     camera orientation [0, 90, 180, 270]
 * @param orientationRaw  camera orientation 0 - 359
 * @return
 */
int levelAngle(int orientation, int orientationRaw) {
	int ori = 0;
	switch (orientation) {
	case 0:
		if (orientationRaw > 315) {
			ori = orientationRaw - 360;
		} else {
			ori = orientationRaw;
		}
		break;
	case 90:
		ori = orientationRaw - 90;
		break;
	case 180:
		ori = orientationRaw - 180;
		break;
	case 270:
		ori = orientationRaw - 270;
		break;
	default:
		break;
	}
	//return abs(ori);
	return ori;

}
} //namespace util
