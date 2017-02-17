/*
 * code_detection.h
 *
 *  Created on: 2015年7月23日
 *      Author: guoyj7
 */

#ifndef SMART_KIT_OBJ_DETECTION_CODE_DETECTION_CODE_DETECTION_H_
#define SMART_KIT_OBJ_DETECTION_CODE_DETECTION_CODE_DETECTION_H_

#include "obj_detection.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef MIN
#  define MIN(a,b)  ((a) > (b) ? (b) : (a))
#endif
#ifndef MAX
#  define MAX(a,b)  ((a) < (b) ? (b) : (a))
#endif

# define CONTRAST_LIMIT 24
# define BLOCK_SIZE 7

typedef struct patternVectorStruct {
	int y; // horizontal
	int x; // vertical
	float s; // size
} patternVector;

typedef struct coordVectorStruct {
	int y; // horizontal
	int x; // vertical
} coordVector;

struct Node {
	int x;
	int y;
	float s;
	struct Node *next;
};

struct thdVal{
	float msThd; //1- maxVariance = modelSize / 2.0

	float avsThd; //2- orderThreePattern() : averageSize
	float raThd; //3- orderThreePattern() : ratio
	float cpThd; //4- orderThreePattern() : crossProduct

	float coThd; //5- qrcode_feature_check() : ratio
	float stdThd; //6- qrcode_feature_check() : imStd
	char  fDThd; // flagD
};



namespace smart {

class CodeDetection: public smart::ObjDetection {

private:
	int roi_qrcode_detect_region_[4];
	bool single_or_multi_;
	unsigned char *buf_yuv420sp_;
	unsigned char *buf_binary_cal_;
	unsigned char *buf_code_check_;

private:
	
	Node  *first_,  *node_temp_, *first_ord_, *node_util_, *node_util_2_;
	thdVal thd_val_1_;

public:
	CodeDetection();
	virtual ~CodeDetection();

	int set_parameter(const int type, void* param);
	int obj_detect(const void* input, const int width, const int height, void* output);

private:
	//=======================img processing function==========================
	void	  calculate_block_thd(unsigned char *yuv420spY, int height, int width, int blockSize, unsigned char *yuv420spY_Dst, unsigned char *thld /*, unsigned char *thldImg*/);
	int				 qrcode_check(unsigned char *yuv420spRes, /*unsigned char *yuv420spCopy, int *qrInfo,*/ int *qrPos,/* int *res ,*/ int res_height, int res_width, unsigned char *roi);
	//=======================img processing function==========================

	//=======================qrcoder rec. function====================
	int		  found_pattern_cross(int stateCount[]);
	void		   pattern_search(unsigned char *yuv420spY, int height, int width, int inHeight, int inWidth, int inHeight2, int inWidth2, bool single /*, int possibleCenters[]*/);
	int		 handlePossibleCenter(unsigned char *yuv420spY, int height, int width, int inHeight, int inWidth, int inHeight2, int inWidth2, int stateCount[], int i, int j /*, int possibleCenters[]*/);
	int				centerFromEnd(int stateCount[], int endPoint);

	int		 cross_check_vertical(unsigned char *yuv420spY, int heigth, int width, int inHeight, int inWidth, int startI, int centerJ, int maxCount, int originalStateCountTotal, int *flagI);
	int	   cross_check_horizontal(unsigned char *yuv420spY, int height, int width, int inHeight, int inWidth, int startJ, int centerI, int maxCount, int originalStateCountTotal, int *flagJ);
	char	 cross_check_diagonal(unsigned char *yuv420spY, int height, int width, int inHeight, int inWidth, int startI, int centerJ, int maxCount, int originalStateCountTotal);
	int		  pattern_equal_check(patternVector newModuleCenter, patternVector oldModuleCenter);
	void pattern_combine_estimate(patternVector newModuleCenter, patternVector oldModuleCenter, patternVector *moduleCenter);
	//=======================qrcoder rec. function====================

	//=======================qrcode check and order==========================================================
	void		order_patterns();
	int	  order_three_patterns(patternVector patterns[], patternVector orderedPatterns2[]);
	float qrcode_feature_check(unsigned char *yuv420spY, int height, int width, patternVector bottomLeftCorner, patternVector bottomRightCorner, patternVector topLeftCorner, patternVector topRightCorner, unsigned char *roi_qrcode_detect_region_);
	//=======================qrcode check and order==========================================================

	//======================= List Control =============================
	void   list_init(int type);
	int   list_length(int type);
	void  list_insert(int type, int module_y, int module_x, float module_s);
	Node*    list_get(int type, int i);
	void list_replace(int i, int module_y, int module_x, float module_s);
	void   list_print(int type);
	void  list_delete(int type);
	//======================= List Control =============================

	//======================= Interface =============================
	int qrcode_detection(unsigned char *yuv420sp, int *hist, int height, int width, int roix, int roiy, int roih, int roiw, int *qrPos, bool single, unsigned char *yuv420spCopy, unsigned char *thld, unsigned char *roi);
	//======================= Interface =============================
};

} /* namespace smart */
#endif /* SMART_KIT_OBJ_DETECTION_CODE_DETECTION_CODE_DETECTION_H_ */
