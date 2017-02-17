/*
 * smart_type.h
 *
 *  Created on: 2015年7月29日
 *      Author: guoyj7
 *
 * 	Add comments on variable
 *
 *  Revised on: 2016/04/8
 *      Author: fengbin1
 */
#include <string>
#ifndef SMART_KIT_SMART_TYPE_H_
#define SMART_KIT_SMART_TYPE_H_

//sensor constant field
#define SMART_SENSOR_TYPE_ACCELEROMETER 	1
#define SMART_SENSOR_TYPE_ORIENTATION 		2
#define SMART_SENSOR_TYPE_LIGHT 			4

//scene type to enable/disable
#define SMART_SCENE_MOTION					1	// 0000 0000 0000 0000 0000 0000 0000 0001
#define SMART_SCENE_PORTRAIT 				2	// 0000 0000 0000 0000 0000 0000 0000 0010
#define SMART_SCENE_LIGHT 					4	// 0000 0000 0000 0000 0000 0000 0000 0100
#define SMART_SCENE_LANDSCAPE 				8	// 0000 0000 0000 0000 0000 0000 0000 1000
#define SMART_SCENE_FOOD 					16	// 0000 0000 0000 0000 0000 0000 0001 0000
#define SMART_SCENE_PORTRAITLIT 			32	// 0000 0000 0000 0000 0000 0000 0010 0000
#define SMART_SCENE_DOCUMENT	 			64	// 0000 0000 0000 0000 0000 0000 0100 0000
#define SMART_SCENE_CODE		 			128	// 0000 0000 0000 0000 0000 0000 1000 0000
#define SMART_GUIDE_PORTRAIT		 		256	// 0000 0000 0000 0000 0000 0001 0000 0000
#define SMART_GUIDE_LANDSCAPE		 		512	// 0000 0000 0000 0000 0000 0010 0000 0000
#define SMART_GUIDE_FOOD		 			1024// 0000 0000 0000 0000 0000 0100 0000 0000

//scene type constant field for tracking
//#define SMART_TRACKING_FLAG_PORTRAIT		1
//#define SMART_TRACKING_FLAG_ELLIPSE			2
//#define SMART_TRACKING_FLAG_LINE			3

//scene type constant field of encode coefficient
#define DEF_DETECT_FLAG_MOTION				1
#define DEF_DETECT_FLAG_PORTRAIT			10
#define DEF_DETECT_FLAG_PORTRAITLIT			100
#define DEF_DETECT_FLAG_LIGHT				1000
#define DEF_DETECT_FLAG_FOOD				10000
#define DEF_DETECT_FLAG_LANDSCAPE			100000
#define DEF_DETECT_FLAG_CODE				1000000
#define DEF_DETECT_FLAG_DOCUMENT			10000000

//scene type constant field for decode
#define FLAG_DETECTION_ERROR				-1
#define FLAG_DETECTION_NORMAL				0
#define FLAG_DETECTION_PORTRAIT				1
#define FLAG_DETECTION_LOWLIT				2
#define FLAG_DETECTION_ULTRA_LOWLIT			3
#define FLAG_DETECTION_PORTRAIT_BACKLIT		4
#define FLAG_DETECTION_PORTRAIT_LOWLIT		5
#define FLAG_DETECTION_BACKLIGHT			6
#define FLAG_DETECTION_LANDSCAPE			7
#define FLAG_DETECTION_FOOD					8
#define FLAG_DETECTION_QRCODE				9
#define FLAG_DETECTION_DOCUMENT				10
#define FLAG_DETECTION_JITTERING			11
#define FLAG_DETECTION_MOVE					12

//#define IMAGE_TYPE_Y						0
//#define IMAGE_TYPE_U    					0
//#define IMAGE_TYPE_V    					0
//#define IMAGE_TYPE_YUV420SP					1

#define SMART_STATUS_RECOGNITION		 	20
#define SMART_STATUS_ALWAYINFOCUS		 	30
#define SMART_STATUS_CASE3		 			40
#define SMART_STATUS_CASE4				 	50

struct SceneTypeBase {
	/** 还未检测 **/
	static const int TYPE_NO_DETECT = -1;

	/** 正常 **/
	static const int TYPE_NORMAL = 0;
	int flag;
	void reset() { flag = TYPE_NO_DETECT; }
	bool is_detected_type() { return (flag != TYPE_NO_DETECT) && (flag != TYPE_NORMAL); }
	int get_flag() { return flag; }
};

struct MotionScene : SceneTypeBase
{
	static const int TYPE_MOVE = 1;
	static const int TYPE_JITTER = 2;
	double jitter_ratio;
	double get_jitter_ratio() { return jitter_ratio; }
};

struct PortraitScene : SceneTypeBase{
	/** single portrait 单人 **/
	static const int TYPE_PORTRAIT = 1;
	/** group portrait 多人 **/
	static const int TYPE_PORTRAIT_GROUP = 2;
};

struct CodeScene : SceneTypeBase{
	/** bar code 一维码 **/
	static const int TYPE_BARCODE = 1;
	/** QR code 二维码 **/
	static const int TYPE_QRCODE = 2;
};

struct DocumentScene : SceneTypeBase{
	/** document 文档 **/
	static const int TYPE_DOCUMENT = 1;
};

struct PortraitLitScene : SceneTypeBase{
	/** portrait_backlit 人像逆光 **/
	static const int TYPE_PORTRAIT_BACKLIT = 1;
	/** portrait_lowlit 人像弱光 **/
	static const int TYPE_PORTRAIT_LOWILIT = 2;
	/** portrait_other 人像其他 **/
	static const int TYPE_PORTRAIT_OTHER = 3;
};

struct FoodScene : SceneTypeBase{
	/** food 食物 **/
	static const int TYPE_FOOD = 1;
};

struct LightScene : SceneTypeBase{
	/** backlit 逆光 **/
	static const int TYPE_BACKLIT = 1;
	/** ultral_lowlit 超低光 **/
	static const int TYPE_ULTRA_LOWLIT = 2;
	//		public static const int TYPE_LOWLIT = 2;
	/** lowlit 暗光 **/
	static const int TYPE_LOWLIT = 3;
};

struct LandscapeScene : SceneTypeBase {
	/** landscape 风景 **/
	static const int TYPE_LANDSCAPE = 1;
};

struct SceneType
{
	MotionScene motion;
	CodeScene code;
	DocumentScene document;
	PortraitScene portrait;
	PortraitLitScene portrait_lit;
	FoodScene food;
	LightScene light;
	LandscapeScene landscape;
	SceneType() {
		reset();
	}
	void reset() {
		motion.reset();
		code.reset();
		document.reset();
		portrait.reset();
		portrait_lit.reset();
		food.reset();
		light.reset();
		landscape.reset();
	}
	MotionScene& get_motion() { return motion; }
	CodeScene& get_code() { return code; }
	DocumentScene& get_document() { return document; }
	PortraitScene& get_portrait() { return portrait; }
	PortraitLitScene& get_portrait_lit() { return portrait_lit; }
	FoodScene& get_food() { return food; }
	LightScene& get_light() { return light; }
	LandscapeScene& get_landscape() { return landscape; }
};


struct Param_Point {
	Param_Point() { x = -1, y = -1; }
	Param_Point(int _x, int _y) { x = _x, y = _y; }
	int x, y;
};

/***
* 构图参数对象
*/
struct Param {
	/**默认值**/
	static const int ZOOM_DEFAULT = 0;
	/**放大**/
	static const int ZOOM_IN = 0;
	/**缩小**/
	static const int ZOOM_OUT = 2;
	/**大小合适**/
	static const int ZOOM_OK = 1;

	bool allow;
	bool ready;
	Param_Point currentPosition; //当前位置
	Param_Point targetPosition; //目标位置
	bool distanceOk; //推荐位置是否满足
	int guideFlag;
	int orientation; //方向
	int zoomFlag; //推荐大小是否满足 0:放大, 2:缩小, 1:满足
	int zoomRatio;

	Param() {
		allow = false;
		ready = false;
	}
	/**
	* 是否满足推荐条件
	* @return
	*/
	bool isAllow() {
		return allow;
	}
	/**
	* 推荐位置、大小是否满足
	* @return
	*/
	bool isReady() {
		return ready;
	}
	/**
	* 当前位置
	* @return
	*/
	Param_Point getCurrentPosition() {
		return currentPosition;
	}
	/**
	* 目标位置
	* @return
	*/
	Param_Point getTargetPosition() {
		return targetPosition;
	}
	/**
	* 推荐位置是否满足
	* @return
	*/
	bool isDistanceOk() {
		return distanceOk;
	}
	/**
	* 推荐大小是否满足
	* @return ZOOM_IN(0):放大, ZOOM_OUT(2):缩小, ZOOM_OK(1):满足
	*/
	int getZoomFlag() {
		return zoomFlag;
	}
	/**
	* 推荐位置尺寸大小
	* @return
	*/
	int getZoomRatio() {
		return zoomRatio;
	}

	/**
	* 获取构图标志位
	* @return
	*/
	int getGuideFlag() {
		return guideFlag;
	}
	/**
	* 获取当前方向
	* @return
	*/
	int getOrientation() {
		return orientation;
	}


	/**
	* 重置构图参数
	*/
	void reset() {
		allow = false;
		ready = false;
		currentPosition.x = 0;
		currentPosition.y = 0;
		targetPosition.x = 0;
		targetPosition.y = 0;
		distanceOk = false;
		zoomFlag = 0;
		zoomRatio = 0;
	}
};

/***
* 人像构图参数对象
*/
struct PortraitParam : Param {
	int leftRatio;
	int rightRatio;
	int ratios[2];

	PortraitParam() { ratios[0] = leftRatio, ratios[1] = rightRatio; }
	/**
	* 获取推荐框显示亮度百分比
	* @return
	*/
	int *getTargetRatio() {
		 ratios[0] = leftRatio, ratios[1] = rightRatio;
		 return ratios;
	}
};
/***
* 风景构图参数对象
*/
struct LandscapeParam : Param {
	bool angleFlag;
	int levelAngle; //水平线角度
	/**
	* 获取角度标志位
	* @return
	*/
	bool getAngleflag() {
		return angleFlag;
	}
	/**
	* 获取当前水平线角度
	* @return
	*/
	int getLevelAngle() {
		return levelAngle;
	}
	/**
	* 重置构图参数
	*/
	void reset() {
		orientation = 0;
		levelAngle = 0;
		allow = false;
		ready = false;
		currentPosition.x = 0;
		currentPosition.y = 0;
		targetPosition.x = 0;
		targetPosition.y = 0;
		distanceOk = false;
		zoomFlag = 0;
		zoomRatio = 0;
	}
};

/***
* 食物构图参数对象
*/
struct FoodParam : Param {
	int shortRadius; //短轴
	int longRadius; //长轴
	int angle; //水平夹角
	float food_attractiveness; // 食物爽歪歪程度

	/**
	* 椭圆短轴长度
	* @return
	*/
	int getShortRadius() {
		return shortRadius;
	}
	/**
	* 椭圆长轴长度
	* @return
	*/
	int getLongRadius() {
		return longRadius;
	}
	/**
	* 椭圆水平夹角
	* @return
	*/
	int getAngle() {
		return angle;
	}
	/**
	* 食物爽歪歪程度
	* @return
	*/
	float getFoodAttractiveness(){
		return food_attractiveness;
	}
	/**
	* 重置构图参数
	*/
	void reset() {
		shortRadius = 0;
		longRadius = 0;
		angle = 0;
		allow = false;
		ready = false;
		currentPosition.x = 0;
		currentPosition.y = 0;
		targetPosition.x = 0;
		targetPosition.y = 0;
		distanceOk = false;
		zoomFlag = 0;
		zoomRatio = 0;
		food_attractiveness = 0.0;
	}
};

struct ScenesParam
{
	bool allow;
	int  scenes[2];

	ScenesParam() {
		allow = false;
		for (int i = 0; i < 2; i++)
			scenes[i] = 0;
	}
	/**
	* 重置所有参数
	*/
	void reset() {
		allow = false;
		for (int i = 0; i < 2; i++)
			scenes[i] = -1;
	}

	bool isAllow(){
		return allow;
	}


};

struct GuideParam
{
	bool allow;
	FoodParam food;
	PortraitParam portrait;
	LandscapeParam landscape;

	GuideParam() {
		allow = false;
	}
	/**
	* 重置所有参数
	*/
	void reset() {
		food.reset();
		portrait.reset();
		landscape.reset();
	}
	/**
	* 获取人像构图对象
	* @return
	*/
	FoodParam& getFood() {
		return food;
	}
	/**
	* 获取人像构图对象
	* @return
	*/
	PortraitParam& getPortrait() {
		return portrait;
	}
	/**
	* 获取风景构图对象
	* @return
	*/
	LandscapeParam& getLandscape() {
		return landscape;
	}

	bool isAllow(){
		return allow;
	}
};

struct TrackParam
{
	bool allow;
	int  rect[4];
	int  pEx_length;

	TrackParam() {
		pEx_length = 0;
		allow = false;
		for (int i = 0; i < 4; i++)
			rect[i] = -1;
	}
	/**
	* 重置所有参数
	*/
	void reset() {
		allow = false;
		pEx_length = 0;
		for (int i = 0; i < 4 ; i++)
			rect[i] = -1;
	}

	bool isAllow(){
		return allow;
	}


};

struct QRCodeParam
{
	bool allow;
	float  rect[8];
	std::string        code_str;                   //decode string
	std::string        code_type;                  //code type
	int                code_format;                //code format

	QRCodeParam() {
		allow = false;
		for (int i = 0; i < 8; i++)
			rect[i] = -1;
		code_str    = "";
		code_type   = "";
		code_format = 0;
	}
	/**
	* 重置所有参数
	*/
	void reset() {
		allow = false;
		for (int i = 0; i < 8; i++)
			rect[i] = -1;
		code_str    = "";
		code_type   = "";
		code_format = 0;
	}

	bool isAllow(){
		return allow;
	}
};

class OutputParam {

public:
	ScenesParam  scenes;
	GuideParam   guide;
	TrackParam   track;
	QRCodeParam  qrcode;

	OutputParam(){
	}

	ScenesParam& getScenesResult(){
		return scenes;
	}

	GuideParam& getGuideParam(){
		return guide;
	}

	TrackParam& getTrackParam(){
		return track;
	}

	QRCodeParam& getQRCodeParam(){
		return qrcode;
	}
};

struct GuideInputParam
{
	int rect[4];
	int center_x;
	int center_y;
	int orientation;
	int orientation_raw;
	int width;
	int height;
	int preview_width;
	int preview_height;
	int screen_width;
	int screen_height;
};
#endif /* SMART_KIT_SMART_TYPE_H_ */
