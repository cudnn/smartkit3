/*
 * smart_recognition.cpp
 *
 *  Created on: 2015/07/28
 *      Author: guoyj7
 *
 * 	Add comments on variable and functions, optimize algorithm flow and clear up the unused part
 *
 *  Revised on: 2016/04/7
 *      Author: fengbin1
 */


#define _ANDROID_
#define DEBUG

#include "compile_definition.h"

#include "mf_tracking.h"

#include "smart_recognition.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#if defined WIN32 || defined _WIN32
#include <opencv2/opencv.hpp>
#endif

//#include "smart_type.h"
#include "log.h"

#include "util/MatrixData.h"
#include "util/util.h"
#include "util/face_util.h"

#ifdef _SMART_MOTION
#include "util/motion_util.h"
#endif

#include "util/light_util.h"

#ifdef _SMART_FOOD
#include "ellipse_detection.h"
#include <KLTracking/food_tracker.h>
#endif

#ifdef _SMART_LANDSCAPE
#include "obj_detection/line_detection/line_detection.h"
#include "feature_extraction/gist_feature/landscape_gist_score.h"
//#include "line_detection.h"
//#include "landscape_gist_score.h"
#endif

#ifdef _SMART_CODE
#include "code_detection.h"
#include "zxing_code_detection.h"
#include <KLTracking/qrcode_tracker.h>
#endif



namespace smart {

std::vector<int> mImgList;               //smooth data list
std::vector<int> mLightList;
std::vector<int> mMotionList;
std::vector<cv::Point2i> mFoodList;
std::vector<float> mAccList;
std::vector<float> mEllpiseList;

//char qrdecodeTxt[1000] = {0};            //QRcode result array
//char qrdecodeType[100] = {0};

#ifdef _SMART_CODE
QrCodeTracker _qrcode_tracker;           //QR code track object
#endif

#ifdef _SMART_FOOD
FoodTracker _food_tracker;               //food track object
#endif
//#define DEBUG_VALUE                    //debug_value switch, 取消注释可打开debug值
//**************************************** inner function *******************************************/
/**
 * encode the sceneType data into an int data
 * @param sceneType  result of scene detection
 * @return           encoded result
 */
int encodeSceneResult(SceneType &sceneType) {
	int scene_flag = 0;

	while (true) {

		//check motion
		if (sceneType.get_motion().is_detected_type()) {
			scene_flag += DEF_DETECT_FLAG_MOTION * sceneType.get_motion().get_flag();
		}

		//check Portrait
		if (sceneType.get_portrait().is_detected_type()) {
			scene_flag += DEF_DETECT_FLAG_PORTRAIT * sceneType.get_portrait().get_flag();
		}

		//check portrait light
		if (sceneType.get_portrait_lit().is_detected_type()) {
			scene_flag += DEF_DETECT_FLAG_PORTRAITLIT * sceneType.get_portrait_lit().get_flag();
		}

		//check food
		if (sceneType.get_food().is_detected_type()) {
			scene_flag += DEF_DETECT_FLAG_FOOD * sceneType.get_food().get_flag();
		}

		//check light
		if (sceneType.get_light().is_detected_type()) {
			scene_flag += DEF_DETECT_FLAG_LIGHT * sceneType.get_light().get_flag();
		}

		//check landscape
		if (sceneType.get_landscape().is_detected_type()) {
			scene_flag += DEF_DETECT_FLAG_LANDSCAPE * sceneType.get_landscape().get_flag();
		}

		//check code
		if (sceneType.get_code().is_detected_type()) {
			scene_flag += DEF_DETECT_FLAG_CODE * sceneType.get_code().get_flag();
		}

		//check document
		if (sceneType.get_document().is_detected_type()) {
			scene_flag += DEF_DETECT_FLAG_DOCUMENT * sceneType.get_document().get_flag();
		}
		break;
	}
	return scene_flag;
}
/**
 * decode scene result(int data)
 * @param retFlag    result of scene detection(int)
 * @param flagScene  result of scene detection(int array:normal scene & motion scene)
 * @return
 */
void decodeSceneResult(int retFlag, int* flagScene) {

	int motionFlag = retFlag % 10;
	int lightFlag = (retFlag / DEF_DETECT_FLAG_LIGHT) % 10;
	int portraitFlag = (retFlag / DEF_DETECT_FLAG_PORTRAIT) % 10;
	int portraitLitFlag = (retFlag / DEF_DETECT_FLAG_PORTRAITLIT) % 10;
	int foodFlag = (retFlag / DEF_DETECT_FLAG_FOOD) % 10;
	int landscapeFlag = (retFlag / DEF_DETECT_FLAG_LANDSCAPE) % 10;
	int codeFlag = (retFlag / DEF_DETECT_FLAG_CODE) % 10;
	int documentFlag = (retFlag / DEF_DETECT_FLAG_DOCUMENT) % 10;

	//-----------------Scene Detection------------------------------
	flagScene[0] = flagScene[1] = 0;//hello

	//-----------------Motion Result-----------------------------
	switch (motionFlag) {
	case MotionScene::TYPE_JITTER:
//		if ((LightScene::TYPE_ULTRA_LOWLIT == lightFlag)
//				||(LightScene::TYPE_LOWLIT == lightFlag)) {
//			flagScene[1] = FLAG_DETECTION_JITTERING;
//		}
		flagScene[1] = FLAG_DETECTION_JITTERING;
		break;
	case MotionScene::TYPE_MOVE:
//		if ((LightScene::TYPE_ULTRA_LOWLIT != lightFlag)
//				&&(LightScene::TYPE_LOWLIT != lightFlag)) {
//			flagScene[1] = FLAG_DETECTION_MOVE;
//		}
		flagScene[1] = FLAG_DETECTION_MOVE;
		break;
	case 0:
	default:
		break;
	}

	//-----------------Light Result-----------------------------
	switch (lightFlag) {
	case LightScene::TYPE_BACKLIT:
		flagScene[0] = FLAG_DETECTION_BACKLIGHT;
		break;
	case LightScene::TYPE_ULTRA_LOWLIT:
		flagScene[0] = FLAG_DETECTION_ULTRA_LOWLIT;
		break;
	case LightScene::TYPE_LOWLIT:
		flagScene[0] = FLAG_DETECTION_LOWLIT;
		break;
	default:
		break;
	}

	//-----------------Portrait Result-----------------------------
	switch (portraitFlag) {
	case PortraitScene::TYPE_PORTRAIT:
		flagScene[0] = FLAG_DETECTION_PORTRAIT;
		break;
//	case SceneType.Portrait.TYPE_PORTRAIT_GROUP:
//		flagScene = FLAG_DETECTION_PORTRAIT_GROUP;
//		break;
	default:
		break;
	}

	//-----------------Portrait-lit Result-----------------------------
	switch (portraitLitFlag) {
	case PortraitLitScene::TYPE_PORTRAIT_BACKLIT:
		flagScene[0] = FLAG_DETECTION_PORTRAIT_BACKLIT;
		break;
	case PortraitLitScene::TYPE_PORTRAIT_LOWILIT:
		flagScene[0] = FLAG_DETECTION_PORTRAIT_LOWLIT;
		break;
	case PortraitLitScene::TYPE_NORMAL:
		if (portraitFlag == PortraitScene::TYPE_PORTRAIT)
			flagScene[0] = FLAG_DETECTION_PORTRAIT;
		break;
	default:
		break;
	}

	//-----------------Result Check-----------------------------
	if (flagScene[0] != FLAG_DETECTION_NORMAL && flagScene[0] != FLAG_DETECTION_LOWLIT) {
		return; //if scene don't equal normal or lowlit scene, then return
	}

	//-----------------Landscape Result-----------------------------
	switch (landscapeFlag) {
	case LandscapeScene::TYPE_LANDSCAPE:
		flagScene[0] = FLAG_DETECTION_LANDSCAPE;
		break;
	default:
		break;
	}

	//-----------------Food Result-----------------------------
	switch (foodFlag) {
	case FoodScene::TYPE_FOOD:
		flagScene[0] = FLAG_DETECTION_FOOD;
		break;
	default:
		break;
	}

	//-----------------Document Result-----------------------------
	switch (documentFlag) {
	case DocumentScene::TYPE_DOCUMENT:
		flagScene[0] = FLAG_DETECTION_DOCUMENT;
		break;
	default:
		break;
	}

	//-----------------QRCode Result-----------------------------
	switch (codeFlag) {
	case CodeScene::TYPE_QRCODE:
		flagScene[0] = FLAG_DETECTION_QRCODE;
		break;
	default:
		break;
	}

	SMART_DEBUG("scene_result Flag1 = %d, Flag2  = %d\n", flagScene[0], flagScene[1]);
}
/**
 * downsize input data
 * @param input   input data for downsize
 * @param output  output data for downsize
 * @param METHOD  downsize mode:cv::INTER_LINEAR|cv::INTER_NEAREST
 * @return
 */
void downsize(const Image* input, Image* output, const int METHOD) {
	cv::Mat gray = cv::Mat(input->height, input->width, CV_8UC1, input->data);
	cv::Mat dst = cv::Mat(output->height, output->width, CV_8UC1);

	cv::resize(gray, dst, cv::Size(dst.cols, dst.rows), 0, 0, METHOD);

	memcpy(output->data, dst.data, dst.cols * dst.rows);
}
/**
 * decode the UV data from YUV data (NV21)
 * @param yuv   input YUV data
 * @param u     output U data
 * @param v     output V data
 * @return
 */
void get_uv(const Image* yuv, Image* u, Image* v) {
	int start = yuv->height * yuv->width;
	int end = start * 3 / 2;

	int u_indx = 0;
	int v_indx = 0;
	for (int i = start; i < end; i += 2) {
		v->data[v_indx++] = yuv->data[i];
		u->data[u_indx++] = yuv->data[i + 1];
	}
}
/**
 * portrait lit check list
 * @param scene_type      scene type
 * @param flashType       flash type
 * @return                result of check list
 */
int PortraitTable(int scene_type, int flashType) {

	if (scene_type == LightScene::TYPE_LOWLIT && flashType == 1)
		return PortraitLitScene::TYPE_PORTRAIT_LOWILIT;
	if (scene_type == LightScene::TYPE_LOWLIT && flashType == 2)
		return PortraitLitScene::TYPE_PORTRAIT_LOWILIT;
	if (scene_type == LightScene::TYPE_LOWLIT && flashType == 3)
		return PortraitLitScene::TYPE_PORTRAIT_LOWILIT;
	if (scene_type == LightScene::TYPE_LOWLIT && flashType == 4)
		return PortraitLitScene::TYPE_PORTRAIT_LOWILIT;
	if (scene_type == LightScene::TYPE_ULTRA_LOWLIT && flashType == 1)
		return PortraitLitScene::TYPE_PORTRAIT_LOWILIT;
	if (scene_type == LightScene::TYPE_ULTRA_LOWLIT && flashType == 2)
		return PortraitLitScene::TYPE_PORTRAIT_LOWILIT;
	if (scene_type == LightScene::TYPE_ULTRA_LOWLIT && flashType == 3)
		return PortraitLitScene::TYPE_PORTRAIT_LOWILIT;
	if (scene_type == LightScene::TYPE_ULTRA_LOWLIT && flashType == 4)
		return PortraitLitScene::TYPE_PORTRAIT_LOWILIT;
	return scene_type;
}
/**
 * judge if the camera angle is in the specific range for landscape detection
 * @param oangle        camera orientation[0, 90, 180, 270]
 * @param oData         orientation sensor data
 * @param relaxiAngle   angle range
 * @return              true:enable|false:disable
 */
bool LandscapeOriLimitEnable(const int oangle, const float* oData, const int relaxiAngle) {
	//根据手机方向调剂倾角阈值
	bool isOriatLimit = false;

	if (oData == NULL)
		return false;

	int OThresh1, OThresh2;
	switch (oangle) {
	case 0:
		OThresh1 = -90 - relaxiAngle;
		OThresh2 = -90 + relaxiAngle;
		if (oData[1] > OThresh1 && oData[1] < OThresh2) {
			isOriatLimit = true;
		}
		break;
	case 90:
		OThresh1 = -90;
		OThresh2 = -90 + relaxiAngle;
		if (oData[2] > OThresh1 && oData[2] < OThresh2) {
			isOriatLimit = true;
		}
		break;
	case 180:
		OThresh1 = 90 - relaxiAngle;
		OThresh2 = 90 + relaxiAngle;
		if (oData[1] > OThresh1 && oData[1] < OThresh2) {
			isOriatLimit = true;
		}
		break;
	case 270:
		OThresh1 = 90 - relaxiAngle;
		OThresh2 = 90;
		if (oData[2] > OThresh1 && oData[2] < OThresh2) {
			isOriatLimit = true;
		}
		break;
	}
	return isOriatLimit;
}
/**
 * judge if the camera angle is in the specific range when looking down
 * @param oangle        camera orientation[0, 90, 180, 270]
 * @param oData         orientation sensor data
 * @param relaxiAngle   angle range
 * @return              true:enable|false:disable
 */
bool LTLookDownAngleT(int oangle, const float* oData, int relaxiAngleLd) {
	//根据手机方向调剂倾角阈值
	bool isOriatLimit = true;

	if (oData == NULL)
		return true;

	switch (oangle) {
	case 0:
		if (oData[1] > -relaxiAngleLd && oData[1] < 0) {
			isOriatLimit = false;
		}
		break;
	case 90:
		if (oData[2] > -relaxiAngleLd && oData[2] < 0 && abs(oData[1]) < 90) { //确保是俯视角，且角度小于一个阈值
			isOriatLimit = false;
		}
		break;
	case 180:
		if (oData[1] > 0 && oData[1] < relaxiAngleLd) {
			isOriatLimit = false;
		}
		break;
	case 270:
		if (oData[2] > 0 && oData[2] < relaxiAngleLd && abs(oData[1]) < 90) { //确保是俯视角，且角度小于一个阈值
			isOriatLimit = false;
		}
		break;
	}
	return isOriatLimit;
}
/**
 * judge if the camera angle is in the specific range(60°) for food detection
 * @param oangle        camera orientation[0, 90, 180, 270]
 * @param oData         orientation sensor data
 * @return              true:enable|false:disable
 */
bool FoodOriLimitEnable(const int oangle, const float* oData) {
	int relaxiAngle = 60;//the specific range
	bool isOriatLimit = false;

	if (oData == NULL)
		return false;
	int OThresh1, OThresh2;
	switch (oangle) {
	case 0:
		OThresh1 = -relaxiAngle;
		OThresh2 = 20;
		if (oData[1] > OThresh1 && oData[1] < OThresh2) {
			isOriatLimit = true;
		}
		break;
	case 90:
		OThresh1 = -relaxiAngle;
		OThresh2 = 20;
		if (oData[2] > OThresh1 && oData[2] < OThresh2) {
			isOriatLimit = true;
		}
		break;
	case 180:
		OThresh1 = -20;
		OThresh2 = relaxiAngle;
		if (oData[1] > OThresh1 && oData[1] < OThresh2) {
			isOriatLimit = true;
		}
		break;
	case 270:
		OThresh1 = -20;
		OThresh2 = relaxiAngle;
		if (oData[2] > OThresh1 && oData[2] < OThresh2) {
			isOriatLimit = true;
		}
		break;
	}
	return isOriatLimit;
}
//****************************************************************************************************/
//**************************************** public function *******************************************/
/**
 * SmartRecognition Constructor
 */
SmartRecognition::SmartRecognition() /*:*/
//		dataYUV(new unsigned char[MAX_LENGTH / 4]),
//		dataThld(new unsigned char[MAX_LENGTH / 4 / 7 / 7]),
//		dataROI(new unsigned char[MAX_LENGTH / 4]),
//		landscape_gist_score(new LandscapeGistScore()),
//		tracker(new MFTracking()),
//		code(new Image(DownsampleWidth16_9, DownsampleHeight16_9, 640 * 480)), //720x405
//		boofcv(new Image(BoofCVWidth16_9, BoofCVHeight16_9, 320 * 240)),       //360x202
//		boofcv_u(new Image(BoofCVWidth16_9, BoofCVHeight16_9, 320 * 240)),
//		boofcv_v(new Image(BoofCVWidth16_9, BoofCVHeight16_9, 320 * 240)),
//		datay(new Image(MATRIX_WIDTH, MATRIX_HEIGHT)),                         //64X64
//		datay32(new Image(DownsampleVarWidth, DownsampleVarHeight)),           //32X32
//		datau1_1(new Image(MATRIX_WIDTH, MATRIX_HEIGHT)),
//		datav1_1(new Image(MATRIX_WIDTH, MATRIX_HEIGHT)),
//		datau16_9(new Image(uv2Width16_9, uv2Height16_9, 64 * 48)),            //64x36
//		datav16_9(new Image(uv2Width16_9, uv2Height16_9, 64 * 48))             //64x36
{

	dataYUV   = new unsigned char[MAX_LENGTH / 4] ;
	dataThld  = new unsigned char[MAX_LENGTH / 4 / 7 / 7] ;
	dataROI   = new unsigned char[MAX_LENGTH / 4] ;

#ifdef _SMART_LANDSCAPE
	landscape_gist_score = new LandscapeGistScore();
#endif
	tracker   = new MFTracking() ;
	code      = new Image(DownsampleWidth16_9, DownsampleHeight16_9, 640 * 480) ;//720x405
	boofcv    = new Image(BoofCVWidth16_9, BoofCVHeight16_9, 320 * 240) ;       //360x202
	boofcv_u  = new Image(BoofCVWidth16_9, BoofCVHeight16_9, 320 * 240) ;
	boofcv_v  = new Image(BoofCVWidth16_9, BoofCVHeight16_9, 320 * 240) ;
	datay     = new Image(MATRIX_WIDTH, MATRIX_HEIGHT) ;                         //64X64
	datay32   = new Image(DownsampleVarWidth, DownsampleVarHeight);              //32X32
	datau1_1  = new Image(MATRIX_WIDTH, MATRIX_HEIGHT);						     //64X64
	datav1_1  = new Image(MATRIX_WIDTH, MATRIX_HEIGHT);
	datau16_9 = new Image(uv2Width16_9, uv2Height16_9, 64 * 48) ;                //64x36
	datav16_9 = new Image(uv2Width16_9, uv2Height16_9, 64 * 48) ;                //64x36

	/* public varaibles initialization */
	memset(ellipse_param, 0, sizeof(ellipse_param));
	memset(line_position, 0, sizeof(line_position));

	process_image_width = BoofCVWidth16_9;                                     //size of downsized data(used for ICG)
	process_image_height = BoofCVHeight16_9;

	is_ready_for_guide = false;                                                //ICG enable flag
	is_ready_for_gl_model = false;                                             //GPU render flag

	memset(tracked_rect_for_guide, 0, sizeof(tracked_rect_for_guide));
	memset(tracked_rect_for_gl_matrix, 0, sizeof(tracked_rect_for_gl_matrix));

	/** private variables initialization */
	image_width = 0;
	image_height = 0;

	//detect_portrait
	FaceHeight = 0.1f;                                  //ratio of face height

	//detect_motion
	Motion_ThresSubNum = 140;                           //threshold of difference between two adjacent frames, default:140
	Motion_ThresSubLow = 15;                            //threshold of subNum, default:15
	Motion_ThresAccLow = 0.5f;                          //threshold of acc data, default:0.5f
	Motion_ThresOccHigh = 1.5f;                         //threshold of occ data, default:1.5f

	//detect_light
	degreeBound[0][0] = 0.44;                           //threshold of light detection
	degreeBound[0][1] = 0.42;							//backlit

	degreeBound[1][0] = 0.45;                           //night
	degreeBound[1][1] = 0.43;

	degreeBound[2][0] = 0.45;                           //lowlit
	degreeBound[2][1] = 9.43;

	Light_ThresFocusDis = 0.2f;                         //threshold of focus distance, default:0.2f
	Light_LowRatio = 0.9f;                              //Ratio to Max Exp-Index, default:0.9
	Light_ThresExpLow = 235;                            //threshold of exp-index(lower), default:235
	Light_ThresExpHigh1 = 310;                          //threshold of exp-index(upper), default:310
	Light_ThresLuxHigh = 5;                             //threshold of lux(upper1), default:5
	Light_ThresLuxHigh1 = 200;                          //threshold of lux(upper2), default:200
	Light_BaseExpIndex = 394;                           //Max Exp-Index parameter, default:394
	Light_BaseLuxIndex = 500;                           //Max lux-Index parameter, default:500
	Light_Offset = 1;                                   //Offset to Max Exp-Index, default:1

	//detect_landscape
	Landscape_ThresFocusDis = 0.25f;                    //threshold of focus distance, default:0.25f
	Landscape_ThresISPExpHigh = 230;                    //threshold of exp-index(upper), default:230
	//parameters of landscape detection(Gist)
	gistIspThresh1 = 60;                                //lower limit of isp data for landscape detection(Gist), default:60
	gistIspThresh2 = 190;                               //upper limit of isp data for landscape detection(Gist), default:190
	gistOrientationThresh = 20;                         //threshold of angle between camera and ground, default:20  手机与地面的夹角活动范围
	gistLandscapeTheshold_H[0] = 0.65f;                 //gist degree of landscape when camera is in landscape mode 横屏判断为风景的gist分值
	gistLandscapeTheshold_H[1] = 0.45f;
	gistLandscapeTheshold_V[0] = 0.8f;                  //gist degree of landscape when camera is in portrait mode 竖屏判断为风景的gist分值
	gistLandscapeTheshold_V[1] = 0.65f;
	gistFocusDisThresh = 1.0f;                          //threshold of focus distance, default:1.0f 对焦距离阈值
	omitbklandGistThreshold = 0.0f;                     //threshold of gist for filter backlit from landscape, default:0.0f 去掉逆光风景的gist阈值
	gistScoreAvalible = false;                          //gistScore computing flag 判断是否计算了gistScore
	bGistDetectLandscape = false;	                    //flag of landscape detection by Gist
	bLinedetectEnalbe = false;                          //line detection enable flag 判断是否开启直线引导
	mBlueSkey = 0.0;                                    //blue score of landscape detection 风景识别-蓝色分量
	mGreenGround = 0.0;                                 //green score of landscape detection 风景识别-绿色分量
	m_landscape_green_t = 14;                           //threshold of green score for landscape detection, default:14  风景识别绿色值的阈值
	m_landscape_blue_t = 40;                            //threshold of blue score for landscape detection, default:40    风景识别蓝色值的阈值
	m_landscape_rank_t = 60;                            //threshold of total score for landscape detection, default:60  风景识别蓝色值的阈值

	//detect_code

	_code_str = " ";
	_code_type = " ";
	_code_format = 0;

	//detect_food
	_food_attractiveness = 0;                           //food attractiveness score

	isp_ExpIndex = 0.0f;
	isp_Lux = 0.0f;
	isp_FocusDis = 0.0f;
	track_frame = 0;
	detect_sequence_index = 0;
	is_normal_sequence = true;
	for (int i = 0; i < 4; ++i)
		detect_sequence[i] = i + 1;

	flashStatus = 3;
	scene_flags[0] = 0;
	scene_flags[1] = 0;
	scene_enable_flag = 0;

	is_tracked = false;
	is_line_tracking_mode = false;

	isEllipseRedetected = false;
	isEllipseRedetectedTrigger= false;
	init();
}
/**
 * SmartRecognition Destructor
 */
SmartRecognition::~SmartRecognition() {
#ifdef _SMART_LANDSCAPE
	delete landscape_gist_score;
	landscape_gist_score = NULL;
#endif
	delete tracker;
	tracker = NULL;

	delete[] dataYUV;
	delete[] dataThld;
	delete[] dataROI;

	delete code;
	delete boofcv;
	delete boofcv_u;
	delete boofcv_v;
	delete datay32;
	delete datay;
	delete datau1_1;
	delete datav1_1;
	delete datau16_9;
	delete datav16_9;

	mAccList.clear();
	mEllpiseList.clear();
	mImgList.clear();
	mMotionList.clear();
	mLightList.clear();
	mFoodList.clear();
}
/**
 * enable the specific scene recognition
 * @param bit  scene type (defined in include/smart_type.h)
 * @return
 */
bool SmartRecognition::enable_scene(const int bit) {
	scene_enable_flag |= bit;
	return true;
}
/**
 * disable the specific scene recognition
 * @param bit  scene type (defined in include/smart_type.h)
 * @return
 */
bool SmartRecognition::disable_scene(const int bit) {
	scene_enable_flag &= ~bit;
	return true;
}
/**
 * judge whether the specific scene is enabled
 * @param bit  scene type (defined in include/smart_type.h)
 * @return true:enable|false:disable
 */
bool SmartRecognition::is_enable(const int bit) {
	return (scene_enable_flag & bit) == bit;
}
/**
 * do the scene detection
 * @param input  input data
 * @param width  width of input data
 * @param height height of input data
 * @return results of scene detection
 */
int SmartRecognition::detect_scene(unsigned char* input, int width, int height) {

	image_width = width;
	image_height = height;

	input_frame = new Image(input, width, height, width * height * 3 / 2);     //construct input data

	//tracking & recognition pipeline
	if (is_tracking_mode()) {
		SMART_DEBUG("detect_scene do tracking ...");
		is_ready_for_guide = true;                                             //ICG and GPU render enable only in tracking pipeline
		is_ready_for_gl_model = true;
		//modified 20160107 no motion detect when tracking, motion flag reset
		scene_type.get_motion().flag = MotionScene::TYPE_NORMAL;

		do_tracking(input_frame);

		is_ready_for_guide = is_tracked;
		is_ready_for_gl_model = is_tracked;
//		 SMART_DEBUG("is_ready_for_gl_model  = %d", is_tracked==true ? 1: 0);
	} else {
		SMART_DEBUG("detect_scene do detection ...");
		is_ready_for_guide = false;                                            //ICG and GPU render disable in recognition pipeline
		is_ready_for_gl_model = false;

		long beginTime = clock();
		do_detection(input_frame);
		long endTime = clock() - beginTime;
	}

	int flag = encodeSceneResult(scene_type);
	SMART_DEBUG("detect_scene flag = %d", flag);
	decodeSceneResult(flag, scene_flags);		                               //get final scene result
	delete input_frame;
	SMART_DEBUG("detect_scene flag = %d, %d", scene_flags[0], scene_flags[1]);
	return scene_flags[0];
}
/**
 * get the results of scene recognition
 * 0:light/portrait/portrait-lit/QRcode/landscape/food/document
 * 1:motion/jitter
 * @return array of results
 */
int* SmartRecognition::get_scene_flag() {
	SMART_DEBUG("scene_result flag = %d, %d", scene_flags[0], scene_flags[1]);
	return scene_flags;
}
/**
 * get the attractiveness of food
 * @return attractiveness result
 */
float SmartRecognition::get_food_attractiveness()
{
	return _food_attractiveness;
}
/**
 * get the content of QRCode
 * @return
 */
std::string&  SmartRecognition::get_qrcode_decoder_str()
{
	return _code_str;
}
/**
 * get the type of QRCode
 * @return
 */
std::string& SmartRecognition::get_qrcode_decoder_type()
{
	return _code_type;
}

/**
 * get code format
 * @return        code format:barcode | QRcode
 */
int SmartRecognition::get_code_format()
{
	return _code_format;
}

//****************************************************************************************************/
//**************************************** private function *******************************************/

/**
 * check the preview ratio is 16:9 or 4:3, adjust parameters
 * @param width   preview width
 * @param height  preview height
 * @return        true:success to check|false:fail to check
 */
bool SmartRecognition::preview_ratio_check(const int width, const int height){
	float preview_ratio = (float)width/height;
		if(preview_ratio > 1.72f && preview_ratio < 1.82f){
			process_image_width = BoofCVWidth16_9;                                     //size of downsized data(used for ICG)
			process_image_height = BoofCVHeight16_9;
			code->width = DownsampleWidth16_9;
			code->height = DownsampleHeight16_9;
			boofcv->width = BoofCVWidth16_9;
			boofcv->height = BoofCVHeight16_9;
			boofcv_u->width = BoofCVWidth16_9;
			boofcv_u->height = BoofCVHeight16_9;
			boofcv_v->width = BoofCVWidth16_9;
			boofcv_v->height = BoofCVHeight16_9;
			datau16_9->width = uv2Width16_9;
			datau16_9->height = uv2Height16_9;
			datav16_9->width = uv2Width16_9;
			datav16_9->height = uv2Height16_9;
			return true;
		}else if(preview_ratio > 1.28f && preview_ratio < 1.38f){
			process_image_width = BoofCVWidth4_3;                                     //size of downsized data(used for ICG)
			process_image_height = BoofCVHeight4_3;
			code->width = DownsampleWidth4_3;
			code->height = DownsampleHeight4_3;
			boofcv->width = BoofCVWidth4_3;
			boofcv->height = BoofCVHeight4_3;
			boofcv_u->width = BoofCVWidth4_3;
			boofcv_u->height = BoofCVHeight4_3;
			boofcv_v->width = BoofCVWidth4_3;
			boofcv_v->height = BoofCVHeight4_3;
			datau16_9->width = uv2Width4_3;
			datau16_9->height = uv2Height4_3;
			datav16_9->width = uv2Width4_3;
			datav16_9->height = uv2Height4_3;
			return true;
		}else{
			return false;
		}
}
/**
 * initialize recognition configuration
 * @return
 */
void SmartRecognition::init() {

//	enable_scene(
//    SMART_SCENE_MOTION |
//	SMART_SCENE_PORTRAIT |
//	SMART_SCENE_LIGHT |
//	SMART_SCENE_LANDSCAPE |
//	SMART_SCENE_FOOD |
//	SMART_SCENE_PORTRAITLIT  |
//	SMART_SCENE_DOCUMENT |
//	SMART_SCENE_CODE
//	);


#ifdef _SMART_FOOD
	enable_scene(SMART_SCENE_FOOD);
#else
	disable_scene(SMART_SCENE_FOOD);
#endif

#ifdef _SMART_MOTION
	enable_scene(SMART_SCENE_MOTION);
#else
	disable_scene(SMART_SCENE_MOTION);
#endif

#ifdef _SMART_DOCUMENT
	enable_scene(SMART_SCENE_DOCUMENT);
#else
	disable_scene(SMART_SCENE_DOCUMENT);
#endif

#ifdef _SMART_CODE
	enable_scene(SMART_SCENE_CODE);
#else
	disable_scene(SMART_SCENE_CODE);
#endif

#ifdef _SMART_LANDSCAPE
	enable_scene(SMART_SCENE_LANDSCAPE);
#else
	disable_scene(SMART_SCENE_LANDSCAPE);
#endif


#ifdef _SMART_LIGHT
	enable_scene(SMART_SCENE_LIGHT);
#else
	disable_scene(SMART_SCENE_LIGHT);
	disable_scene(SMART_SCENE_PORTRAITLIT);
#endif

#ifdef _SMART_PORTRAIT
	enable_scene(SMART_SCENE_PORTRAIT);
#else
	disable_scene(SMART_SCENE_PORTRAIT);
	disable_scene(SMART_SCENE_PORTRAITLIT);
#endif

#ifdef _SMART_PORTRAITLIT
	enable_scene(SMART_SCENE_PORTRAITLIT);
	#ifndef _SMART_LIGHT
		disable_scene(SMART_SCENE_PORTRAITLIT);
	#endif

	#ifndef _SMART_PORTRAIT
		disable_scene(SMART_SCENE_PORTRAITLIT);
	#endif
#else
	disable_scene(SMART_SCENE_PORTRAITLIT);
#endif


	extern_manager = SmartExternManager::get_instance();

	mAccList.resize(LIST_ACC_LENGTH, 0.f);
	mImgList.resize(LIST_IMG_LENGTH, 0);
	mMotionList.resize(LIST_MOTION_LENGTH, 0);
	mLightList.resize(LIST_LIGHT_LENGTH, 0);
	mFoodList.resize(LIST_FOOD_LENGTH, cv::Point2i(0, 0));
	mEllpiseList.resize(LIST_FOOD_LENGTH, 0.f);

	//tracking paramters
	MFTrackingParam tracking_param;
	tracking_param.matched_points_thresh = 4;
	tracking_param.optical_flow_pyramid_levels = 3;
	tracking_param.tracking_point_cols = 10;
	tracking_param.tracking_point_rows = 10;
	tracker->set_parameter(TRACKING_PARAM_MFT, (void*) &tracking_param);//set paramters for tracker
}
/**
 * judge the scene type for tracking
 * true if the scene is portrait/food/landscape(detect line)/QRcode
 * @return ture:on-tracking|false:off-tracking
 */
bool SmartRecognition::is_tracking_mode() {
	switch (scene_flags[0]) {
	case FLAG_DETECTION_PORTRAIT:
//	case FLAG_DETECTION_PORTRAIT_BACKLIT:
//	case FLAG_DETECTION_PORTRAIT_LOWLIT:
	case FLAG_DETECTION_FOOD:
		return true;
		break;
	case FLAG_DETECTION_LANDSCAPE:
		if (is_line_tracking_mode) {
			return true;
		}
		break;
	case FLAG_DETECTION_QRCODE:
		return true;
		break;
	default:
		return false;
	}
}
/**
 * Tracking pipeline: data process & tracking
 * @param input  input data for tracking
 * @return
 */
void SmartRecognition::do_tracking(Image* input) {
	static bool first_tracked = true;
	static int ellipseFrame = 0;

	preview_ratio_check(input->width, input->height);

	SMART_DEBUG("xxx-first_tracked=%d", first_tracked);
	if (!first_tracked && track_frame++ % 2 == 0) {
		// do data downsize every other frame (frame 1)
		// long start = clock();
		downsize_for_tracking(input, boofcv);                                  //for portrait/food/line tracking and ellipse detection, attractiveness computation
		downsize_for_tracking(input, code);                                    //for QRcode detection
		downsize_for_tracking_uv(input, boofcv_u, boofcv_v);                   //for attractiveness computation
		// long during = clock() - start;
		// SMART_DEBUG("yyy---during=%d", during);
	} else {
		//do tracking every other frame (frame 2)
		if(scene_flags[0] == FLAG_DETECTION_FOOD){
			//food tracking pipeline
			ellipseFrame ++;
			isEllipseRedetected = false;

			if(ellipseFrame >= 5){
				//do ellipse detection and attractiveness compute every 5 frames
#ifdef _SMART_FOOD
				EllipseDetection ellipse_detector;

				//frame flag reset
				ellipseFrame = 0;

				//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
				//ellipse_detector initialization
				SMART_DEBUG("fooddetect centre1_x = %f , centre1_y = %f" ,
						(float)(tracked_rect_for_guide[0]+(tracked_rect_for_guide[2]-tracked_rect_for_guide[0])/2) ,
						(float)(tracked_rect_for_guide[1]+(tracked_rect_for_guide[3]-tracked_rect_for_guide[1])/2) );
//				SMART_DEBUG("fooddetect track_bb [  %d , %d , %d , %d ]" ,
//						tracked_rect_for_guide[0],
//						tracked_rect_for_guide[1],
//						tracked_rect_for_guide[2],
//						tracked_rect_for_guide[3]);
				cv::Point2f ellicentre = cv::Point2f(
						(float)(tracked_rect_for_guide[0]+(tracked_rect_for_guide[2]-tracked_rect_for_guide[0])/2) ,
						(float)(tracked_rect_for_guide[1]+(tracked_rect_for_guide[3]-tracked_rect_for_guide[1])/2) );
				ellipse_detector.set_parameter(DETECTION_PARAM_ELLIPSE_DETECT_INITCENTRE, &ellicentre);         // param 2 *

				float fMaxDisFromInitCentre = (float)(boofcv->width * boofcv->width + boofcv->height * boofcv->height ) * 0.001;
				ellipse_detector.set_parameter(DETECTION_PARAM_ELLIPSE_DETECT_MAXDISFROMINITCENTRE,&fMaxDisFromInitCentre);   // param 3 *

				float fInitB = (tracked_rect_for_guide[2]-tracked_rect_for_guide[0])/2;
				ellipse_detector.set_parameter(DETECTION_PARAM_ELLIPSE_DETECT_INITB,&fInitB);                                 // param 4 *

				float fTolRatioB = 0.25f;
				ellipse_detector.set_parameter(DETECTION_PARAM_ELLIPSE_DETECT_TOLRATIOB,&fTolRatioB);                         // param 5 *

				float mfMinScore = 0.7f;
				ellipse_detector.set_parameter(DETECTION_PARAM_ELLIPSE_DETECT_MINSCORE,&mfMinScore);                          // param 6 *

				float mfMinReliability = 0.7f;
				ellipse_detector.set_parameter(DETECTION_PARAM_ELLIPSE_DETECT_MINRELIABILITY,&mfMinReliability);              // param 7 *

				//ellipse_detector start(ellipse detect and attractiveness compute)
				int res = ellipse_detector.obj_attractiveness_detect(boofcv,boofcv_u, boofcv_v, (void *) ellipse_param, _food_attractiveness);

				SMART_DEBUG("fooddetect centre2_x = %f , centre2_y = %f" ,(float)(ellipse_param[1]) ,(float)(ellipse_param[2]) );
				SMART_DEBUG("fooddetect res = %d" , res);

				//enable the bounding-box update flag, update the bounding-box in the next frame
				if(res == 1){
					isEllipseRedetected  = true;
					isEllipseRedetectedTrigger = isEllipseRedetected;
				}else{
					isEllipseRedetected = false;
					isEllipseRedetectedTrigger = isEllipseRedetected;
				}

				//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#endif
			}else{
				//food tracking
				is_tracked = tracking(scene_flags[0]);
			}
		}else{
			//other tracking pipeline(portrait/line/QRCode)
			is_tracked = tracking(scene_flags[0]);
		}

		//first_tracked is true only when it's first tracked after each detection
		first_tracked = !is_tracked;

		//reset flag when track lost
		if (!is_tracked) {
			track_frame = 0;
			scene_flags[0] = 0;
			scene_type.reset();
		}
	}
}
/**
 * Recognition pipeline: data process & scene recognition
 * @param input  input data for detection
 * @return
 */
void SmartRecognition::do_detection(Image* input) {

	//detection control flag
	int do_what = detect_sequence[detect_sequence_index];

	long beginTime;
	long endTime;

	preview_ratio_check(input->width, input->height);

	SMART_DEBUG("process data size, %d * %d, %d * %d, %d * %d, %d * %d, %d * %d, %d * %d ",
			    code->height, code->width,
			    boofcv->height, boofcv->width,
			    boofcv_u->height, boofcv_u->width,
			    boofcv_v->height, boofcv_v->width,
			    datau16_9->height, datau16_9->width,
			    datav16_9->height, datav16_9->width
			     );
	//get light sensor and isp sensor data
	lLux = extern_manager->l_sensor[0];
	int ori_raw = extern_manager->orientation_raw;
	int ori = extern_manager->orientation;
	SMART_DEBUG("L sensor: %f ,raw_ori: %d, ori: %d", lLux, ori_raw, ori);

	if (extern_manager->is_isp_usable) {
		float* ispData = extern_manager->isp_data;
		isp_ExpIndex = ispData[3];
		isp_Lux = ispData[5];
		isp_FocusDis = ispData[11];
		SMART_DEBUG("isp enabled, %f, %f, %f", isp_ExpIndex, isp_Lux, isp_FocusDis);
	}
#ifdef DEBUG_VALUE
	debug_value[0] = extern_manager->isp_data[0];
	debug_value[1] = extern_manager->isp_data[1];
	debug_value[2] = extern_manager->isp_data[2];
	debug_value[3] = extern_manager->isp_data[3]
	debug_value[4] = extern_manager->isp_data[4];
	debug_value[5] = extern_manager->isp_data[5];
	debug_value[6] = extern_manager->isp_data[6];
	debug_value[7] = extern_manager->isp_data[7];
	debug_value[8] = extern_manager->isp_data[8];
	debug_value[9] = extern_manager->isp_data[9];
	debug_value[10] = extern_manager->isp_data[10];
	debug_value[11] = extern_manager->isp_data[11];
	debug_value[12] = extern_manager->isp_data[12];
	debug_value[13] = extern_manager->l_sensor[0];
	debug_value[14] = extern_manager->orientation_raw;
#endif
	switch (do_what) {
	case 1:
	{
	/**** down size process********************************************************************************/
		beginTime = clock();

		downsize_for_detection(input, code, boofcv, datay, datay32, boofcv_u, boofcv_v, datau1_1, datav1_1, datau16_9, datav16_9);

		endTime = clock() - beginTime;
		SMART_DEBUG("frame--1 downsize, time=%d", (int)( endTime/1000 ) );
	}
	{
	/**** detect_document ********************************************************************************/
#ifdef _SMART_DOCUMENT
//		beginTime = clock();
		if (is_enable(SMART_SCENE_DOCUMENT)) {
			scene_type.get_document().flag = detect_document(datay32);
		} else {
			scene_type.get_document().reset();
		}
//		endTime = clock() - beginTime;
//		//SMART_DEBUG("frame--1 detect_document, time=%d", endTime/1000);
#endif
	}
	{
	/**** detect_motion ********************************************************************************/
#ifdef _SMART_MOTION
//		beginTime = clock();
		if (is_enable(SMART_SCENE_MOTION)) {
			scene_type.get_motion().flag = detect_motion(datay32, getAcc(), getOcc());
		} else {
			scene_type.get_motion().reset();
		}
//		endTime = clock() - beginTime;
//		//SMART_DEBUG("frame--1 detect_motion, time=%d", endTime/1000);
#endif
	}
	{
	/**** histogram process ********************************************************************************/
//		beginTime = clock();
		getHistogram(datay, datau1_1, datav1_1,
				     HISTO_SRC_BINNUM, HISTO_DST_BINNUM,
				     histoY_src, histoU_src, histoV_src,
				     histoY_dst);

//		endTime = clock() - beginTime;
//		SMART_DEBUG("frame--1 getHistogram, time=%d", endTime/1000);
	}
		break;
	case 2:
//		beginTime = clock();
	{
	/**** detect_code ********************************************************************************/
#ifdef _SMART_CODE
		// modified 20160107 detect QRCode on name card(change code data to original data)
		if (is_enable(SMART_SCENE_CODE)) {
			scene_type.get_code().flag = detect_code(input, mQRPos);
		} else {
			scene_type.get_code().reset();
		}
		//adjust the QR coordination to the size of tracking data(original size -> 720*405)
		if(scene_type.get_code().flag!=-1){
			float ratioQRCode = (float)code->width/input->width;
			SMART_DEBUG("QRCode ratio: %f ", ratioQRCode);
			for(int i = 0; i < 5; i++){
				mQRPos[2*i + 1] = mQRPos[2*i + 1]*ratioQRCode;
				mQRPos[2*i + 2] = mQRPos[2*i + 2]*ratioQRCode;
			}
			SMART_DEBUG("zxing qrcode markers 1-1: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
					mQRPos[1], mQRPos[2], mQRPos[3], mQRPos[4], mQRPos[5], mQRPos[6], mQRPos[7], mQRPos[8], mQRPos[9], mQRPos[10]);
		}
		//old QRCode detection algorithm by Bob Xu
//		if (is_enable(SMART_SCENE_CODE)) {
//			scene_type.get_code().flag = detect_code(code, mQRPos, dataYUV, dataThld, dataROI);
//		} else {
//			scene_type.get_code().reset();
//		}
#endif
	}
	{
	/**** detect_light ********************************************************************************/
		float blockY[BLOCK_SIDE * BLOCK_SIDE]; //7x7 1D block on channel Y
		util::get_block_1D(code, blockY, BLOCK_SIDE);

		if (is_enable(SMART_SCENE_LIGHT)) {
			SMART_DEBUG("-- detect_light -- ");
			scene_type.get_light().flag = detect_light(blockY, histoY_src, histoU_src, histoV_src, histoY_dst,
					                                   Light_BaseExpIndex, Light_BaseLuxIndex, Light_Offset, Light_LowRatio);
		} else {
			scene_type.get_light().reset();
		}
	}
	{
	/**** detect_portrait_lit ********************************************************************************/
		if (is_enable(SMART_SCENE_PORTRAITLIT)) {
			SMART_DEBUG("-- detect_portraitlit -- ");
			int params[10];
			scene_type.get_portrait_lit().flag = detect_portrait_lit(code,extern_manager->face_data, extern_manager->orientation, params);
		} else {
			scene_type.get_portrait_lit().reset();
		}
		//SMART_DEBUG("detect_portrait_lit test 1-1 flag =%d", scene_type.get_portrait_lit().flag);
	}
	{
	/**** detect_portrait ********************************************************************************/
		if (is_enable(SMART_SCENE_PORTRAIT)) {
			SMART_DEBUG("-- detect_portrait -- ");
			scene_type.get_portrait().flag = detect_portrait(extern_manager->face_data);
		} else {
			scene_type.get_portrait().reset();
		}

		//SMART_DEBUG("detect_portrait_lit test 1-2 flag =%d", scene_type.get_portrait_lit().flag);
	}

	//change the priority of detection order
//		if (scene_type.get_portrait().is_detected_type()||scene_type.get_code().is_detected_type()) {
//			detect_sequence[0] = 1;
//			detect_sequence[1] = 2;
//			detect_sequence[2] = 3;
//			detect_sequence[3] = 4;
//			is_normal_sequence = false;
//			detect_sequence_index = 0;
//			return;
//		}
//		endTime = clock() - beginTime;
//		//SMART_DEBUG("frame--2, time=%d", endTime/1000);
		break;
	case 3:
//		beginTime = clock();
	{
		/**** detect_motion ********************************************************************************/
#ifdef _SMART_MOTION
//		beginTime = clock();
		downsize(input,    datay32, cv::INTER_LINEAR);
		if (is_enable(SMART_SCENE_MOTION)) {
			scene_type.get_motion().flag = detect_motion(datay32, getAcc(), getOcc());
		} else {
			scene_type.get_motion().reset();
		}
//		endTime = clock() - beginTime;
//		//SMART_DEBUG("frame--1 detect_motion, time=%d", endTime/1000);
#endif
	}
	{
		SMART_DEBUG("-- detect_landscape -- ");
#ifdef _SMART_LANDSCAPE
		/**** detect_landscape ********************************************************************************/
		if (is_enable(SMART_SCENE_LANDSCAPE)) {
			scene_type.get_landscape().flag = detect_landscape(datay32, datau1_1, datav1_1);
		} else {
			scene_type.get_landscape().reset();
		}
#endif
	}
//		endTime = clock() - beginTime;
//		SMART_DEBUG("frame--3, time=%d", endTime/1000);
		break;
	case 4:
//		beginTime = clock();
	{
		/**** detect_food ********************************************************************************/

#ifdef _SMART_FOOD
		if (is_enable(SMART_SCENE_FOOD)) {
			scene_type.get_food().flag = detect_food(boofcv, datau16_9,	datav16_9);
		} else {
			scene_type.get_food().reset();
		}
#endif
	}
	{
#ifdef _SMART_LANDSCAPE
		/**** detect_line ********************************************************************************/
		if (bLinedetectEnalbe) {

			float leveltheta = util::levelAngle(extern_manager->orientation, extern_manager->orientation_raw);
			int line_num = detect_line(boofcv, line_position, extern_manager->orientation, leveltheta, 0, 7);

			if (line_num > 0) {
				is_line_tracking_mode = true;
			}
			SMART_DEBUG("detect_landscape LINE_NUM %d is_line_tracking_mode=%d", line_num, is_line_tracking_mode);
		}
#endif
	}
		//change the priority of detection order
//		if (scene_type.get_food().is_detected_type()) {
//			detect_sequence[0] = 1;
//			detect_sequence[1] = 4;
//			detect_sequence[2] = 2;
//			detect_sequence[3] = 3;
//			is_normal_sequence = false;
//			detect_sequence_index = 0;
//			return;
//		}
//		endTime = clock() - beginTime;
//		SMART_DEBUG("frame--4, time=%d", endTime/1000);
		break;
	}

	//reset the priority of detection order
//	if (!is_normal_sequence && detect_sequence_index == 3) {
//		detect_sequence[0] = 1;
//		detect_sequence[1] = 2;
//		detect_sequence[2] = 3;
//		detect_sequence[3] = 4;
//		is_normal_sequence = true;
//		detect_sequence_index = 0;
//		return;
//	}
	detect_sequence_index++;
	if (detect_sequence_index >= 4) {
		detect_sequence_index = 0;
	}
}
/**
 * downsize Y data for tracking
 * @param in   input data of downsize
 * @param out  output data of downsize
 * @return
 */
void SmartRecognition::downsize_for_tracking(const Image* input, Image* output) {
	int method1 = cv::INTER_LINEAR;
	int method2 = cv::INTER_NEAREST;

	downsize(input, output, method2);
}
/**
 * downsize UV data for tracking, mainly for attractiveness of food
 * @param input     input data of downsize
 * @param output_u  output data of U
 * @param output_v  output data of V
 * @return
 */
void SmartRecognition::downsize_for_tracking_uv(const Image* input,
		Image* output_u, Image* output_v) {

	int method1 = cv::INTER_LINEAR;
	int method2 = cv::INTER_NEAREST;

	Image* u = new Image(input->width * 0.5, input->height * 0.5);
	Image* v = new Image(input->width * 0.5, input->height * 0.5);
	/** get u and v from yuv */
	get_uv(input, u, v);

	downsize(u, output_u, method1); //->boofcv_u
	downsize(v, output_v, method1); //->boofcv_v
}
/**
 * tracking process
 * @param track_flag   scene type for tracking(defined in include/smart_type.h)
 * @return
 */
bool SmartRecognition::tracking(int track_flag) {
#define MEDIAN_FLOW_TRACKING
	//initialize bounding box for tracking
	if (!is_tracked) {
		int tmp_rect[5];
		//bounding box object
		MFTrackingBoundingBox bounding_box;

		switch (track_flag) {
		case FLAG_DETECTION_PORTRAIT:
		case FLAG_DETECTION_PORTRAIT_BACKLIT:
		case FLAG_DETECTION_PORTRAIT_LOWLIT:
			//MF tracking for portrait
			util::change_position_from_differnent_size(extern_manager->face_data, 1, extern_manager->preview_width,extern_manager->preview_height, boofcv->width,boofcv->height, 1, tmp_rect);
			//init bounding box object
			bounding_box.left = tmp_rect[1];
			bounding_box.top = tmp_rect[2];
			bounding_box.right = tmp_rect[3];
			bounding_box.bottom = tmp_rect[4];
			bounding_box.image_data = (char*) boofcv->data;
			bounding_box.image_height = boofcv->height;
			bounding_box.image_width = boofcv->width;
			tracker->set_parameter(TRACKING_PARAM_MFT_BOUNDING_BOX,(void*) &bounding_box);				//set bounding box
			SMART_DEBUG(
					"detect_landscape tracking update-bounding_box: 1(%d, %d), 2(%d, %d), scene_flags[0]=%d ",
					bounding_box.left, bounding_box.top, bounding_box.right,
					bounding_box.bottom, scene_flags[0]);

			break;
		case FLAG_DETECTION_FOOD:
#ifdef MEDIAN_FLOW_TRACKING //MF tracking for food
			//init bounding box object
			bounding_box.left   = ellipse_param[6];
			bounding_box.top    = ellipse_param[7];
			bounding_box.right  = ellipse_param[8];
			bounding_box.bottom = ellipse_param[9];
			bounding_box.image_data = (char*) boofcv->data;
			bounding_box.image_height = boofcv->height;
			bounding_box.image_width = boofcv->width;
			tracker->set_parameter(TRACKING_PARAM_MFT_BOUNDING_BOX,(void*) &bounding_box);			    //set bounding box
			SMART_DEBUG(
					"detect_landscape tracking update-bounding_box: 1(%d, %d), 2(%d, %d), scene_flags[0]=%d ",
					bounding_box.left, bounding_box.top, bounding_box.right,
					bounding_box.bottom, scene_flags[0])
			;
			break;
#else   //KLT tracking for food
		{
/*			//init bounding box object(not used:KLT tracking use minimal circumscribed rectangle of an ellipse(椭圆外切矩形) as bounding box
				                                Food detection transform inscribed rectangle of an ellipse(椭圆内接矩形)
				                                so could not use detection result to initialize KLT bounding box of food )*/
			cv::Mat inputIm = cv::Mat(boofcv->height, boofcv->width, CV_8UC1, (char*) boofcv->data);
			cv::Point2f markers[5];
			for (int i = 0; i < 4; i++) {
				markers[i].x = 0;
				markers[i].y = 0;
			}
			_food_tracker.init_tracking(inputIm, markers);
		}
			break;
#endif
		case FLAG_DETECTION_LANDSCAPE:
			//MF tracking for landscape
			if (is_line_tracking_mode) {
				//init bounding box object
				bounding_box.left = line_position[1];
				bounding_box.top = line_position[2];
				bounding_box.right = line_position[3];
				bounding_box.bottom = line_position[4];
				bounding_box.image_data = (char*) boofcv->data;
				bounding_box.image_height = boofcv->height;
				bounding_box.image_width = boofcv->width;
				tracker->set_parameter(TRACKING_PARAM_MFT_BOUNDING_BOX, (void*) &bounding_box);		    //set bounding box
			}
			break;
		case FLAG_DETECTION_QRCODE: {
#ifdef _SMART_CODE
			//KLT tracking for QRCode
//			SMART_DEBUG("zxing qrcode init tracker 1-1");
			cv::Mat inputIm = cv::Mat(code->height, code->width, CV_8UC1, (char*) code->data);
			cv::Point2f markers[5];
			for (int i = 0; i < 5; i++) {
				markers[i].x = mQRPos[i * 2 + 1];
				markers[i].y = mQRPos[i * 2 + 2];
			}
//			SMART_DEBUG("zxing qrcode markers 1-2: %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2, %.2f, %.2",
//					    markers[0].x, markers[0].y, markers[1].x, markers[1].y, markers[2].x, markers[2].y, markers[3].x, markers[3].y, markers[4].x, markers[4].y);
//			SMART_DEBUG("zxing qrcode init tracker 1-2");
			_qrcode_tracker.init_tracking(inputIm, markers);                                            //set bounding box
//			SMART_DEBUG("zxing qrcode markers 1-3: %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2, %.2f, %.2",
//								    markers[0].x, markers[0].y, markers[1].x, markers[1].y, markers[2].x, markers[2].y, markers[3].x, markers[3].y, markers[4].x, markers[4].y);
//			//SMART_DEBUG("zxing qrcode init tracker 1-3");
#endif
		}
			break;
		default:
			return false;
		}
	}

	//do tracking
	switch (track_flag) {
	case FLAG_DETECTION_PORTRAIT:
	case FLAG_DETECTION_PORTRAIT_BACKLIT:
	case FLAG_DETECTION_PORTRAIT_LOWLIT: {
		//MF tracking for portrait
		SvRect tracked_rect;
		is_tracked = tracker->obj_track((void*)boofcv->data, boofcv->width, boofcv->height, (void*)&tracked_rect);
		int tmp_tracked_rect[] = {tracked_rect.x, tracked_rect.y, tracked_rect.width + tracked_rect.x, tracked_rect.height + tracked_rect.y};

		if (is_tracked) {
			util::change_position_from_differnent_size(tmp_tracked_rect, 0,
					boofcv->width,boofcv->height,
					extern_manager->preview_width,
					extern_manager->preview_height,
					0, tracked_rect_for_guide);
		}
	}
	break;
	case FLAG_DETECTION_FOOD:
#ifdef MEDIAN_FLOW_TRACKING  //MF tracking for food
	{
		SvRect tracked_rect;
		is_tracked = tracker->obj_track((void*)boofcv->data, boofcv->width, boofcv->height, (void*)&tracked_rect);
		int tmp_tracked_rect[] = {tracked_rect.x, tracked_rect.y, tracked_rect.width + tracked_rect.x, tracked_rect.height + tracked_rect.y};
		SMART_DEBUG("foodtrackingbb_test_1 : %d, %d, %d, %d",tracked_rect.x,tracked_rect.y,tracked_rect.width,tracked_rect.height);
		if (is_tracked) {

			//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			//update bounding box while food tracking
			if (isEllipseRedetectedTrigger ){
				isEllipseRedetectedTrigger = false;
				//compare current bounding box center to the ellipse center
				if (   abs((tracked_rect.x + tracked_rect.width / 2)  - ellipse_param[1]) > 0.05*tracked_rect.width
					|| abs((tracked_rect.y + tracked_rect.height / 2) - ellipse_param[2]) > 0.05*tracked_rect.width
					|| abs(tracked_rect.width - 2 * ellipse_param[4]) > 0.05*tracked_rect.width){
					//Correcting the tracking position

					float weight = 0.9 - 0.4 * MIN(0.05 * tracked_rect.width, sqrt((ellipse_param[1] - tracked_rect.x)*(ellipse_param[1] - tracked_rect.x) + (ellipse_param[2] - tracked_rect.y)*(ellipse_param[2] - tracked_rect.y))) / (0.05 * tracked_rect.width);

					tracked_rect.x      = weight*tracked_rect.x      + (1 - weight)*(ellipse_param[1] - ellipse_param[4]);
					tracked_rect.y      = weight*tracked_rect.y      + (1 - weight)*(ellipse_param[2] - ellipse_param[4]);
					tracked_rect.width  = weight*tracked_rect.width  + (1 - weight)* 2 * ellipse_param[4];
					tracked_rect.height = weight*tracked_rect.height + (1 - weight)* 2 * ellipse_param[4];

					MFTrackingBoundingBox bounding_box;

					bounding_box.left   = tracked_rect.x ;
					bounding_box.top    = tracked_rect.y ;
					bounding_box.right  = tracked_rect.x + tracked_rect.width;
					bounding_box.bottom = tracked_rect.y + tracked_rect.height;
					bounding_box.image_data = (char*) boofcv->data;
					bounding_box.image_height = boofcv->height;
					bounding_box.image_width = boofcv->width;

					tracker->set_parameter(TRACKING_PARAM_MFT_BOUNDING_BOX,(void*) &bounding_box);      //set bounding box
//					SMART_DEBUG("foodtrackingbb_test_3: %d, %d, %d, %d",tracked_rect.x,tracked_rect.y,tracked_rect.width,tracked_rect.height);
//					SMART_DEBUG("fooddetect : centre3_x = %f , centre3_y = %f",(float)ellipse_param[1],(float)ellipse_param[2]);
				}
			}
			//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

			//update ICG parameter and GL render parameter
			SMART_DEBUG("foodtrackingbb_test_2 : %d, %d, %d, %d",tmp_tracked_rect[0],tmp_tracked_rect[1],tmp_tracked_rect[2],tmp_tracked_rect[3]);
			tracked_rect_for_guide[0] = tmp_tracked_rect[0]; //left
			tracked_rect_for_guide[1] = tmp_tracked_rect[1]; //top
			tracked_rect_for_guide[2] = tmp_tracked_rect[2]; //right
			tracked_rect_for_guide[3] = tmp_tracked_rect[3]; //Bottom
			tracked_rect_for_guide[4] = tmp_tracked_rect[0] + (tmp_tracked_rect[2] - tmp_tracked_rect[0])/2;
			tracked_rect_for_guide[5] = tmp_tracked_rect[1] + (tmp_tracked_rect[3] - tmp_tracked_rect[1])/2;

			int previewWidth = extern_manager->preview_width;
			int previewHeight = extern_manager->preview_height;
			//modified on 2016-04-12 output the position based on preview size from recognition module
			//gl_matrix output order: [left, bottom], [left, top], [right, top], [right, bottom]
			tracked_rect_for_gl_matrix[0] = ((float) tmp_tracked_rect[0] / boofcv->width)* previewWidth;
			tracked_rect_for_gl_matrix[1] = ((float) tmp_tracked_rect[3] / boofcv->height)* previewHeight;
			tracked_rect_for_gl_matrix[2] = ((float) tmp_tracked_rect[0] / boofcv->width)* previewWidth;
			tracked_rect_for_gl_matrix[3] = ((float) tmp_tracked_rect[1] / boofcv->height)* previewHeight;
			tracked_rect_for_gl_matrix[4] = ((float) tmp_tracked_rect[2] / boofcv->width)* previewWidth;
			tracked_rect_for_gl_matrix[5] = ((float) tmp_tracked_rect[1] / boofcv->height)* previewHeight;
			tracked_rect_for_gl_matrix[6] = ((float) tmp_tracked_rect[2] / boofcv->width)* previewWidth;
			tracked_rect_for_gl_matrix[7] = ((float) tmp_tracked_rect[3] / boofcv->height)* previewHeight;
		}
	}
	break;
#else  //KLT tracking for food
	{
		cv::Mat inputIm = cv::Mat(boofcv->height, boofcv->width, CV_8UC1, (char*)boofcv->data);
		cv::Point2f detected_markers[5];
		is_tracked = _food_tracker.tracking(inputIm, detected_markers);
		if(is_tracked)
		{
			float mEllipsePos[20];
			mEllipsePos[0] = 1;
			for(int i = 0; i < 5; i++)
			{
				mEllipsePos[i*2+1] = detected_markers[i].x;
				mEllipsePos[i*2+2] = detected_markers[i].y;
			}

			int elPos[5];
			elPos[1] = MIN(mEllipsePos[1], mEllipsePos[3]);
			elPos[2] = MIN(mEllipsePos[4], mEllipsePos[6]);
			elPos[3] = MAX(mEllipsePos[5], mEllipsePos[7]);
			elPos[4] = MAX(mEllipsePos[2], mEllipsePos[8]);
			int tmp_tracked_rect[] = {elPos[1], elPos[2], elPos[3], elPos[4]};
			tracked_rect_for_guide[0] = tmp_tracked_rect[0];
			tracked_rect_for_guide[1] = tmp_tracked_rect[1];
			tracked_rect_for_guide[2] = tmp_tracked_rect[2];
			tracked_rect_for_guide[3] = tmp_tracked_rect[3];
			tracked_rect_for_guide[4] = detected_markers[4].x;
			tracked_rect_for_guide[5] = detected_markers[4].y;

			SMART_DEBUG("detect food position %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f",mEllipsePos[1],
					mEllipsePos[2], mEllipsePos[3], mEllipsePos[4], mEllipsePos[5], mEllipsePos[6], mEllipsePos[7], mEllipsePos[8]);
			int previewWidth = extern_manager->preview_width;
			int previewHeight = extern_manager->preview_height;
			//modified on 2016-04-12 output the position based on preview size from recognition module
			//gl_matrix output order: [left, bottom], [left, top], [right, top], [right, bottom]
			tracked_rect_for_gl_matrix[0] = ((float)  mEllipsePos[1] / boofcv->width)* previewWidth;
			tracked_rect_for_gl_matrix[1] = ((float)  mEllipsePos[2] / boofcv->height)* previewHeight;
			tracked_rect_for_gl_matrix[2] = ((float)  mEllipsePos[3] / boofcv->width)* previewWidth;
			tracked_rect_for_gl_matrix[3] = ((float)  mEllipsePos[4] / boofcv->height)* previewHeight;
			tracked_rect_for_gl_matrix[4] = ((float)  mEllipsePos[5] / boofcv->width)* previewWidth;
			tracked_rect_for_gl_matrix[5] = ((float)  mEllipsePos[6] / boofcv->height)* previewHeight;
			tracked_rect_for_gl_matrix[6] = ((float)  mEllipsePos[7] / boofcv->width)* previewWidth;
			tracked_rect_for_gl_matrix[7] = ((float)  mEllipsePos[8] / boofcv->height)* previewHeight;
			SMART_DEBUG("detect food position %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f",tracked_rect_for_gl_matrix[0],
					tracked_rect_for_gl_matrix[1], tracked_rect_for_gl_matrix[2], tracked_rect_for_gl_matrix[3], tracked_rect_for_gl_matrix[4], tracked_rect_for_gl_matrix[5], tracked_rect_for_gl_matrix[6], tracked_rect_for_gl_matrix[7]);
		}
	}
	break;
#endif
	case FLAG_DETECTION_LANDSCAPE:
	{
		//MF tracking for landscape
		SvRect tracked_rect;
		is_tracked = tracker->obj_track((void*)boofcv->data, boofcv->width, boofcv->height, (void*)&tracked_rect);
		int tmp_tracked_rect[] = {tracked_rect.x, tracked_rect.y, tracked_rect.width + tracked_rect.x, tracked_rect.height + tracked_rect.y};

		if (is_tracked) {
			tracked_rect_for_guide[0] = tmp_tracked_rect[0];
			tracked_rect_for_guide[1] = tmp_tracked_rect[1];
			tracked_rect_for_guide[2] = tmp_tracked_rect[2];
			tracked_rect_for_guide[3] = tmp_tracked_rect[3];
		} else {
			is_line_tracking_mode = false;
			bLinedetectEnalbe = false;
		}
	}
	break;
	case FLAG_DETECTION_QRCODE:
	{
#ifdef _SMART_CODE
		//KLT tracking for QRCode
		cv::Mat inputIm = cv::Mat(code->height, code->width, CV_8UC1, (char*)code->data);
		cv::Point2f detected_markers[5];
		is_tracked = _qrcode_tracker.tracking(inputIm, detected_markers);
		if(is_tracked)
		{
			mQRPos[0] = 1;
			for(int i = 0; i < 4; i++)
			{
				mQRPos[i*2+1] = detected_markers[i].x;
				mQRPos[i*2+2] = detected_markers[i].y;
			}
			SMART_DEBUG("zxing qrcode position 1-1 : %d, %d, %d, %d, %d, %d, %d, %d ",
					mQRPos[1], mQRPos[2], mQRPos[3], mQRPos[4],
					mQRPos[5], mQRPos[6], mQRPos[7], mQRPos[8]);
			int previewWidth = extern_manager->preview_width;
			int previewHeight = extern_manager->preview_height;
			SMART_DEBUG("qrcode preview size:preview W*H: %d* %d, QRCode W*H: %d * %d",
					previewWidth, previewHeight, code->width, code->height);
			//modified on 2016-04-12 output the position based on preview size from recognition module
			//gl_matrix output order: [left, bottom], [left, top], [right, top], [right, bottom]
			tracked_rect_for_gl_matrix[0] = ((float) mQRPos[1] / code->width)* previewWidth;
			tracked_rect_for_gl_matrix[1] = ((float) mQRPos[2] / code->height)* previewHeight;
			tracked_rect_for_gl_matrix[2] = ((float) mQRPos[3] / code->width)* previewWidth;
			tracked_rect_for_gl_matrix[3] = ((float) mQRPos[4] / code->height)* previewHeight;
			tracked_rect_for_gl_matrix[4] = ((float) mQRPos[5] / code->width)* previewWidth;
			tracked_rect_for_gl_matrix[5] = ((float) mQRPos[6] / code->height)* previewHeight;
			tracked_rect_for_gl_matrix[6] = ((float) mQRPos[7] / code->width)* previewWidth;
			tracked_rect_for_gl_matrix[7] = ((float) mQRPos[8] / code->height)* previewHeight;

			SMART_DEBUG("zxing qrcode position 1-2 : %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f ",
					tracked_rect_for_gl_matrix[0], tracked_rect_for_gl_matrix[1], tracked_rect_for_gl_matrix[2], tracked_rect_for_gl_matrix[3],
					tracked_rect_for_gl_matrix[4], tracked_rect_for_gl_matrix[5], tracked_rect_for_gl_matrix[6], tracked_rect_for_gl_matrix[7]);
		}
		//SMART_DEBUG("zxing qrcode start tracking %d ", is_tracked==true ? 1 : 0);
#endif
	}
	break;
	default:
	return false;
	}
	return is_tracked;
}
/**
 * downsize data for detection pipeline
 * @param yuv420sp     (input)original data for downsize, also for code
 * @param code_data    (output)downsize Y data(720*405/640*480) for light/portrait light
 * @param boofcv_data  (output)downsize Y data(360*202/320*240) for food/line/face-check
 * @param datay        (output)downsize Y data(64*64) for Histogram
 * @param datay32      (output)downsize Y data(32*32) for document/motion/landscape
 * @param boofcv_u     (output)downsize U data(360*202/320*240) for face-check
 * @param boofcv_v     (output)downsize V data(360*202/320*240) for face-check
 * @param datau1_1     (output)downsize U data(64*64) for Histogram/landscape
 * @param datav1_1     (output)downsize V data(64*64) for Histogram/landscape
 * @param datau16_9    (output)downsize U data(64*36/64*48) for food
 * @param datav16_9    (output)downsize V data(64*36/64*48) for food
 * @return
 */
void SmartRecognition::downsize_for_detection(const Image* yuv420sp,
		Image* code_data, Image* boofcv_data, Image* datay, Image* datay32,
		Image* boofcv_u, Image* boofcv_v, Image* datau1_1, Image* datav1_1,
		Image* datau16_9, Image* datav16_9) {

	int method1 = cv::INTER_LINEAR ;
	int method2 = cv::INTER_NEAREST;

	downsize(yuv420sp, code_data, method2);

	downsize(code_data, boofcv_data, method1);
	downsize(code_data, datay, method1);
	downsize(code_data, datay32, method1);

	Image* u = new Image(yuv420sp->width * 0.5, yuv420sp->height * 0.5);
	Image* v = new Image(yuv420sp->width * 0.5, yuv420sp->height * 0.5);
	/** get u and v from yuv */
	get_uv(yuv420sp, u, v);

	downsize(u, boofcv_u, method1);
	downsize(v, boofcv_v, method1);

	downsize(u, datau1_1, method1);
	downsize(v, datav1_1, method1);

	downsize(u, datau16_9, method1);
	downsize(v, datav16_9, method1);

	delete u;
	delete v;

}
/**
 * document detection
 * @param input   input data for detection
 * @return        -1:No detect|0:normal|1:doc
 */
#ifdef _SMART_DOCUMENT
int SmartRecognition::detect_document(const Image* input) {

	int doc_flag = DocumentScene::TYPE_NORMAL;

	int sumY = 0;
	int lengthY = input->height * input->width;
	for (int i = 0; i < lengthY; i++) {
		sumY += input->data[i];
	}

	double meanY = (double) sumY / lengthY;

	double denominator = 0, numerator = 0;
	for (int i = 0; i < lengthY; i++) {
		denominator += pow(input->data[i] - meanY, 2);
		numerator += pow(input->data[i] - meanY, 3);
	}

	denominator = pow(sqrt(denominator / lengthY), 3);
	numerator = numerator / lengthY;
	double s = numerator / denominator;

	if (s < -1.0 && meanY > 110) {
		doc_flag = DocumentScene::TYPE_DOCUMENT;
	} else {
		doc_flag = DocumentScene::TYPE_NORMAL;
	}

	return doc_flag;
}
#endif
/**
 * motion detection
 * @param input   input data for detection
 * @return        -1:No detect|0:normal|1:move|2:jitter
 */
#ifdef _SMART_MOTION
int SmartRecognition::detect_motion(const Image* datay32, const float a_param, const float o_param) {

	const unsigned char* dataY32 = datay32->data;
	const int width = datay32->width;
	const int height = datay32->height;
	SMART_DEBUG("detect_motion test 1-1 asensor=%f, osensor=%f", a_param, o_param);
	bool rule1 = (a_param > Motion_ThresAccLow);
	bool rule2 = (o_param > Motion_ThresOccHigh);
#ifdef DEBUG_VALUE
	debug_value[15] = a_param;
	debug_value[16] = o_param;
#endif

	if (rule1 || rule2) {
		return MotionScene::TYPE_JITTER;
	}

	static bool sub_flag = true;
	static unsigned char dataY32Sub1[DownsampleVarWidth * DownsampleVarHeight];
	static unsigned char dataY32Sub2[DownsampleVarWidth * DownsampleVarHeight];

	int subLength = width * height;
	int subNum = 0;
	if (sub_flag) {
		for (int i = 0; i < subLength; i++) {
			dataY32Sub1[i] = dataY32[i];
			if (abs(dataY32Sub1[i] - dataY32Sub2[i]) > Motion_ThresSubNum)
				subNum++;
		}
	} else {
		for (int i = 0; i < subLength; i++) {
			dataY32Sub2[i] = dataY32[i];
			if (abs(dataY32Sub1[i] - dataY32Sub2[i]) > Motion_ThresSubNum)
				subNum++;
		}
	}

	sub_flag = !sub_flag;
#ifdef DEBUG_VALUE
	debug_value[17] = subNum;
#endif
	bool rule3 = (subNum > Motion_ThresSubLow);
	int motion_flag = MotionScene::TYPE_NORMAL;
	if (rule3) {
		motion_flag = MotionScene::TYPE_MOVE;
	} else {
		motion_flag = MotionScene::TYPE_NORMAL;
	}

	SMART_DEBUG("detect_motion test 1-2 asensor=%f, osensor=%f, subNum = %d, flag = %d", a_param, o_param, subNum, motion_flag);
	return motion_flag;
}
#endif

#define ZXING_CODE_DETECTION
#ifndef ZXING_CODE_DETECTION
int SmartRecognition::detect_code(const Image *yuv420sp, int *qrPos, unsigned char *yuv420spCopy, unsigned char *thld , unsigned char *roi) {
	//old QRcode detection algorithm by Bob Xu
	CodeDetection code_detector;
	int roi_detect[4] = {-1, -1, yuv420sp->height, yuv420sp->width};

	code_detector.set_parameter(DETECTION_PARAM_QRCODE_ROI, roi_detect);

	bool sm_flag = true;
	code_detector.set_parameter(DETECTION_PARAM_QRCODE_SINGLE_OR_MULT, &sm_flag);

	code_detector.set_parameter(DETECTION_PARAM_QRCODE_IMG_BUF, yuv420spCopy);

	code_detector.set_parameter(DETECTION_PARAM_QRCODE_THLD_BUF, thld);

	code_detector.set_parameter(DETECTION_PARAM_QRCODE_MID_ZONE_BUF, roi);

	int res = code_detector.obj_detect(yuv420sp->data, yuv420sp->width, yuv420sp->height, qrPos);

	SMART_DEBUG("detect_code flag = %d", res);
	return res;
}
#else
/**
 * QRCode detection
 * @param code           (input) data for code detection
 * @param qrPos          (output) data of QRCode position
 * @return               -1:No detect|0:normal|1:BarCode|2:QRCode
 */
#ifdef _SMART_CODE
int SmartRecognition::detect_code(const Image *yuv420sp, int *qrPos) {
	//ZXing QRcode detection algorithm by Mikey Hu
	ZxingCodeDetection code_detector;
	int roi_detect[4] = { 0, 0, yuv420sp->height, yuv420sp->width };

	code_detector.detect(yuv420sp->data, yuv420sp->height, yuv420sp->width,
			roi_detect[0], roi_detect[1], roi_detect[2], roi_detect[3], true);
	//get QRCode coordinate
	int res = code_detector.getCodePositon(qrPos, 100);
	_code_str = code_detector.getDecodeStr();
	_code_type = code_detector.getType();
	_code_format = code_detector.getFormat();

	return res;
}
#endif

#endif
/**
 * [SmartRecognition::detect_light description]
 * @param  blockY        [description]
 * @param  histYL        [description]
 * @param  histU         [description]
 * @param  histV         [description]
 * @param  histYS        [description]
 * @param  featureVector [description]
 * @param  baseExpIndex  [description]
 * @param  baseLuxIndex  [description]
 * @param  offset        [description]
 * @param  lowRatio      [description]
 * @return               [description]
 */
int SmartRecognition::detect_light(const float blockY[], const float histYL[],
		const float histU[], const float histV[], const float histYS[],
		const int baseExpIndex, const int baseLuxIndex, const int offset,
		const float lowRatio) {

	float sceneDegree[3] = { 0.0f, 0.0f, 0.0f };
	int flag[3] = { 0, 0, 0 };
	int light_flag = LightScene::TYPE_NORMAL;
    //set the threshold for light detection
	float th1_lowlight = baseExpIndex * lowRatio;
	float th2_lowlight = baseExpIndex - offset;
	float th3_lowlight = baseLuxIndex;

	float featureVector[24];
	util::feature_extraction(blockY, histYL, histU, histV, histYS, featureVector);				//saved in featureVector
	util::degree_computation(featureVector, SceneFeature_alphaVector,
			                 SceneFeature_indxVector, SceneFeature_dirVector,
			                 SceneFeature_thldVector, 3, LIGHT_NUMOFWEIGHT, sceneDegree);       //saved in sceneDegree
	SMART_DEBUG("detect_light_sceneDegree[0]=%f, sceneDegree[1]=%f, sceneDegree[2]=%f", sceneDegree[0], sceneDegree[1], sceneDegree[2]);
	static float meanDegree[3] = { 0.0f, 0.0f, 0.0f };
	static int countLight = 0;
	for (int k = 0; k < 3; k++) {
		if (countLight == 0) {
			meanDegree[k] = sceneDegree[k];
			countLight = 1;
		} else {
			meanDegree[k] += COE_DEGREE * (sceneDegree[k] - meanDegree[k]);
		}

		if (meanDegree[k] >= degreeBound[k][0]) {
			flag[k] = 1;
		}
		if (flag[k] == 1 && meanDegree[k] <= degreeBound[k][1]) {
			flag[k] = 0;
		}
	}

#ifdef DEBUG_VALUE
	debug_value[18] = meanDegree[0];
	debug_value[19] = meanDegree[1];
	debug_value[20] = meanDegree[2];
#endif
	SMART_DEBUG("detect_light_isp_ExpIndex = %f, lLux = %f, isp_FocusDis = %f", isp_ExpIndex, lLux, isp_FocusDis);

	if (extern_manager->is_isp_usable) {
		//light detection with isp data
		if (isp_ExpIndex >= th2_lowlight && isp_Lux >= th3_lowlight) {
			light_flag = LightScene::TYPE_ULTRA_LOWLIT;
		} else if (isp_ExpIndex >= th1_lowlight) {
			light_flag = LightScene::TYPE_LOWLIT;
		} else {
			if (flag[0] == 1/* && (meanDegree[0] > meanDegree[1]) && (meanDegree[0] > meanDegree[2])*/) {
				if (isp_ExpIndex > Light_ThresExpLow) {
					//indoor environment
					if (lLux < Light_ThresLuxHigh && isp_ExpIndex < Light_ThresExpHigh1 && isp_FocusDis > Light_ThresFocusDis) {
						//camera towards to window
						light_flag = LightScene::TYPE_BACKLIT;
					} else {
						light_flag = LightScene::TYPE_NORMAL;
					}
				} else {
					//outdoor environment
					light_flag = LightScene::TYPE_BACKLIT;
				}
			} else {
				light_flag = LightScene::TYPE_NORMAL;
			}
		}
	} else {
		//light detection without isp data
		if (flag[0] == 1) {
			light_flag = LightScene::TYPE_BACKLIT;
		} else if (flag[1] == 1 && (meanDegree[1] > meanDegree[2]) && lLux < Light_ThresLuxHigh1) {
			light_flag = LightScene::TYPE_LOWLIT;
		} else if (flag[2] == 1 && (meanDegree[1] <= meanDegree[2]) && lLux < Light_ThresLuxHigh1) {
			light_flag = LightScene::TYPE_ULTRA_LOWLIT;
		} else {
			light_flag = LightScene::TYPE_NORMAL;
		}
	}
	SMART_DEBUG("detect_light flag = %d", light_flag);
	return light_flag;
}
/**
 * portrait detection pipeline
 * @param faces           faces data
 * @return                -1:No detect|0:normal|1:portrait|2:portraitGrop
 */
int SmartRecognition::detect_portrait(const int* faces) {

	int portrait_flag = PortraitScene::TYPE_NORMAL;

	int orientation = extern_manager->orientation;

	SMART_DEBUG("detect_portrait %d,%d", image_width, image_height);
	//portrait detect
	bool porFlag = portraitDetect(faces, image_width, image_height, orientation,FaceHeight);
	//portrait double check
	bool porCheck = portraitCheck(faces, image_width, image_height, boofcv->data, boofcv_u->data, boofcv_v->data);

	if (porFlag && porCheck) {
		portrait_flag = PortraitScene::TYPE_PORTRAIT;
	}

	SMART_DEBUG("detect_portrait flag=%d %d,%d,%d", portrait_flag, porFlag, porCheck, orientation);
	return portrait_flag;
}
/**
 * portrait detection
 * @param faces           faces data
 * @param width           preview width
 * @param height          preview height
 * @param ori             camera orientation
 * @param ratioF          face ratio of width/height
 * @return                false:normal|true:portrait
 */
bool SmartRecognition::portraitDetect(const int* faces, const int width,
		const int height, const int ori, const float ratioF) {
	//faces number is in [1,5]
	if (faces[0] >= 1 && faces[0] <= 5) {
		if (ori == 0 || ori == 180) {
			//Vertical screen
			for (int i = 0; i < faces[0]; i++) {
				//right - left
				if ((faces[4 * i + 3] - faces[4 * i + 1]) >= width * ratioF) {
					return true;
				}
			}
		} else {
			//Horizon screen
			for (int i = 0; i < faces[0]; i++) {
				//bottom - top
				if ((faces[4 * i + 4] - faces[4 * i + 2]) >= height * ratioF) {
					return true;
				}
			}
		}
	}
	return false;
}
/**
 * portrait double-check
 * @param faces           faces data
 * @param width           preview width
 * @param height          preview height
 * @param _boofCVDataY    data Y for double-check process
 * @param _boofCVDataU    data U for double-check process
 * @param _boofCVDataV    data V for double-check process
 * @return                false:normal|true:portrait
 */
bool SmartRecognition::portraitCheck(const int* faces, const int width, const int height,
		unsigned char *_boofCVDataY, unsigned char *_boofCVDataU, unsigned char *_boofCVDataV) {
	bool isPortrait = false;
	if (faces[0] >= 1 && faces[0] <= 5) {
		if (faces[0] == 1) {
			cv::Mat imY = cv::Mat(boofcv->height, boofcv->width, CV_8UC1, _boofCVDataY);
			cv::Mat imU = cv::Mat(boofcv->height, boofcv->width, CV_8UC1, _boofCVDataU);
			cv::Mat imV = cv::Mat(boofcv->height, boofcv->width, CV_8UC1, _boofCVDataV);

			std::vector<cv::Mat> matlist;
			matlist.push_back(imY);
			matlist.push_back(imU);
			matlist.push_back(imV);

			cv::Mat imYUV(boofcv->height, boofcv->width, CV_8UC3);
			cv::merge(matlist, imYUV);

			float rH = (float) imYUV.rows / height;
			int sxf = faces[1] * rH;
			int syf = faces[2] * rH;
			int exf = faces[3] * rH;
			int eyf = faces[4] * rH;

			if (sxf < 0)
				sxf = 0;
			if (syf < 0)
				syf = 0;
			if (exf > imYUV.cols - 1)
				exf = imYUV.cols - 1;
			if (eyf > imYUV.rows - 1)
				eyf = imYUV.rows - 1;

			int hf = eyf - syf;
			int wf = exf - sxf;
            //get the ROI face in the img
			cv::Mat mat_face = imYUV(cv::Rect(sxf, syf, wf, hf));
			isPortrait = util::check_face_YUV(mat_face);
		} else {
			isPortrait = true;
		}
		return isPortrait;
	} else {
		return isPortrait;
	}
}
/**
 * portrait_lit detection pipeline
 * @param code            data for portrait_lit detection
 * @param faces           faces data
 * @param ori             camera orientation
 * @param params          portrait_lit parameters
 * @return                -1:No detect|0:normal|1:portrait_backlit|2:portrait_lowlit
 */
int SmartRecognition::detect_portrait_lit(Image* code, int* faces, const int ori, int* params) {
    int flag = 0;
	SMART_DEBUG("detect_portrait_lit test face num =%d", faces[0]);
	if (faces[0] == 1) {
		//portrait flag and light flag composition
		if (scene_type.get_light().flag == LightScene::TYPE_LOWLIT || scene_type.get_light().flag == LightScene::TYPE_ULTRA_LOWLIT) {
			return PortraitTable(scene_type.get_light().flag, flashStatus);
		} else if (scene_type.get_light().flag == LightScene::TYPE_BACKLIT) {
			return PortraitLitScene::TYPE_PORTRAIT_BACKLIT;
		} else {
			scene_type.get_portrait_lit().flag = PortraitLitScene::TYPE_NORMAL;
		}

		int tmpface[5];
		float rH = (float) code->height / image_height;
		tmpface[0] = faces[0];
		tmpface[1] = (int) (faces[1] * rH);
		tmpface[2] = (int) (faces[2] * rH);
		tmpface[3] = (int) (faces[3] * rH);
		tmpface[4] = (int) (faces[4] * rH);

		SMART_DEBUG("detect_portrait_lit: rH= %f, H=%d ", rH, image_height);

		int orientation = extern_manager->orientation;
		//algorithm computation
		int flag = portraitLitDetect(code->data, tmpface, code->width, code->height, orientation, params);
		SMART_DEBUG("detect_portrait_lit param: [1]= %d , [2] =%d , [3]=%d , [4]=%d , flag = %d \n",
				     params[1], params[2], params[3], params[4], flag);
		if (flag == PortraitLitScene::TYPE_PORTRAIT_BACKLIT) {
			return PortraitLitScene::TYPE_PORTRAIT_BACKLIT;
		}
	} else {
		return PortraitLitScene::TYPE_NORMAL;
	}
}
/**
 * portrait_lit detection
 * @param code_data       data for portrait_lit detection
 * @param faces           data for portrait_lit detection
 * @param height          data height
 * @param width           data width
 * @param ori             camera orientation
 * @param params          portrait_lit parameters
 * @return                -1:No detect|0:normal|1:portrait_backlit|2:portrait_lowlit
 */
int SmartRecognition::portraitLitDetect(unsigned char* yuv420sp, int* faces,
		     const int width, const int height, const int ori, int* params) {

	unsigned char bg_area_up, bg_area_left, bg_area_right;
	int mean_face = 0.0;
	int flag_p = PortraitLitScene::TYPE_NORMAL;
	if (faces[0] == 1) {
		mean_face = util::imgMean(yuv420sp, faces[1], faces[2], faces[3],faces[4], height, width);

		int xstart = faces[1];
		int ystart = faces[2];
		int xend = faces[3];
		int yend = faces[4];
		int sub_width = width - 1;
		int sub_height = height - 1;
		// ori = 50;
		switch (ori) {
		case 270: {
			bg_area_up = util::imgMaxVal(yuv420sp, 0, 0, ystart, width - 1,
					height, width);
			bg_area_left = util::imgMaxVal(yuv420sp, 0, 0, height - 1, xstart,
					height, width);
			bg_area_right = util::imgMaxVal(yuv420sp, 0, xend, height - 1,
					width - 1, height, width);
			break;
		}
		case 0: {
			bg_area_up = util::imgMaxVal(yuv420sp, 0, 0, height - 1, xstart,
					height, width);
			bg_area_left = util::imgMaxVal(yuv420sp, yend, xstart, height - 1,
					width - 1, height, width);
			bg_area_right = util::imgMaxVal(yuv420sp, 0, 0, ystart, width - 1,
					height, width);
			break;
		}
		case 90: {
			bg_area_up = util::imgMaxVal(yuv420sp, yend, 0, height - 1,
					width - 1, height, width);
			bg_area_left = util::imgMaxVal(yuv420sp, 0, xend, height - 1,
					width - 1, height, width);
			bg_area_right = util::imgMaxVal(yuv420sp, 0, 0, height - 1, xstart,
					height, width);
			break;
		}
		case 180: {
			bg_area_up = util::imgMaxVal(yuv420sp, 0, xend, height - 1,
					width - 1, height, width);
			bg_area_left = util::imgMaxVal(yuv420sp, 0, 0, ystart, width - 1,
					height, width);
			bg_area_right = util::imgMaxVal(yuv420sp, yend, 0, height - 1,
					width - 1, height, width);
			break;
		}
		case 50: {
			bg_area_up = util::imgMaxVal(yuv420sp, 0, 0, height - 1, width - 1,
					height, width);
			bg_area_left = 0;
			bg_area_right = 0;
		}
		default:
			break;
		}

	} else {
		// portrait scene detection	: multi-portrait
		int mean_allf = 0;
		int sx = 1;
		int sy = 2;
		int ex = 3;
		int ey = 4;
		for (int i = 0; i < faces[0]; i++) {
			mean_allf = mean_allf
					+ util::imgMean(yuv420sp, faces[sx], faces[sy], faces[ex],
							faces[ey], height, width);
			sx = sx + 4;
			sy = sy + 4;
			ex = ex + 4;
			ey = ey + 4;
		}
		mean_allf = mean_allf / faces[0];
		mean_face = mean_allf;
		bg_area_up = util::imgMaxVal(yuv420sp, 0, 0, height - 1, width - 1,
				height, width);
		bg_area_left = 0;
		bg_area_right = 0;
	}

	unsigned char maxv = bg_area_up;
	if (maxv < bg_area_left) {
		maxv = bg_area_left;
	}
	if (maxv < bg_area_right) {
		maxv = bg_area_right;
	}
	unsigned char s_diff_2 = maxv - mean_face;

	int mean_img = util::imgMean(yuv420sp, 0, 0, width - 1, height - 1, height,
			width);

#ifdef DEBUG_VALUE
	debug_value[21] = mean_face;
	debug_value[22] = mean_img;
	debug_value[23] = s_diff_2;
#endif
	bool rule1 = (mean_face < 70);
	bool rule2 = (mean_img < 40);
	bool rule3 = (s_diff_2 > 180);

	params[0] = 555;
	params[1] = mean_face;
	params[2] = maxv;
	params[3] = mean_img;
	params[4] = s_diff_2;

	if (rule1) {
		if (rule2) {
			flag_p = PortraitLitScene::TYPE_PORTRAIT_LOWILIT;
		} else {
			if (rule3) {
				flag_p = PortraitLitScene::TYPE_PORTRAIT_BACKLIT;
			} else {
				flag_p = PortraitLitScene::TYPE_PORTRAIT_OTHER;
			}
		}
	} else {
		flag_p = PortraitLitScene::TYPE_NORMAL;
	}

	return flag_p;
}
/**
 * landscape detection pipeline
 * @param datay32         data Y for landscape detection
 * @param datau1_1        data U for landscape detection
 * @param datav1_1        data V for landscape detection
 * @return                -1:No detect|0:normal|1:landscape
 */
#ifdef _SMART_LANDSCAPE
int SmartRecognition::detect_landscape(Image* datay32, Image* _datau1_1, Image* _datav1_1) {
	unsigned char* dataY32 = datay32->data;
	const int width = datay32->width;
	const int height = datay32->height;

	gistScoreAvalible = false;
	//landscape detection merged from GPU algorithm
	int landscape_flag = landscapeDetect(_datau1_1, _datav1_1);
	//landscape detection with gist algorithm
	if (landscape_flag != LandscapeScene::TYPE_LANDSCAPE) {
		landscape_flag = landscapeDetectNative(dataY32, width, height);
		bGistDetectLandscape = true;
	} else {
		bGistDetectLandscape = false;
	}
	//whether to enable line detection
	if (landscape_flag == LandscapeScene::TYPE_LANDSCAPE) {
		bLinedetectEnalbe = isEnableLinedetect(dataY32, width, height,mBlueSkey, mGreenGround,
				                               gistScoreAvalible, bGistDetectLandscape);
	}else {
		bLinedetectEnalbe = false;
	}

	SMART_DEBUG("detect_landscape flag=%d, bLinedetectEnalbe=%d", landscape_flag, bLinedetectEnalbe);
	return landscape_flag;
}
#endif
/**
 * landscape detection(merged from GPU version)
 * @param datau1_1        data U for landscape detection
 * @param datav1_1        data V for landscape detection
 * @return                -1:No detect|0:normal|1:landscape
 */
#ifdef _SMART_LANDSCAPE
int SmartRecognition::landscapeDetect(Image* _datau1_1, Image* _datav1_1) {
	//adjust threshold of green/blue score under landscape detection without gist computation
	adjustLandscapeGreenBlueThreshold();

	int flag = LandscapeScene::TYPE_NORMAL;
	SMART_DEBUG("detect_landscape lLux=%f", lLux);
	if (extern_manager->is_isp_usable) {
		//with isp data
		if (isp_ExpIndex > 0 && isp_ExpIndex < Landscape_ThresISPExpHigh) {
			//out door
			flag = landscapeGPUDetect(_datau1_1, _datav1_1);
		} else {
			//in door
			flag = LandscapeScene::TYPE_NORMAL;
		}
	} else {
		//without isp data
		if (lLux > 0) {
			//out door
			flag = landscapeGPUDetect(_datau1_1, _datav1_1);
		} else {
			//in door
			flag = LandscapeScene::TYPE_NORMAL;
		}
	}
	return flag;
}
#endif
/**
 * landscape detection(merged from GPU version)
 * @param _datau1_1       data U for landscape detection
 * @param _datav1_1       data V for landscape detection
 * @return                -1:No detect|0:normal|1:landscape
 */
#ifdef _SMART_LANDSCAPE
int SmartRecognition::landscapeGPUDetect(Image* _datau1_1, Image* _datav1_1) {
	unsigned char* dataU1_1 = _datau1_1->data;
	unsigned char* dataV1_1 = _datav1_1->data;
	const int MATRIX_WIDTH = _datau1_1->width;
	const int MATRIX_HEIGHT = _datau1_1->height;

	int flag = LandscapeScene::TYPE_NORMAL;
	long nSumSky = 0, nSumVeg = 0;
	double nRank = 0, nAvgSky = 0, nAvgVeg = 0;
	int ylength = MATRIX_WIDTH * MATRIX_HEIGHT;

	int orientation = extern_manager->orientation;

	for (int i = 0; i < ylength; i++) {
		 if ((dataU1_1[i] & 0xff) > 135 && (dataV1_1[i] & 0xff) < 120) {
//		/** 20150827 ** this solves the bug that u and v are upside-down */
//		if ((dataV1_1[i] & 0xff) > 135 && (dataU1_1[i] & 0xff) < 120) {
			switch (orientation) {
			case 0:
				nSumSky += MatrixData_MatrixSkymskByte0[i];
				break;
			case 90:
				nSumSky += MatrixData_MatrixSkymskByte90[i];
				break;
			case 180:
				nSumSky += MatrixData_MatrixSkymskByte180[i];
				break;
			case 270:
				nSumSky += MatrixData_MatrixSkymskByte270[i];
				break;
			default:
				break;
			}
		} else {
			nSumSky += 0;
		}
		 if ((dataU1_1[i] & 0xff) < 125 && (dataV1_1[i] & 0xff) < 130) {
//		/** 20150827 ** this solves the bug tha u and v are upside-down */
//		if ((dataV1_1[i] & 0xff) < 125 && (dataU1_1[i] & 0xff) < 130) {
			switch (orientation) {
			case 0:
				nSumVeg += MatrixData_MatrixVegmskByte0[i];
				break;
			case 90:
				nSumVeg += MatrixData_MatrixVegmskByte90[i];
				break;
			case 180:
				nSumVeg += MatrixData_MatrixVegmskByte180[i];
				break;
			case 270:
				nSumVeg += MatrixData_MatrixVegmskByte270[i];
				break;
			default:
				break;
			}
		} else {
			nSumVeg += 0;
		}
	}
	//blue score
	nAvgSky = (double) nSumSky * 10 / ylength;
	//green score
	nAvgVeg = (double) nSumVeg / ylength;
	//total score
	nRank = nAvgSky + nAvgVeg;
#ifdef DEBUG_VALUE
	debug_value[24] = nAvgSky;
	debug_value[25] = nAvgVeg;
	debug_value[26] = s_diff_2;
#endif
	mBlueSkey = nAvgSky;
	mGreenGround = nAvgVeg;

	SMART_DEBUG("detect_landscape %f,%f,%f", nRank, nAvgSky, nAvgVeg);

	if (LandscapeOriLimitEnable(orientation, extern_manager->o_sensor, 55)
	&& (nRank > m_landscape_rank_t || nAvgSky > m_landscape_blue_t || nAvgVeg > m_landscape_green_t)) {
		flag = LandscapeScene::TYPE_LANDSCAPE;
		if (nAvgSky > m_landscape_blue_t && nAvgVeg < m_landscape_green_t) {
			//ignore landscape result(blue) when camera towards to ground
			if (!LTLookDownAngleT(orientation, extern_manager->o_sensor, 60)) {
				flag = LandscapeScene::TYPE_NORMAL;
			}
		}
		if (extern_manager->is_isp_usable && isp_FocusDis > 0 && isp_FocusDis < Landscape_ThresFocusDis)
			//with isp data enable, ignore landscape result when target is too close to camera
			flag = LandscapeScene::TYPE_NORMAL;
	} else {
		flag = LandscapeScene::TYPE_NORMAL;
	}
	return flag;
}
#endif
/**
 * landscape detection(Gist version)
 * @param data           data for landscape detection
 * @param height         data height
 * @param width          data width
 * @return               -1:No detect|0:normal|1:landscape
 */
#ifdef _SMART_LANDSCAPE
int SmartRecognition::landscapeDetectNative(unsigned char *data, const int width, const int height) {
	float landscapeThreshold = adjustLandscapeThreshold();                            //adjust threshold of gist

	int flag = LandscapeScene::TYPE_NORMAL;
	int orientation = extern_manager->orientation;
	bool isOriatLimit = LandscapeOriLimitEnable(orientation, extern_manager->o_sensor, gistOrientationThresh);
	isOriatLimit = true;
	float landscapeGistScore = 0.0f;

	if (extern_manager->is_isp_usable) {
		//with isp data
		if (isp_ExpIndex > gistIspThresh1 && isp_ExpIndex < gistIspThresh2             //isp limit
		&& isp_FocusDis > gistFocusDisThresh                                           //focusDistance limit
		&& isOriatLimit ){                                                             //pos limit
			landscape_gist_score->compute_gist_score(data, width, height, orientation);
			landscapeGistScore = landscape_gist_score->get_score();
			gistScoreAvalible = true;

			if (landscapeGistScore > landscapeThreshold)                               //final gist score limit
				flag = LandscapeScene::TYPE_LANDSCAPE;
		}
	}else{//without isp data
		if (isOriatLimit) { //pos limit
			landscape_gist_score->compute_gist_score(data, width, height, orientation);
			landscapeGistScore = landscape_gist_score->get_score();
			gistScoreAvalible = true;
			if (landscapeGistScore > landscapeThreshold)                               //final gist score limit
				flag = LandscapeScene::TYPE_LANDSCAPE;
		}
	}
#ifdef DEBUG_VALUE
	debug_value[27] = landscapeGistScore;
#endif
	return flag;
}
#endif
/**
 * Adjust threshold of landscape detection(GPU) dynamically
 * @return
 */
void SmartRecognition::adjustLandscapeGreenBlueThreshold() {
	if (scene_type.get_landscape().flag == LandscapeScene::TYPE_LANDSCAPE && !bGistDetectLandscape) {
		m_landscape_green_t = 8;
		m_landscape_blue_t = 35;
		m_landscape_rank_t = 50;
	} else {
		m_landscape_green_t = 14;
		m_landscape_blue_t = 40;
		m_landscape_rank_t = 60;
	}
}
/**
 * Adjust threshold of landscape detection(Gist) dynamically
 * @return               Adjusted threshold
 */
float SmartRecognition::adjustLandscapeThreshold() {
	float landscapeThreshold = 0.0f; //风景识别gist阈值
	int ori = extern_manager->orientation; // sensorService.getOrientation();
	if (scene_type.get_landscape().flag == LandscapeScene::TYPE_LANDSCAPE && bGistDetectLandscape) {
		if (ori == 0 || ori == 180) {
			landscapeThreshold = gistLandscapeTheshold_V[1];
		} else {
			landscapeThreshold = gistLandscapeTheshold_H[1];
		}
	} else {
		if (ori == 0 || ori == 180) {
			landscapeThreshold = gistLandscapeTheshold_V[0];
		} else {
			landscapeThreshold = gistLandscapeTheshold_H[0];
		}
	}
	return landscapeThreshold;
}
/**
 * judge whether to enable line detection
 * @param data                  data for landscape detection
 * @param height                data height
 * @param width                 data width
 * @param mBlueSkey2            sky parameter of landscape detection(GPU)
 * @param mGreenGround2         green parameter of landscape detection(GPU)
 * @param isGistScoreAvaliable  gist computation flag
 * @param isGistLandscape       gist landscape detection flag
 * @return                      false:disable line detection|true:enable line detection
 */
#ifdef _SMART_LANDSCAPE
bool SmartRecognition::isEnableLinedetect(unsigned char*data, int width,
		            int height, double mBlueSkey2, double mGreenGround2,
		            bool isGistScoreAvaliable, bool isGistLandscape) {
	bool retvalue = false;
	float landscapeGistScore = 0.0f;

	int orientation = extern_manager->orientation;
	float* o_sensor = extern_manager->o_sensor;
	if (!isGistLandscape) {
		//blue/green landscape
		if (mGreenGround2 > mBlueSkey2 || mGreenGround2 > 20) {

		} else {
			if (!isGistScoreAvaliable) {
				landscape_gist_score->compute_gist_score(data, width, height,
						orientation);
				landscapeGistScore = landscape_gist_score->get_score();
				SMART_DEBUG("detect_landscape landscapeGistScore=%f", landscapeGistScore);
			}
			if (landscapeGistScore > 0.65f && LandscapeOriLimitEnable(orientation, o_sensor, 30)) {
				retvalue = true;
			}
		}
	} else {
		//gist landscape
		retvalue = true;
	}
	return retvalue;
}
#endif

/**
 * food detection
 * @param food_img              data Y for food detection
 * @param datau16_9             data U for food detection
 * @param datav16_9             data V for food detection
 * @return                      -1:No detect|0:normal|1:food
 */
#ifdef _SMART_FOOD
int SmartRecognition::detect_food(Image* food_img, Image* _datau16_9, Image* _datav16_9) {
	int flag = SceneTypeBase::TYPE_NORMAL;
	if (FoodOriLimitEnable(extern_manager->orientation,	extern_manager->o_sensor)) {
		EllipseDetection ellipse_detector;

		int ori = extern_manager->orientation;
		ellipse_detector.set_parameter(DETECTION_PARAM_ELLIPSE_DETECT_ANGLE,&ori);                                   // param 1

		cv::Point2f ellicentre = cv::Point2f(0.0f,0.0f);
		ellipse_detector.set_parameter(DETECTION_PARAM_ELLIPSE_DETECT_INITCENTRE, &ellicentre);                      // param 2

		float fMaxDisFromInitCentre = FLT_MAX;
		ellipse_detector.set_parameter(DETECTION_PARAM_ELLIPSE_DETECT_MAXDISFROMINITCENTRE,&fMaxDisFromInitCentre);  // param 3

		float fInitB = 0.0f;
		ellipse_detector.set_parameter(DETECTION_PARAM_ELLIPSE_DETECT_INITB,&fInitB);                                // param 4

		float fTolRatioB = 0.0f;
		ellipse_detector.set_parameter(DETECTION_PARAM_ELLIPSE_DETECT_TOLRATIOB,&fTolRatioB);                        // param 5

		float mfMinScore = 0.4f;
		ellipse_detector.set_parameter(DETECTION_PARAM_ELLIPSE_DETECT_MINSCORE,&mfMinScore);                         // param 6

		float mfMinReliability = 0.4f;
		ellipse_detector.set_parameter(DETECTION_PARAM_ELLIPSE_DETECT_MINRELIABILITY,&mfMinReliability);             // param 7

		int res = ellipse_detector.obj_detect((void *) food_img->data,food_img->width, food_img->height, (void *) ellipse_param  ); // detecting

		SMART_DEBUG("detect_food ellipse_param:x=%d,y=%d,a=%d,b=%d,ct=%d",
				ellipse_param[1], ellipse_param[2], ellipse_param[3], ellipse_param[4], ellipse_param[5]);

		//compute the food score
		float uv_score = 0.0;
		if (res == 1) {
			float ratioElli = _datau16_9->width / food_img->width;
			int X = ellipse_param[1] * ratioElli;
			int Y = ellipse_param[2] * ratioElli;
			int a = ellipse_param[3] * ratioElli;
			int b = ellipse_param[4] * ratioElli;
			float cita = ellipse_param[5];

			uv_score = foodScoreMean(_datau16_9, _datav16_9, a, b, cita, X, Y);
			SMART_DEBUG("detect_food uvScore=%f", uv_score);
		}
#ifdef DEBUG_VALUE
	debug_value[28] = uv_score;
#endif
		if (uv_score > Food_ThresScore) {
			flag = FoodScene::TYPE_FOOD;
		} else {
			flag = SceneTypeBase::TYPE_NORMAL;
		}
	}
	SMART_DEBUG("detect_food flag=%d", flag);
	return flag;
}
#endif

/**
 * food score computation
 * @param datau16_9             data U for food score computation
 * @param datav16_9             data V for food score computation
 * @param a                     major semi-Axis of ellipse
 * @param b                     minor semi-Axis of ellipse
 * @param cita                  angle of ellipse
 * @param X                     center X coordinate of ellipse
 * @param Y                     center Y coordinate of ellipse
 * @return                      food score
 */
#ifdef _SMART_FOOD
double SmartRecognition::foodScoreMean(Image* datau16_9, Image* datav16_9,
		int a, int b, float cita, int X, int Y) {

	int uvH = datau16_9->height;//36 or 48
	int uvW = datau16_9->width; //64

	cv::Mat mCb = cv::Mat(uvH, uvW, CV_8UC1, datau16_9->data);
	cv::Mat mCr = cv::Mat(uvH, uvW, CV_8UC1, datav16_9->data);

	int tempCb = 0, tempCr = 0;
	for (int i = 0; i < 360; i++) {
		for (int j = 95; j > 85; j--) {
			int sa = (int) (a * j / 100.0);
			int sb = (int) (b * j / 100.0);
			int x = sa * cos(i * CV_PI / 180) * cos(cita * CV_PI / 180)
					- sb * sin(i * CV_PI / 180) * sin(cita * CV_PI / 180) + X;
			int y = sa * cos(i * CV_PI / 180) * sin(cita * CV_PI / 180)
					+ sb * sin(i * CV_PI / 180) * cos(cita * CV_PI / 180) + Y;

			if (x < 0)
				x = 0;
			if (y < 0)
				y = 0;
			if (x >= mCb.cols)
				x = mCb.cols - 1;
			if (y >= mCb.rows)
				y = mCb.rows - 1;

			uchar* dataU = mCb.ptr<uchar>(y);
			uchar* dataV = mCr.ptr<uchar>(y);
			tempCb += dataU[x];
			tempCr += dataV[x];
		}
	}

	double count = 360 * (95 - 85);
	double meanU = tempCb / count;
	double meanV = tempCr / count;
	int w = b;

	cv::Rect r(X - w / 2, Y - w / 2, w, w);
	cv::Rect r_img(0, 0, mCb.cols - 1, mCb.rows - 1);
	r = r & r_img;
	cv::Mat cropCb = mCb(r);
	cv::Mat cropCr = mCr(r);
	cv::Scalar mMeanCb = cv::mean(cropCb);
	cv::Scalar mMeanCr = cv::mean(cropCr);

	double distrr = (mMeanCb[0] - meanU) * (mMeanCb[0] - meanU)
			+ (mMeanCr[0] - meanV) * (mMeanCr[0] - meanV);
	distrr = sqrt(distrr);

	return distrr;
}
#endif
/**
 * line detection
 * @param line_img              data for line detection
 * @param pos                   bounding box of line
 * @param inputOritation        camera orientation
 * @param leveltheta            angle of camera level
 * @param thetaInput            angle of line
 * @param toleranceAngle        Max angle of line from Horizon
 * @return                      Number of line
 */
#ifdef _SMART_LANDSCAPE
int SmartRecognition::detect_line(Image* line_img, int *pos, int inputOritation,
		float leveltheta, float *output_theta, int toleranceAngle) {
	if (abs(leveltheta) > 7) { // only detect near horizon
		pos[0] = 0;
		return 0;
	}

	int oritation = 0;
	if (inputOritation == 0 || inputOritation == 180)
		oritation = smart::IMAGE_VERTICAL;
	else
		oritation = smart::IMAGE_HORIZONTAL;

	LineDetection line_detector;
	HoughFootLineParam param;
	std::vector<LineParameter> detected_lines;
	param.binary_threshold = 60;
	param.max_line_num = 1;
	param.max_radius = 10;
	param.min_density = 30;
	param.orientation = oritation;
	param.min_distance = 0;

	line_detector.set_parameter(DETECTION_PARAM_HOUGHFOOT, (void*) &param);
	line_detector.set_parameter(DETECTION_PARAM_HOUGHFOOT_ORIENTATION, (void*) (&oritation));

	line_detector.obj_detect(line_img->data, line_img->width, line_img->height, (void*) (&detected_lines));

	pos[0] = detected_lines.size();
	if (detected_lines.size() > 0) {
		int line_num = detected_lines.size();
		for (int i = 0; i < line_num; i++) {
			pos[i * 4 + 1] = detected_lines[i].terminal_points[0].x;
			pos[i * 4 + 2] = detected_lines[i].terminal_points[0].y;
			pos[i * 4 + 3] = detected_lines[i].terminal_points[1].x;
			pos[i * 4 + 4] = detected_lines[i].terminal_points[1].y;
		}
	}
	if (pos[0] == 0) {
		return 0;
	} else {
		if (output_theta != NULL) {
			for (int i = 0; i < detected_lines.size(); i++)
				output_theta[i] = detected_lines[i].theta_;
		}
		return 1;
	}
}
#endif
/**
 * get accelerometer data
 * @return       accelerometer data
 */
#ifdef _SMART_MOTION
float SmartRecognition::getAcc() {
	float acc = util::IVCLib_accDiff(extern_manager->a_sensor);

	std::vector<float> tmp(mAccList.begin() + 1, mAccList.end());
	mAccList.swap(tmp);
	mAccList.push_back(acc);
	float sumacc = 0;
	for (int i = 0; i < mAccList.size(); i++) {
		sumacc += mAccList.at(i);
	}
	static float mMeanAcc = 0.0f;
	static int countAcc = 0;
	float accImg = sumacc / mAccList.size();
	if (countAcc == 0) {
		mMeanAcc = accImg;
		countAcc = 1;
	} else {
		mMeanAcc = mMeanAcc + COE_ACC * (accImg - mMeanAcc);
	}
	return mMeanAcc;
}
#endif
/**
 * get orientation data
 * @return       orientation data
 */
#ifdef _SMART_MOTION
float SmartRecognition::getOcc() {

	float occ = 0.0f;
	occ = util::IVCLib_accDiff(extern_manager->o_sensor);

	static float mMeanOcc = 0.0f;
	static int countOcc = 0;
	if (countOcc == 0) {
		mMeanOcc = occ;
		countOcc = 1;
	} else {
		mMeanOcc = mMeanOcc + COE_OCC * (occ - mMeanOcc);
	}

	return occ;
}
#endif
/**
 * compute histogram
 * @param _datay                (input) data Y
 * @param _datau1_1             (input) data U
 * @param _datav1_1             (input) data V
 * @param HISTO_SRC_BINNUM      Base num of histogram Bin
 * @param HISTO_DST_BINNUM      Target num of histogram Bin
 * @param histoY_src       		(output)histogram of Y data with base bin
 * @param histoU_src            (output)histogram of U data with base bin
 * @param histoV_src            (output)histogram of V data with base bin
 * @param histoY_dst            (output)histogram of Y data with target bin
 * @return
 */
void SmartRecognition::getHistogram(const Image* _datay, const Image* _datau1_1, const Image* _datav1_1,
									const int HISTO_SRC_BINNUM, const int HISTO_DST_BINNUM,
									float* histoY_src, float* histoU_src, float* histoV_src, float* histoY_dst) {

	unsigned char* dataY = _datay->data;
	const int MATRIX_WIDTH = _datay->width;
	const int MATRIX_HEIGHT = _datay->height;

	unsigned char* dataU1_1 = _datau1_1->data;
	unsigned char* dataV1_1 = _datav1_1->data;

	float resultHistgramY[HISTO_SRC_BINNUM];
	float resultHistgramU[HISTO_SRC_BINNUM];
	float resultHistgramV[HISTO_SRC_BINNUM];

	memset(resultHistgramY, 0, sizeof(resultHistgramY));
	memset(resultHistgramU, 0, sizeof(resultHistgramU));
	memset(resultHistgramV, 0, sizeof(resultHistgramV));

	//histogram process
	for (int i = 0; i < MATRIX_WIDTH * MATRIX_HEIGHT; ++i) {
		resultHistgramY[dataY[i]]++;
		resultHistgramU[dataU1_1[i]]++;
		resultHistgramV[dataV1_1[i]]++;
	}

	//cumulative histogram process
	float sumYBin = 0;
	float sumUBin = 0;
	float sumVBin = 0;
	for (int i = 0; i < HISTO_SRC_BINNUM; ++i) {
		sumYBin += resultHistgramY[i];
		sumUBin += resultHistgramU[i];
		sumVBin += resultHistgramV[i];
	}

	//histogram normalization process
	int indx = 0;
	for (int i = 0; i < HISTO_SRC_BINNUM; i++) {
		histoY_src[i] = resultHistgramY[i] / sumYBin;
		histoU_src[i] = resultHistgramU[i] / sumUBin;
		histoV_src[i] = resultHistgramV[i] / sumVBin;

		//target bin histogram
		indx = (int) ((double) i * (HISTO_DST_BINNUM - 1) / 255 + 0.5);
		histoY_dst[indx] += resultHistgramY[i] / sumYBin;
	}
  }

bool SmartRecognition::get_tracking_mode(){
	return is_ready_for_guide;
}

/**
 * set the threshold for tuning composition
 * @param paramI   int type parameters
 * @param len1     length of param1
 * @param paramF   float type parameters
 * @param len2     length of param2
 * @return         true:success to tuning|false:fail to tuning
 */
bool SmartRecognition::setSmartParameter(void* paramI, int len1, void* paramF, int len2) {
	int* param1 = (int*)paramI;
	float* param2 = (float*)paramF;
	if (param1 == NULL || param2 == NULL) {
			return false;
		} else {
			Light_BaseExpIndex = param1[6];                           //Max Exp-Index parameter, default:394
			Light_Offset       = param1[7];                           //Offset to Max Exp-Index, default:1
			Light_BaseLuxIndex = param1[8];                           //Max lux-Index parameter, default:500

			Light_LowRatio     = param2[8];                               //Ratio to Max Exp-Index, default:0.9

			return true;
		}
}

}
/* namespace smart */
