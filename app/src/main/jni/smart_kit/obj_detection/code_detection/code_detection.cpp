/*
 * code_detection.cpp
 *
 *  Created on: 2015年7月23日
 *      Author: guoyj7
 */

#include "code_detection.h"

namespace smart {

CodeDetection::CodeDetection() {
	// TODO Auto-generated constructor stub
	thd_val_1_.msThd  = 2.0; //maxVariance
	thd_val_1_.avsThd = 0.2; //patternSize average
	thd_val_1_.raThd  = 0.95; //distance_Ratio
	thd_val_1_.cpThd  = 0.98; //cross_Product
	thd_val_1_.coThd  = 1.7; //qrcode_feature_check.ratio
	thd_val_1_.stdThd = 45; //qrcode_feature_check.im_std

}

CodeDetection::~CodeDetection() {
	// TODO Auto-generated destructor stub
}

int CodeDetection::set_parameter(const int type, void* param) {

	if (type == DETECTION_PARAM_QRCODE_ROI)
	{
		int* tmp_roi_qrcode_detect_region_ = (int *)param;
		roi_qrcode_detect_region_[0] = tmp_roi_qrcode_detect_region_[0];
		roi_qrcode_detect_region_[1] = tmp_roi_qrcode_detect_region_[1];
		roi_qrcode_detect_region_[2] = tmp_roi_qrcode_detect_region_[2];
		roi_qrcode_detect_region_[3] = tmp_roi_qrcode_detect_region_[3];
	}

	if (type == DETECTION_PARAM_QRCODE_SINGLE_OR_MULT)
	{
		bool* tmp_sig_or_mult = (bool *)param;
		single_or_multi_ = *tmp_sig_or_mult;
	}

	if (type == DETECTION_PARAM_QRCODE_IMG_BUF)
		buf_yuv420sp_ = (unsigned char *)param;

	if (type == DETECTION_PARAM_QRCODE_THLD_BUF)
		 buf_binary_cal_ = (unsigned char *)param;

	if (type == DETECTION_PARAM_QRCODE_MID_ZONE_BUF)
		buf_code_check_ = (unsigned char *)param;

	return 0;
}

/*
* code_detection.cpp
* output : res-1: -1 : no qrcode was detected
*          res-2: >0 : num of qrcode were detected , where:
						[0]:num of qrcode
						[1]:topRightCorner.x
						[2]:topRightCorner.y
						[3]:topLeftCorner.x
						[4]:topLeftCorner.y
						[5]:bottomLeftCorner.x
						[6]:bottomLeftCorner.y
						[7]:bottomRightCorner.x
						[8]:bottomRightCorner.y
*  created on: 2015年7月20日
*  Author: bob xu
*/
int CodeDetection::obj_detect(const void* input, const int width, const int height, void* output) {
	
	int* qr_pos = (int *)output;
	unsigned char *yuvframe = (unsigned char *)input;

	int res = qrcode_detection(
		yuvframe,
		NULL,
		height, width,
		roi_qrcode_detect_region_[0], roi_qrcode_detect_region_[1], height, width,
		qr_pos,
		single_or_multi_,
		buf_yuv420sp_,
		buf_binary_cal_,
		buf_code_check_);

	return res;
}

int CodeDetection::qrcode_detection(unsigned char *yuv420sp, int *hist, int height, int width, int roix, int roiy, int roih, int roiw, int *qrPos, bool single, unsigned char *yuv420spCopy, unsigned char *thld, unsigned char *roi) {

	//cv::Mat gray = cv::Mat(height, width, CV_8UC1, yuv420sp);
	//cv::imshow("gray", gray);

	qrPos[0] = 0;

	calculate_block_thd(yuv420sp, height, width, BLOCK_SIZE, yuv420spCopy, thld/*, thldImg*/);

	//for (int i = 0; i < HEIGHT*WIDTH; i++){
	//	if( yuv420spCopy[i] == 1)
	//		yuv420spCopy[i] = 255;
	//}
	//cv::Mat gray = cv::Mat(height, width, CV_8UC1, yuv420spCopy);
	//cv::imshow("gray", gray);

	int inH, inW, inH2, inW2;

	if (roix < 0 || roiy < 0 || (roih + roix) > height || (roiw + roiy) > width){
		//*4-
		inH = 0;
		inW = 0;
		inH2 = 0;
		inW2 = 0;
	}
	else {
		inH = roix;
		inW = roiy;
		inH2 = height - roih - roix;//		inH2 = res_height - roih - roix;
		inW2 = width - roiw - roiy; //		inW2 = res_width - roiw - roiy; 
	}

	list_init(0);


	pattern_search(yuv420spCopy, height, width, inH, inW, inH2, inW2, single /*,  possibleCenters*/);
	//const int numOfPatterns = list_length(0);//possibleCenters[0];
	//std::cout << numOfPatterns << std::endl;

	int check_re = qrcode_check(yuv420sp,/*yuv420spCopy, qrInfo,*/ qrPos, /*res,*/ height, width, roi);

	return check_re;

}

int CodeDetection::qrcode_check(unsigned char *yuv420spRes,/*unsigned char *yuv420spCopy, int *qrInfo, */int *qrPos,/* int *res, */int res_height, int res_width, unsigned char *roi){

	patternVector bottomLeftCenter, topLeftCenter, topRightCenter, bottomRightCenter;
	coordVector bottomLeftCorner, bottomRightCorner, topLeftCorner, topRightCorner;

	list_init(1);
	order_patterns();

	float ratio = 0.1;
	float r = 0;
	int cnt = 0;

	int flag = list_length(1) / 3;

	if (flag > 0) {

		for (int t = 0; t < flag; t++) {

			node_temp_ = list_get(1, t * 3 + 1);
			bottomLeftCenter.x = node_temp_->x;
			bottomLeftCenter.y = node_temp_->y;
			bottomLeftCenter.s = node_temp_->s;
			node_temp_ = list_get(1, t * 3 + 2);
			topLeftCenter.x = node_temp_->x;
			topLeftCenter.y = node_temp_->y;
			topLeftCenter.s = node_temp_->s;
			node_temp_ = list_get(1, t * 3 + 3);
			topRightCenter.x = node_temp_->x;
			topRightCenter.y = node_temp_->y;
			topRightCenter.s = node_temp_->s;

			bottomRightCenter.x = topRightCenter.x + bottomLeftCenter.x - topLeftCenter.x;
			bottomRightCenter.y = topRightCenter.y + bottomLeftCenter.y - topLeftCenter.y;

			topRightCorner.x = (1 + ratio) * topRightCenter.x - ratio * bottomLeftCenter.x;
			topRightCorner.y = (1 + ratio) * topRightCenter.y - ratio * bottomLeftCenter.y;

			bottomLeftCorner.x = (1 + ratio) * bottomLeftCenter.x - ratio * topRightCenter.x;
			bottomLeftCorner.y = (1 + ratio) * bottomLeftCenter.y - ratio * topRightCenter.y;

			topLeftCorner.x = (1 + ratio) * topLeftCenter.x - ratio * bottomRightCenter.x;
			topLeftCorner.y = (1 + ratio) * topLeftCenter.y - ratio * bottomRightCenter.y;

			bottomRightCorner.x = topRightCorner.x + bottomLeftCorner.x - topLeftCorner.x;
			bottomRightCorner.y = topRightCorner.y + bottomLeftCorner.y - topLeftCorner.y;

			//LOGI("----1");
			//*8-1
			//clock_t start = clock();
			r = qrcode_feature_check(yuv420spRes, res_height, res_width, bottomLeftCenter, bottomRightCenter, topLeftCenter, topRightCenter, roi);
			//r =  qrcode_feature_check(yuv420sp    , height     , width     , bottomLeftCenter, bottomRightCenter, topLeftCenter, topRightCenter);
			//clock_t end = clock();
			//double d = (double)( end -start) /  CLOCKS_PER_SEC ;
			//LOGI("---:>imqrcode_feature_check: %f" , d);
			//*8-1
			//LOGI("----2");

			if (r == 1) {
				qrPos[0]++;
				qrPos[cnt * 8 + 1] = topRightCorner.x;
				qrPos[cnt * 8 + 2] = topRightCorner.y;
				qrPos[cnt * 8 + 3] = topLeftCorner.x;
				qrPos[cnt * 8 + 4] = topLeftCorner.y;
				qrPos[cnt * 8 + 5] = bottomLeftCorner.x;
				qrPos[cnt * 8 + 6] = bottomLeftCorner.y;
				qrPos[cnt * 8 + 7] = bottomRightCorner.x;
				qrPos[cnt * 8 + 8] = bottomRightCorner.y;

				//qrInfo[cnt * 1 + 1] = getDistance(topRightCorner, topLeftCorner)* getDistance(topRightCorner, topLeftCorner);
				cnt++;
			}
		}
		//qrInfo[0] = qrPos[0];
		//		LOGI("--:--1 %d",qrInfo[0]);
		// ****** have to *****************
		list_delete(0);
		list_delete(1);
		//*9-1
		//free(yuv420spRes);
		//*9-1
		//free(yuv420spCopy);
		// ****** have to *****************
		//res[0] = 0;
		return qrPos[0];

	}
	else {
		//		LOGI("--:-- flag == 0");
		//for (int i = 0; i < sizeof(qrInfo); i++) {
		//	qrInfo[i] = 0;
		//}
		// ****** have to *****************
		list_delete(0);
		list_delete(1);
		//*9-4
		//free(yuv420spRes);
		//*9-4
		//free(yuv420spCopy);
		// ****** have to *****************
		//res[0] = 0;
		return -1;
	}
}



/**********************************************************************************************************
**  Func  ： calculate_block_thd
**  Author：Zhimin XU , Feb. 24, 2014
**********************************************************************************************************/
void CodeDetection::calculate_block_thd(unsigned char *yuv420spY, int height, int width, int blockSize, unsigned char *yuv420spY_Dst, unsigned char *thld/*, unsigned char *thldImg*/) {

	int subHeight = ceil((int)height / (int)blockSize);
	int subWidth = ceil((int)width / (int)blockSize);
	int indx, yStart, yEnd, xStart, xEnd, subIndx, subRowIndx;
	unsigned char value, maxValue, minValue, filtValue;
	//float sumValue, meanValue;
	int sumValue, meanValue;

	for (int j = 0; j < subHeight; j++) {
		yStart = j * blockSize;
		yEnd = MIN((j + 1) * blockSize, height);
		for (int i = 0; i < subWidth; i++) {
			xStart = i * blockSize;
			xEnd = MIN((i + 1) * blockSize, width);

			maxValue = 0;
			minValue = 255;
			sumValue = 0;
			for (int x = xStart; x < xEnd; x++) {
				for (int y = yStart; y < yEnd; y++) {
					indx = y * width + x;
					value = yuv420spY[indx];
					sumValue += value;
					if (value > maxValue) {
						maxValue = value;
					}
					if (value < minValue) {
						minValue = value;
					}
				}
			}
			if (maxValue - minValue > CONTRAST_LIMIT) {
				meanValue = sumValue / (blockSize * blockSize);
			}
			else {
				meanValue = minValue / 2;
			}
			subIndx = j * subWidth + i;
			thld[subIndx] = meanValue;
		}
	}

	for (int j = 2; j < subHeight - 2; j++) //  85 ms   ->  40ms
	{
		yStart = j * blockSize;
		yEnd = MIN((j + 1) * blockSize, height);

		for (int i = 2; i < subWidth - 2; i++) {
			subIndx = j * subWidth + i;
			sumValue = 0;
			for (int k = -2; k <= 2; k++) {
				subRowIndx = (j + k) * subWidth + i;
				sumValue += thld[subRowIndx - 2] + thld[subRowIndx - 1]
					+ thld[subRowIndx] + thld[subRowIndx + 1]
					+ thld[subRowIndx + 2];
			}
			filtValue = sumValue / 25; //thldImg[subIndx] = sumValue / 25;

			xStart = i * blockSize;
			xEnd = MIN((i + 1) * blockSize, width);
			//subIndx = j * subWidth + i;
			for (int x = xStart; x < xEnd; x++) {
				for (int y = yStart; y < yEnd; y++) {
					indx = y * width + x;
					//					yuv420spY[indx] =(yuv420spY[indx] < filtValue/*thldImg[subIndx]*/) ? 1 : 0;
					yuv420spY_Dst[indx] = (yuv420spY[indx] < filtValue/*thldImg[subIndx]*/) ? 1 : 0;
				}
			}

		}
	}

}

/**********************************************************************************************************
**  Fucn  ： imgBinarizeLocal
**  app   ： imgBinarizeLocal
**  Author：Zhimin XU, Dec. 12, 2013
**********************************************************************************************************/
//void imgBinarizeLocal(unsigned char *yuv420spY, int height, int width,
//	int blockSize, unsigned char *thldImg) {
//	int subHeight = ceil((int)height / (int)blockSize);
//	int subWidth = ceil((int)width / (int)blockSize);
//	int indx, yStart, yEnd, xStart, xEnd, subIndx;
//
//	for (int j = 0; j < subHeight; j++) {
//		yStart = j * blockSize;
//		yEnd = MIN((j + 1) * blockSize, height);
//		for (int i = 0; i < subWidth; i++) {
//			xStart = i * blockSize;
//			xEnd = MIN((i + 1) * blockSize, width);
//
//			subIndx = j * subWidth + i;
//			for (int x = xStart; x < xEnd; x++) {
//				for (int y = yStart; y < yEnd; y++) {
//					indx = y * width + x;
//					yuv420spY[indx] =
//						(yuv420spY[indx] < thldImg[subIndx]) ? 1 : 0;
//				}
//			}
//		}
//	}
//}

void CodeDetection::pattern_search(unsigned char *yuv420spY, int height, int width, int inHeight, int inWidth, int inHeight2, int inWidth2, bool single /* , int possibleCenters[]*/) {


	//	int bnum = 0 ;

	int MIN_SKIP = 3;
	int MAX_MODULES = 57;

	//[height, width] = size(bwImg);

	int iSkip = (3 * height) / (4 * MAX_MODULES);
	if (iSkip < MIN_SKIP)
		iSkip = MIN_SKIP;

	int stateCount[5] = { 0, 0, 0, 0, 0 };
	//	int possibleCenters[37] = {0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0} ;
	int currentState = 0;
	int yp, i, j;

	for (i = iSkip + inHeight; i < height - inHeight2; i += iSkip)
	{
		stateCount[0] = 0;
		stateCount[1] = 0;
		stateCount[2] = 0;
		stateCount[3] = 0;
		stateCount[4] = 0;
		currentState = 0;

		yp = i*width;

		for (j = 0 + inWidth; j < width - inWidth2; j++)
		{
			unsigned char Y = yuv420spY[yp + j];

			if (Y == 1)
			{
				if ((currentState == 1) || (currentState == 3))
				{
					currentState = currentState + 1;
				}
				stateCount[currentState] = stateCount[currentState] + 1;
			}
			else
			{
				if ((currentState == 0) || (currentState == 2) || (currentState == 4))
				{
					if (currentState == 4)
					{
						if (found_pattern_cross(stateCount))
						{
							int confirmed = handlePossibleCenter(yuv420spY, height, width, inHeight, inWidth, inHeight2, inWidth2, stateCount, i, j /*, possibleCenters*/);
							//---------- if one code detected , return - 2013-12-07----------
							if (single == true) {
								if (list_length(0) >= 5) {
									return;
								}
							}
							if (list_length(0) >= 14) {
								return;
							}
							//---------- if one code detected , return - 2013-12-07----------
							if (confirmed == 1)
							{
								iSkip = 2;
							}
							else
							{
								//stateCount = [stateCount(3:5), 1, 0];
								stateCount[0] = stateCount[2];
								stateCount[1] = stateCount[3];
								stateCount[2] = stateCount[4];
								stateCount[3] = 1;
								stateCount[4] = 0;
								currentState = 3;
								continue;
							}
							//stateCount = zeros(1, 5);
							stateCount[0] = 0;
							stateCount[1] = 0;
							stateCount[2] = 0;
							stateCount[3] = 0;
							stateCount[4] = 0;
							currentState = 0;
						}
						else
						{
							//stateCount = [stateCount(3:5), 1, 0];
							stateCount[0] = stateCount[2];
							stateCount[1] = stateCount[3];
							stateCount[2] = stateCount[4];
							stateCount[3] = 1;
							stateCount[4] = 0;
							currentState = 3;
						}
					}
					else
					{
						currentState = currentState + 1;
						stateCount[currentState] = stateCount[currentState] + 1;
					}
				}
				else
				{
					//currentState = currentState + 1;
					stateCount[currentState] = stateCount[currentState] + 1;
				}
			}
		}
	}

	if (found_pattern_cross(stateCount))
	{
		int confirmed = handlePossibleCenter(yuv420spY, height, width, inHeight, inWidth, inHeight2, inWidth2, stateCount, i, width /*, possibleCenters*/);
		//---------- if one code detected , return - 2013-12-07----------
		if (single == true) {
			if (list_length(0) >= 5) {
				return;
			}
		}
		//---------- if one code detected , return - 2013-12-07----------
		if (list_length(0) >= 14) {
			return;
		}
		
		if (confirmed)
			iSkip = stateCount[0];
	}

	//	bnum = possibleCenters[0];
	//	return bnum;

}

/**********************************************************************************************************
**  Func  : found_pattern_cross
**  Author：Zhimin XU ,  Bob XU, Dec. 12, 2013
**********************************************************************************************************/
int CodeDetection::found_pattern_cross(int stateCount[]) {
	int totalModuleSize = 0;
	int count = 0;
	int ret = 0;
	for (int i = 0; i < 5; i++) {
		count = stateCount[i];
		if (count == 0) {
			ret = 0;
			return ret;
		}
		totalModuleSize = totalModuleSize + count;
	}

	if (totalModuleSize < 7) {
		ret = 0;
		return ret;
	}

	float moduleSize = totalModuleSize / 7.0;
	//	float maxVariance = moduleSize      / 2.0;
	float maxVariance = moduleSize / thd_val_1_.msThd;

	bool b1 = abs(moduleSize - stateCount[0]) < maxVariance;
	bool b2 = abs(moduleSize - stateCount[1]) < maxVariance;
	bool b3 = abs(3 * moduleSize - stateCount[2]) < 3 * maxVariance;
	bool b4 = abs(moduleSize - stateCount[3]) < maxVariance;
	bool b5 = abs(moduleSize - stateCount[4]) < maxVariance;

	if (b1 && b2 && b3 && b4 && b5) {
		ret = 1;
	}
	return ret;
}

/**********************************************************************************************************
**  Func     ： handlePossibleCenter
**  Author：Zhimin XU ,  Bob XU, Dec. 12, 2013
**********************************************************************************************************/
int CodeDetection::handlePossibleCenter(unsigned char *yuv420spY, int height, int width, int inHeight, int inWidth, int inHeight2, int inWidth2, int stateCount[], int i, int j /*, int possibleCenters[]*/) {
	int flagI, flagJ, flagD, flagPC, found;
	patternVector newModuleCenter;//int newModuleCenter[3];
	patternVector oldModuleCenter;//int oldModuleCenter[3];
	//	int p1 = 1;
	//	int p2 = 2;
	//	int p3 = 3;

	int stateCountTotal = stateCount[0] + stateCount[1] + stateCount[2] + stateCount[3] + stateCount[4];
	int centerJ = centerFromEnd(stateCount, j);
	int centerI = cross_check_vertical(yuv420spY, height, width, inHeight2, inWidth, i, centerJ, stateCount[2], stateCountTotal, &flagI);

	if (flagI)
	{
		centerJ = cross_check_horizontal(yuv420spY, height, width, inHeight, inWidth2, centerJ, centerI, stateCount[2], stateCountTotal, &flagJ);
		flagD = cross_check_diagonal(yuv420spY, height, width, inHeight2, inWidth2, centerI, centerJ, stateCount[2], stateCountTotal);
		if (flagJ && flagD)
			//		if (flagJ)
		{
			float estimatedModuleSize = stateCountTotal / 7.0;
			newModuleCenter.y = centerI;//newModuleCenter[0] = centerI;  第I行
			newModuleCenter.x = centerJ;//newModuleCenter[1] = centerJ;  第J列
			newModuleCenter.s = estimatedModuleSize;//newModuleCenter[2] = estimatedModuleSize;

			found = 0;

			//int num_x = possibleCenters[0];
			//int num_y = list_length(0);

			for (int index = 0; index < list_length(0)/*possibleCenters[0]*/; index++)
			{
				node_temp_ = list_get(0, index + 1);
				oldModuleCenter.y = node_temp_->y;
				oldModuleCenter.x = node_temp_->x;
				oldModuleCenter.s = node_temp_->s;

				//	int step_pC = index * 3;
				//	oldModuleCenter.y = possibleCenters[step_pC+1];//oldModuleCenter[0] = possibleCenters[step_pC+1];
				//	oldModuleCenter.x = possibleCenters[step_pC+2];//oldModuleCenter[1] = possibleCenters[step_pC+2];
				//	oldModuleCenter.s = possibleCenters[step_pC+3];//oldModuleCenter[2] = possibleCenters[step_pC+3];

				int flagE = pattern_equal_check(newModuleCenter, oldModuleCenter);

				if (flagE != 0)
				{
					//int moduleCenter[3];
					patternVector moduleCenter;

					pattern_combine_estimate(newModuleCenter, oldModuleCenter, &moduleCenter);

					//	possibleCenters[step_pC+1] = moduleCenter.y;//possibleCenters[step_pC+1] = moduleCenter[0];
					//	possibleCenters[step_pC+2] = moduleCenter.x;//possibleCenters[step_pC+2] = moduleCenter[1];
					//	possibleCenters[step_pC+3] = moduleCenter.s;//possibleCenters[step_pC+3] = moduleCenter[2];
					//	list_insert( 0 ,  moduleCenter.y , moduleCenter.x , moduleCenter.s);
					list_replace(index + 1, moduleCenter.y, moduleCenter.x, moduleCenter.s);

					found = 1;
					break;
				}
			}
			if (!found)
			{
				//	int step = possibleCenters[0] * 3;
				//	possibleCenters[0]++;
				//	possibleCenters[step+1] = newModuleCenter.y;//possibleCenters[step+1] = newModuleCenter[0];
				//	possibleCenters[step+2] = newModuleCenter.x;//possibleCenters[step+2] = newModuleCenter[1];
				//	possibleCenters[step+3] = newModuleCenter.s;//possibleCenters[step+3] = newModuleCenter[2];

				list_insert(0, newModuleCenter.y, newModuleCenter.x, newModuleCenter.s);

			}
			flagPC = 1;
			return flagPC;
		}
	}
	flagPC = 0;
	return  flagPC;
}

/**********************************************************************************************************
**  Func     ： centerFromEnd
**  Author：Zhimin XU ,  Bob XU, Dec. 12, 2013
**********************************************************************************************************/
int CodeDetection::centerFromEnd(int stateCount[], int endPoint) {
	int center = endPoint - stateCount[4] - stateCount[3] - (stateCount[2] - 1) / 2;
	return center;
}

/**********************************************************************************************************
**  Func     ： cross_check_vertical
**  Author：Zhimin XU ,  Bob XU, Dec. 12, 2013
**********************************************************************************************************/
int CodeDetection::cross_check_vertical(unsigned char *yuv420spY, int height, int width, int inHeight, int inWidth, int startI, int centerJ, int maxCount, int originalStateCountTotal, int *flagI) {
	int maxI = height - inHeight;
	int stateCount[] = { 0, 0, 0, 0, 0 };
	int ret = 0;
	int i = startI;

	while (i > 0 && yuv420spY[i*width + centerJ])
	{
		stateCount[2] = stateCount[2] + 1;
		i = i - 1;
	}

	if (i == 0)
	{
		*flagI = 0;
		return ret;
	}
	while (i > 0 && !yuv420spY[i*width + centerJ] && stateCount[1] <= maxCount)
	{
		stateCount[1] = stateCount[1] + 1;
		i = i - 1;
	}
	if (i == 0 || stateCount[1] > maxCount)
	{
		*flagI = 0;
		return ret;
	}
	while (i > 0 && yuv420spY[i*width + centerJ] && stateCount[0] <= maxCount)
	{
		stateCount[0] = stateCount[0] + 1;
		i = i - 1;
	}
	if (stateCount[0] > maxCount)
	{
		*flagI = 0;
		return ret;
	}
	i = startI + 1;
	while (i < maxI && yuv420spY[i*width + centerJ])
	{
		stateCount[2] = stateCount[2] + 1;
		//i = i + width;
		i = i + 1;
	}
	if (i >= maxI)
	{
		*flagI = 0;
		return ret;
	}
	while (i < maxI && !yuv420spY[i*width + centerJ] && stateCount[3] < maxCount)
	{
		stateCount[3] = stateCount[3] + 1;
		i = i + 1;
	}
	if (i >= maxI || stateCount[3] >= maxCount)
	{
		*flagI = 0;
		return ret;
	}
	while (i < maxI && yuv420spY[i*width + centerJ] && stateCount[4] < maxCount)
	{
		stateCount[4] = stateCount[4] + 1;
		i = i + 1;
	}
	if (stateCount[4] >= maxCount)
	{
		*flagI = 0;
		return ret;
	}
	int stateCountTotal = stateCount[0] + stateCount[1] + stateCount[2] + stateCount[3] + stateCount[4];
	if (5 * abs(stateCountTotal - originalStateCountTotal) >= 2 * originalStateCountTotal)
	{
		*flagI = 0;
		return ret;
	}

	int fPC = found_pattern_cross(stateCount);

	if (fPC)
	{
		*flagI = 1;
		ret = centerFromEnd(stateCount, i);
		return ret;
	}
	else
	{
		*flagI = 0;
		return ret;
	}
}

/**********************************************************************************************************
**  Func     ： cross_check_horizontal
**  Author：Zhimin XU ,  Bob XU, Dec. 12, 2013
**********************************************************************************************************/
int CodeDetection::cross_check_horizontal(unsigned char *yuv420spY, int height, int width,
	int inHeight, int inWidth, int startJ, int centerI, int maxCount,
	int originalStateCountTotal, int *flagJ) {
	int maxJ = width - inWidth;
	int stateCount[] = { 0, 0, 0, 0, 0 };
	int ret = 0;
	int j = startJ;

	while (j > 0 && yuv420spY[centerI*width + j])
	{
		stateCount[2] = stateCount[2] + 1;
		j = j - 1;
	}
	if (j == 0)
	{
		*flagJ = 0;
		return ret;
	}
	while (j > 0 && !yuv420spY[centerI*width + j] && stateCount[1] <= maxCount)
	{
		stateCount[1] = stateCount[1] + 1;
		j = j - 1;
	}
	if (j == 0 || stateCount[1] > maxCount)
	{
		*flagJ = 0;
		return ret;
	}
	while (j > 0 && yuv420spY[centerI*width + j] && stateCount[0] <= maxCount)
	{
		stateCount[0] = stateCount[0] + 1;
		j = j - 1;
	}
	if (stateCount[0] > maxCount)
	{
		*flagJ = 0;
		return ret;
	}
	j = startJ + 1;
	while (j < maxJ && yuv420spY[centerI*width + j])
	{
		stateCount[2] = stateCount[2] + 1;
		j = j + 1;
	}
	if (j >= maxJ)
	{
		*flagJ = 0;
		return ret;
	}
	while (j < maxJ && !yuv420spY[centerI*width + j] && stateCount[3] < maxCount)
	{
		stateCount[3] = stateCount[3] + 1;
		j = j + 1;
	}
	if (j >= maxJ || stateCount[3] >= maxCount)
	{
		*flagJ = 0;
		return ret;
	}
	while (j < maxJ && yuv420spY[centerI*width + j] && stateCount[4] < maxCount)
	{
		stateCount[4] = stateCount[4] + 1;
		j = j + 1;
	}
	if (stateCount[4] >= maxCount)
	{
		*flagJ = 0;
		return ret;
	}
	int stateCountTotal = stateCount[0] + stateCount[1] + stateCount[2] + stateCount[3] + stateCount[4];
	if (5 * abs(stateCountTotal - originalStateCountTotal) >= originalStateCountTotal)
	{
		*flagJ = 0;
		return ret;
	}
	int fPC = found_pattern_cross(stateCount);
	if (fPC)
	{
		*flagJ = 1;
		ret = centerFromEnd(stateCount, j);
		return ret;
	}
	else
	{
		*flagJ = 0;
		return ret;
	}
}

/**********************************************************************************************************
**  Func     ： cross_check_diagonal
**  Author：Zhimin XU , Feb. 18, 2013
**********************************************************************************************************/
char CodeDetection::cross_check_diagonal(unsigned char *yuv420spY, int height, int width,
	int inHeight, int inWidth, int startI, int centerJ, int maxCount,
	int originalStateCountTotal) {
	int maxI = height - inHeight;
	int maxJ = width - inWidth;
	int stateCount[] = { 0, 0, 0, 0, 0 };
	int ret = 0;
	char flag = 0;

	// Down
	int i = 0;
	while (startI - i >= 0 && centerJ - i >= 0 && yuv420spY[(startI - i) * width + (centerJ - i)])
	{
		stateCount[2]++;
		i++;
	}
	if (startI - i < 0 || centerJ - i < 0)
	{
		flag = 0;
		return flag;
	}

	while (startI - i >= 0 && centerJ - i >= 0 && !yuv420spY[(startI - i) * width + (centerJ - i)] && stateCount[1] <= maxCount)
	{
		stateCount[1]++;
		i++;
	}
	if (startI - i < 0 || centerJ - i < 0 || stateCount[1] > maxCount)
	{
		flag = 0;
		return flag;
	}

	while (startI - i >= 0 && centerJ - i >= 0 && yuv420spY[(startI - i) * width + (centerJ - i)] && stateCount[0] <= maxCount)
	{
		stateCount[0]++;
		i++;
	}
	if (stateCount[0] > maxCount)
	{
		flag = 0;
		return flag;
	}

	// Up
	i = 1;
	while (startI + i < maxI && centerJ + i < maxJ && yuv420spY[(startI + i) * width + (centerJ + i)])
	{
		stateCount[2]++;
		i++;
	}
	if (startI + i >= maxI || centerJ + i >= maxJ)
	{
		flag = 0;
		return flag;
	}

	while (startI + i < maxI && centerJ + i < maxJ && !yuv420spY[(startI + i) * width + (centerJ + i)] && stateCount[3] <= maxCount)
	{
		stateCount[3]++;
		i++;
	}
	if (startI + i >= maxI || centerJ + i >= maxJ || stateCount[3] > maxCount)
	{
		flag = 0;
		return flag;
	}

	while (startI + i < maxI && centerJ + i < maxJ && yuv420spY[(startI + i) * width + (centerJ + i)] && stateCount[4] <= maxCount)
	{
		stateCount[4]++;
		i++;
	}
	if (stateCount[4] > maxCount)
	{
		flag = 0;
		return flag;
	}

	int stateCountTotal = stateCount[0] + stateCount[1] + stateCount[2] + stateCount[3] + stateCount[4];

	if (abs(stateCountTotal - originalStateCountTotal) >= 2 * originalStateCountTotal)
	{
		flag = 0;
		return flag;
	}

	if (found_pattern_cross(stateCount))
	{
		flag = 1;
		return flag;
	}
	else
	{
		flag = 0;
		return flag;
	}
}

/**********************************************************************************************************
**  Func     ： pattern_equal_check
**  Author：Zhimin XU ,  Bob XU, Dec. 12, 2013
**********************************************************************************************************/
int CodeDetection::pattern_equal_check(patternVector newModuleCenter, patternVector oldModuleCenter) {
	bool flag = 0;
	if (abs(newModuleCenter.y - oldModuleCenter.y) <= newModuleCenter.s && abs(newModuleCenter.x - oldModuleCenter.x) <= newModuleCenter.s)
	{
		int moduleSizeDiff = abs(newModuleCenter.s - oldModuleCenter.s);
		flag = (moduleSizeDiff <= 1) || (moduleSizeDiff <= oldModuleCenter.s);
		return flag;
	}
	if (flag == true)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/**********************************************************************************************************
**  Func     ： pattern_combine_estimate
**  Auhtor：Zhimin XU ,  Bob XU, Dec. 12, 2013
**********************************************************************************************************/
void CodeDetection::pattern_combine_estimate(patternVector newModuleCenter, patternVector oldModuleCenter, patternVector *moduleCenter) {
	moduleCenter->y = (oldModuleCenter.y + newModuleCenter.y) / 2;
	moduleCenter->x = (oldModuleCenter.x + newModuleCenter.x) / 2;
	moduleCenter->s = (oldModuleCenter.s + newModuleCenter.s) / 2;
}


/**********************************************************************************************************
**  Func     ： order_patterns
**  Author：Zhimin XU  , Bob XU Mar. 4, 2014
**********************************************************************************************************/
void CodeDetection::order_patterns() {

	Node *ti, *tj, *tk;

	patternVector pattern[3];
	patternVector orderedPatterns2[3];

	//int mark[7] = {1,1,1,1,1,1,1};
	int cnt = 0;

	for (int i = 0; i< list_length(0)/*possibleCenters[0]*/; i++)
	{
		for (int j = 0; j< list_length(0) /*possibleCenters[0]*/; j++)
		{
			if (j != i)
			{
				for (int k = 0; k<list_length(0) /*possibleCenters[0]*/; k++)
				{
					if (k != i && k != j)
					{
						//if ( ( mark[i] != 0 ) && ( mark[j] != 0 ) && ( mark[k] != 0 ) )
						//if ( ( possibleCenters[i].s !=0 ) && (possibleCenters[j].s != 0 ) && ( possibleCenters[k].s != 0 ))
						ti = list_get(0, i + 1);
						tj = list_get(0, j + 1);
						tk = list_get(0, k + 1);
						if ((ti->s != 0) && (tj->s != 0) && (tk->s != 0))
						{
							//pattern[0] = possibleCenters[i];//patterns{1} = possiblePatterns{i};
							//pattern[1] = possibleCenters[j];//patterns{2} = possiblePatterns{j};
							//pattern[2] = possibleCenters[k];//patterns{3} = possiblePatterns{k};

							pattern[0].x = ti->x;
							pattern[0].y = ti->y;
							pattern[0].s = ti->s;
							pattern[1].x = tj->x;
							pattern[1].y = tj->y;
							pattern[1].s = tj->s;
							pattern[2].x = tk->x;
							pattern[2].y = tk->y;
							pattern[2].s = tk->s;

							int flag_empty = order_three_patterns(pattern, orderedPatterns2);
							if (flag_empty != 0)
							{

								//orderedPatterns[cnt]   = orderedPatterns2[0];
								//orderedPatterns[cnt+1] = orderedPatterns2[1];
								//orderedPatterns[cnt+2] = orderedPatterns2[2];

								list_insert(1, orderedPatterns2[0].y, orderedPatterns2[0].x, orderedPatterns2[0].s);
								list_insert(1, orderedPatterns2[1].y, orderedPatterns2[1].x, orderedPatterns2[1].s);
								list_insert(1, orderedPatterns2[2].y, orderedPatterns2[2].x, orderedPatterns2[2].s);

								cnt += 3;
								//orderedPatterns{cnt} = result;
								//possibleCenters[i].s = 0 ;
								//possibleCenters[j].s = 0 ;
								//possibleCenters[k].s = 0 ;
								ti->s = 0;
								tj->s = 0;
								tk->s = 0;
								//mark[i] = 0 ;//possiblePatterns{i} ={};
								//mark[j] = 0 ;//possiblePatterns{j} ={};
								//mark[k] = 0 ;//possiblePatterns{k} ={};

							}
						}
					}
				}
			}
		}
	}

}

/**********************************************************************************************************
**  Func  : order_three_patterns
**  Author：Zhimin XU  , Bob XU Mar. 4, 2014
**********************************************************************************************************/
int CodeDetection::order_three_patterns(patternVector patterns[],
	patternVector orderedPatterns2[]) {

	int flagEmpty = 0;
	patternVector pointA, pointB, pointC;
	int ABDistance, BCDistance, ACDistance;

	float averageSize = (patterns[0].s + patterns[1].s + patterns[2].s) / 3.0;

	if ((abs(patterns[0].s - averageSize) > 0.2 * averageSize) ||
		(abs(patterns[1].s - averageSize) > 0.2 * averageSize) ||
		(abs(patterns[2].s - averageSize) > 0.2 * averageSize)){
		//orderedPatterns = {};
		return flagEmpty;
	}
	ABDistance = sqrt((patterns[0].x - patterns[1].x) * (patterns[0].x - patterns[1].x) + (patterns[0].y - patterns[1].y) * (patterns[0].y - patterns[1].y));
	BCDistance = sqrt((patterns[1].x - patterns[2].x) * (patterns[1].x - patterns[2].x) + (patterns[1].y - patterns[2].y) * (patterns[1].y - patterns[2].y));
	ACDistance = sqrt((patterns[0].x - patterns[2].x) * (patterns[0].x - patterns[2].x) + (patterns[0].y - patterns[2].y) * (patterns[0].y - patterns[2].y));

	if (BCDistance >= ABDistance && BCDistance >= ACDistance)
	{
		pointB = patterns[0];
		pointA = patterns[1];
		pointC = patterns[2];
	}
	else if (ACDistance >= BCDistance && ACDistance >= ABDistance){
		pointB = patterns[1];
		pointA = patterns[0];
		pointC = patterns[2];
	}
	else{
		pointB = patterns[2];
		pointA = patterns[0];
		pointC = patterns[1];
	}
	ABDistance = sqrt((pointA.x - pointB.x)*(pointA.x - pointB.x) + (pointA.y - pointB.y)*(pointA.y - pointB.y));//sqrt((pointA.x - pointB.x)^2 + (pointA.y - pointB.y)^2);
	BCDistance = sqrt((pointB.x - pointC.x)*(pointB.x - pointC.x) + (pointB.y - pointC.y)*(pointB.y - pointC.y));//sqrt((pointB.x - pointC.x)^2 + (pointB.y - pointC.y)^2);

	// 	float averageDistance = (ABDistance + BCDistance) / 2.0;
	// 	if ( (abs(ABDistance - averageDistance) > 0.1 * averageDistance) ||  (abs(BCDistance - averageDistance) > 0.1 * averageDistance) ){
	// 			//orderedPatterns = {};
	// 			return flagEmpty;
	// 	}

	float ratio;
	if (ABDistance > BCDistance){
		ratio = BCDistance / (float)ABDistance;
	}
	else{
		ratio = ABDistance / (float)BCDistance;
	}
	if (ratio < 0.95){
		return flagEmpty;
	}

	float crossProduct = ((pointC.x - pointB.x) * (pointA.y - pointB.y) - (pointC.y - pointB.y) * (pointA.x - pointB.x)) / (float)(ABDistance*BCDistance);
	if (abs(crossProduct) < 0.98/*0.98*/)
	{
		//orderedPatterns = {};
		return flagEmpty;
	}
	else{
		if (crossProduct < 0)
		{
			patternVector temp;
			temp = pointA;
			pointA = pointC;
			pointC = temp;
		}
		orderedPatterns2[0] = pointA;
		orderedPatterns2[1] = pointB;
		orderedPatterns2[2] = pointC;
		flagEmpty = 1;
		return flagEmpty;
	}
}

void CodeDetection::list_init(int type){

	if (type == 0)
	{
		first_ = (Node *)malloc(sizeof(Node));  //first_ = new Node;
		first_->next = NULL;
		node_util_ = first_;
		return;
	}

	if (type == 1)
	{
		first_ord_ = (Node *)malloc(sizeof(Node)); // ord_first = new Node;
		first_ord_->next = NULL;
		node_util_2_ = first_ord_;
		return;
	}
}


int CodeDetection::list_length(int type){
	Node *p;

	if (type == 0)
	{
		p = first_->next;
	}
	else
	{
		p = first_ord_->next;
	}

	int count = 0;
	while (p != NULL)
	{
		p = p->next;
		count++;
	}
	return count;
}

void CodeDetection::list_insert(int  type, int module_y, int module_x, float module_s){

	Node *s = (Node *)malloc(sizeof(Node));
	s->y = module_y;
	s->x = module_x;
	s->s = module_s;
	s->next = NULL;

	if (type == 0)
	{
		node_util_->next = s;
		node_util_ = s;
	}
	else
	{
		node_util_2_->next = s;
		node_util_2_ = s;
	}

}


Node* CodeDetection::list_get(int type, int i){
	Node *p;
	int count = 1;
	if (type == 0)
	{
		p = first_->next;
	}
	else
	{
		p = first_ord_->next;
	}
	while (p != NULL && count < i)
	{
		p = p->next;
		count++;
	}
	//if ( p !=NULL)
	//	{
	return p;
	//	}
}

void CodeDetection::list_replace(int i, int module_y, int module_x, float module_s){
	Node *p;
	int count = 1;
	p = first_->next;
	while (p != NULL && count < i)
	{
		p = p->next;
		count++;
	}
	p->x = module_x;
	p->y = module_y;
	p->s = module_s;
}

void CodeDetection::list_print(int type){

	Node *node_for_print_;

	if (type == 0)
	{
		node_for_print_ = first_->next;
	}
	else
	{
		node_for_print_ = first_ord_->next;
	}

	while (node_for_print_ != NULL)
	{
		printf("x,y,s = %d,%d,%f\n", node_for_print_->x, node_for_print_->y, node_for_print_->s);
		node_for_print_ = node_for_print_->next;
	}

	node_for_print_ = NULL;
}

void CodeDetection::list_delete(int type)
{
	Node *q;
	if (type == 0)
	{
		while (first_ != NULL)
		{
			q = first_;
			first_ = first_->next;
			free(q);
		}

	}
	else
	{
		while (first_ord_ != NULL)
		{
			q = first_ord_;
			first_ord_ = first_ord_->next;
			free(q);
		}
	}

	node_util_ = NULL;
	//node_for_print_ = NULL;
	node_temp_ = NULL;

}

float CodeDetection::qrcode_feature_check(unsigned char *yuv420spY, int height, int width,
	patternVector bottomLeftCorner, patternVector bottomRightCorner,
	patternVector topLeftCorner, patternVector topRightCorner, unsigned char *roi) {

	//	IplImage *colorImg2 = cvLoadImage(PATH);

	//	float dis = getDistance(topLeftCorner , bottomLeftCorner);
	float dis = sqrt((bottomLeftCorner.y - topLeftCorner.y)* (bottomLeftCorner.y - topLeftCorner.y) + (bottomLeftCorner.x - topLeftCorner.x) * (bottomLeftCorner.x - topLeftCorner.x));

	float a = (bottomLeftCorner.y - topLeftCorner.y) / dis;
	float b = (bottomLeftCorner.x - topLeftCorner.x) / dis;
	float c = (topRightCorner.y - topLeftCorner.y) / dis;
	float d = (topRightCorner.x - topLeftCorner.x) / dis;
	float e = topLeftCorner.y;
	float f = topLeftCorner.x;

	int length = (int)dis;

	//	float A[3][3] = {{a,b},{c,d},{e,f}};
	//	unsigned char *roi = (unsigned char *) malloc(length * length * sizeof(unsigned char));
	//	LOGI("---:>LENGTH: %d" , length * length);
	int cnt = 0;
	int roi_1 = 0;
	//	LOGI("step_3-1");
	for (int x = 0; x < length; x++) {
		for (int y = 0; y < length; y++) {
			int indx = x * a + y * c + e;
			int indy = x * b + y * d + f;
			if (indx <= 0)
				indx = 0;
			if (indy <= 0)
				indy = 0;
			if (indx >= height)
				indx = height;
			if (indy >= width)
				indy = width;
			//			LOGI("indx=%d,indy=%d",indx,indy);
			//			cvCircle(colorImg2, cvPoint(indy, indx), 2, cvScalar(0,200,0), 2, 2, 0);
			roi[cnt] = yuv420spY[indx * width + indy];
			roi_1 += yuv420spY[indx * width + indy];
			cnt++;
		}
	}
	//	LOGI("step_3-2");
	//	cvShowImage(" ", colorImg2);
	//	cvWaitKey(0);

	int mean_roi = roi_1 / (cnt - 1);

	int brightSum = 0;
	int darkSum = 0;
	int brighL = 0;
	int darkL = 0;

	int im_sum = 0;
	int sum = 0;
	int temp = 0;

	for (int i = 0; i < cnt; i++) {
		if (roi[i] >= mean_roi) {
			brightSum += roi[i];
			brighL++;
		}
		if (roi[i] < mean_roi) {
			darkSum += roi[i];
			darkL++;
		}

		temp = (roi[i] - mean_roi) * (roi[i] - mean_roi);
		im_sum += temp;
	}
	//	LOGI("step_3-3");
	im_sum = im_sum / cnt;
	float im_std = sqrt(im_sum);

	float r = 0;

	if (darkL == 0 || darkSum == 0) {
		r = 1000;
	}
	else if (brighL == 0 || brightSum == 0) {
		r = 0;
	}
	else {
		r = ((float)brightSum / brighL) / ((float)darkSum / darkL);
	}
	//	LOGI("ratio :%f ,  std : %f", r , im_std);
	float rs = 0;
	// 	if( r >1.7 && im_std > 45 ){
	if (r > thd_val_1_.coThd && im_std > thd_val_1_.stdThd) {
		rs = 1;
	}
	else {
		rs = 0;
	}
	//	LOGI("step_3-4:r=%f , std=%f",r,im_std);
	//	free(roi);
	return rs;
}

} /* namespace smart */
