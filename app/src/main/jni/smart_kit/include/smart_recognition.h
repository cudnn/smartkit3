/*
 * smart_recognition.h
 *
 *  Created on: 2015年7月28日
 *      Author: guoyj7
 *
 * 	Add comments on variable and functions, clear up the unused part
 *
 *  Revised on: 2016/04/6
 *      Author: fengbin1
 */

#ifndef SMART_KIT_SMART_RECOGNITION_H_
#define SMART_KIT_SMART_RECOGNITION_H_

#define byte unsigned char

#include "smart_extern_manager.h"
#include "smart_type.h"
#include "data_structs.h"
#include <string>

namespace smart {
class LandscapeGistScore;
class MFTracking;

class SmartRecognition {
public:
	SmartRecognition();
	virtual ~SmartRecognition();

    /**
     * enable the specific scene recognition
     * @param bit  scene type (defined in include/smart_type.h)
     * @return
     */
	bool enable_scene(const int bit);
    /**
     * disable the specific scene recognition
     * @param bit  scene type (defined in include/smart_type.h)
     * @return
     */
	bool disable_scene(const int bit);
    /**
     * judge whether the specific scene is enabled
     * @param bit  scene type (defined in include/smart_type.h)
     * @return true:enable|false:disable
     */
	bool is_enable(const int bit);
    /**
     * do the scene detection
     * @param input  input data
     * @param width  width of input data
     * @param height height of input data
     * @return results of scene detection
     */
	int detect_scene(unsigned char* input, int width, int height);
    /**
     * get the results of scene recognition
     * @return array of results
     */
	int* get_scene_flag();
    /**
     * get the attractiveness of food
     * @return attractiveness result
     */
	float get_food_attractiveness();
    /**
     * get the content of QRCode
     * @return
     */
	std::string& get_qrcode_decoder_str();
    /**
     * get the type of QRCode
     * @return
     */
	std::string& get_qrcode_decoder_type();
    /**
     * get the format of code
     * @return
     */
	int get_code_format();

	int ellipse_param[10];               //parameters of ellipse(position/major semi-Axis/minor semi-Axis/angle/bounding box)
	int line_position[41];               //parameters of line(bounding box)

    int process_image_width;             //width of processed image for recognition and tracking, may be used in composition guide
    int process_image_height;		     //height of processed image for recognition and tracking, may be used in composition guide

    //guoyj for guide
    bool is_ready_for_guide;             //composition guide flag
    int tracked_rect_for_guide[6];       //position for composition guide: left top right bottom center_x center_y
    //for OpenGL display models
    bool is_ready_for_gl_model;          //gl render flag
    float tracked_rect_for_gl_matrix[8]; //position for gl render: left-top right-top right-bottom left-bottom

    float debug_value[32];               //temporary array for debug value

    bool isEllipseRedetected;            //ellipse re-detection flag while tracking
    bool isEllipseRedetectedTrigger;     //ellipse re-detection trigger for the next tracking frame, update the tracking bounding box

	bool get_tracking_mode();

	/**
	 * set the threshold for tuning recognition
	 * @param paramI   int type parameters
	 * @param len1     length of param1
	 * @param paramF   float type parameters
	 * @param len2     length of param2
	 * @return         true:success to tuning|false:fail to tuning
	 */
	bool setSmartParameter(void* paramI, int len1, void* paramF, int len2);

private:
    /**
     * check the preview ratio is 16:9 or 4:3, adjust parameters
     * @param width   preview width
     * @param height  preview height
     * @return        true:success to check|false:fail to check
     */
	bool preview_ratio_check(const int width, const int height);
    /**
     * initialize recognition configuration
     * @return
     */
	void init();
    /**
     * judge the scene type for tracking
     * @return ture:on-tracking|false:off-tracking
     */
	bool is_tracking_mode();
	// void do_tracking(unsigned char* input);
	// void do_detection(unsigned char* input);
	/** 20150827 ** new type of input data*/
    /**
     * Tracking pipeline: data process & tracking
     * @param input  input data for tracking
     * @return
     */
	void do_tracking(Image* input);
	/**
     * Recognition pipeline: data process & scene recognition
     * @param input  input data for detection
     * @return
     */
	void do_detection(Image* input);
    /**
     * downsize UV data for tracking, mainly for attractiveness of food
     * @param input     input data of downsize
     * @param output_u  output data of U
     * @param output_v  output data of V
     * @return
     */
	void downsize_for_tracking_uv(const Image* input, Image* output_u, Image* output_v);
    /**
     * downsize Y data for tracking
     * @param in   input data of downsize
     * @param out  output data of downsize
     * @return
     */
	void downsize_for_tracking(const Image* in, Image* out);
    /**
     * tracking process
     * @param track_flag   scene type for tracking(defined in include/smart_type.h)
     * @return
     */
	bool tracking(int track_flag);
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
	void downsize_for_detection(const Image* yuv420sp,
			                    Image* code_data, Image* boofcv_data,
			                    Image* datay, Image* datay32,
			                    Image* boofcv_u, Image* boofcv_v,
			                    Image* datau1_1, Image* datav1_1,
			                    Image* datau16_9, Image* datav16_9);

    /**
     * document detection
     * @param input   input data for detection
     * @return        -1:No detect|0:normal|1:doc
     */
	int detect_document(const Image* input);
    /**
     * motion detection
     * @param input   input data for detection
     * @return        -1:No detect|0:normal|1:move|2:jitter
     */
	int detect_motion(const Image* datay32, const float a_param, const float o_param);
    /**
     * QRCode detection
     * @param code           (input) data for code detection
     * @param qrPos          (output) data of QRCode position
     * @param yuv420spCopy   (deprecated) copy of input data
     * @param thld           (deprecated) QRcode threshold
     * @param roi            (deprecated) QRcode detect ROI
     * @return               -1:No detect|0:normal|1:BarCode|2:QRCode
     */
	//old QRcode detection algorithm by Bob Xu
	//int detect_code(const Image* code, int *qrPos, unsigned char *yuv420spCopy, unsigned char *thld , unsigned char *roi);
	//ZXing QRcode detection algorithm by Mikey Hu
	int detect_code(const Image *yuv420sp, int *qrPos);
    /**
     * light detection
     * @param blockY[]        7x7 1D block Y data
     * @param histYL[]        Y data histogram(256 bin)
     * @param histU[]         U data histogram(256 bin)
     * @param histV[]         V data histogram(256 bin)
     * @param histYS[]        Y data histogram(16 bin)
     * @param baseExpIndex    Max Exp-Index parameter from ISP data
     * @param baseLuxIndex    Max Lux-Index parameter from ISP data
     * @param offset          Offset to Max Exp-Index for threshold
     * @param lowRatio        Ratio to Max Exp-Index for threshold
     * @return                -1:No detect|0:normal|1:backlit|2:ultralowlit|3:lowlit
     */
	int detect_light(const float blockY[], const float histYL[], const float histU[], const float histV[], const float histYS[],
			         const int baseExpIndex, const int baseLuxIndex, const int offset, const float lowRatio);
    /**
     * portrait detection pipeline
     * @param faces           faces data
     * @return                -1:No detect|0:normal|1:portrait|2:portraitGrop
     */
	int detect_portrait(const int* faces);
    /**
     * portrait detection
     * @param faces           faces data
     * @param width           preview width
     * @param height          preview height
     * @param ori             camera orientation
     * @param ratioF          face ratio of width/height
     * @return                false:normal|true:portrait
     */
	bool portraitDetect(const int* faces, const int width, const int height, const int ori, const float ratioF); //added
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
	bool portraitCheck(const int* faces, const int width, const int height,
			           unsigned char *_boofCVDataY, unsigned char *_boofCVDataU, unsigned char *_boofCVDataV);												  //added
    /**
     * portrait_lit detection pipeline
     * @param code            data for portrait_lit detection
     * @param faces           faces data
     * @param ori             camera orientation
     * @param params          portrait_lit parameters
     * @return                -1:No detect|0:normal|1:portrait_backlit|2:portrait_lowlit
     */
	int detect_portrait_lit(Image* code, int* faces, const int ori, int* params);
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
	int portraitLitDetect(unsigned char* code_data, int* faces, const int height, const int width, const int ori, int* params);
    /**
     * landscape detection pipeline
     * @param datay32         data Y for landscape detection
     * @param datau1_1        data U for landscape detection
     * @param datav1_1        data V for landscape detection
     * @return                -1:No detect|0:normal|1:landscape
     */
	int detect_landscape(Image* datay32, Image* datau1_1, Image* datav1_1);
    /**
     * landscape detection(merged from GPU version)
     * @param datau1_1        data U for landscape detection
     * @param datav1_1        data V for landscape detection
     * @return                -1:No detect|0:normal|1:landscape
     */
	int landscapeDetect(Image* datau1_1, Image* datav1_1);//added
	int landscapeGPUDetect(Image* datau1_1, Image* datav1_1);//added
    /**
     * landscape detection(Gist version)
     * @param data           data for landscape detection
     * @param height         data height
     * @param width          data width
     * @return               -1:No detect|0:normal|1:landscape
     */
	int landscapeDetectNative(unsigned char *data, const int height, const int width);//added
    /**
     * Adjust threshold of landscape detection(GPU) dynamically
     * @return
     */
	void adjustLandscapeGreenBlueThreshold();
    /**
     * Adjust threshold of landscape detection(Gist) dynamically
     * @return               Adjusted threshold
     */
	float adjustLandscapeThreshold();
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
	bool isEnableLinedetect(unsigned char*data, int height, int width,
			                double mBlueSkey2, double mGreenGround2, bool isGistScoreAvaliable,bool isGistLandscape);
    /**
     * food detection
     * @param food_img              data Y for food detection
     * @param datau16_9             data U for food detection
     * @param datav16_9             data V for food detection
     * @return                      -1:No detect|0:normal|1:food
     */
	int detect_food(Image* food_img, Image* datau16_9, Image* datav16_9);
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
	double foodScoreMean(Image* datau16_9, Image* datav16_9, int a, int b, float cita, int X, int Y);
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
	int detect_line(Image* line_img, int *pos, int inputOritation, float leveltheta, float *thetaInput, int toleranceAngle);
    /**
     * get accelerometer data
     * @return       accelerometer data
     */
	float getAcc();
    /**
     * get orientation data
     * @return       orientation data
     */
	float getOcc();
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
	void getHistogram(const Image* _datay, const Image* _datau1_1, const Image* _datav1_1,
			          const int HISTO_SRC_BINNUM, const int HISTO_DST_BINNUM,
			          float* histoY_src, float* histoU_src, float* histoV_src, float* histoY_dst);



	int image_width;       //preview data width
	int image_height;      //preview data height

	SmartExternManager* extern_manager;             //sensor data manager
	LandscapeGistScore* landscape_gist_score;       //gist score
	MFTracking* tracker;                            //MF tracking object

	static const int MAX_LENGTH = 640 * 480 * 4;    //max length of data
	unsigned char* dataYUV/*[MAX_LENGTH / 4]*/;     //(deprecated)new memory, same size as YUV data
	unsigned char* dataThld/*[MAX_LENGTH / 4 / 7 / 7]*/; //(deprecated)threshold array
	unsigned char* dataROI/*[MAX_LENGTH / 4]*/;     //(deprecated)ROI

	static const int LIST_LIGHT_LENGTH = 5;         //Length of light scene list
	static const int LIST_FOOD_LENGTH = 3;          //Length of food scene list
	static const int LIST_MOTION_LENGTH = 6;        //Length of motion scene list
	static const int LIST_ACC_LENGTH = 5;           //Length of acc data list
	static const int LIST_IMG_LENGTH = 5;           //Length of img data list

	static const float COE_DEGREE = 0.9f,           //smooth coefficient of degree data
					   COE_ACC = 0.9f,              //smooth coefficient of acc data
	                   COE_OCC = 0.9f;              //smooth coefficient of occ data

    //detect_QRCode
	int                mQRPos[50];                 //position of QR code
	std::string        _code_str;                   //decode string
	std::string        _code_type;                  //code type
	int                _code_format;                //code format

	//detect_portrait
	float FaceHeight;                               //ratio of face height

	//detect_motion
	int Motion_ThresSubNum;                         //threshold of difference between two adjacent frames
	int Motion_ThresSubLow;                         //threshold of subNum
	float Motion_ThresAccLow;                       //threshold of acc data
	float Motion_ThresOccHigh;                      //threshold of occ data

	//detect_light
	double degreeBound[3][2];                       //threshold of light detection
	float Light_ThresFocusDis;                      //threshold of focus distance, default:0.2f
    float Light_LowRatio;                           //Ratio to Max Exp-Index, default:0.9
	int Light_ThresExpLow;                          //threshold of exp-index(lower), default:235
	int Light_ThresExpHigh1;                        //threshold of exp-index(upper), default:310
	int Light_ThresLuxHigh;                         //threshold of lux(upper1), default:5
	int Light_ThresLuxHigh1;                        //threshold of lux(upper2), default:200
	int Light_BaseExpIndex;                         //Max Exp-Index parameter, default:394
	int Light_BaseLuxIndex;                         //Max lux-Index parameter, default:500
	int Light_Offset;                               //Offset to Max Exp-Index, default:1

	//detect_landscape
	float Landscape_ThresFocusDis;                  //threshold of focus distance, default:0.25f
	int Landscape_ThresISPExpHigh;                  //threshold of exp-index(upper), default:230
	//parameters of landscape detection(Gist)
	int gistIspThresh1;                             //lower limit of isp data for landscape detection(Gist)
	int gistIspThresh2;                             //upper limit of isp data for landscape detection(Gist)
	int gistOrientationThresh;                      //threshold of angle between camera and ground 手机与地面的夹角活动范围
	float gistLandscapeTheshold_H[2];               //gist degree of landscape when camera is in landscape mode 横屏判断为风景的gist分值
	float gistLandscapeTheshold_V[2];               //gist degree of landscape when camera is in portrait mode 竖屏判断为风景的gist分值
	float gistFocusDisThresh;                       //threshold of focus distance 对焦距离阈值
	float omitbklandGistThreshold;                  //threshold of gist for filter backlit from landscape 去掉逆光风景的gist阈值
	bool gistScoreAvalible;                         //gistScore computing flag 判断是否计算了gistScore
	bool bGistDetectLandscape;                      //flag of landscape detection by Gist
	bool bLinedetectEnalbe;                         //line detection enable flag 判断是否开启直线引导

	double mBlueSkey;                               //blue score of landscape detection 风景识别-蓝色分量
	double mGreenGround;                            //green score of landscape detection 风景识别-绿色分量
	float m_landscape_blue_t;                       //threshold of blue score for landscape detection 风景识别蓝色值的阈值
	float m_landscape_green_t;                      //threshold of green score for landscape detection 风景识别绿色值的阈值
	float m_landscape_rank_t;                       //threshold of total score for landscape detection 风景识别蓝色值的阈值

	//detect_food
    float _food_attractiveness;                     //food attractiveness score

	static const int MATRIX_WIDTH = 64;             //size of datay/datau1_1/datav1_1
	static const int MATRIX_HEIGHT = 64;

	static const int DownsampleVarWidth = 32;       //size of datay32
	static const int DownsampleVarHeight = 32;

	//4x3: 640x480  -- memory_size
	//16x9
	static const int DownsampleWidth16_9 = 720;     //size of code(16:9)
	static const int DownsampleHeight16_9 = 405;

	static const int DownsampleWidth4_3 = 640;      //size of code(4:3)
	static const int DownsampleHeight4_3 = 480;

	//4x3: 320x240  -- memory_size
	//16x9
	static const int BoofCVWidth16_9 = 360;         //size of boofcv/boofcv_u/boofcv_v(16:9)
	static const int BoofCVHeight16_9 = 202;

	static const int BoofCVWidth4_3 = 320;          //size of boofcv/boofcv_u/boofcv_v(4:3)
	static const int BoofCVHeight4_3 = 240;

	//4x3: 64x48  -- memory_size
	//16x9
	static const int uv2Width16_9 = 64;             //size of datau16_9/datav16_9(16:9)
	static const int uv2Height16_9 = 36;

	static const int uv2Width4_3 = 64;             //size of datau16_9/datav16_9(4:3)
	static const int uv2Height4_3 = 48;

	static const float Food_ThresScore = 18.0f;     //threshold of food score

	static const int BLOCK_SIDE = 7;                //size of block

	static const int LIGHT_NUMOFWEIGHT = 50;        //number of weight for light feature
	static const int HISTO_SRC_BINNUM = 256;        //256 bin histogram
	static const int HISTO_DST_BINNUM = 16;         //16 bin histogram

	float histoY_src[HISTO_SRC_BINNUM];
	float histoU_src[HISTO_SRC_BINNUM];
	float histoV_src[HISTO_SRC_BINNUM];
	float histoY_dst[HISTO_DST_BINNUM];

	float lLux;                                     //lux data of light sensor
	float isp_ExpIndex;                             //ExpIndex data of isp sensor
	float isp_Lux;                                  //lux data of isp sensor
	float isp_FocusDis;                             //focus distance data of isp sensor

	int track_frame;                                //number of tracking frame
	int detect_sequence[4];                         //detection pipeline {1, 2, 3, 4}
	int detect_sequence_index;                      //index of detection pipeline
	bool is_normal_sequence;                        //detection priority flag

	SceneType scene_type;                           //detection result parameter

	int flashStatus;                                //flash status flag
	int scene_flags[2];                             //scene recognition result
	int scene_enable_flag;                          //scene recognition enable flag

	bool is_tracked;                                //tracking flag
	bool is_line_tracking_mode;                     //line tracing flag

	/** 20150827 ** new data structure */
	Image* input_frame;                             //original data, for QRcode
	//720x405
	Image* code;                                    //data for light/portrait light
	//360x202
	Image* boofcv;                                  //data for food/line/face-check
	Image* boofcv_u;                                //data for face-check
	Image* boofcv_v;                                //data for face-check
	//64x64
	Image* datay;                                   //data for Histogram
	//32x32
	Image* datay32;                                 //data for document/motion/landscape
	//64x64
	Image* datau1_1;                                //data for Histogram/landscape
	Image* datav1_1;                                //data for Histogram/landscape
	//64x36
	Image* datau16_9;                               //data for food
	Image* datav16_9;                               //data for food
};
} /* namespace smart */
#endif /* SMART_KIT_SMART_RECOGNITION_H_ */
