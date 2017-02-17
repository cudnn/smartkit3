/*
 * smart_compos.cpp
 *
 *  Created on: 2015年7月27日
 *      Author: guoyj7
 *
 * 	Add comments on variable and functions, clear up the unused part
 *
 *  Revised on: 2016/04/8
 *      Author: fengbin1
 */

#define _ANDROID_
#define DEBUG

#include "compile_definition.h"

#include "smart_composition.h"

#include <math.h>
#include <stdlib.h>

#include "log.h"
#include "util.h"

namespace smart {

int resPoGuideVALUE[14] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
int resPoGuideFLAG[5] = { -1, -1, -1, -1, -1 };


//**************************************** inner function *******************************************/
static double distanceCaculator(int x1, int y1, int x2, int y2) {
	double d = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
	return d;
}
//****************************************************************************************************/
//**************************************** public function *******************************************/
SmartComposition::SmartComposition() {

	Face_ThresLow = 100;
	Face_ThresHigh = 120;
	Face_ThresGood = 0.06f;
	Face_ThresPerfect = 0.02f;
	FaceHeight = 0.2f;
	FaceTrigger = 2 / 3.f;
	FaceRegion = 0.22f;
	Food_Threslow = 95;
	Food_Threshigh = 105;
	Food_ThresGood = 0.08f;
	Food_ThresPerfect = 0.04f;
	Line_ThresAngle1 = 6;
	Line_ThresAngle2 = 3;
	Line_ThresDisratio = 0.05f;
	compose_enable_flag= 0;

#ifdef _SMART_PORTRAIT_GUIDE
	enable_composition(SMART_GUIDE_PORTRAIT);
#else
	disable_composition(SMART_GUIDE_PORTRAIT);
#endif

#ifdef _SMART_LANDSCAPE_GUIDE
	enable_composition(SMART_GUIDE_LANDSCAPE);
#else
	disable_composition(SMART_GUIDE_LANDSCAPE);
#endif

#ifdef _SMART_FOOD_GUIDE
	enable_composition(SMART_GUIDE_FOOD);
#else
	disable_composition(SMART_GUIDE_FOOD);
#endif
}

SmartComposition::~SmartComposition() {
	// TODO Auto-generated destructor stub
}
/**
 * enable the specific scene composition guide
 * @param bit  scene type (defined in include/smart_type.h)
 * @return
 */
bool SmartComposition::enable_composition(int bit) {
	compose_enable_flag |= bit;
	return true;
}
/**
 * disable the specific scene composition guide
 * @param bit  scene type (defined in include/smart_type.h)
 * @return
 */
bool SmartComposition::disable_composition(int bit) {
	compose_enable_flag &= ~bit;
	return true;
}
/**
 * judge whether the specific scene composition guide is enabled
 * @param bit  scene type (defined in include/smart_type.h)
 * @return     true:enable|false:disable
 */
bool SmartComposition::is_enable(const int bit) {
	return (compose_enable_flag & bit) == bit;
}
/**
 * set the threshold for tuning composition
 * @param paramI   int type parameters
 * @param len1     length of param1
 * @param paramF   float type parameters
 * @param len2     length of param2
 * @return         true:success to tuning|false:fail to tuning
 */
bool SmartComposition::setSmartParameter(void* paramI, int len1, void* paramF, int len2) {
	int* param1 = (int*)paramI;
	float* param2 = (float*)paramF;
	if (param1 == NULL || param2 == NULL) {
			return false;
		} else {
			Face_ThresLow = param1[0];     //100;
			Face_ThresHigh = param1[1];    //120;
			Food_Threslow = param1[2];     //95;
			Food_Threshigh = param1[3];    //105;
			Line_ThresAngle1 = param1[4];  //6;
			Line_ThresAngle2 = param1[5];  //3;

			FaceHeight = param2[0];        //0.2f;
			FaceTrigger = param2[1];       //2/3f;
			FaceRegion = param2[2];        //0.22f;
			Face_ThresGood = param2[3];    //0.06f;
			Face_ThresPerfect = param2[4]; //0.02f;
			Food_ThresGood = param2[5];    //0.08f;
			Food_ThresPerfect = param2[6]; //0.04f;
			Line_ThresDisratio = param2[7];//0.05f;
			return true;
		}
}
/**
 * do scene composition guide
 * @param scene   scene flag, which scene to be composition
 * @param input   input data for composition process
 * @param output  output data for ICG UI design
 * @return
 */
void SmartComposition::compose_scene(int scene, void* input, void* output) {
	switch(scene) {
	case FLAG_DETECTION_PORTRAIT:
	case FLAG_DETECTION_PORTRAIT_BACKLIT:
	case FLAG_DETECTION_PORTRAIT_LOWLIT:
		if( is_enable(SMART_GUIDE_PORTRAIT) ){
			__android_log_print(ANDROID_LOG_INFO, "SMART_KIT", "SMART_GUIDE_PORTRAIT is enable.");
			compose_portrait((GuideInputParam*) input, (GuideParam*) output);
		}else{
			__android_log_print(ANDROID_LOG_INFO, "SMART_KIT", "SMART_GUIDE_PORTRAIT is disable.");
			compose_reset(scene ,  (GuideParam*) output);
		}

		break;
	case FLAG_DETECTION_LANDSCAPE:

		if( is_enable(SMART_GUIDE_LANDSCAPE) ){
			compose_line((GuideInputParam*) input, (GuideParam*) output);
		}else{
			compose_reset(scene ,  (GuideParam*) output);
		}

		break;
	case FLAG_DETECTION_FOOD:

		if( is_enable(SMART_GUIDE_FOOD) ){
			compose_food((GuideInputParam*) input, (GuideParam*) output);
		}else{
			compose_reset(scene ,  (GuideParam*) output);
		}

		break;
	}
}
//****************************************************************************************************/
//**************************************** public function *******************************************/
void SmartComposition::compose_reset(int scene , GuideParam* output){
	switch(scene) {
	case FLAG_DETECTION_PORTRAIT:
	case FLAG_DETECTION_PORTRAIT_BACKLIT:
	case FLAG_DETECTION_PORTRAIT_LOWLIT:
		output->getPortrait().reset();
		break;
	case FLAG_DETECTION_LANDSCAPE:
		output->getLandscape().reset();
		break;
	case FLAG_DETECTION_FOOD:
		output->getFood().reset();
		break;
	}
}

/**
 * do portrait composition guide
 * @param input   input data for composition process
 * @param output  output data for ICG UI design
 * @return
 */
int SmartComposition::compose_portrait(GuideInputParam* input, GuideParam* output) {

	//initial threshold
	int faceThres1 = Face_ThresLow;
	int faceThres2 = Face_ThresHigh;
	float ratioZ = FaceTrigger;
	float ratioF = FaceRegion;
	float ratioGood = Face_ThresGood;
	float ratioPerfect = Face_ThresPerfect;

	//set the target area and other parameters
	int left = input->rect[0];
	int top = input->rect[1];
	int right = input->rect[2];
	int bottom = input->rect[3];
	int orientation = input->orientation;
	int previewWidth = input->preview_width;
	int previewHeight = input->preview_height;
	int screenWidth = input->screen_width;
	int screenHeight = input->screen_height;

	//portrait guide trigger area
	float ratioZ1 = (1 - ratioZ) / 2;
	float ratioZ2 = 1 - (1 - ratioZ) / 2;
	if (left < previewWidth * ratioZ1 || top < previewHeight * ratioZ1
			|| right > previewWidth * ratioZ2 || bottom > previewHeight * ratioZ2) {
		return -1;
	}

	Param_Point faceLeftTop = Param_Point(left, top);
	Param_Point faceRightBottom = Param_Point(right, bottom);
	Param_Point faceCenter = Param_Point(-1, -1);
	faceCenter.x = (faceRightBottom.x - faceLeftTop.x) / 2 + faceLeftTop.x;
	faceCenter.y = (faceRightBottom.y - faceLeftTop.y) / 2 + faceLeftTop.y;

	resPoGuideVALUE[4] = faceCenter.x;
	resPoGuideVALUE[5] = faceCenter.y;
	//resPoGuideVALUE[6] = (faceRightBottom.x - faceLeftTop.x) / 2;

	Param_Point recoPositionHorizontal_1 = Param_Point(-1, -1);
	Param_Point recoPositionHorizontal_2 = Param_Point(-1, -1);
	Param_Point recoPositionHorizontal_3 = Param_Point(-1, -1);
	bool mFlagCamAngle = false;
//	IVCSCENE_DEBUG("test_step_8-1 line= %d fcx= %d , fcy=%d , angle= %d\n",__LINE__,faceCenter.x,faceCenter.y , mCameraAngle);
	if (orientation == 270 || orientation == 90) {
		//set target position for rule of thirds when camera is horizon
		mFlagCamAngle = true;
		resPoGuideFLAG[0] = 0;
		if (orientation == 270) {
			recoPositionHorizontal_1.x = previewWidth / 3;
			recoPositionHorizontal_1.y = previewHeight / 3;
			recoPositionHorizontal_2.x = previewWidth * 2 / 3;
			recoPositionHorizontal_2.y = previewHeight / 3;

			resPoGuideVALUE[7] = recoPositionHorizontal_1.x;
			resPoGuideVALUE[8] = recoPositionHorizontal_1.y;
			resPoGuideVALUE[9] = recoPositionHorizontal_2.x;
			resPoGuideVALUE[10] = recoPositionHorizontal_2.y;
		} else {
			recoPositionHorizontal_1.x = previewWidth * 2 / 3;
			recoPositionHorizontal_1.y = previewHeight * 2 / 3;
			recoPositionHorizontal_2.x = previewWidth / 3;
			recoPositionHorizontal_2.y = previewHeight * 2 / 3;

			resPoGuideVALUE[7] = recoPositionHorizontal_1.x;
			resPoGuideVALUE[8] = recoPositionHorizontal_1.y;
			resPoGuideVALUE[9] = recoPositionHorizontal_2.x;
			resPoGuideVALUE[10] = recoPositionHorizontal_2.y;
		}
	} else {
		//set target position for rule of symmetry when camera is vertical
		mFlagCamAngle = false;
		resPoGuideFLAG[0] = 1;
		if (orientation == 0) {
			recoPositionHorizontal_3.x = previewWidth / 3;
			recoPositionHorizontal_3.y = previewHeight / 2;
			resPoGuideVALUE[11] = recoPositionHorizontal_3.x;
			resPoGuideVALUE[12] = recoPositionHorizontal_3.y;
		} else {
			recoPositionHorizontal_3.x = previewWidth * 2 / 3;
			recoPositionHorizontal_3.y = previewHeight / 2;
			resPoGuideVALUE[11] = recoPositionHorizontal_3.x;
			resPoGuideVALUE[12] = recoPositionHorizontal_3.y;
		}
	}
//	IVCSCENE_DEBUG("test_step_8-2 line= %d p1x= %d , p1y=%d , p2x = %d , p2y= %d , p3x= %d , p3y= %d\n",__LINE__,resPoGuideVALUE[7],resPoGuideVALUE[8],resPoGuideVALUE[9],resPoGuideVALUE[10],resPoGuideVALUE[11],resPoGuideVALUE[12]);

	float faceZone = faceRightBottom.y - faceLeftTop.y;
	float suitZone;
	int ratioFace;
	int mFlagZoom = -1;

	//-----judge the zoom case(far or close from camera,相机离远离近判断)--------------
	if (mFlagCamAngle) {
		//camera is horizon横屏
		suitZone = previewHeight * ratioF/*0.22f*/;
		ratioFace = (int) (faceZone / suitZone * 100);

		if (ratioFace <= faceThres2 && ratioFace >= faceThres1) {
			mFlagZoom = 1;
			resPoGuideFLAG[1] = 1;
		} else {
			if (ratioFace < faceThres1) {
				mFlagZoom = 0;
				resPoGuideFLAG[1] = 0;
			} else {
				mFlagZoom = 2;
				resPoGuideFLAG[1] = 2;
			}
		}
	} else {
		//camera is vertical竖屏
		suitZone = previewWidth * ratioF/*0.22f*/;
		ratioFace = (int) (faceZone / suitZone * 100);

		if (ratioFace <= faceThres2 && ratioFace >= faceThres1) {
			mFlagZoom = 1;
			resPoGuideFLAG[1] = 1;
		} else {
			if (ratioFace < faceThres1) {
				mFlagZoom = 0;
				resPoGuideFLAG[1] = 0;
			} else {
				mFlagZoom = 2;
				resPoGuideFLAG[1] = 2;
			}
		}
	}

	//---大于100设定为100-----------
	if (ratioFace > 100) {
		resPoGuideVALUE[0] = 100;
	} else {
		resPoGuideVALUE[0] = ratioFace;
	}
	//	resPoGuideVALUE[0] = ratioFace;
//	IVCSCENE_DEBUG("test_step_8-3 line= %d ratioF= %d , fThd1= %d , fThd2= %d ,flag[1]= %d\n",__LINE__,ratioFace,faceThres1,faceThres2,resPoGuideFLAG[1]);

	//-----judge the face angle case(人脸侧面或正面)--------------
	bool mFlagFaceAngle = false;
	double mFaceAngle = 0.1;
	if (mFlagZoom == 1) {
		//横屏+大小满足

		if (mFaceAngle > -0.2 && mFaceAngle < 0.2) {
			mFlagFaceAngle = true;
			resPoGuideFLAG[2] = 0;
		} else {
			mFlagFaceAngle = false;
			resPoGuideFLAG[2] = 1;
		}
	} else {
		//横屏+大小不满足
		// 前后移动
	}
	//-----judge the distance case(good or perfect,目标距离引导位置)--------------
	double dleft = 0.f, dright = 0.f;
	int dDegreeL = 0, dDegreeR = 0, dist = 0;
	double levelOne = previewWidth * ratioGood; //0.06
	double levelTwo = previewWidth * ratioPerfect; //0.02
	int mFlagDistance = -1;
	int mFlagGuideCase = -1;
	int ratioDis = -1;
	if (mFlagCamAngle == true) {
		if (mFlagFaceAngle) {
			//横屏+大小满足+正脸
			mFlagGuideCase = 0;
			resPoGuideFLAG[4] = 0;
			dleft = distanceCaculator(faceCenter.x, faceCenter.y,
					recoPositionHorizontal_1.x, recoPositionHorizontal_1.y);
			dright = distanceCaculator(faceCenter.x, faceCenter.y,
					recoPositionHorizontal_2.x, recoPositionHorizontal_2.y);

			if (dleft < dright) {
				dDegreeL = 100;
				dDegreeR = (int) (((dleft / dright)) * 100);
				dist = (int) dleft;
//				IVCSCENE_DEBUG("test_step_8-4 line= %d dL= %f , dR = %f ,dDL = %d , dDR= %d\n",__LINE__,dleft,dright ,dDegreeL,dDegreeR);
			} else {
				dDegreeL = (int) (((dright / dleft)) * 100);
				dDegreeR = 100;
				dist = (int) dright;
//				IVCSCENE_DEBUG("test_step_8-4 line= %d dL= %f , dR = %f ,dDL = %d , dDR= %d\n",__LINE__,dleft,dright ,dDegreeL,dDegreeR);
			}
			ratioDis = (int) ((1
					- (double) dist
							/ distanceCaculator(recoPositionHorizontal_1.x,
									recoPositionHorizontal_1.y, previewWidth,
									previewHeight)) * 100);
			resPoGuideVALUE[13] = ratioDis;
			//Log.i("distance", ""+ratioDis);
			if (dist <= levelOne) {
				mFlagDistance = 1; // good
				resPoGuideFLAG[3] = 1;
				if (dist <= levelTwo) {
					mFlagDistance = 2; // perfect
					resPoGuideFLAG[3] = 2;
				}
			} else {
				mFlagDistance = 0; // ng
				resPoGuideFLAG[3] = 0;
			}
//			IVCSCENE_DEBUG("test_step_8-5 line= %d , flagDis = %d\n",__LINE__ ,resPoGuideFLAG[3]);
		} else {
			//横屏+大小满足+侧脸
			mFlagGuideCase = 1;
			resPoGuideFLAG[4] = 1;
			if (mFaceAngle < -0.3) {
				resPoGuideFLAG[4] = 1;
				dist = (int) distanceCaculator(faceCenter.x, faceCenter.y,
						recoPositionHorizontal_2.x, recoPositionHorizontal_2.y);
				resPoGuideVALUE[3] = 100;
				ratioDis = (int) ((1
						- (double) dist
								/ distanceCaculator(recoPositionHorizontal_2.x,
										recoPositionHorizontal_2.y, 0,
										previewHeight)) * 100);
				resPoGuideVALUE[13] = ratioDis;
				if (dist <= levelOne) {
					mFlagDistance = 1; //good
					resPoGuideFLAG[3] = 1;
					if (dist <= levelTwo) {
						mFlagDistance = 2; // perfect
						resPoGuideFLAG[3] = 2;
					}
				} else {
					mFlagDistance = 0; //ng
					resPoGuideFLAG[3] = 0;
				}

			}
			if (mFaceAngle > 0.3) {
				resPoGuideFLAG[4] = 2;
				dist = (int) distanceCaculator(faceCenter.x, faceCenter.y,
						recoPositionHorizontal_1.x, recoPositionHorizontal_1.y);
				ratioDis = (int) ((1
						- (double) dist
								/ distanceCaculator(recoPositionHorizontal_1.x,
										recoPositionHorizontal_1.y, previewWidth,
										previewHeight)) * 100);
				resPoGuideVALUE[13] = ratioDis;
				resPoGuideVALUE[3] = 100;
				if (dist <= levelOne) {
					mFlagDistance = 1; //good
					resPoGuideFLAG[3] = 1;
					if (dist <= levelTwo) {
						mFlagDistance = 2; // perfect
						resPoGuideFLAG[3] = 2;
					}
				} else {
					mFlagDistance = 0; //ng
					resPoGuideFLAG[3] = 0;
				}
			}
		}
	} else {
		//竖屏
		if (mFlagZoom == 1) {
			mFlagGuideCase = 2;
			resPoGuideFLAG[4] = 3;
			dist = (int) distanceCaculator(faceCenter.x, faceCenter.y,
					recoPositionHorizontal_3.x, recoPositionHorizontal_3.y);
			ratioDis = (int) ((1
					- (double) dist
							/ distanceCaculator(recoPositionHorizontal_3.x,
									recoPositionHorizontal_3.y, previewWidth,
									previewHeight)) * 100);
			resPoGuideVALUE[13] = ratioDis;
			resPoGuideVALUE[3] = 100;
			if (dist <= levelOne) {
				mFlagDistance = 1; //good
				resPoGuideFLAG[3] = 1;
				if (dist <= levelTwo) {
					mFlagDistance = 2; // perfect
					resPoGuideFLAG[3] = 2;
				}
			} else {
				mFlagDistance = 0; //ng
				resPoGuideFLAG[3] = 0;
			}
		}
	}

	resPoGuideVALUE[1] = (int) dDegreeL;
	resPoGuideVALUE[2] = (int) dDegreeR;
//	IVCSCENE_DEBUG("test_step_8-6 line= %d , dDL = %d , dDR= %d\n",__LINE__,resPoGuideVALUE[1],resPoGuideVALUE[2]);


	float ratioW = (float) screenHeight / previewWidth;
	float ratioH = (float) screenWidth / previewHeight;
	output->getPortrait().currentPosition.x = (int) ((previewHeight - resPoGuideVALUE[5]) * ratioH);
	output->getPortrait().currentPosition.y = (int) (resPoGuideVALUE[4] * ratioW);
	output->getPortrait().zoomRatio = resPoGuideVALUE[0];
	output->getPortrait().zoomFlag = resPoGuideFLAG[1];
	output->getPortrait().orientation = orientation;
	output->getPortrait().leftRatio = resPoGuideVALUE[1];
	output->getPortrait().rightRatio = resPoGuideVALUE[2];
	output->getPortrait().guideFlag = resPoGuideFLAG[3];
	output->getPortrait().allow = true;
	return 1;
}
/**
 * do food composition guide
 * @param input   input data for composition process
 * @param output  output data for ICG UI design
 * @return
 */
int SmartComposition::compose_food(GuideInputParam* input, GuideParam* output) {
	//initial threshold
	int foodThres1 = Food_Threslow;
	int foodThres2 = Food_Threshigh;
	float ratioGood = Food_ThresGood;
	float ratioPerfect = Food_ThresPerfect;
	SMART_DEBUG("Threshold tuning test thld1 = %d , thld2 = %d\n",foodThres1, foodThres2);
	//set the target area and other parameters
	int left = input->rect[0];
	int top = input->rect[1];
	int right = input->rect[2];
	int bottom = input->rect[3];
	int orientation = input->orientation;
	int width = input->width;
	int height = input->height;
	int previewWidth = input->preview_width;
	int previewHeight = input->preview_height;
	int screenWidth = input->screen_width;
	int screenHeight = input->screen_height;

	int params[9];

	int thresGood = (int) (height * ratioGood);
	int thresPerfect = (int) (height * ratioPerfect);
	int center_x = input->center_x;
	int center_y = input->center_y;

	double distances = sqrt(
			(double) ((center_x - width / 2) * (center_x - width / 2)
					+ (center_y - height / 2) * (center_y - height / 2)));

	//-----judge the zoom case(far or close from camera,相机离远离近判断)--------------
	int elwidth = right - left;
	int elheight = bottom - top;
	float ratio = (float) (elheight + 25) / height * 100;

	if (ratio < foodThres1) {
		params[1] = 0;
	} else if (ratio > foodThres2) {
		params[1] = 2;
	} else {
		params[1] = 1;
	}
	if (ratio >= 100) {
		ratio = 100;
	}

	//-----judge the distance case(good or perfect,目标距离引导位置)--------------
	if (params[1] == 1) {
		if (distances <= thresPerfect) {
			params[2] = 2;
		} else if ((distances <= thresGood)) {
			params[2] = 1;
		} else {
			params[2] = 0;
		}
	} else {
		params[2] = 0;
	}
	SMART_DEBUG("guide_food zoomRatio=%f, ratioFlag=%d, ratio1=%d, ratio2=%d, guideFlag=%d, guideDis=%f, guide1=%d, guide2=%d", ratio, params[1], foodThres1, foodThres2, params[2], distances, thresPerfect, thresGood);

	params[3] = center_x;
	params[4] = center_y;
	params[5] = width / 2;
	params[6] = height / 2;
	params[7] = (int) ratio;

	float ratioW = (float) screenHeight / previewWidth;
	float ratioH = (float) screenWidth / previewHeight;
	output->getFood().allow = true;
	output->getFood().currentPosition.x = (int) ((1 - (float) params[4] / height) * previewHeight * ratioH); // (int)((1 - (float)elGuideParams[4] / DataBoofCVHeight) * previewHeight * ratioH);
	output->getFood().currentPosition.y = (int) (((float) params[3] / width) * previewWidth * ratioW); // (int)(((float)elGuideParams[3]/DataBoofCVWidth) * previewWidth * ratioW);
	output->getFood().targetPosition.x = (int) ((1 - (float) params[6] / height) * previewHeight * ratioH); //previewHeight / 2;															 //
	output->getFood().targetPosition.y = (int) (((float) params[5] / width) * previewWidth * ratioW); // previewWidth / 2;															 //
	output->getFood().zoomRatio = params[7];
	output->getFood().zoomFlag = params[1];
	output->getFood().guideFlag = params[2];

	if (output->getFood().guideFlag != 0) {
		output->getFood().ready = true;
	} else {
		output->getFood().ready = false;
	}

	//when the height ratio of food < 40, stop food guide
	if (output->getFood().zoomRatio < 40) {
		output->getFood().allow = false;
	}
}
/**
 * do line composition guide
 * @param input   input data for composition process
 * @param output  output data for ICG UI design
 * @return
 */
int SmartComposition::compose_line(GuideInputParam* input, GuideParam* output) {

	//initial threshold
	int LineThresAngle1 = Line_ThresAngle1;
	int LineThresAngle2 = Line_ThresAngle2;
	float LineThresDisratio = Line_ThresDisratio;

	//set the target area and other parameters
	int left = input->rect[0];
	int top = input->rect[1];
	int right = input->rect[2];
	int bottom = input->rect[3];
	int orientation = input->orientation;
	int orientation_raw = input->orientation_raw;
	int width = input->width;
	int height = input->height;
	int previewWidth = input->preview_width;
	int previewHeight = input->preview_height;
	int screenWidth = input->screen_width;
	int screenHeight = input->screen_height;

	int param[7];

	int horizonAng = util::levelAngle(orientation, orientation_raw);

	if (orientation == 0 || orientation == 180) {
		left = (left + right) / 2;
		right = left;
		top = 0;
		bottom = height;
	} else {
		top = (top + bottom) / 2;
		bottom = top;
		left = 0;
		right = width;
	}


	int center_x = abs(right - left) / 2 + left;
	int center_y = abs(bottom - top) / 2 + top;
	int target_x = 0, target_y = 0;
	float distances = 0.0f;

	switch (orientation) {
	case 0:
		distances = abs((float) width * 2 / 3 - center_x);
		target_x = width * 2 / 3;
		target_y = height / 2;
		break;
	case 90:
		distances = abs((float) height * 1 / 3 - center_y);
		target_x = width / 2;
		target_y = height * 1 / 3;
		break;
	case 180:
		distances = abs((float) width * 1 / 3 - center_x);
		target_x = width * 1 / 3;
		target_y = height / 2;
		break;
	case 270:
		distances = abs((float) height * 2 / 3 - center_y);
		target_x = width / 2;
		target_y = height * 2 / 3;
		break;
	default:
		break;
	}

	//-------judge distance case 直线距离判断-----------------------
	if (distances < height * LineThresDisratio) {
		param[0] = 1;
	} else {
		param[0] = 0;
	}

	//------judge angle case 角度判断---------------------------
	if (abs(horizonAng) > LineThresAngle1) {
		param[1] = 0;
	} else {
		param[1] = 1;
	}

	//-----good or perfect case 判断达标------------------------------
	if (param[0] == 1) {
		if (abs(horizonAng) <= LineThresAngle2) {
			param[2] = 2;
		} else if (abs(horizonAng) <= LineThresAngle1) {
			param[2] = 1;
		} else {
			param[2] = 0;
		}
	} else {
		param[2] = 0;
	}

	param[3] = center_x;
	param[4] = center_y;
	param[5] = target_x;
	param[6] = target_y;


	output->getLandscape().allow = true;
	output->getLandscape().levelAngle = horizonAng;
	if (param[0] == 1) {
		output->getLandscape().distanceOk = true;
	} else {
		output->getLandscape().distanceOk = false;
	}

	if (param[1] == 0) {
		output->getLandscape().angleFlag = false;
	} else {
		output->getLandscape().angleFlag = true;
	}


	float ratioW = (float) screenHeight / previewWidth ;
	float ratioH = (float) screenWidth  / previewHeight;

	output->getLandscape().guideFlag = param[2];
	output->getLandscape().orientation = orientation;
	output->getLandscape().currentPosition.x = (int) ((1 - (float) param[4] / height) * previewHeight * ratioH);
	output->getLandscape().currentPosition.y = (int) (((float) param[3] / width) * previewWidth * ratioW);
	output->getLandscape().targetPosition.x = (int) ((1 - (float) param[6] / height) * previewHeight * ratioH);
	output->getLandscape().targetPosition.y = (int) (((float) param[5] / width) * previewWidth * ratioW);

}
}/* namespace smart */
